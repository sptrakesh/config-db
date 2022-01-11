//
// Created by Rakesh on 24/12/2021.
//

#include <catch2/catch.hpp>
#include "../../src/lib/db/storage.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::db;
using spt::configdb::model::RequestData;

SCENARIO( "CRUD test", "crud" )
{
  GIVEN( "A valid key" )
  {
    auto key = "key"sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto status = set( RequestData{ key, "value"sv } );
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
      const auto status = set( RequestData{ key, "value modified"sv } );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key again" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value modified"sv );
    }

    AND_WHEN( "Updating rejected with if not exists" )
    {
      auto opts = RequestData::Options{};
      opts.ifNotExists = true;
      const auto status = set( RequestData{ key, "value"sv, opts } );
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
  }

  GIVEN( "An empty key" )
  {
    auto key = ""sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto status = set( RequestData{ key, "value"sv } );
      REQUIRE_FALSE( status );
    }

    AND_WHEN( "Reading the key" )
    {
      const auto value = get( key );
      REQUIRE_FALSE( value );
    }

    AND_WHEN( "Removing the key" )
    {
      const auto status = remove( key );
      REQUIRE_FALSE( status );
    }
  }
}