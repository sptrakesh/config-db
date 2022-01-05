//
// Created by Rakesh on 30/12/2021.
//

#pragma once

#include <string_view>

namespace spt::configdb::client
{
  void get( std::string_view server, std::string_view port, std::string_view key );
  void list( std::string_view server, std::string_view port, std::string_view path );
  void set( std::string_view server, std::string_view port, std::string_view key, std::string_view value );
  void move( std::string_view server, std::string_view port, std::string_view key, std::string_view destination );
  void remove( std::string_view server, std::string_view port, std::string_view key );
  void import( std::string_view server, std::string_view port, std::string_view file );
}