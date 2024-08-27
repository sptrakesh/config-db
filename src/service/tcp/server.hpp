//
// Created by Rakesh on 25/12/2021.
//

#pragma once

#include <string_view>

namespace spt::configdb::tcp
{
  int start( int port, bool ssl );
  int notifier( int port, bool ssl );
  int listen( std::string_view host, std::string_view port, bool ssl );
}