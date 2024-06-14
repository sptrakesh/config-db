//
// Created by Rakesh on 24/12/2021.
//

#include "encrypter.h"
#include "storage.h"
#include "util/cache.h"
#include "../common/model/configuration.h"
#include "../common/model/tree_generated.h"
#include "../common/pool/pool.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <limits>
#include <set>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/slice_transform.h>
#include <rocksdb/table.h>
#include <rocksdb/utilities/transaction_db.h>

using std::operator ""s;
using std::operator ""sv;

namespace
{
  namespace internal
  {
    using namespace spt::configdb;
    using namespace spt::configdb::db;

    std::unique_ptr<Encrypter> create()
    {
      return std::make_unique<Encrypter>( model::Configuration::instance().encryption.secret );
    }

    spt::configdb::pool::Configuration poolConfig()
    {
      auto config = spt::configdb::pool::Configuration{};
      config.initialSize = model::Configuration::instance().storage.encrypterInitialPoolSize;
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
        if ( key.empty() )
        {
          LOG_INFO << "Rejecting request for empty key";
          return std::nullopt;
        }

        const auto& conf = model::Configuration::instance();
        auto& vc = spt::util::getValueCache();
        auto ks = getKey( key );

        if ( conf.enableCache )
        {
          if ( auto iter = vc.find( ks ); iter != std::end( vc ) )
          {
            return iter->second;
          }
        }

        std::string value;

        if ( const auto s = db->Get( rocksdb::ReadOptions{}, handles[1],
              rocksdb::Slice{ ks }, &value );
            !s.ok() )
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

      bool set( const model::RequestData& data )
      {
        if ( data.key.empty() )
        {
          LOG_INFO << "Rejecting request for empty key";
          return false;
        }

        if ( data.options.cache && data.options.expirationInSeconds == 0 )
        {
          LOG_WARN << "Request to cache " << data.key << " without expiration";
          return false;
        }

        auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );
        if ( const auto saved = set( data.key, data.value, data.options, txn ); !saved ) return saved;

        if ( const auto s = txn->Commit(); !s.ok() )
        {
          LOG_WARN << "Error writing to key [" << data.key << "]. " << s.ToString();
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
        if ( const auto s = txn->Commit(); !s.ok() )
        {
          LOG_WARN << "Error removing key [" << key << "]. " << s.ToString();
          return false;
        }

        return true;
      }

      bool move( const model::RequestData& data )
      {
        if ( data.key.empty() )
        {
          LOG_INFO << "Rejecting request for empty key";
          return false;
        }

        auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );
        if ( const auto s = move( data.key, data.value, data.options, txn ); !s ) return s;

        if ( const auto s = txn->Commit(); !s.ok() )
        {
          LOG_WARN << "Error removing key [" << data.key << "]. " << s.ToString();
          return false;
        }

        return true;
      }

      std::chrono::seconds ttl( std::string_view key )
      {
        auto ks = getKey( key );
        std::string value;
        if ( const auto s = db->Get( rocksdb::ReadOptions{}, handles[3], rocksdb::Slice{ ks }, &value ); s.ok() )
        {
          return parseTTL( value );
        }

        return std::chrono::seconds{ 0 };
      }

