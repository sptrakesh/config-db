//
// Created by Rakesh on 27/12/2021.
//
#include <catch2/catch_test_macros.hpp>
#include "../../src/lib/db/storage.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::db;
using spt::configdb::model::RequestData;

SCENARIO( "Tree concept test" )
{
  GIVEN( "Two root nodes" )
  {
    auto key1 = "/key1"sv;
    auto key2 = "/key2"sv;

    WHEN( "Setting both root nodes" )
    {
      auto status = set( RequestData{ key1, "value1"sv } );
      REQUIRE( status );
      status = set( RequestData{ key2, "value2"sv } );
      REQUIRE( status );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key1"s );
      REQUIRE( (*children)[1] == "key2"s );
    }

    AND_WHEN( "Removing first root" )
    {
      const auto status = remove( key1 );
      REQUIRE( status );
    }

    AND_WHEN( "Listing root node again" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key2"s );
    }

    AND_WHEN( "Removing second root" )
    {
      const auto status = remove( key2 );
      REQUIRE( status );
    }

    AND_WHEN( "Listing non-existent root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Caching a key" )
    {
      REQUIRE( set( RequestData{ key1, "value1"sv, RequestData::Options{ 1, false, true } } ) );
      CHECK_FALSE( list( "/"sv ) );
      const auto value = get( key1 );
      REQUIRE( value );
      CHECK( *value == "value1"sv );
      CHECK( remove( key1 ) );
    }
  }

  GIVEN( "A deep tree" )
  {
    auto key = "/key1/key2/key3"sv;

    WHEN( "Setting a value for the deep path" )
    {
      auto status = set( RequestData{ key, "value"sv } );
      REQUIRE( status );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key1"s );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key2"s );
    }

    AND_WHEN( "Listing second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key3"s );
    }

    AND_WHEN( "Listing leaf node" )
    {
      const auto children = list( key );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Removing the deep path" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }

    AND_WHEN( "Listing second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Caching the key" )
    {
      REQUIRE( set( RequestData{ key, "value1"sv, RequestData::Options{ 1, false, true } } ) );
      CHECK_FALSE( list( "/"sv ) );
      const auto value = get( key );
      REQUIRE( value );
      CHECK( *value == "value1"sv );
      CHECK( remove( key ) );
    }
  }

  GIVEN( "Multi-level deep tree" )
  {
    auto key1 = "/key1/key2/key3"sv;
    auto key2 = "/key1/key2/key4"sv;
    auto key3 = "/key1/key21/key3"sv;
    auto key4 = "/key1/key21/key4"sv;

    WHEN( "Creating the multi-level tree" )
    {
      auto status = set( RequestData{ key1, "value"sv } );
      REQUIRE( status );
      status = set( RequestData{ key2, "value"sv } );
      REQUIRE( status );
      status = set( RequestData{ key3, "value"sv } );
      REQUIRE( status );
      status = set( RequestData{ key4, "value"sv } );
      REQUIRE( status );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key1"s );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key2"s );
      REQUIRE( (*children)[1] == "key21"s );
    }

    AND_WHEN( "Listing first second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key3"s );
      REQUIRE( (*children)[1] == "key4"s );
    }

    AND_WHEN( "Listing second second node" )
    {
      const auto children = list( "/key1/key21"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key3"s );
      REQUIRE( (*children)[1] == "key4"s );
    }

    AND_WHEN( "Removing first deep path" )
    {
      const auto status = remove( key1 );
      REQUIRE( status );
    }

    AND_WHEN( "Listing first second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key4"s );
    }

    AND_WHEN( "Listing second second node" )
    {
      const auto children = list( "/key1/key21"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key3"s );
      REQUIRE( (*children)[1] == "key4"s );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key2"s );
      REQUIRE( (*children)[1] == "key21"s );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key1"s );
    }

    AND_WHEN( "Setting a value for the first deep path parent" )
    {
      const auto status = set( RequestData{ "/key1/key2"sv, "value"sv } );
      REQUIRE( status );
    }

    AND_WHEN( "Removing second deep path" )
    {
      const auto status = remove( key2 );
      REQUIRE( status );
    }

    AND_WHEN( "Listing first second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing second second node" )
    {
      const auto children = list( "/key1/key21"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key3"s );
      REQUIRE( (*children)[1] == "key4"s );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 2 );
      REQUIRE( (*children)[0] == "key2"s );
      REQUIRE( (*children)[1] == "key21"s );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key1"s );
    }

    AND_WHEN( "Removing first deep path parent" )
    {
      const auto status = remove( "/key1/key2"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key21"s );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key1"s );
    }

    AND_WHEN( "Removing third deep path" )
    {
      const auto status = remove( key3 );
      REQUIRE( status );
    }

    AND_WHEN( "Listing first second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing second second node" )
    {
      const auto children = list( "/key1/key21"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key4"s );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key21"s );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE( children );
      REQUIRE( children->size() == 1 );
      REQUIRE( (*children)[0] == "key1"s );
    }

    AND_WHEN( "Removing fourth deep path" )
    {
      const auto status = remove( key4 );
      REQUIRE( status );
    }

    AND_WHEN( "Listing first second node" )
    {
      const auto children = list( "/key1/key2"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing second second node" )
    {
      const auto children = list( "/key1/key21"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing first node" )
    {
      const auto children = list( "/key1"sv );
      REQUIRE_FALSE( children );
    }

    AND_WHEN( "Listing root node" )
    {
      const auto children = list( "/"sv );
      REQUIRE_FALSE( children );
    }
  }
}
