//
// Created by Rakesh on 25/12/2021.
//

#include <catch2/catch.hpp>

#include <boost/asio/io_context.hpp>

#include "connection.h"
#include "../../src/lib/log/NanoLog.h"
#include "../../src/lib/model/request_generated.h"

using namespace spt::configdb::model;
using namespace std::string_view_literals;

SCENARIO( "Simple CRUD test suite", "[crud]" )
{
  boost::asio::io_context ioc;

  GIVEN( "Connected to TCP Service" )
  {
    spt::configdb::itest::tcp::Connection c{ ioc };

    WHEN( "Setting a key-value pair" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto key = fb.CreateString( "/key" );
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
    }

    AND_WHEN( "Reading key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto key = fb.CreateString( "/key" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::Get, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Value );
      auto resp = response->value_as<Value>();
      REQUIRE( resp );
      REQUIRE( resp->value()->string_view() == "value"sv );
    }

    AND_WHEN( "Updating value" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto key = fb.CreateString( "/key" );
      auto value = fb.CreateString( "value modified" );
      auto request = CreateRequest( fb, Action::Put, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Set" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp );
      REQUIRE( resp->value() );
    }

    AND_WHEN( "Reading updated value" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto key = fb.CreateString( "/key" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::Get, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Value );
      auto resp = response->value_as<Value>();
      REQUIRE( resp );
      REQUIRE( resp->value()->string_view() == "value modified"sv );
    }

    AND_WHEN( "Deleting key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto key = fb.CreateString( "/key" );
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
    }

    AND_WHEN( "Retrieving deleted key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto key = fb.CreateString( "/key" );
      auto value = fb.CreateString( "" );
      auto request = CreateRequest( fb, Action::Get, key, value );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get deleted" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResponseValue::Error );
      auto resp = response->value_as<Error>();
      REQUIRE( resp );
      REQUIRE( resp->value() );
      REQUIRE( resp->message()->string_view() == "Key not found"sv );
    }

    AND_WHEN( "Sending noop message" )
    {
      auto size = c.noop();
      REQUIRE( size == 4 );
    }
  }
}
