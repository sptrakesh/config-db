//
// Created by Rakesh on 11/01/2022.
//

#include <catch2/catch.hpp>
#include "../../src/lib/db/storage.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::db;
using spt::configdb::model::RequestData;

SCENARIO( "TTL test", "ttl" )
{
  GIVEN( "A valid key" )
  {
    auto key = "key"sv;
    auto dest = "key1"sv;

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

    AND_WHEN( "Getting non-existent TTL for key" )
    {
      const auto exp = ttl( key );
      REQUIRE( exp.count() == 0 );
    }

    AND_WHEN( "Setting TTL for key" )
    {
      const auto status = set( RequestData{ key, "value"sv, RequestData::Options{ 60u } } );
      REQUIRE( status );
    }

    AND_WHEN( "Getting TTL for key" )
    {
      const auto exp = ttl( key );
      REQUIRE( exp.count() > 0 );
    }

    AND_WHEN( "Moving key to dest" )
    {
      const auto status = move( RequestData{ key, dest } );
      REQUIRE( status );
    }

    AND_WHEN( "Getting TTL for dest" )
    {
      const auto exp = ttl( dest );
      REQUIRE( exp.count() > 0 );
    }

    AND_WHEN( "Removing the dest" )
    {
      const auto status = remove( dest );
      REQUIRE( status );
    }
  }
}
