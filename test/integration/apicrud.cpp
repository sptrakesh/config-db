//
// Created by Rakesh on 04/01/2022.
//

#include <catch2/catch.hpp>

#include "../../src/api/api.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::api;

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
      auto opts = spt::configdb::model::RequestData::Options{};
      opts.ifNotExists = true;
      auto data = spt::configdb::model::RequestData{ key, "value"sv, opts };
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
  }
}
