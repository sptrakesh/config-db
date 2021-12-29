//
// Created by Rakesh on 25/12/2021.
//

#pragma once

#include <string>

namespace spt::configdb::http
{
  int start( const std::string& port, int threads );
}