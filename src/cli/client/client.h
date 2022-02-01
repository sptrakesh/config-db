//
// Created by Rakesh on 30/12/2021.
//

#pragma once

#include <string>
#include <string_view>

namespace spt::configdb::client
{
  int get( std::string_view server, std::string_view port, std::string_view key, bool ssl );
  int list( std::string_view server, std::string_view port, std::string_view path, bool ssl );
  int set( std::string_view server, std::string_view port, std::string_view key, std::string_view value, bool ssl );
  int move( std::string_view server, std::string_view port, std::string_view key, std::string_view destination, bool ssl );
  int remove( std::string_view server, std::string_view port, std::string_view key, bool ssl );
  int import( std::string_view server, std::string_view port, const std::string& file, bool ssl );
}