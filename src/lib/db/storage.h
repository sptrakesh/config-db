//
// Created by Rakesh on 24/12/2021.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace spt::configdb::db
{
  void init();

  // CRUD
  std::optional<std::string> get( std::string_view key );
  bool set( std::string_view key, std::string_view value );
  bool remove( std::string_view key );

  // Hierarchy
  std::optional<std::vector<std::string>> list( std::string_view key );

  // For testing only
  void reopen();
}