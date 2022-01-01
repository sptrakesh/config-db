//
// Created by Rakesh on 24/12/2021.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace spt::configdb::db
{
  void init();

  // CRUD
  std::optional<std::string> get( std::string_view key );
  bool set( std::string_view key, std::string_view value );
  bool remove( std::string_view key );

  // Hierarchy
  using Nodes = std::optional<std::vector<std::string>>;
  Nodes list( std::string_view key );

  // Bulk
  using KeyValue = std::pair<std::string, std::optional<std::string>>;
  std::vector<KeyValue> mget( const std::vector<std::string_view>& keys );

  using Pair = std::pair<std::string_view, std::string_view>;
  bool mset( const std::vector<Pair>& kvs );

  bool mremove( const std::vector<std::string_view>& keys );

  using NodePair = std::tuple<std::string, std::optional<std::vector<std::string>>>;
  std::vector<NodePair> mlist( const std::vector<std::string_view>& keys );

  // For testing only
  void reopen();
}