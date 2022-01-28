//
// Created by Rakesh on 30/12/2021.
//

#include "client.h"
#include "../api/impl/connection.h"

#include <iostream>

void spt::configdb::client::get( std::string_view server,
    std::string_view port, std::string_view key, bool ssl )
{
  std::unique_ptr<api::impl::BaseConnection> connection;
  if ( ssl ) connection = std::make_unique<api::impl::SSLConnection>( server, port );
  else connection = std::make_unique<api::impl::Connection>( server, port );

  auto response = connection->get( key );
  if ( !response )
  {
    LOG_WARN << "Error retrieving key " << key;
    std::cout << "Error retrieving key " << key << '\n';
    return;
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    LOG_WARN << "Error retrieving key " << key;
    std::cout << "Error retrieving key " << key << '\n';
    return;
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_WARN << "Error retrieving key " << key;
    std::cout << "Error retrieving key " << key << '\n';
    return;
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Value )
  {
    LOG_WARN << "Error retrieving key " << key;
    std::cout << "Error retrieving key " << key << '\n';
    return;
  }
  auto value = resp->value()->Get( 0 )->value_as<model::Value>();
  LOG_INFO << "Retrieved value for key " << key;
  std::cout << value->value()->string_view() << '\n';
}

void spt::configdb::client::list( std::string_view server,
    std::string_view port, std::string_view path, bool ssl )
{
  std::unique_ptr<api::impl::BaseConnection> connection;
  if ( ssl ) connection = std::make_unique<api::impl::SSLConnection>( server, port );
  else connection = std::make_unique<api::impl::Connection>( server, port );

  auto response = connection->list( path );
  if ( !response )
  {
    LOG_WARN << "Error list path " << path;
    std::cout << "Error listing path " << path << '\n';
    return;
  }

  if ( response->value_type() != model::ResultVariant::KeyValueResults )
  {
    std::cout << "Error retrieving path " << path << '\n';
    return;
  }

  auto resp = response->value_as<model::KeyValueResults>();
  if ( resp->value()->size() != 1 )
  {
    LOG_WARN << "Error listing path " << path;
    std::cout << "Error listing path " << path << '\n';
    return;
  }

  if ( resp->value()->Get( 0 )->value_type() != model::ValueVariant::Children )
  {
    LOG_WARN << "Error listing path " << path;
    std::cout << "Error listing path " << path << '\n';
    return;
  }

  auto value = resp->value()->Get( 0 )->value_as<model::Children>();
  if ( value->value()->size() == 0 )
  {
    LOG_WARN << "Error listing path " << path;
    std::cout << "Error listing path " << path << '\n';
    return;
  }

  LOG_INFO << "Retrieved children for path " << path;
  for ( auto&& v : *value->value() )
  {
    std::cout << v->string_view() << '\n';
  }
}

void spt::configdb::client::set( std::string_view server,
    std::string_view port, std::string_view key, std::string_view value, bool ssl )
{
  std::unique_ptr<api::impl::BaseConnection> connection;
  if ( ssl ) connection = std::make_unique<api::impl::SSLConnection>( server, port );
  else connection = std::make_unique<api::impl::Connection>( server, port );

  auto response = connection->set( model::RequestData{ key, value } );
  if ( !response )
  {
    LOG_WARN << "Error setting key " << key;
    std::cout << "Error setting key " << key << '\n';
    return;
  }

  if ( response->value_type() != model::ResultVariant::Success )
  {
    LOG_WARN << "Error setting key " << key;
    std::cout << "Error setting key " << key << '\n';
    return;
  }

  auto resp = response->value_as<model::Success>();
  if ( resp->value() )
  {
    LOG_INFO << "Set value for key " << key;
    std::cout << "Set value for key " << key << '\n';
    return;
  }

  LOG_WARN << "Error setting key " << key;
  std::cout << "Error setting key " << key << '\n';
}

void spt::configdb::client::move( std::string_view server,
    std::string_view port, std::string_view key, std::string_view dest, bool ssl )
{
  std::unique_ptr<api::impl::BaseConnection> connection;
  if ( ssl ) connection = std::make_unique<api::impl::SSLConnection>( server, port );
  else connection = std::make_unique<api::impl::Connection>( server, port );

  auto response = connection->move( model::RequestData{ key, dest } );
  if ( !response )
  {
    LOG_WARN << "Error moving key " << key << " to " << dest;
    std::cout << "Error moving key " << key << " to " << dest << '\n';
    return;
  }

  if ( response->value_type() != model::ResultVariant::Success )
  {
    LOG_WARN << "Error moving key " << key << " to " << dest;
    std::cout << "Error moving key " << key << " to " << dest << '\n';
    return;
  }

  auto resp = response->value_as<model::Success>();
  if ( resp->value() )
  {
    LOG_INFO << "Moved key " << key << " to " << dest;
    std::cout << "Moved key " << key << " to " << dest << '\n';
    return;
  }

  LOG_WARN << "Error moving key " << key << " to " << dest;
  std::cout << "Error moving key " << key << " to " << dest << '\n';
}

void spt::configdb::client::remove( std::string_view server,
    std::string_view port, std::string_view key, bool ssl )
{
  std::unique_ptr<api::impl::BaseConnection> connection;
  if ( ssl ) connection = std::make_unique<api::impl::SSLConnection>( server, port );
  else connection = std::make_unique<api::impl::Connection>( server, port );

  auto response = connection->remove( key );
  if ( !response )
  {
    LOG_WARN << "Error removing key " << key;
    std::cout << "Error removing key " << key << '\n';
    return;
  }

  if ( response->value_type() != model::ResultVariant::Success )
  {
    LOG_WARN << "Error removing key " << key;
    std::cout << "Error removing key " << key << '\n';
    return;
  }

  auto resp = response->value_as<model::Success>();
  if ( !resp->value() )
  {
    LOG_WARN << "Error removing key " << key;
    std::cout << "Error removing key " << key << '\n';
  }
  else
  {
    std::cout << "Removed key " << key << '\n';
  }
}

void spt::configdb::client::import( std::string_view server, std::string_view port, const std::string& file, bool ssl )
{
  std::unique_ptr<api::impl::BaseConnection> connection;
  if ( ssl ) connection = std::make_unique<api::impl::SSLConnection>( server, port );
  else connection = std::make_unique<api::impl::Connection>( server, port );

  const auto& [response, count, total] = connection->import( file );
  if ( !response )
  {
    LOG_WARN << "Error importing (" << int(count) << '/' << total << ") keys from file " << file;
    std::cout << "Error importing (" << count << '/' << total << ") keys from file " << file << '\n';
    return;
  }

  if ( response->value_type() != model::ResultVariant::Success )
  {
    LOG_WARN << "Error importing (" << int(count) << '/' << total << ") keys from file " << file;
    std::cout << "Error importing (" << count << '/' << total << ") keys from file " << file << '\n';
    return;
  }

  if ( const auto resp = response->value_as<model::Success>(); resp->value() )
  {
    LOG_WARN << "Imported (" << int(count) << '/' << total << ") keys from file " << file;
    std::cout << "Imported (" << count << '/' << total << ") keys from file " << file << '\n';
    return;
  }

  LOG_WARN << "Error importing (" << int(count) << '/' << total << ") keys from file " << file;
  std::cout << "Error importing (" << count << '/' << total << ") keys from file " << file << '\n';
}
