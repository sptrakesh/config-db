//
// Created by Rakesh on 02/01/2022.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace spt::configdb::api
{
  /**
   * Invoke once before using API.  Initialises connection pool to the service.
   *
   * @param server The hostname of the server to connect to.
   * @param port The TCP port to connect to.
   */
  void init( std::string_view server, std::string_view port );

  /**
   * Retrieve the stored value for the specified `key`.
   *
   * @param key The key to look up in the database.
   * @return The value if found or `std::nullopt`.
   */
  std::optional<std::string> get( std::string_view key );

  /**
   * Set the value for the specified `key`.  Creates or updates in the database.
   *
   * Adds child nodes to the hierarchy as appropriate.
   *
   * @param key  The key to create/update.
   * @param value The value to set
   * @return `true` if transaction succeeded.
   */
  bool set( std::string_view key, std::string_view value );

  /**
   * Remove the specified `key` from the database.
   *
   * Removes the child from parent hierarchy.
   *
   * @param key The key to remove.
   * @return `true` if transaction succeeds.
   */
  bool remove( std::string_view key );

  using Nodes = std::optional<std::vector<std::string>>;
  /**
   * Retrieve child node names for the specified `path`.
   *
   * @param path The `path` to look up in the hierarchy.
   * @return The child node names, or `std::nullopt` if no children.
   */
  Nodes list( std::string_view path );

  using KeyValue = std::pair<std::string, std::optional<std::string>>;
  /**
   * Batch retrieve values for keys from the database.
   *
   * Returns empty vector if connection to service returns error.
   *
   * **Note:** The returned vector maybe in different order if caching is
   * enabled. If some of the input keys are in the cache, and others not, the
   * cached key-values are added first to the response, and then the remaining
   * key-values retrieved from the database.
   *
   * @param keys The keys to retrieve.
   * @return Vector of *key-value* pairs.
   */
  std::vector<KeyValue> mget( const std::vector<std::string_view>& keys );

  using Pair = std::pair<std::string_view, std::string_view>;
  /**
   * Set a batch of *key-value* pairs in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param kvs The batch of *key-value* pairs to set.
   * @return `true` if transaction succeeds.
   */
  bool mset( const std::vector<Pair>& kvs );

  /**
   * Remove a batch of `keys` from the database in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param keys The batch of `keys` to remove.
   * @return `true` if the transaction succeeds.
   */
  bool mremove( const std::vector<std::string_view>& keys );

  using NodePair = std::tuple<std::string, std::optional<std::vector<std::string>>>;
  /**
   * Retrieve child node names for the batch of `paths`.
   *
   * @param keys The batch of paths to retrieve children for.
   * @return The child node names for each `path`.  If a `path` has no children, returns `std::nullopt` for it.
   */
  std::vector<NodePair> mlist( const std::vector<std::string_view>& paths );
}