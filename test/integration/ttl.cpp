//
// Created by Rakesh on 11/01/2022.
//

#include <catch2/catch.hpp>
#include <iostream>
#include <thread>

#include "../../src/api/api.h"

using namespace spt::configdb::api;
using namespace spt::configdb::model;
using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "TTL test", "[ttl]" )
{
  GIVEN( "Given a batch of keys" )
  {
    const auto key1 = "/akey1"sv;
    const auto key2 = "/akey2"sv;

    const auto dest1 = "/bkey1"sv;
    const auto dest2 = "/bkey2"sv;

    WHEN( "Creating keys" )
    {
      const auto kvs = std::vector<Pair>{ { key1, "value"sv }, { key2, "value"sv } };
      const auto status = set( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving the keys" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 2; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second );
        REQUIRE( *results[i].second == "value"s );
      }
    }

    AND_WHEN( "Retrieving non-existent TTL" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2 };
      const auto results = ttl( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 2; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second.count() == 0 );
      }
    }

    AND_WHEN( "Setting a TTL for the keys" )
    {
      auto opts = RequestData::Options{};
      opts.expirationInSeconds = 2;
      auto kvs = std::vector<RequestData>{};
      kvs.reserve( 2 );
      kvs.emplace_back( key1, "value"sv, opts );
      kvs.emplace_back( key2, "value"sv, opts );
      const auto status = set( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving the TTL values" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2 };
      const auto results = ttl( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 2; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second.count() > 0 );
      }
    }

    AND_WHEN( "Moving the keys" )
    {
      auto opts = RequestData::Options{};
      opts.expirationInSeconds = 4;
      auto kvs = std::vector<RequestData>{};
      kvs.reserve( 2 );
      kvs.emplace_back( key1, dest1, opts );
      kvs.emplace_back( key2, dest2, opts );
      const auto status = spt::configdb::api::move( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving the TTL for destinations" )
    {
      const auto keys = std::vector<std::string_view>{ dest1, dest2 };
      const auto results = ttl( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 2; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second.count() > 2 );
      }
    }

    AND_WHEN( "Reading the auto deleted keys" )
    {
      std::cout << "Sleeping 5s to expire key\n";
      std::this_thread::sleep_for( std::chrono::seconds{ 5 } );
      const auto keys = std::vector<std::string_view>{ dest1, dest2 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 2; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE_FALSE( results[i].second );
      }
    }

    AND_WHEN( "Retrieving the TTL for auto deleted destination" )
    {
      const auto keys = std::vector<std::string_view>{ dest1, dest2 };
      const auto results = ttl( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 2; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second.count() == 0 );
      }
    }
  }
}

