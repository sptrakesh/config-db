//
// Created by Rakesh on 11/01/2022.
//

#include <iostream>
#include <thread>
#include <catch2/catch_test_macros.hpp>
#include "../../src/lib/db/storage.hpp"

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
      REQUIRE( *value == "value" );
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

    AND_WHEN( "Reading the key after 5 seconds" )
    {
      std::cout << "Sleeping 5s to ensure we can read TTL key\n";
      std::this_thread::sleep_for( std::chrono::seconds{ 5 } );

      const auto value = get( dest );
      REQUIRE( value );
      REQUIRE( *value == "value" );
    }

    AND_WHEN( "Removing the dest" )
    {
      const auto status = remove( dest );
      REQUIRE( status );
    }

    AND_WHEN( "Listing root node" )
    {
      CHECK_FALSE( list( "/"sv ) );
    }
  }

  GIVEN( "A cache key" )
  {
    auto key = "key"sv;
    auto dest = "key1"sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto status = set( RequestData{ key, "value"sv, RequestData::Options{ 60u, false, true } } );
      REQUIRE( status );
    }

    AND_WHEN( "Reading the key" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value" );
    }

    AND_WHEN( "Listing root node" )
    {
      CHECK_FALSE( list( "/"sv ) );
    }

    AND_WHEN( "Getting TTL for key" )
    {
      const auto exp = ttl( key );
      REQUIRE( exp.count() > 0 );
    }

    AND_WHEN( "Moving key to dest" )
    {
      const auto status = move( RequestData{ key, dest, RequestData::Options{ 60u, false, true } } );
      REQUIRE( status );
    }

    AND_WHEN( "Getting TTL for dest" )
    {
      const auto exp = ttl( dest );
      REQUIRE( exp.count() > 0 );
    }

    AND_WHEN( "Listing root node" )
    {
      CHECK_FALSE( list( "/"sv ) );
    }

    AND_WHEN( "Removing the dest" )
    {
      CHECK( remove( key ) );
      CHECK( remove( dest ) );
    }
  }
}
