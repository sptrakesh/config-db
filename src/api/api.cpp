//
// Created by Rakesh on 02/01/2022.
//

#include "api.h"
#include "impl/connection.h"
#include "../common/pool/pool.h"
#include "../common/log/NanoLog.h"

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

    spt::configdb::pool::Pool<impl::Connection> pool{ spt::configdb::api::impl::create, pool::Configuration{} };

  private:
    PoolHolder() = default;
  };
}

std::optional<std::string> spt::configdb::api::get( std::string_view key )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return std::nullopt;
  }

  auto response = (*popt)->get( key );
  if ( !response )
  {
    LOG_WARN << "Error retrieving value for key " << key;
    return std::nullopt;
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_WARN << "Error retrieving key " << key;
    return std::nullopt;
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_WARN << "Error retrieving key " << key;
    return std::nullopt;
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Value )
  {
    LOG_WARN << "Error retrieving key " << key;
    return std::nullopt;
  }

  auto value = resp->value()->Get( 0 )->value_as<model::Value>();
  LOG_INFO << "Retrieved value for key " << key;
  return value->value()->str();
}

bool spt::configdb::api::set( std::string_view key, std::string_view value )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = (*popt)->set( key, value );
  if ( !response )
  {
    LOG_WARN << "Unable to set key " << key;
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_WARN << "Error setting key " << key;
    return false;
  }

  LOG_INFO << "Set key " << key;
  return true;
}

bool spt::configdb::api::remove( std::string_view key )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->remove( key );
  if ( !response )
  {
    LOG_WARN << "Error removing key " << key;
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_WARN << "Error removing key " << key;
    return false;
  }

  LOG_INFO << "Removed key " << key;
  return true;
}

bool spt::configdb::api::move( std::string_view key, std::string_view dest )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = (*popt)->move( key, dest );
  if ( !response )
  {
    LOG_WARN << "Unable to move key " << key << " to destination " << dest;
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_WARN << "Error moving key " << key << " to destination " << dest;
    return false;
  }

  LOG_INFO << "Moved key " << key << " to dest " << dest;
  return true;
}

auto spt::configdb::api::list( std::string_view path ) -> Nodes
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return std::nullopt;
  }

  auto response = (*popt)->list( path );
  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_WARN << "Error retrieving path " << path;
    return std::nullopt;
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_WARN << "Error retrieving path " << path;
    return std::nullopt;
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Children )
  {
    LOG_WARN << "Error listing path " << path;
    return std::nullopt;
  }

  auto value = resp->value()->Get( 0 )->value_as<model::Children>();
  if ( value->value()->size() == 0 )
  {
    LOG_WARN << "Error listing path " << path;
    return std::nullopt;
  }

  LOG_INFO << "Retrieved children for path " << path;
  std::vector<std::string> results;
  for ( auto&& v : *value->value() ) results.emplace_back( v->string_view() );
  return results;
}

auto spt::configdb::api::get( const std::vector<std::string_view>& keys ) -> std::vector<KeyValue>
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto response = ( *popt )->get( keys );
  if ( !response )
  {
    LOG_WARN << "Error retrieving values for " << int(keys.size()) << " keys";
    return {};
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_WARN << "Error retrieving values for " << int(keys.size()) << " keys";
    return {};
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != keys.size() )
  {
    LOG_WARN << "Error retrieving values for " << int(keys.size()) << " keys";
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
      LOG_WARN << "Error retrieving key " << r->key()->string_view();
      results.emplace_back( r->key()->str(), std::nullopt );
    }
  }

  LOG_INFO << "Retrieved values for " << int(size(keys)) << " keys";
  return results;
}

bool spt::configdb::api::set( const std::vector<Pair>& kvs )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->set( kvs );
  if ( !response )
  {
    LOG_WARN << "Unable to set values for " << int(size(kvs)) << " keys";
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_WARN << "Error setting values for " << int(size(kvs)) << " keys";
    return false;
  }

  LOG_INFO << "Set values for " << int(size(kvs)) << " keys";
  return true;
}

bool spt::configdb::api::remove( const std::vector<std::string_view>& keys )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->remove( keys );
  if ( !response )
  {
    LOG_WARN << "Unable to remove " << int(size(keys)) << " keys";
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_WARN << "Error removing " << int(size(keys)) << " keys";
    return false;
  }

  LOG_INFO << "Removed " << int(size(keys)) << " keys";
  return true;
}

bool spt::configdb::api::move( const std::vector<Pair>& kvs )
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return false;
  }

  auto response = ( *popt )->move( kvs );
  if ( !response )
  {
    LOG_WARN << "Unable to move " << int(size(kvs)) << " keys";
    return false;
  }

  if ( response->value_type() != model::ResultVariant::Success ||
      !response->value_as<model::Success>()->value() )
  {
    LOG_WARN << "Error moving " << int(size(kvs)) << " keys";
    return false;
  }

  LOG_INFO << "Moved " << int(size(kvs)) << " keys";
  return true;
}

auto spt::configdb::api::list( const std::vector<std::string_view>& paths ) -> std::vector<NodePair>
{
  auto popt = papi::PoolHolder::instance().pool.acquire();
  if ( !popt )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto response = ( *popt )->list( paths );
  if ( !response )
  {
    LOG_WARN << "Unable to list " << int(size(paths)) << " paths";
    return {};
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_WARN << "Error retrieving " << int(paths.size()) << " paths";
    return {};
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != paths.size() )
  {
    LOG_WARN << "Error retrieving " << int(paths.size()) << " paths";
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
      LOG_WARN << "Error retrieving key " << r->key()->string_view();
      results.emplace_back( r->key()->str(), std::nullopt );
    }
  }

  LOG_INFO << "Retrieved " << int(size(paths)) << " paths";
  return results;
}
