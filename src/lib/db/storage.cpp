//
// Created by Rakesh on 24/12/2021.
//

#include "encrypter.h"
#include "crud.h"
#include "model/configuration.h"
#include "util/cache.h"
#include "../common/log/NanoLog.h"
#include "../common/model/tree_generated.h"
#include "../common/pool/pool.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <memory>

#include <rocksdb/cache.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/utilities/transaction_db.h>

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace spt::configdb::db::internal
{
  std::unique_ptr<Encrypter> create()
  {
    return std::make_unique<Encrypter>( model::Configuration::instance().encryption.secret );
  }

  spt::configdb::pool::Configuration poolConfig()
  {
    auto config = spt::configdb::pool::Configuration{};
    config.initialSize = 5;
    config.maxPoolSize = std::numeric_limits<int>::max();
    config.maxConnections = std::numeric_limits<int>::max();
    config.maxIdleTime = std::chrono::days{ std::numeric_limits<int>::max() };
    return config;
  }

  struct Database
  {
    struct DbOpenException : std::exception
    {
      const char* what() const noexcept override
      {
        return "Error opening database";
      }
    };

    static Database& instance()
    {
      static Database db;
      return db;
    }

    std::optional<std::string> get( std::string_view key )
    {
      if ( key.empty())
      {
        LOG_INFO << "Rejecting request for empty key";
        return std::nullopt;
      }

      const auto& conf = model::Configuration::instance();
      auto& vc = util::getValueCache();
      auto ks = getKey( key );

      if ( conf.enableCache )
      {
        if ( auto iter = vc.find( ks ); iter != std::end( vc ))
        {
          return iter->second;
        }
      }

      std::string value;

      if ( const auto s = db->Get( rocksdb::ReadOptions{}, handles[1],
            rocksdb::Slice{ ks }, &value );
          !s.ok())
      {
        LOG_WARN << "Error retrieving key [" << key << "]. " << s.ToString();
        return std::nullopt;
      }

      auto encrypter = pool.acquire();
      if ( !encrypter )
      {
        LOG_CRIT << "Unable to acquire encrypter from pool";
        return std::nullopt;
      }
      auto ret = ( *encrypter )->decrypt( value );

      if ( conf.enableCache ) vc.put( ks, ret );
      return ret;
    }

    bool set( std::string_view key, std::string_view value )
    {
      if ( key.empty() )
      {
        LOG_INFO << "Rejecting request for empty key";
        return false;
      }

      auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );
      if ( const auto saved = set( key, value, txn ); !saved ) return saved;

      if ( const auto s = txn->Commit(); !s.ok() )
      {
        LOG_WARN << "Error writing to key [" << key << "]. " << s.ToString();
        return false;
      }

      return true;
    }

    bool remove( std::string_view key )
    {
      if ( key.empty() )
      {
        LOG_INFO << "Rejecting request for empty key";
        return false;
      }

      auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );
      if ( const auto s = remove( key, txn ); !s ) return s;
      if ( const auto s = txn->Commit(); !s.ok())
      {
        LOG_WARN << "Error removing key [" << key << "]. " << s.ToString();
        return false;
      }

      return true;
    }

    bool move( std::string_view key, std::string_view dest )
    {
      if ( key.empty() )
      {
        LOG_INFO << "Rejecting request for empty key";
        return false;
      }

      auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );
      if ( const auto s = move( key, dest, txn ); !s ) return s;

      if ( const auto s = txn->Commit(); !s.ok())
      {
        LOG_WARN << "Error removing key [" << key << "]. " << s.ToString();
        return false;
      }

      return true;
    }

    Nodes list( std::string_view key )
    {
      if ( key.empty())
      {
        LOG_INFO << "Rejecting request for empty key";
        return std::nullopt;
      }

      auto ks = getKey( key );
      std::string value;
      if ( const auto s = db->Get( rocksdb::ReadOptions{}, handles[2],
            rocksdb::Slice{ ks }, &value );
          s.ok())
      {
        const auto d = reinterpret_cast<const uint8_t*>( value.data() );
        auto response = model::GetNode( d );

        if ( response )
        {
          auto vec = std::vector<std::string>{};
          vec.reserve( response->children()->size() + 1 );
          for ( auto&& child : *response->children() ) vec.push_back( child->str() );
          return vec;
        }
        else
        {
          LOG_CRIT << "Error marshalling buffer for path " << key;
        }
      }
      else
      {
        LOG_WARN << "Error retrieving child nodes for path " << ks << ". " << s.ToString();
      }

      return std::nullopt;
    }

    std::vector<KeyValue> get( const std::vector<std::string_view>& vec )
    {
      const auto& conf = model::Configuration::instance();
      auto& vc = util::getValueCache();

      std::vector<KeyValue> result;
      result.reserve( vec.size() );

      auto encrypter = pool.acquire();
      if ( !encrypter )
      {
        LOG_CRIT << "Unable to acquire encrypter from pool";
        return result;
      }

      std::vector<std::string> skeys;
      skeys.reserve( vec.size() );
      for ( auto&& key : vec ) skeys.push_back( getKey( key ) );

      std::vector<rocksdb::Slice> keys;
      keys.reserve( vec.size() );

      for ( std::size_t i = 0; i < vec.size(); ++i )
      {
        if ( conf.enableCache )
        {
          if ( auto iter = vc.find( skeys[i] ); iter != std::end( vc ) )
          {
            result.emplace_back( skeys[i], iter->second );
          }
          else
          {
            keys.emplace_back( skeys[i] );
          }
        }
        else
        {
          keys.emplace_back( skeys[i] );
        }
      }

      if ( keys.empty() ) return result;

      std::vector<rocksdb::PinnableSlice> values;
      values.resize( keys.size() );
      std::vector<rocksdb::Status> statuses;
      statuses.resize( keys.size() );

      db->MultiGet( rocksdb::ReadOptions{}, handles[1], keys.size(),
          keys.data(), values.data(), statuses.data() );

      for ( std::size_t i = 0; i < keys.size(); ++i )
      {
        auto keystr = keys[i].ToString();
        if ( statuses[i].ok() )
        {
          auto ret = ( *encrypter )->decrypt( values[i].ToStringView() );
          if ( conf.enableCache ) vc.put( keys[i].ToString(), ret );
          result.emplace_back( keystr, std::move( ret ) );
        }
        else
        {
          LOG_WARN << "Error retrieving key " << keystr << ". " << statuses[i].ToString();
          result.emplace_back( keystr, std::nullopt );
        }
      }

      return result;
    }

    bool set( const std::vector<Pair>& kvs )
    {
      auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );

      for ( auto&& [key, value] : kvs )
      {
        if ( const auto s = set( key, value, txn ); !s )
        {
          LOG_WARN << "Error setting key " << key;
          return s;
        }
      }

      if ( const auto s = txn->Commit(); !s.ok() )
      {
        LOG_WARN << "Error writing batch. " << s.ToString();
        return false;
      }

      return true;
    }

    bool remove( const std::vector<std::string_view>& keys )
    {
      auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );

      for ( auto&& key : keys )
      {
        if ( const auto s = remove( key, txn ); !s )
        {
          LOG_WARN << "Error removing key " << key;
          return s;
        }
      }

      if ( const auto s = txn->Commit(); !s.ok() )
      {
        LOG_WARN << "Error writing batch. " << s.ToString();
        return false;
      }

      return true;
    }

    bool move( const std::vector<Pair>& kvs )
    {
      auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );

      for ( auto&& [key, dest] : kvs )
      {
        if ( const auto s = move( key, dest, txn ); !s )
        {
          LOG_WARN << "Error moving key " << key << " to destination " << dest;
          return s;
        }
      }

      if ( const auto s = txn->Commit(); !s.ok() )
      {
        LOG_WARN << "Error moving batch. " << s.ToString();
        return false;
      }

      return true;
    }

    std::vector<NodePair> list( const std::vector<std::string_view>& vec )
    {
      std::vector<rocksdb::Slice> keys;
      keys.reserve( vec.size() );
      std::vector<rocksdb::PinnableSlice> values;
      std::vector<rocksdb::Status> statuses;

      for ( auto&& key: vec ) keys.emplace_back( key );

      values.resize( keys.size() );
      statuses.resize( keys.size() );
      db->MultiGet( rocksdb::ReadOptions{}, handles[2], keys.size(),
          keys.data(), values.data(), statuses.data() );

      std::vector<NodePair> result;
      result.reserve( vec.size() );
      for ( std::size_t i = 0; i < vec.size(); ++i )
      {
        auto ks = keys[i].ToString();

        if ( statuses[i].ok() )
        {
          const auto d = reinterpret_cast<const uint8_t*>( values[i].data() );
          auto response = model::GetNode( d );

          if ( response )
          {
            auto rvec = std::vector<std::string>{};
            rvec.reserve( response->children()->size() + 1 );
            for ( auto&& child : *response->children() ) rvec.push_back( child->str() );
            result.emplace_back( ks, std::move( rvec ) );
          }
          else
          {
            LOG_CRIT << "Error marshalling buffer for path " << vec[i];
            result.emplace_back( ks, std::nullopt );
          }
        }
        else
        {
          LOG_WARN << "Error retrieving path " << vec[i] << ". " << statuses[i].ToString();
          result.emplace_back( ks, std::nullopt );
        }
      }

      return result;
    }

    void reopen()
    {
      close();
      init();
      util::getValueCache().clearAll();
    }

    ~Database()
    {
      close();
    }

  private:
    Database()
    {
      auto start = std::chrono::high_resolution_clock::now();
      init();
      auto finish = std::chrono::high_resolution_clock::now();
      LOG_INFO << "Opening database took " <<
        std::chrono::duration_cast<std::chrono::milliseconds>( finish - start ).count() <<
        " milliseconds";
    }

    static std::string getKey( std::string_view key )
    {
      if ( key.empty() ) return "/"s;
      if ( key[0] == '/' ) return { key.data(), key.size() };

      auto ks = std::string{};
      ks.reserve( key.size() + 1 );
      ks.push_back( '/' );
      ks.append( key.data(), key.size() );
      LOG_DEBUG << "Corrected key " << ks;
      return ks;
    }

    bool set( std::string_view key, std::string_view value, rocksdb::Transaction* txn )
    {
      const auto rollback = [txn]()
      {
        if ( const auto s = txn->Rollback(); !s.ok())
        {
          LOG_CRIT << "Error rolling back transaction. " << s.ToString();
        }
        return false;
      };

      auto encrypter = pool.acquire();
      if ( !encrypter )
      {
        LOG_CRIT << "Unable to acquire encrypter from pool";
        return false;
      }
      auto v = ( *encrypter )->encrypt( value );

      auto ks = getKey( key );
      LOG_INFO << "Setting key " << ks;
      if (
          const auto s = txn->Put( handles[1], rocksdb::Slice{ ks }, rocksdb::Slice{ v } );
          !s.ok() )
      {
        LOG_WARN << "Error setting key " << key << ". " << s.ToString();
        return rollback();
      }

      if ( !manageTree( ks, txn ) )
      {
        LOG_WARN << "Error managing tree for key " << key;
        return rollback();
      }

      if ( model::Configuration::instance().enableCache )
      {
        util::getValueCache().put( ks, std::string{ value.data(), value.size() } );
      }
      return true;
    }

    bool manageTree( std::string_view key, rocksdb::Transaction* txn )
    {
      using rocksdb::Slice;

      auto tuples = treeComponents( key );
      if ( tuples.empty() ) return true;

      auto opts = rocksdb::ReadOptions{};
      std::string value;
      for ( auto&& [path, child] : tuples )
      {
        if ( const auto s = txn->Get( opts, handles[2], Slice{ path }, &value ); s.ok() )
        {
          if ( !addChild( path, child, value, txn ) ) return false;
        }
        else if ( s.IsNotFound() )
        {
          if ( !createChild( path, child, txn ) ) return false;
        }
        else
        {
          LOG_WARN << "Error processing path: " << path << ", child: " << child <<
            ". " << s.ToString();
          return false;
        }
      }

      return true;
    }

    bool addChild( std::string_view path, std::string_view child,
        std::string_view value, rocksdb::Transaction* txn )
    {
      using rocksdb::Slice;

      const auto d = reinterpret_cast<const uint8_t*>( value.data() );
      auto response = model::GetNode( d );
      if ( response )
      {
        auto vec = std::vector<std::string_view>{};
        vec.reserve( response->children()->size() + 1 );
        for ( auto&& item : *response->children() ) vec.push_back( item->string_view() );

        if ( std::find( std::begin( vec ), std::end( vec ), child ) == std::end( vec ) )
        {
          vec.push_back( child );
          std::sort( std::begin( vec ), std::end( vec ) );
        }
        else
        {
          LOG_DEBUG << "Path " << path << " already contains child " << child;
          return true;
        }

        auto fb = flatbuffers::FlatBufferBuilder{ 128 };
        auto children = std::vector<flatbuffers::Offset<flatbuffers::String>>{};
        children.reserve( vec.size() );
        for ( auto&& c : vec ) children.push_back( fb.CreateString( c ) );

        auto nc = fb.CreateVector( children );
        auto n = model::CreateNode( fb, nc );
        fb.Finish( n );

        if ( const auto s = txn->Put( handles[2], Slice{ path }, Slice{ reinterpret_cast<const char *>( fb.GetBufferPointer() ), fb.GetSize() } );
            s.ok() )
        {
          LOG_DEBUG << "Saved path " << path << " with " << int(vec.size()) << " components";
        }
        else
        {
          LOG_WARN << "Error saving path: " << path << " with " << int(vec.size()) <<
            " components. " << s.ToString();
          return false;
        }
      }
      else
      {
        LOG_CRIT << "Error marshalling buffer for path: " << path;
        return false;
      }

      return true;
    }

    bool createChild( std::string_view path, std::string_view child, rocksdb::Transaction* txn )
    {
      using rocksdb::Slice;

      auto fb = flatbuffers::FlatBufferBuilder{ 128 };
      auto children = std::vector<flatbuffers::Offset<flatbuffers::String>>{ fb.CreateString( child ) };
      auto nc = fb.CreateVector( children );
      auto n = model::CreateNode( fb, nc );
      fb.Finish( n );

      if ( const auto s = txn->Put( handles[2], Slice{ path }, Slice{ reinterpret_cast<const char *>( fb.GetBufferPointer() ), fb.GetSize() } );
          s.ok() )
      {
        LOG_DEBUG << "Created path " << path << " with child " << child;
      }
      else
      {
        LOG_WARN << "Error saving path: " << path << " with component " << child <<
          ". " << s.ToString();
        return false;
      }

      return true;
    }

    bool remove( std::string_view key, rocksdb::Transaction* txn )
    {
      const auto rollback = [&txn]()
      {
        if ( const auto s = txn->Rollback(); !s.ok())
        {
          LOG_CRIT << "Error rolling back transaction. " << s.ToString();
        }
        return false;
      };

      auto ks = getKey( key );
      LOG_INFO << "Removing key " << ks;
      if ( const auto s = txn->Delete( handles[1], rocksdb::Slice{ ks } ); !s.ok() )
      {
        LOG_WARN << "Error removing key " << key << ". " << s.ToString();
        return rollback();
      }

      if ( !removeChild( ks, txn ) )
      {
        LOG_WARN << "Error managing tree after removing key " << key;
        return rollback();
      }

      if ( model::Configuration::instance().enableCache ) util::getValueCache().erase( ks );
      return true;
    }

    bool removeChild( std::string_view key, rocksdb::Transaction* txn )
    {
      const auto tuples = treeComponents( key );
      if ( tuples.empty() ) return true;

      for ( auto iter = std::rbegin( tuples ); iter != std::rend( tuples ); ++iter )
      {
        const auto& [path, child] = *iter;
        std::string value;
        if ( const auto s = txn->Get( rocksdb::ReadOptions{}, handles[2],
              rocksdb::Slice{ path }, &value ); s.ok() )
        {
          auto [status,empty] = removeChildNode( path, child, value, txn );
          if ( !status ) return false;
          if ( !empty ) break;
        }
        else if ( s.IsNotFound() )
        {
          return true;
        }
        else
        {
          LOG_WARN << "Error processing path: " << key << ", child: " << child <<
            ". " << s.ToString();
          return false;
        }
      }

      return true;
    }

    std::tuple<bool,bool> removeChildNode( std::string_view path, std::string_view child,
        std::string_view value, rocksdb::Transaction* txn )
    {
      const auto d = reinterpret_cast<const uint8_t*>( value.data() );
      auto response = model::GetNode( d );
      if ( response )
      {
        auto vec = std::vector<std::string_view>{};
        vec.reserve( response->children()->size() );

        for ( flatbuffers::uoffset_t i = 0; i < response->children()->size(); ++i )
        {
          auto item = response->children()->Get( i );
          if ( child != item->string_view() ) vec.push_back( item->string_view() );
        }

        if ( vec.empty() )
        {
          if ( const auto s = txn->Delete( handles[2], rocksdb::Slice{ path } ); s.ok() )
          {
            LOG_INFO << "Removed empty path " << path;
            return {true, true};
          }
          else
          {
            LOG_WARN << "Error removing empty path " << path << ". " << s.ToString();
            return {false, false};
          }
        }

        auto fb = flatbuffers::FlatBufferBuilder{ 128 };
        auto children = std::vector<flatbuffers::Offset<flatbuffers::String>>{};
        children.reserve( vec.size() );
        for ( auto&& c : vec ) children.push_back( fb.CreateString( c ) );

        auto nc = fb.CreateVector( children );
        auto n = model::CreateNode( fb, nc );
        fb.Finish( n );

        if ( const auto s = txn->Put( handles[2], rocksdb::Slice{ path },
            rocksdb::Slice{ reinterpret_cast<const char *>( fb.GetBufferPointer() ), fb.GetSize() } );
            s.ok() )
        {
          LOG_DEBUG << "Saved path " << path << " with " << int(vec.size()) << " components";
        }
        else
        {
          LOG_WARN << "Error saving path: " << path << " with " << int(vec.size()) <<
            " components. " << s.ToString();
          return {false, false};
        }
      }
      else
      {
        LOG_CRIT << "Error marshalling buffer for path " << path;
      }

      return {true, false};
    }

    using Tuple = std::tuple<std::string_view, std::string_view>;
    using Tuples = std::vector<Tuple>;
    static Tuples treeComponents( std::string_view key )
    {
      auto tuples = Tuples{};
      tuples.reserve( 8 );

      std::size_t index = 0;

      while ( index != std::string_view::npos )
      {
        if ( index == 0 )
        {
          auto idx = key.find( '/', index + 1 );
          tuples.emplace_back( key.substr( 0, index + 1 ), key.substr( index + 1, idx - index - 1 ) );
        }

        index = key.find( '/', index + 1 );
        if ( ( index + 1 ) < key.size() )
        {
          auto idx = key.find( '/', index + 1 );
          auto sub = idx != std::string_view::npos ?
              key.substr( index + 1, idx - index - 1 ) :
              key.substr( index + 1, key.size() - index - 1 );
          if ( !sub.empty() )
          {
            tuples.emplace_back( key.substr( 0, index ), sub );
          }
        }
      }

      return tuples;
    }

    bool move( std::string_view key, std::string_view dest, rocksdb::Transaction* txn )
    {
      auto encrypter = pool.acquire();
      if ( !encrypter )
      {
        LOG_CRIT << "Unable to acquire encrypter from pool";
        return false;
      }

      std::string value;

      if ( const auto s = txn->Get( rocksdb::ReadOptions{}, handles[1], rocksdb::Slice{ key }, &value );
          !s.ok() )
      {
        if ( s.IsNotFound() ) LOG_WARN << "Key " << key << " not found.";
        else LOG_WARN << "Error retrieving key " << key << ". " << s.ToString();
        return false;
      }
      value = ( *encrypter )->decrypt( value );

      if ( const auto s = remove( key, txn ); !s )
      {
        if ( const auto st = txn->Rollback(); !st.ok() )
        {
          LOG_CRIT << "Error rolling back transaction. " << st.ToString();
        }
        return s;
      }

      if ( const auto s = set( dest, value, txn ); !s )
      {
        if ( const auto st = txn->Rollback(); !st.ok() )
        {
          LOG_CRIT << "Error rolling back transaction. " << st.ToString();
        }
        return s;
      }

      return true;
    }

    void init()
    {
#ifdef __APPLE__
      static const std::string dbpath{ "/tmp/config-db" };
#else
      static const std::string dbpath{ "/opt/spt/data" };
#endif
      const auto cfopts = rocksdb::ColumnFamilyOptions{};
      std::vector<rocksdb::ColumnFamilyDescriptor> families{
          { rocksdb::kDefaultColumnFamilyName, cfopts },
          { "data"s, cfopts },
          { "forrest"s, cfopts }
      };

      handles.reserve( families.size() );

      auto dbopts = rocksdb::DBOptions{};
      dbopts.OptimizeForSmallDb( &cache );
      dbopts.create_if_missing = true;
      dbopts.create_missing_column_families = true;

      rocksdb::TransactionDB* tdb;

      if ( const auto status = rocksdb::TransactionDB::Open(
            dbopts, rocksdb::TransactionDBOptions{}, dbpath, families, &handles, &tdb );
          !status.ok() )
      {
        LOG_CRIT << "Error opening database at path " << dbpath << ". " << status.ToString();
        throw DbOpenException{};
      }

      db.reset( tdb );
    }

    void close()
    {
      LOG_INFO << "Closing database";

      for ( auto&& handle : handles )
      {
        if ( const auto s = db->DestroyColumnFamilyHandle( handle );
            !s.ok() )
        {
          LOG_CRIT << "Error destroying column family handle [" <<
                   handle->GetName() << "]. " << s.ToString();
        }
      }

      if ( const auto s = db->Close(); !s.ok() )
      {
        LOG_CRIT << "Error closing database. " << s.ToString();
      }
    }

    spt::configdb::pool::Pool<Encrypter> pool{ spt::configdb::db::internal::create, poolConfig() };
    std::vector<rocksdb::ColumnFamilyHandle*> handles;
    std::unique_ptr<rocksdb::TransactionDB> db{ nullptr };
    std::shared_ptr<rocksdb::Cache> cache = rocksdb::NewLRUCache( 1024 );
  };
}

