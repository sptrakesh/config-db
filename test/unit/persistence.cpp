//
// Created by Rakesh on 05/01/2022.
//

#include <catch2/catch_test_macros.hpp>
#include "../../src/lib/db/storage.h"

using namespace spt::configdb::db;
using spt::configdb::model::RequestData;
using std::operator ""s;
using std::operator ""sv;

SCENARIO( "Persistence test", "persistence" )
{
  GIVEN( "A valid key" )
  {
    auto key = "key"sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto data = RequestData{ key, "value"sv };
      const auto status = set( data );
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
      const auto status = set( RequestData{ key, "value modified"sv } );
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
