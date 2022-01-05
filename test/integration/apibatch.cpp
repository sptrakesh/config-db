//
// Created by Rakesh on 05/01/2022.
//

#include <catch2/catch.hpp>

#include "../../src/api/api.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::api;

SCENARIO( "API Batch test", "api-batch" )
{
  GIVEN( "A set of keys" )
  {
    const auto key1 = "/key1/key2/key3"s;
    const auto key2 = "/key1/key2/key4"s;
    const auto key3 = "/key1/key21/key3"s;
    const auto key4 = "/key1/key21/key4"s;

    WHEN( "Creating keys" )
    {
      const auto kvs = std::vector<Pair>{
          { key1, "value"sv }, { key2, "value"sv },
          { key3, "value"sv }, { key4, "value"sv } };
      const auto status = set( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving the keys" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2, key3, key4 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty());
      REQUIRE( results.size() == keys.size());
      for ( auto i = 0; i < 4; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second );
        REQUIRE( *results[i].second == "value"s );
      }
    }

    AND_WHEN( "Updating the keys" )
    {
      const auto kvs = std::vector<Pair>{
          { key1, "value modified"sv }, { key2, "value modified"sv },
          { key3, "value modified"sv }, { key4, "value modified"sv } };
      const auto status = set( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving the modified keys" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2, key3, key4 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty());
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 4; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second );
        REQUIRE( *results[i].second == "value modified"s );
      }
    }

    AND_WHEN( "Listing multiple paths" )
    {
      const auto paths = std::vector<std::string_view>{ "/"sv, "/key1"sv, "/key1/key2"sv, "/key1/key21"sv };
      const auto results = list( paths );
      REQUIRE_FALSE( results.empty());
      REQUIRE( results.size() == paths.size() );

      REQUIRE( std::get<0>( results[0] ) == paths[0] );
      auto vec = std::get<1>( results[0] );
      REQUIRE( vec );
      REQUIRE( vec->size() == 1 );
      REQUIRE( vec->at( 0 ) == "key1"s );

      REQUIRE( std::get<0>( results[1] ) == paths[1] );
      vec = std::get<1>( results[1] );
      REQUIRE( vec );
      REQUIRE( vec->size() == 2 );
      REQUIRE( vec->at( 0 ) == "key2"s );
      REQUIRE( vec->at( 1 ) == "key21"s );

      REQUIRE( std::get<0>( results[2] ) == paths[2] );
      vec = std::get<1>( results[2] );
      REQUIRE( vec );
      REQUIRE( vec->size() == 2 );
      REQUIRE( vec->at( 0 ) == "key3"s );
      REQUIRE( vec->at( 1 ) == "key4"s );

      REQUIRE( std::get<0>( results[3] ) == paths[3] );
      vec = std::get<1>( results[3] );
      REQUIRE( vec );
      REQUIRE( vec->size() == 2 );
      REQUIRE( vec->at( 0 ) == "key3"s );
      REQUIRE( vec->at( 1 ) == "key4"s );
    }

    AND_WHEN( "Removing the keys" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2, key3, key4 };
      const auto results = remove( keys );
      REQUIRE( results );
    }

    AND_WHEN( "Retrieving the deleted keys" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2, key3, key4 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty());
      REQUIRE( results.size() == keys.size());
      for ( auto i = 0; i < 4; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE_FALSE( results[i].second );
      }
    }

    AND_WHEN( "Listing deleted multiple paths" )
    {
      const auto paths = std::vector<std::string_view>{ "/"sv, "/key1"sv, "/key1/key2"sv, "/key1/key21"sv };
      const auto results = list( paths );
      REQUIRE_FALSE( results.empty());
      REQUIRE( results.size() == paths.size() );

      REQUIRE( std::get<0>( results[0] ) == paths[0] );
      REQUIRE_FALSE( std::get<1>( results[0] ) );

      REQUIRE( std::get<0>( results[1] ) == paths[1] );
      REQUIRE_FALSE( std::get<1>( results[1] ) );

      REQUIRE( std::get<0>( results[2] ) == paths[2] );
      REQUIRE_FALSE( std::get<1>( results[2] ) );

      REQUIRE( std::get<0>( results[3] ) == paths[3] );
      REQUIRE_FALSE( std::get<1>( results[3] ) );
    }
  }
}

