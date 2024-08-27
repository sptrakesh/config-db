//
// Created by Rakesh on 29/12/2021.
//

#pragma once

#include <string_view>

namespace spt::configdb::client
{
  int run( std::string_view server, std::string_view port, bool ssl );
}