void spt::configdb::db::init()
{
  internal::Database::instance();
}

std::optional<std::string> spt::configdb::db::get( std::string_view key )
{
  return internal::Database::instance().get( key );
}

bool spt::configdb::db::set( std::string_view key, std::string_view value )
{
  return internal::Database::instance().set( key, value );
}

bool spt::configdb::db::remove( std::string_view key )
{
  return internal::Database::instance().remove( key );
}

bool spt::configdb::db::move( std::string_view key, std::string_view dest )
{
  return internal::Database::instance().move( key, dest );
}

auto spt::configdb::db::list( std::string_view key ) -> Nodes
{
  return internal::Database::instance().list( key );
}

auto spt::configdb::db::get( const std::vector<std::string_view>& keys ) -> std::vector<KeyValue>
{
  return internal::Database::instance().get( keys );
}

bool spt::configdb::db::set( const std::vector<Pair>& kvs )
{
  return internal::Database::instance().set( kvs );
}

bool spt::configdb::db::remove( const std::vector<std::string_view>& keys )
{
  return internal::Database::instance().remove( keys );
}

bool spt::configdb::db::move( const std::vector<Pair>& kvs )
{
  return internal::Database::instance().move( kvs );
}

auto spt::configdb::db::list( const std::vector<std::string_view>& keys ) -> std::vector<NodePair>
{
  return internal::Database::instance().list( keys );
}

void spt::configdb::db::reopen()
{
  internal::Database::instance().reopen();
}