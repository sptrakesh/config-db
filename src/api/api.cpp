//
// Created by Rakesh on 02/01/2022.
//

#include "api.hpp"
#include "impl/connection.hpp"
#include "../common/pool/pool.hpp"

#include <charconv>

namespace spt::configdb::api::papi
{
  struct PoolHolder
  {
    static PoolHolder& instance()
    {
      static PoolHolder p;
      return p;
    }

    ~PoolHolder() = default;
    PoolHolder(const PoolHolder&) = delete;
    PoolHolder& operator=(const PoolHolder&) = delete;

    auto acquire() { return pool.acquire(); }

  private:
    PoolHolder() = default;
    spt::configdb::pool::Pool<impl::BaseConnection> pool{ spt::configdb::api::impl::create, pool::Configuration{} };
  };
}

std::optional<std::string> spt::configdb::api::get( std::string_view key )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return std::nullopt;
  }

  auto response = (*popt)->get( key );
  if ( !response )
  {
    LOG_DEBUG << "Error retrieving value for key " << key;
    return std::nullopt;
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_DEBUG << "Error retrieving key " << key;
    return std::nullopt;
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_DEBUG << "Error retrieving key " << key;
    return std::nullopt;
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Value )
  {
    LOG_DEBUG << "Error retrieving key " << key;
    return std::nullopt;
  }

  auto value = resp->value()->Get( 0 )->value_as<model::Value>();
  LOG_DEBUG << "Retrieved value for key " << key;
  return value->value()->str();
}

bool spt::configdb::api::set( std::string_view key, std::string_view value )
{
  auto data = model::RequestData{ key, value };
  return set( data );
}

bool spt::configdb::api::set( const model::RequestData& data )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = (*popt)->set( data );
  if ( !response )
  {
    LOG_DEBUG << "Unable to set key " << data.key;
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error setting key " << data.key;
    return false;
  }

  LOG_DEBUG << "Set key " << data.key;
  return true;
}

bool spt::configdb::api::remove( std::string_view key )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->remove( key );
  if ( !response )
  {
    LOG_DEBUG << "Error removing key " << key;
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error removing key " << key;
    return false;
  }

  LOG_DEBUG << "Removed key " << key;
  return true;
}

bool spt::configdb::api::move( std::string_view key, std::string_view dest )
{
  auto data = model::RequestData{ key, dest };
  return move( data );
}

bool spt::configdb::api::move( const model::RequestData& data )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = (*popt)->move( data );
  if ( !response )
  {
    LOG_DEBUG << "Unable to move key " << data.key << " to destination " << data.value;
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error moving key " << data.key << " to destination " << data.value;
    return false;
  }

  LOG_DEBUG << "Moved key " << data.key << " to dest " << data.value;
  return true;
}

auto spt::configdb::api::list( std::string_view path ) -> Nodes
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return std::nullopt;
  }

  auto response = (*popt)->list( path );
  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_DEBUG << "Error retrieving path " << path;
    return std::nullopt;
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_DEBUG << "Error retrieving path " << path;
    return std::nullopt;
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Children )
  {
    LOG_DEBUG << "Error listing path " << path;
    return std::nullopt;
  }

  auto value = resp->value()->Get( 0 )->value_as<model::Children>();
  if ( value->value()->size() == 0 )
  {
    LOG_DEBUG << "Error listing path " << path;
    return std::nullopt;
  }

  LOG_DEBUG << "Retrieved children for path " << path;
  std::vector<std::string> results;
  for ( auto&& v : *value->value() ) results.emplace_back( v->string_view() );
  return results;
}

std::chrono::seconds spt::configdb::api::ttl( std::string_view key )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return std::chrono::seconds{0};
  }

  auto response = ( *popt )->ttl( key );
  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_DEBUG << "Error retrieving TTL for key " << key;
    return std::chrono::seconds{ 0 };
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_DEBUG << "Error retrieving TTL for key " << key;
    return std::chrono::seconds{ 0 };
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Value )
  {
    LOG_DEBUG << "Error retrieving TTL for key " << key;
    return std::chrono::seconds{ 0 };
  }

  auto value = resp->value()->Get( 0 )->value_as<model::Value>();
  LOG_DEBUG << "Retrieved TTL for key " << key;
  auto sv = value->value()->string_view();
  uint64_t v;
  auto [p, ec] = std::from_chars( sv.data(), sv.data() + sv.size(), v );
  if ( ec != std::errc() )
  {
    LOG_WARN << "Invalid TTL: " << sv;
    return std::chrono::seconds{ 0 };
  }
  return std::chrono::seconds{ v };
}

auto spt::configdb::api::get( const std::vector<std::string_view>& keys ) -> std::vector<KeyValue>
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto response = ( *popt )->get( keys );
  if ( !response )
  {
    LOG_DEBUG << "Error retrieving values for " << int(keys.size()) << " keys";
    return {};
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_DEBUG << "Error retrieving values for " << int(keys.size()) << " keys";
    return {};
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != keys.size() )
  {
    LOG_DEBUG << "Error retrieving values for " << int(keys.size()) << " keys";
    return {};
  }

  std::vector<KeyValue> results;
  results.reserve( keys.size() );

  for ( auto&& r : *resp->value() )
  {
    if ( r->value_type() == model::ValueVariant::Value )
    {
      auto rv = r->value_as<model::Value>();
      results.emplace_back( r->key()->str(), rv->value()->str() );
    }
    else
    {
      LOG_DEBUG << "Error retrieving key " << r->key()->string_view();
      results.emplace_back( r->key()->str(), std::nullopt );
    }
  }

  LOG_DEBUG << "Retrieved values for " << int(size(keys)) << " keys";
  return results;
}

