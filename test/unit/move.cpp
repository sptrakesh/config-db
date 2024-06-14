//
// Created by Rakesh on 05/01/2022.
//

#include <catch2/catch_test_macros.hpp>
#include "../../src/lib/db/storage.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::db;
using spt::configdb::model::RequestData;

SCENARIO( "Move test", "move" )
{
  GIVEN( "A key that is to be moved" )
  {
    const auto key = "/key1/key2"sv;
    const auto dest = "/key2/key3"sv;

    WHEN( "Creating the key" )
    {
      const auto status = set( RequestData{ key, "value"sv } );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key" )
    {
      const auto resp = get( key );
      REQUIRE( resp );
      REQUIRE( *resp == "value"s );
    }

    AND_WHEN( "Listing the parent path" )
    {
      const auto resp = list( "/key1"sv );
      REQUIRE( resp );
      REQUIRE_FALSE( resp->empty() );
      REQUIRE( resp->size() == 1 );
      REQUIRE( resp->at( 0 ) == "key2"sv );
    }

    AND_WHEN( "Moving the key to dest" )
    {
      const auto status = move( RequestData{ key, dest } );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key" )
    {
      const auto resp = get( key );
      REQUIRE_FALSE( resp );
    }

    AND_WHEN( "Reading the destination key" )
    {
      const auto resp = get( dest );
      REQUIRE( resp );
      REQUIRE( *resp == "value"s );
    }

    AND_WHEN( "Listing the old parent path" )
    {
      const auto resp = list( "/key1"sv );
      REQUIRE_FALSE( resp );
    }

    AND_WHEN( "Listing the new parent path" )
    {
      const auto resp = list( "/key2"sv );
      REQUIRE( resp );
      REQUIRE_FALSE( resp->empty() );
      REQUIRE( resp->size() == 1 );
      REQUIRE( resp->at( 0 ) == "key3"sv );
    }

    AND_WHEN( "Removing the key" )
    {
      CHECK( remove( key ) );
      CHECK( remove( dest ) );
    }
  }
}

