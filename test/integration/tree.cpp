//
// Created by Rakesh on 28/12/2021.
//

#include <catch2/catch_test_macros.hpp>

#include "../../src/api/api.hpp"

using namespace spt::configdb::api;
using namespace std::string_view_literals;

SCENARIO( "Deep tree test", "[tree]" )
{
  GIVEN( "A set of keys" )
  {
    const auto key1 = "/key1/key2/key3"sv;
    const auto key2 = "/key1/key2/key4"sv;
    const auto key3 = "/key1/key21/key3"sv;
    const auto key4 = "/key1/key21/key4"sv;

    WHEN( "Creating keys" )
    {
      const auto kvs = std::vector<Pair>{
          { key1, "value"sv }, { key2, "value"sv },
          { key3, "value"sv }, { key4, "value"sv } };
      const auto status = set( kvs );
      REQUIRE( status );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( children->at( 0 ) == "key1"sv );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( children->at( 0 ) == "key2"sv );
      REQUIRE( children->at( 1 ) == "key21"sv );
    }

    AND_WHEN( "Listing first second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( children->at( 0 ) == "key3"sv );
      REQUIRE( children->at( 1 ) == "key4"sv );
    }

    AND_WHEN( "Listing second second node" )
    {
      const auto children = list( "/key1/key21"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( children->at( 0 ) == "key3"sv );
      REQUIRE( children->at( 1 ) == "key4"sv );
    }

    AND_WHEN( "Removing the keys" )
    {
      const auto keys = std::vector{ key1, key2, key3, key4 };
      const auto results = remove( keys );
      REQUIRE( results );
    }

    AND_WHEN( "Listing the root node" )
    {
      REQUIRE_FALSE( list( "/"sv ) );
    }
  }
}