auto spt::configdb::api::get( const std::vector<std::string>& keys ) -> std::vector<KeyValue>
{
  auto vec = std::vector<std::string_view>{};
  vec.reserve( keys.size() );
  for ( const auto& key : keys ) vec.emplace_back( key );
  return get( vec );
}

bool spt::configdb::api::set( const std::vector<Pair>& kvs )
{
  auto vec = std::vector<model::RequestData>{};
  vec.reserve( kvs.size() );
  for ( auto&& [key, value] : kvs ) vec.emplace_back( key, value );
  return set( vec );
}

bool spt::configdb::api::set( const std::vector<model::RequestData>& kvs )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->set( kvs );
  if ( !response )
  {
    LOG_DEBUG << "Unable to set values for " << int(size(kvs)) << " keys";
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error setting values for " << int(size(kvs)) << " keys";
    return false;
  }

  LOG_DEBUG << "Set values for " << int(size(kvs)) << " keys";
  return true;
}

bool spt::configdb::api::remove( const std::vector<std::string_view>& keys )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->remove( keys );
  if ( !response )
  {
    LOG_DEBUG << "Unable to remove " << int(size(keys)) << " keys";
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error removing " << int(size(keys)) << " keys";
    return false;
  }

  LOG_DEBUG << "Removed " << int(size(keys)) << " keys";
  return true;
}

bool spt::configdb::api::move( const std::vector<Pair>& kvs )
{
  auto vec = std::vector<model::RequestData>{};
  vec.reserve( kvs.size() );
  for ( auto&& [key, value] : kvs ) vec.emplace_back( key, value );
  return api::move( vec );
}

bool spt::configdb::api::move( const std::vector<model::RequestData>& kvs )
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->move( kvs );
  if ( !response )
  {
    LOG_DEBUG << "Unable to move " << int(size(kvs)) << " keys";
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error moving " << int(size(kvs)) << " keys";
    return false;
  }

  LOG_DEBUG << "Moved " << int(size(kvs)) << " keys";
  return true;
}

auto spt::configdb::api::list( const std::vector<std::string_view>& paths ) -> std::vector<NodePair>
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto response = ( *popt )->list( paths );
  if ( !response )
  {
    LOG_DEBUG << "Unable to list " << int(size(paths)) << " paths";
    return {};
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_DEBUG << "Error retrieving " << int(paths.size()) << " paths";
    return {};
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != paths.size() )
  {
    LOG_DEBUG << "Error retrieving " << int(paths.size()) << " paths";
    return {};
  }

  std::vector<NodePair> results;
  results.reserve( paths.size() );

  for ( auto&& r : *resp->value() )
  {
    if ( r->value_type() == model::ValueVariant::Children )
    {
      auto rv = r->value_as<model::Children>();
      auto rvec = std::vector<std::string>{};
      rvec.reserve( rv->value()->size() );
      for ( auto&& rvi : *rv->value() ) rvec.emplace_back( rvi->string_view() );
      results.emplace_back( r->key()->str(), rvec );
    }
    else
    {
      LOG_DEBUG << "Error retrieving key " << r->key()->string_view();
      results.emplace_back( r->key()->str(), std::nullopt );
    }
  }

  LOG_DEBUG << "Retrieved " << int(size(paths)) << " paths";
  return results;
}

auto spt::configdb::api::ttl( const std::vector<std::string_view>& keys ) -> std::vector<TTLPair>
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto response = ( *popt )->ttl( keys );
  if ( !response )
  {
    LOG_DEBUG << "Error retrieving TTL values for " << int(keys.size()) << " keys";
    return {};
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_DEBUG << "Error retrieving TTL values for " << int(keys.size()) << " keys";
    return {};
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != keys.size() )
  {
    LOG_DEBUG << "Error retrieving TTL values for " << int(keys.size()) << " keys";
    return {};
  }

  std::vector<TTLPair> results;
  results.reserve( keys.size() );

  for ( auto&& r : *resp->value() )
  {
    if ( r->value_type() == model::ValueVariant::Value )
    {
      auto rv = r->value_as<model::Value>();
      auto sv = rv->value()->string_view();
      uint64_t v{ 0 };
      auto [p, ec] = std::from_chars( sv.data(), sv.data() + sv.size(), v );
      if ( ec != std::errc() )
      {
        LOG_WARN << "Invalid TTL value: " << sv;
        results.emplace_back( r->key()->str(), std::chrono::seconds{ 0 } );
      }
      results.emplace_back( r->key()->str(), std::chrono::seconds{ v } );
    }
    else
    {
      LOG_DEBUG << "Error retrieving TTL for key " << r->key()->string_view();
      results.emplace_back( r->key()->str(), std::chrono::seconds{ 0 } );
    }
  }

  LOG_DEBUG << "Retrieved TTL values for " << int(size(keys)) << " keys";
  return results;
}

auto spt::configdb::api::ttl( const std::vector<std::string>& keys ) -> std::vector<TTLPair>
{
  auto vec = std::vector<std::string_view>{};
  vec.reserve( keys.size() );
  for ( const auto& key : keys ) vec.emplace_back( key );
  return ttl( vec );
}

auto spt::configdb::api::import( const std::string& file ) -> ImportResponse
{
  auto popt = papi::PoolHolder::instance().acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {false, 0, 0};
  }

  const auto& [response, size, count] = ( *popt )->import( file );
  if ( !response )
  {
    LOG_DEBUG << "Unable to import (" << int(size) << '/' << count << ") key-values";
    return { false, size, count };
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_DEBUG << "Error importing (" << int(size) << '/' << count << ") key-values";
    return { false, size, count };
  }

  LOG_DEBUG << "Imported (" << int(size) << '/' << count << ") key-values";
  return { true, size, count };
}