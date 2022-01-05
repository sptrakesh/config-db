//
// Created by Rakesh on 05/01/2022.
//

#include <catch2/catch.hpp>
#include "../../src/lib/db/crud.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::db;

SCENARIO( "Persistence test", "persistence" )
{
  GIVEN( "A valid key" )
  {
    auto key = "key"sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto status = set( key, "value"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Reopening the database" )
    {
      reopen();
    }

    AND_WHEN( "Reading the key" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value"sv );
    }

    AND_WHEN( "Reopening the database" )
    {
      reopen();
    }

    AND_WHEN( "Updating the value" )
    {
      const auto status = set( key, "value modified"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Reopening the database" )
    {
      reopen();
    }

    AND_WHEN( "Reading the key again" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value modified"sv );
    }

    AND_WHEN( "Reopening the database" )
    {
      reopen();
    }

    AND_WHEN( "Removing the key" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }

    AND_WHEN( "Reopening the database" )
    {
      reopen();
    }

    AND_WHEN( "Reading the key after remove" )
    {
      const auto value = get( key );
      REQUIRE_FALSE( value );
    }
  }
}
