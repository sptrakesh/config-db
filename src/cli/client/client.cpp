//
// Created by Rakesh on 30/12/2021.
//

#include "client.h"
#include "connection.h"
#include "../lib/log/NanoLog.h"

#include <iostream>

void spt::configdb::client::get( std::string_view server,
    std::string_view port, std::string_view key )
{
  auto connection = Connection{ server, port };
  auto response = connection.get( key );
  if ( !response )
  {
    LOG_WARN << "Error retrieving key " << key;
    std::cout << "Error retrieving key " << key << '\n';
    return;
  }

  auto resp = response->value_as<model::Value>();
  LOG_INFO << "Retrieved value for key " << key;
  std::cout << resp->value()->string_view() << '\n';
}

void spt::configdb::client::set( std::string_view server,
    std::string_view port, std::string_view key, std::string_view value )
{
  auto connection = Connection{ server, port };
  auto response = connection.set( key, value );
  if ( !response )
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
  }
}

void spt::configdb::client::list( std::string_view server,
    std::string_view port, std::string_view path )
{
  auto connection = Connection{ server, port };
  auto response = connection.list( path );
  if ( !response )
  {
    LOG_WARN << "Error list path " << path;
    std::cout << "Error listing path " << path << '\n';
    return;
  }

  if ( response->value_type() != model::ResponseValue::Children )
  {
    std::cout << "Error retrieving path " << path << '\n';
    return;
  }

  auto resp = response->value_as<model::Children>();
  for ( auto&& v : *resp->value() )
  {
    std::cout << v->string_view() << '\n';
  }
}

void spt::configdb::client::remove( std::string_view server,
    std::string_view port, std::string_view key )
{
  auto connection = Connection{ server, port };
  auto response = connection.remove( key );
  if ( !response )
  {
    LOG_WARN << "Error removing key " << key;
    std::cout << "Error removing key " << key << '\n';
    return;
  }

  if ( response->value_type() != model::ResponseValue::Success )
  {
    std::cout << "Error removing key " << key << '\n';
    return;
  }

  auto resp = response->value_as<model::Success>();
  if ( !resp->value())
  {
    std::cout << "Error removing key " << key << '\n';
  }
  else
  {
    std::cout << "Removed key " << key << '\n';
  }
}