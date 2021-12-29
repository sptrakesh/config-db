//
// Created by Rakesh on 28/12/2021.
//

#include <catch2/catch.hpp>

#include "connection.h"
#include "../../src/lib/model/request_generated.h"

using namespace spt::configdb::model;
using namespace std::string_view_literals;

SCENARIO( "Deep tree test", "[tree]" )
{
  boost::asio::io_context ioc;

  GIVEN( "Connected to TCP Service" )
  {
    static const auto key1 = "/key1/key2/key3"sv;
    static const auto key2 = "/key1/key2/key4"sv;
    static const auto key3 = "/key1/key21/key3"sv;
    static const auto key4 = "/key1/key21/key4"sv;

    spt::configdb::itest::tcp::Connection c{ ioc };

    WHEN( "Creating keys" )
    {
      const auto fn = [&c]( std::string_view k )
      {
        auto fb = flatbuffers::FlatBufferBuilder{ 256 };
        auto key = fb.CreateString( k );
        auto value = fb.CreateString( "value" );
        auto request = CreateRequest( fb, Action::Put, key, value );
        fb.Finish( request );

        const auto [response, isize, osize] = c.write( fb, "Set" );
        REQUIRE( isize != osize );
        REQUIRE( response );

        REQUIRE( response->value_type() == ResponseValue::Success );
        auto resp = response->value_as<Success>();
        REQUIRE( resp );
        REQUIRE( resp->value() );
      };

      fn( key1 );
      fn( key2 );
      fn( key3 );
      fn( key4 );
    }

    AND_WHEN( "Listing root node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto key = fb.CreateString( "/" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::List, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Children );
      auto resp = response->value_as<Children>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->string_view() == "key1"sv );
    }

    AND_WHEN( "Listing first node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto key = fb.CreateString( "/key1" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::List, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Children );
      auto resp = response->value_as<Children>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 2 );
      REQUIRE( resp->value()->Get( 0 )->string_view() == "key2"sv );
      REQUIRE( resp->value()->Get( 1 )->string_view() == "key21"sv );
    }

    AND_WHEN( "Listing first second node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto key = fb.CreateString( "/key1/key2" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::List, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Children );
      auto resp = response->value_as<Children>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 2 );
      REQUIRE( resp->value()->Get( 0 )->string_view() == "key3"sv );
      REQUIRE( resp->value()->Get( 1 )->string_view() == "key4"sv );
    }

    AND_WHEN( "Listing second second node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto key = fb.CreateString( "/key1/key21" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::List, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Children );
      auto resp = response->value_as<Children>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 2 );
      REQUIRE( resp->value()->Get( 0 )->string_view() == "key3"sv );
      REQUIRE( resp->value()->Get( 1 )->string_view() == "key4"sv );
    }

    AND_WHEN( "Removing the keys" )
    {
      const auto fn = [&c]( std::string_view k )
      {
        auto fb = flatbuffers::FlatBufferBuilder{ 256 };
        auto key = fb.CreateString( k );
        auto value = fb.CreateString( "" );
        auto request = CreateRequest( fb, Action::Delete, key, value );
        fb.Finish( request );

        const auto [response, isize, osize] = c.write( fb, "Delete" );
        REQUIRE( isize != osize );
        REQUIRE( response );
        REQUIRE( response->value_type() == ResponseValue::Success );
        auto resp = response->value_as<Success>();
        REQUIRE( resp );
        REQUIRE( resp->value() );
      };

      fn( key1 );
      fn( key2 );
      fn( key3 );
      fn( key4 );
    }
  }
}

