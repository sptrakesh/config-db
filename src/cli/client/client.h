//
// Created by Rakesh on 30/12/2021.
//

#pragma once

#include <string>
#include <string_view>

namespace spt::configdb::client
{
  void get( std::string_view server, std::string_view port, std::string_view key, bool ssl );
  void list( std::string_view server, std::string_view port, std::string_view path, bool ssl );
  void set( std::string_view server, std::string_view port, std::string_view key, std::string_view value, bool ssl );
  void move( std::string_view server, std::string_view port, std::string_view key, std::string_view destination, bool ssl );
  void remove( std::string_view server, std::string_view port, std::string_view key, bool ssl );
  void import( std::string_view server, std::string_view port, const std::string& file, bool ssl );
}