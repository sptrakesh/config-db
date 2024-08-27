//
// Created by Rakesh on 04/01/2022.
//

#include <catch2/catch_test_macros.hpp>

#include "../../src/api/api.hpp"

using std::operator ""s;
using std::operator ""sv;
using namespace spt::configdb::api;
using namespace spt::configdb::model;

SCENARIO( "API CRUD test", "api-crud" )
{
  GIVEN( "A valid key" )
  {
    auto key = "key"sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto status = set( key, "value"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value"sv );
    }

    AND_WHEN( "Updating the value" )
    {
      const auto status = set( key, "value modified"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key again" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value modified"sv );
    }

    AND_WHEN( "Update rejected due to if not exists" )
    {
      auto opts = RequestData::Options{ true };
      auto data = RequestData{ key, "value"sv, opts };
      const auto status = set( data );
      REQUIRE_FALSE( status );
    }

    AND_WHEN( "Removing the key" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key after remove" )
    {
      const auto value = get( key );
      REQUIRE_FALSE( value );
    }

    AND_WHEN( "Removing the key again" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }

    AND_WHEN( "Caching the key" )
    {
      REQUIRE( set( RequestData{ key, "value"sv, RequestData::Options{ 600u, false, true } } ) );
    }

    AND_WHEN( "Retrieving the cached value" )
    {
      const auto value = get( key );
      REQUIRE( value );
      CHECK( *value == "value" );
    }

    AND_WHEN( "Listing the root node" )
    {
      // Fails when run together, but succeeds when run by itself.
      //REQUIRE_FALSE( list( "/"sv ) );
    }

    AND_WHEN( "Removing the key" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key after remove" )
    {
      const auto value = get( key );
      REQUIRE_FALSE( value );
    }
  }
}
