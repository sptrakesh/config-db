//
// Created by Rakesh on 05/01/2022.
//

#include <catch2/catch.hpp>

#include "../../src/api/api.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::api;

SCENARIO( "API Move test", "api-move" )
{
  GIVEN( "A set of keys" )
  {
    const auto key1 = "/key1/key2/key3"s;
    const auto key2 = "/key1/key2/key4"s;
    const auto key3 = "/key1/key21/key3"s;
    const auto key4 = "/key1/key21/key4"s;

    const auto dest1 = "/key/key2/key3"s;
    const auto dest2 = "/key/key2/key4"s;
    const auto dest3 = "/key/key21/key3"s;
    const auto dest4 = "/key/key21/key4"s;

    WHEN( "Creating keys" )
    {
      const auto kvs = std::vector<Pair>{
          { key1, "value"sv },
          { key2, "value"sv },
          { key3, "value"sv },
          { key4, "value"sv } };
      const auto status = set( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Moving the keys" )
    {
      const auto kvs = std::vector<Pair>{
          { key1, dest1 },
          { key2, dest2 },
          { key3, dest3 },
          { key4, dest4 } };
      const auto status = move( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving the moved keys" )
    {
      const auto keys = std::vector<std::string_view>{ key1, key2, key3, key4 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 4; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE_FALSE( results[i].second );
      }
    }

    AND_WHEN( "Retrieving the destination keys" )
    {
      const auto keys = std::vector<std::string_view>{ dest1, dest2, dest3, dest4 };
      const auto results = get( keys );
      REQUIRE_FALSE( results.empty() );
      REQUIRE( results.size() == keys.size() );
      for ( auto i = 0; i < 4; ++i )
      {
        REQUIRE( results[i].first == keys[i] );
        REQUIRE( results[i].second );
        REQUIRE( *results[i].second == "value"s );
      }
    }

    AND_WHEN( "Listing multiple paths" )
    {
      const auto paths = std::vector<std::string_view>{ "/"sv, "/key"sv, "/key/key2"sv, "/key/key21"sv };
      const auto results = list( paths );
      REQUIRE_FALSE( results.empty());
      REQUIRE( results.size() == paths.size() );

      REQUIRE( std::get<0>( results[0] ) == paths[0] );
      auto vec = std::get<1>( results[0] );
      REQUIRE( vec );
      REQUIRE( vec->size() == 1 );
      REQUIRE( vec->at( 0 ) == "key"s );

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
      const auto keys = std::vector<std::string_view>{ dest1, dest2, dest3, dest4 };
      const auto results = remove( keys );
      REQUIRE( results );
    }
  }
}

