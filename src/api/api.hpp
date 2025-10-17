//
// Created by Rakesh on 02/01/2022.
//

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "../common/contextholder.hpp"
#include "../common/model/request.hpp"

namespace spt::configdb::api
{
  /**
   * Invoke once before using API.  Initialises connection pool to the service.
   *
   * @param server The hostname of the server to connect to.
   * @param port The TCP port to connect to.
   * @param ssl Specify `true` to use SSL connection to service.
   */
  void init( std::string_view server, std::string_view port, bool ssl,
      boost::asio::io_context& ioc = ContextHolder::instance().ioc );

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
   * Set the value for the specified `key`.  Creates or updates in the database.
   *
   * If the `options.ifNotExists` is set to `true`, will only create if `key`
   * does not exist.  Update will fail.
   *
   * @param data The request data.
   * @return `true` if the transaction succeeded.
   */
  bool set( const model::RequestData& data );

  /**
   * Remove the specified `key` from the database.
   *
   * Removes the child from parent hierarchy.
   *
   * @param key The key to remove.
   * @return `true` if transaction succeeds.
   */
  bool remove( std::string_view key );

  /**
   * Move the specified `key` to the `dest` in a transaction.
   *
   * Internally a combination of `get-remove-set`.  Transaction is rolled back
   * if any of the operations fail.  Inherent operations such as hierarchy
   * management also operate as with their direct counterparts.
   *
   * **Note:** If the source `key` had an expiration policy, it will be applied
   * to the destination.
   *
   * @param key The key that is to be moved (path change).
   * @param dest The destination path for the value.
   * @return `true` if transaction succeeds.
   */
  bool move( std::string_view key, std::string_view dest );

  /**
   * Move the specified `key` to the `dest` in a transaction.
   *
   * If the `options.ifNotExists` is set to `true`, will only succeed if the
   * destination path specified by the `value` does not exist.
   *
   * **Note:** If the source `key` had an expiration policy, it will be applied
   * to the destination unless a non-zero expiration policy is specified.
   *
   * @param data The request data with the `key` and `value` which represents the destination`key`.
   * @return `true` if transaction succeeds.
   */
  bool move( const model::RequestData& data );

  using Nodes = std::optional<std::vector<std::string>>;
  /**
   * Retrieve child node names for the specified `path`.
   *
   * @param path The `path` to look up in the hierarchy.
   * @return The child node names, or `std::nullopt` if no children.
   */
  Nodes list( std::string_view path );

  /**
   * Retrieve the TTL value for the specified key.  The returned value is relative
   * to the *current time*, and not the original `expirationInSeconds` value
   * specified during the {@see set} operation.
   *
   * @param key The key for which the TTL value is to be retrieved.
   * @return The seconds till expiration from now, or 0 if no TTL was set.
   */
  std::chrono::seconds ttl( std::string_view key );

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
  std::vector<KeyValue> get( const std::vector<std::string_view>& keys );

  /**
   * Batch retrieve values for the specified keys.
   *
   * Delegates to the function that takes a vector of `std::string_view`.
   *
   * @param keys The keys to retrieve
   * @return  Vector of *key-value* pairs.
   */
  std::vector<KeyValue> get( const std::vector<std::string>& keys );

  using Pair = std::pair<std::string_view, std::string_view>;
  /**
   * Set a batch of *key-value* pairs in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param kvs The batch of *key-value* pairs to set.
   * @return `true` if transaction succeeds.
   */
  bool set( const std::vector<Pair>& kvs );

  /**
   * Set a batch of *key-value* pairs in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param kvs
   * @return
   */
  bool set( const std::vector<model::RequestData>& kvs );

  /**
   * Remove a batch of `keys` from the database in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param keys The batch of `keys` to remove.
   * @return `true` if the transaction succeeds.
   */
  bool remove( const std::vector<std::string_view>& keys );

  /**
   * Move the specified batch of keys in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param kvs The batch of *key-dest* pairs to move.
   * @return `true` if the transaction succeeds.
   */
  bool move( const std::vector<Pair>& kvs );

  /**
   * Move the specified batch of keys in a single transaction.
   *
   * If any operation in the batch fails, the entire transaction is *rolled back*.
   *
   * @param kvs
   * @return
   */
  bool move( const std::vector<model::RequestData>& kvs );

  using NodePair = std::tuple<std::string, std::optional<std::vector<std::string>>>;
  /**
   * Retrieve child node names for the batch of `paths`.
   *
   * @param keys The batch of paths to retrieve children for.
   * @return The child node names for each `path`.  If a `path` has no children, returns `std::nullopt` for it.
   */
  std::vector<NodePair> list( const std::vector<std::string_view>& paths );

  using TTLPair = std::pair<std::string, std::chrono::seconds>;
  /**
   * Retrieve the TTL values from current time for the specified keys.
   *
   * @param keys The batch of keys to retrieve TTL values for.
   * @return Pairs of `key-ttl` values.  Returns 0 for keys that does not have TTL set.
   */
  std::vector<TTLPair> ttl( const std::vector<std::string_view>& keys );

  /**
   * Retrieve the TTL values from current time for the specified keys.
   *
   * Delegates to the function that takes a vector of `std::string_view`.
   *
   * @param keys The batch of keys to retrieve TTL values for.
   * @return Pairs of `key-ttl` values.  Returns 0 for keys that does not have TTL set.
   */
  std::vector<TTLPair> ttl( const std::vector<std::string>& keys );

  /**
   * Tuple holding the response along with the number of records imported out
   * of total number of lines in the input file.  The number of records imported
   * may be lower than the total if lines were omitted due to not having a space
   * separated *key* and *value*.
   */
  using ImportResponse = std::tuple<bool, std::size_t, uint32_t>;
  /**
   * Bulk import *key-value* pairs from a space separated file.  The first space
   * in a line is taken as the delimiter between a *key* and its associated *value*.
   * Rest of the line is read as the *value*.
   *
   * **Note:** The entire file is imported in a *single* transaction.  This
   * imposes memory related constraints on the size of file that can be imported.
   *
   * @param file The space separated plain text file to bulk import.
   * @return `true` if the transaction succeeds.
   */
  ImportResponse import( const std::string& file );
}