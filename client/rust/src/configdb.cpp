//
// Created by Rakesh on 15/10/2025.
//
#include "configdb/include/configdb.hpp"

#include <exception>
#include <string_view>
#include <boost/json/value.hpp>
#include <configdb/api/api.hpp>
#include <configdb/common/model/request.hpp>
#include <log/NanoLog.hpp>

namespace
{
  namespace pcdb
  {
    spt::configdb::model::RequestData to( const RequestData& data )
    {
      auto ks = std::string_view( data.key.data(), data.key.size() );
      auto vs = std::string_view( data.value.data(), data.value.size() );
      auto rd = spt::configdb::model::RequestData{ ks, vs };
      rd.options.expirationInSeconds = data.expirationInSeconds;
      rd.options.ifNotExists = data.ifNotExists;
      rd.options.cache = data.cache;
      return rd;
    }
  }
}

void init_logger( Logger conf )
{
  switch ( conf.level )
  {
  case LogLevel::DEBUG: nanolog::set_log_level( nanolog::LogLevel::DEBUG ); break;
  case LogLevel::INFO: nanolog::set_log_level( nanolog::LogLevel::INFO ); break;
  case LogLevel::WARN: nanolog::set_log_level( nanolog::LogLevel::WARN ); break;
  case LogLevel::CRIT: nanolog::set_log_level( nanolog::LogLevel::CRIT ); break;
  }

  auto dir = std::string( conf.path.data(), conf.path.size() );
  auto name = std::string( conf.name.data(), conf.name.size() );
  nanolog::initialize( nanolog::GuaranteedLogger(), dir, name, conf.console );
}

void init( Configuration conf )
{
  auto sh = std::string_view{ conf.host.begin(), conf.host.end() };
  auto sp = std::to_string( conf.port );
  spt::configdb::api::init( sh, sp, conf.ssl );
}

rust::String get( rust::Str key )
{
  auto str = std::string_view( key.data(), key.size() );
  if ( auto result = spt::configdb::api::get( str ); result ) return *result;
  throw std::runtime_error( "Not found" );
}

rust::Vec<KeyValue> get_multiple( const rust::Vec<rust::String>& keys )
{
  auto vec = std::vector<std::string_view>{};
  vec.reserve( keys.size() );
  for ( const auto& key : keys ) vec.emplace_back( key.data(), key.size() );

  auto out = rust::Vec<KeyValue>{};
  const auto result = spt::configdb::api::get( vec );
  out.reserve( result.size() );
  for ( const auto& [k, v] : result )
  {
    if ( !v ) continue;
    out.emplace_back( k, *v );
  }
  return out;
}

bool set( const RequestData& data )
{
  auto rd = pcdb::to( data );
  return spt::configdb::api::set( rd );
}

bool set_multiple( const rust::Vec<RequestData>& keys )
{
  auto kvs = std::vector<spt::configdb::model::RequestData>{};
  kvs.reserve( keys.size() );
  for ( const auto& kv : keys ) kvs.push_back( pcdb::to( kv ) );

  return spt::configdb::api::set( kvs );
}

bool rename( rust::Str key, rust::Str dest )
{
  auto ks = std::string_view( key.data(), key.size() );
  auto vs = std::string_view( dest.data(), dest.size() );
  return spt::configdb::api::move( ks, vs );
}

bool rename_multiple( const rust::Vec<KeyValue>& keys )
{
  auto kvs = std::vector<spt::configdb::api::Pair>{};
  kvs.reserve( keys.size() );
  for ( const auto& kv : keys )
  {
    auto ks = std::string_view( kv.key.data(), kv.key.size() );
    auto vs = std::string_view( kv.value.data(), kv.value.size() );
    kvs.push_back( std::make_pair( ks, vs ) );
  }

  return spt::configdb::api::move( kvs );
}

bool remove( rust::Str key )
{
  auto ks = std::string_view( key.data(), key.size() );
  return spt::configdb::api::remove( ks );
}

bool remove_multiple( const rust::Vec<rust::String>& keys )
{
  auto vec = std::vector<std::string_view>{};
  vec.reserve( keys.size() );
  for ( const auto& key : keys ) vec.emplace_back( key.data(), key.size() );
  return spt::configdb::api::remove( vec );
}

rust::Vec<rust::String> list( rust::Str key )
{
  auto ks = std::string_view( key.data(), key.size() );
  const auto result = spt::configdb::api::list( ks );
  if ( !result ) throw std::runtime_error( "Not found" );

  auto out = rust::Vec<rust::String>{};
  out.reserve( result->size() );
  for ( const auto& path : *result ) out.emplace_back( path );
  return out;
}

uint32_t ttl( rust::Str key )
{
  auto ks = std::string_view( key.data(), key.size() );
  const auto result = spt::configdb::api::ttl( ks );
  return result.count();
}

rust::Vec<TTL> ttl_multiple( const rust::Vec<rust::String>& keys )
{
  auto vec = std::vector<std::string_view>{};
  vec.reserve( keys.size() );
  for ( const auto& key : keys ) vec.emplace_back( key.data(), key.size() );
  const auto result = spt::configdb::api::ttl( vec );

  auto out = rust::Vec<TTL>{};
  out.reserve( result.size() );
  for ( const auto& [k, v] : result ) out.emplace_back( k, v.count() );
  return out;
}
