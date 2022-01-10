//
// Created by Rakesh on 24/12/2021.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "../../common/model/request.h"

namespace spt::configdb::db
{
  void init();

  // CRUD
  std::optional<std::string> get( std::string_view key );
  bool set( const model::RequestData& data );
  bool remove( std::string_view key );

  bool move( const model::RequestData& data );

  // Hierarchy
  using Nodes = std::optional<std::vector<std::string>>;
  Nodes list( std::string_view key );

  // Bulk
  using KeyValue = std::pair<std::string, std::optional<std::string>>;
  std::vector<KeyValue> get( const std::vector<std::string_view>& keys );

  bool set( const std::vector<model::RequestData>& kvs );

  bool remove( const std::vector<std::string_view>& keys );
  bool move( const std::vector<model::RequestData>& kvs );

  using NodePair = std::tuple<std::string, std::optional<std::vector<std::string>>>;
  std::vector<NodePair> list( const std::vector<std::string_view>& keys );

  // For testing only
  void reopen();
}