      Nodes list( std::string_view key )
      {
        if ( key.empty() )
        {
          LOG_INFO << "Rejecting request for empty key";
          return std::nullopt;
        }

        auto ks = getKey( key );
        std::string value;
        if ( const auto s = db->Get( rocksdb::ReadOptions{}, handles[2],
              rocksdb::Slice{ ks }, &value );
            s.ok() )
        {
          const auto d = reinterpret_cast<const uint8_t*>( value.data() );
          auto response = model::GetNode( d );

          if ( response )
          {
            auto vec = std::vector<std::string>{};
            vec.reserve( response->children()->size() + 1 );
            for ( const auto& child : *response->children() ) vec.push_back( child->str() );
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
        auto& vc = spt::util::getValueCache();

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
        for ( const auto& key : vec ) skeys.push_back( getKey( key ) );

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

      bool set( const std::vector<model::RequestData>& kvs )
      {
        auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );

        for ( const auto& data : kvs )
        {
          if ( const auto s = set( data.key, data.value, data.options, txn ); !s )
          {
            LOG_WARN << "Error setting key " << data.key;
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

        for ( const auto& key : keys )
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

      bool move( const std::vector<model::RequestData>& kvs )
      {
        auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );

        for ( const auto& data : kvs )
        {
          if ( const auto s = move( data.key, data.value, data.options, txn ); !s )
          {
            LOG_WARN << "Error moving key " << data.key << " to destination " << data.value;
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

      std::vector<TTLPair> ttl( const std::vector<std::string_view>& vec )
      {
        std::vector<rocksdb::Slice> keys;
        keys.reserve( vec.size() );
        std::vector<rocksdb::PinnableSlice> values;
        std::vector<rocksdb::Status> statuses;

        for ( const auto& key: vec ) keys.emplace_back( key );

        values.resize( keys.size() );
        statuses.resize( keys.size() );
        db->MultiGet( rocksdb::ReadOptions{}, handles[3], keys.size(),
            keys.data(), values.data(), statuses.data() );

        std::vector<TTLPair> result;
        result.reserve( vec.size() );
        for ( std::size_t i = 0; i < vec.size(); ++i )
        {
          auto ks = keys[i].ToString();

          if ( statuses[i].ok() )
          {
            auto ttl = parseTTL( values[i].ToStringView() );
            result.emplace_back( ks, ttl );
          }
          else
          {
            LOG_WARN << "Error retrieving path " << vec[i] << ". " << statuses[i].ToString();
            result.emplace_back( ks, std::chrono::seconds{ 0 } );
          }
        }

        return result;
      }

      std::vector<NodePair> list( const std::vector<std::string_view>& vec )
      {
        std::vector<rocksdb::Slice> keys;
        keys.reserve( vec.size() );
        std::vector<rocksdb::PinnableSlice> values;
        std::vector<rocksdb::Status> statuses;

        for ( const auto& key: vec ) keys.emplace_back( key );

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
              for ( const auto& child : *response->children() ) rvec.push_back( child->str() );
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

      struct IteratorHolder
      {
        IteratorHolder( rocksdb::TransactionDB& db, rocksdb::ColumnFamilyHandle* handle )
        {
          auto now = std::chrono::high_resolution_clock::now();
          auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>( now.time_since_epoch() ).count();
          limit = std::to_string( ns );
          lv = limit;
          slice = rocksdb::Slice{ limit };

          auto opts = rocksdb::ReadOptions{};
          opts.iterate_upper_bound = &slice;

          it.reset( db.NewIterator( opts, handle ) );
        }

        IteratorHolder(const IteratorHolder&) = delete;
        IteratorHolder& operator=(const IteratorHolder&) = delete;

        auto SeekToFirst() { return it->SeekToFirst(); }
        [[nodiscard]] auto Valid() const { return it->Valid(); }
        [[nodiscard]] auto Next() const { return it->Next(); }
        [[nodiscard]] auto key() const { return it->key(); }
        [[nodiscard]] auto value() const { return it->value(); }

      private:
        std::string limit;
        std::string_view lv;
        rocksdb::Slice slice;
        std::unique_ptr<rocksdb::Iterator> it;
      };

      void clearExpired( const std::atomic_bool& stop )
      {
        auto txn = db->BeginTransaction( rocksdb::WriteOptions{} );
        auto it = IteratorHolder{ *db, handles[4] };

        std::vector<std::tuple<std::string,std::string>> keys;
        keys.reserve( 32 );

        for ( it.SeekToFirst(); it.Valid(); it.Next() )
        {
          keys.emplace_back( it.key().ToString(), it.value().ToString() );
          if ( stop.load() ) break;
        }

        if ( stop.load() ) return;
        auto total = 0;
        auto count = 0;
        for ( const auto& [key, value] : keys )
        {
          LOG_INFO << "Removing expired key " << value;
          if ( remove( value, txn, false ) )
          {
            ++count;

            if ( const auto s = txn->Delete( handles[4], rocksdb::Slice{ key } ); !s.ok() )
            {
              LOG_WARN << "Error deleting expired key handle " << key;
            }
          }
          else LOG_WARN << "Error removing expired key " << value;
          ++total;

          if ( stop.load() ) break;
        }

        if ( const auto s = txn->Commit(); !s.ok() )
        {
          LOG_CRIT << "Error committing transaction after removing expired keys";
        }
        else if ( count > 0 )
        {
          LOG_INFO << "Removed " << count << '/' << total << " expired keys";
        }
      }

      void reopen()
      {
        close();
        init();
        spt::util::getValueCache().clearAll();
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

      bool set( std::string_view key, std::string_view value,
          const model::RequestData::Options& opts, rocksdb::Transaction* txn )
      {
        const auto rollback = [txn]()
        {
          if ( const auto s = txn->Rollback(); !s.ok() )
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

        if ( opts.ifNotExists )
        {
          std::string va;
          const auto s = txn->Get( rocksdb::ReadOptions{}, handles[1],
              rocksdb::Slice{ ks }, &va );
          if ( s.ok() )
          {
            LOG_INFO << "Key " << key << " exists and ifNotExists set to true";
            return false;
          }
          if ( !s.IsNotFound() )
          {
            LOG_WARN << "Error checking existence of Key " << key << ". "
                     << s.ToString();
            return false;
          }
        }

        LOG_INFO << "Setting key " << ks;
        if ( const auto s = txn->Put( handles[1], rocksdb::Slice{ ks }, rocksdb::Slice{ v } );
            !s.ok() )
        {
          LOG_WARN << "Error setting key " << key << ". " << s.ToString();
          return rollback();
        }

        if ( !opts.cache && !manageTree( ks, txn ) )
        {
          LOG_WARN << "Error managing tree for key " << key;
          return rollback();
        }

        if ( opts.expirationInSeconds > 0 )
        {
          auto now = std::chrono::high_resolution_clock::now();
          now += std::chrono::seconds{ opts.expirationInSeconds };
          auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>( now.time_since_epoch() ).count();

          std::string ts;
          ts.reserve( 32 );
          ts.append( std::to_string( ns ) );

          if ( const auto s = txn->Put( handles[3], rocksdb::Slice{ ks }, rocksdb::Slice{ ts } );
              !s.ok() )
          {
            LOG_WARN << "Error setting expiration for key " << key << ". " << s.ToString();
            return rollback();
          }

          now = std::chrono::high_resolution_clock::now();
          auto ns1 = std::chrono::duration_cast<std::chrono::nanoseconds>( now.time_since_epoch() ).count();
          ts.append( std::to_string( ns1 ) );

          if ( const auto s = txn->Put( handles[4], rocksdb::Slice{ ts }, rocksdb::Slice{ ks } );
            !s.ok() )
          {
            LOG_WARN << "Error setting expiration for key " << key << ", dest " << ts << ". " << s.ToString();
            return rollback();
          }
          else LOG_DEBUG << "Setting expiration for key " << key << ", dest " << ts << ". " << s.ToString();
        }

        if ( model::Configuration::instance().enableCache )
        {
          spt::util::getValueCache().put( ks, std::string{ value.data(), value.size() } );
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
        for ( const auto& [path, child] : tuples )
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
            LOG_WARN << "Error processing path: " << path << ", child: " << child << ". " << s.ToString();
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
        if ( auto response = model::GetNode( d ); response )
        {
          auto set = std::set<std::string_view, std::less<>>{};
          for ( const auto& item : *response->children() ) set.insert( item->string_view() );

          if ( !set.contains( child ) ) set.insert( child );
          else
          {
            LOG_DEBUG << "Path " << path << " already contains child " << child;
            return true;
          }

          auto fb = flatbuffers::FlatBufferBuilder{ 128 };
          auto children = std::vector<flatbuffers::Offset<flatbuffers::String>>{};
          children.reserve( set.size() );
          for ( const auto& c : set ) children.push_back( fb.CreateString( c ) );

          auto nc = fb.CreateVector( children );
          auto n = model::CreateNode( fb, nc );
          fb.Finish( n );

          if ( const auto s = txn->Put( handles[2], Slice{ path }, Slice{ reinterpret_cast<const char *>( fb.GetBufferPointer() ), fb.GetSize() } );
              s.ok() )
          {
            LOG_DEBUG << "Saved path " << path << " with " << int(set.size()) << " components";
          }
          else
          {
            LOG_WARN << "Error saving path: " << path << " with " << int(set.size()) <<
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
          LOG_WARN << "Error saving path: " << path << " with component " << child << ". " << s.ToString();
          return false;
        }

        return true;
      }

      bool remove( std::string_view key, rocksdb::Transaction* txn, bool rollbackOnError = true )
      {
        const auto rollback = [&txn, rollbackOnError]()
        {
          if ( !rollbackOnError ) return false;
          if ( const auto s = txn->Rollback(); !s.ok() )
          {
            LOG_CRIT << "Error rolling back transaction. " << s.ToString();
          }
          return false;
        };

        auto ks = getKey( key );
        LOG_INFO << "Removing key " << ks;
        if ( const auto s = txn->Delete( handles[1], rocksdb::Slice{ ks } ); !s.ok() )
        {
          LOG_WARN << "Error removing key " << key << " from keys column. " << s.ToString();
          return rollback();
        }

        if ( const auto s = txn->Delete( handles[3], rocksdb::Slice{ ks } ); !s.ok() )
        {
          LOG_WARN << "Error removing key " << key << " from expiration column. " << s.ToString();
          return rollback();
        }

        if ( !removeChild( ks, txn ) )
        {
          LOG_WARN << "Error managing tree after removing key " << key;
          return rollback();
        }

        if ( model::Configuration::instance().enableCache ) spt::util::getValueCache().erase( ks );
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

            if ( const auto s1 = txn->Get( rocksdb::ReadOptions{}, handles[1],
                rocksdb::Slice{ path }, &value ); s1.ok() )
            {
              LOG_INFO << "Path " << path << " has value, not moving up tree";
              break;
            }
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
          for ( const auto& c : vec ) children.push_back( fb.CreateString( c ) );

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

      std::chrono::seconds parseTTL( std::string_view value )
      {
        std::chrono::seconds exp{ 0 };
        uint64_t v{ 0 };
        auto [p, ec] = std::from_chars( value.data(), value.data() + value.size(), v );
        if ( ec != std::errc() )
        {
          LOG_WARN << "Error parsing expiration time " << value;
          return exp;
        }

        std::chrono::nanoseconds ns{ v };
        if ( const auto cns = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );
            ns > cns )
        {
          exp = std::chrono::duration_cast<std::chrono::seconds>( ns - cns );
          LOG_DEBUG << "Parsed ttl " << int(exp.count());
        }

        return exp;
      }

      bool move( std::string_view key, std::string_view dest,
          const model::RequestData::Options& opts, rocksdb::Transaction* txn )
      {
        auto encrypter = pool.acquire();
        if ( !encrypter )
        {
          LOG_CRIT << "Unable to acquire encrypter from pool";
          return false;
        }

        auto ks = getKey( key );
        auto ds = getKey( dest );

        if ( opts.ifNotExists )
        {
          std::string va;
          const auto s = txn->Get( rocksdb::ReadOptions{}, handles[1], rocksdb::Slice{ ds }, &va );
          if ( s.ok() )
          {
            LOG_INFO << "Destination key " << ds << " exists and ifNotExists set to true";
            return false;
          }
          if ( !s.IsNotFound() )
          {
            LOG_WARN << "Error checking existence of destination key " << ds << ". " << s.ToString();
            return false;
          }
        }

        std::string value;
        if ( const auto s = txn->Get( rocksdb::ReadOptions{}, handles[1], rocksdb::Slice{ ks }, &value );
            !s.ok() )
        {
          if ( s.IsNotFound() ) LOG_WARN << "Key " << ks << " not found.";
          else LOG_WARN << "Error retrieving key " << ks << ". " << s.ToString();
          return false;
        }
        value = ( *encrypter )->decrypt( value );

        std::chrono::seconds exp{ opts.expirationInSeconds };
        if ( opts.expirationInSeconds == 0 )
        {
          std::string nsv;
          if ( const auto s = txn->Get( rocksdb::ReadOptions{}, handles[3], rocksdb::Slice{ ks }, &nsv ); s.ok() )
          {
            exp = parseTTL( nsv );
          }
        }

        if ( const auto s = remove( ks, txn ); !s )
        {
          if ( const auto st = txn->Rollback(); !st.ok() )
          {
            LOG_CRIT << "Error rolling back transaction. " << st.ToString();
          }
          return s;
        }

        auto nopts = model::RequestData::Options{};
        nopts.ifNotExists = opts.ifNotExists;
        nopts.expirationInSeconds = static_cast<uint32_t>( exp.count() );
        nopts.cache = opts.cache;
        if ( const auto s = set( ds, value, nopts, txn ); !s )
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
        auto& conf = model::Configuration::instance();

        auto cfopts = rocksdb::ColumnFamilyOptions{};
        cfopts.prefix_extractor.reset( rocksdb::NewFixedPrefixTransform( 8 ) );
        cfopts.table_factory.reset( rocksdb::NewPlainTableFactory() );
        cfopts.OptimizeForPointLookup( conf.storage.blockCacheSizeMb );
        cfopts.level_compaction_dynamic_level_bytes = true;
        //cfopts.write_buffer_size = 64 * 1024 * 1024;
        cfopts.min_write_buffer_number_to_merge = 2;
        std::vector<rocksdb::ColumnFamilyDescriptor> families{
            { rocksdb::kDefaultColumnFamilyName, cfopts },
            { "data"s, cfopts },
            { "forrest"s, cfopts },
            { "expiration"s, cfopts },
            { "expiring"s, rocksdb::ColumnFamilyOptions{} }
        };

        handles.reserve( families.size() );

        auto o = rocksdb::Options{};

        auto dbopts = rocksdb::DBOptions{};
        dbopts.allow_mmap_reads = true;
        dbopts.create_if_missing = true;
        dbopts.create_missing_column_families = true;
        dbopts.wal_bytes_per_sync = 32768;
        dbopts.bytes_per_sync = 32768;
        dbopts.db_write_buffer_size = 128 * 1024 * 1024;

        rocksdb::TransactionDB* tdb;

        if ( const auto status = rocksdb::TransactionDB::Open(
              dbopts, rocksdb::TransactionDBOptions{}, conf.storage.dbpath, families, &handles, &tdb );
            !status.ok() )
        {
          LOG_CRIT << "Error opening database at path " << conf.storage.dbpath << ". " << status.ToString();
          throw DbOpenException{};
        }

        db.reset( tdb );
      }

      void close()
      {
        LOG_INFO << "Closing database";

        for ( const auto& handle : handles )
        {
          if ( const auto s = db->DestroyColumnFamilyHandle( handle ); !s.ok() )
          {
            LOG_CRIT << "Error destroying column family handle [" << handle->GetName() << "]. " << s.ToString();
          }
        }

        if ( const auto s = db->Close(); !s.ok() )
        {
          LOG_CRIT << "Error closing database. " << s.ToString();
        }
      }

      pool::Pool<Encrypter> pool{ create, poolConfig() };
      std::vector<rocksdb::ColumnFamilyHandle*> handles;
      std::unique_ptr<rocksdb::TransactionDB> db{ nullptr };
    };
  }
}

void spt::configdb::db::init()
{
  internal::Database::instance();
}

std::optional<std::string> spt::configdb::db::get( std::string_view key )
{
  return internal::Database::instance().get( key );
}

bool spt::configdb::db::set( const model::RequestData& data )
{
  return internal::Database::instance().set( data );
}

bool spt::configdb::db::remove( std::string_view key )
{
  return internal::Database::instance().remove( key );
}

bool spt::configdb::db::move( const model::RequestData& data )
{
  return internal::Database::instance().move( data );
}

std::chrono::seconds spt::configdb::db::ttl( std::string_view key )
{
  return internal::Database::instance().ttl( key );
}

auto spt::configdb::db::list( std::string_view key ) -> Nodes
{
  return internal::Database::instance().list( key );
}

auto spt::configdb::db::get( const std::vector<std::string_view>& keys ) -> std::vector<KeyValue>
{
  return internal::Database::instance().get( keys );
}

bool spt::configdb::db::set( const std::vector<model::RequestData>& kvs )
{
  return internal::Database::instance().set( kvs );
}

bool spt::configdb::db::remove( const std::vector<std::string_view>& keys )
{
  return internal::Database::instance().remove( keys );
}

bool spt::configdb::db::move( const std::vector<model::RequestData>& kvs )
{
  return internal::Database::instance().move( kvs );
}

auto spt::configdb::db::ttl( const std::vector<std::string_view>& keys ) -> std::vector<TTLPair>
{
  return internal::Database::instance().ttl( keys );
}

auto spt::configdb::db::list( const std::vector<std::string_view>& keys ) -> std::vector<NodePair>
{
  return internal::Database::instance().list( keys );
}

void spt::configdb::db::clearExpired( const std::atomic_bool& stop )
{
  internal::Database::instance().clearExpired( stop );
}

void spt::configdb::db::reopen()
{
  internal::Database::instance().reopen();
}