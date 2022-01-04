//
// Created by Rakesh on 28/12/2021.
//

#include <catch2/catch.hpp>

#include "connection.h"
#include "../../src/common/model/request_generated.h"

using namespace spt::configdb::model;
using namespace std::string_view_literals;

SCENARIO( "Deep tree test", "[tree]" )
{
  boost::asio::io_context ioc;

  GIVEN( "Connected to TCP Service" )
  {
    const auto key1 = "/key1/key2/key3"sv;
    const auto key2 = "/key1/key2/key4"sv;
    const auto key3 = "/key1/key21/key3"sv;
    const auto key4 = "/key1/key21/key4"sv;

    spt::configdb::itest::tcp::Connection c{ ioc };

    WHEN( "Creating keys" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{};
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( key1 ), fb.CreateString( "value" ) ),
          CreateKeyValue( fb, fb.CreateString( key2 ), fb.CreateString( "value" ) ),
          CreateKeyValue( fb, fb.CreateString( key3 ), fb.CreateString( "value" ) ),
          CreateKeyValue( fb, fb.CreateString( key4 ), fb.CreateString( "value" ) )
      };

      auto request = CreateRequest( fb, Action::Put, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Set" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp );
      REQUIRE( resp->value() );
    }

    AND_WHEN( "Listing root node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto vec = std::vector<flatbuffers::Offset<KeyValue>>{ CreateKeyValue( fb, fb.CreateString( "/" ) ) };
      auto request = CreateRequest( fb, Action::List, fb.CreateVector( vec ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Children );
      auto v = resp->value()->Get( 0 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 1 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key1"sv );
    }

    AND_WHEN( "Listing first node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto vec = std::vector<flatbuffers::Offset<KeyValue>>{ CreateKeyValue( fb, fb.CreateString( "/key1" ) ) };
      auto request = CreateRequest( fb, Action::List, fb.CreateVector( vec ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/key1"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Children );
      auto v = resp->value()->Get( 0 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 2 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key2"sv );
      REQUIRE( v->value()->Get( 1 )->string_view() == "key21"sv );
    }

    AND_WHEN( "Listing first second node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto vec = std::vector<flatbuffers::Offset<KeyValue>>{ CreateKeyValue( fb, fb.CreateString( "/key1/key2" ) ) };
      auto request = CreateRequest( fb, Action::List, fb.CreateVector( vec ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/key1/key2"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Children );
      auto v = resp->value()->Get( 0 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 2 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key3"sv );
      REQUIRE( v->value()->Get( 1 )->string_view() == "key4"sv );
    }

    AND_WHEN( "Listing second second node" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto vec = std::vector<flatbuffers::Offset<KeyValue>>{ CreateKeyValue( fb, fb.CreateString( "/key1/key21" ) ) };
      auto request = CreateRequest( fb, Action::List, fb.CreateVector( vec ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/key1/key21"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Children );
      auto v = resp->value()->Get( 0 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 2 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key3"sv );
      REQUIRE( v->value()->Get( 1 )->string_view() == "key4"sv );
    }

    AND_WHEN( "Listing all nodes" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 64 };
      auto vec = std::vector<flatbuffers::Offset<KeyValue>>{
        CreateKeyValue( fb, fb.CreateString( "/" ) ),
        CreateKeyValue( fb, fb.CreateString( "/key1" ) ),
        CreateKeyValue( fb, fb.CreateString( "/key1/key2" ) ),
        CreateKeyValue( fb, fb.CreateString( "/key1/key21" ) )
      };
      auto request = CreateRequest( fb, Action::List, fb.CreateVector( vec ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "List" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 4 );

      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Children );
      auto v = resp->value()->Get( 0 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 1 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key1"sv );

      REQUIRE( resp->value()->Get( 1 )->key()->string_view() == "/key1"sv );
      REQUIRE( resp->value()->Get( 1 )->value_type() == ValueVariant::Children );
      v = resp->value()->Get( 1 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 2 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key2"sv );
      REQUIRE( v->value()->Get( 1 )->string_view() == "key21"sv );

      REQUIRE( resp->value()->Get( 2 )->key()->string_view() == "/key1/key2"sv );
      REQUIRE( resp->value()->Get( 2 )->value_type() == ValueVariant::Children );
      v = resp->value()->Get( 2 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 2 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key3"sv );
      REQUIRE( v->value()->Get( 1 )->string_view() == "key4"sv );

      REQUIRE( resp->value()->Get( 3 )->key()->string_view() == "/key1/key21"sv );
      REQUIRE( resp->value()->Get( 3 )->value_type() == ValueVariant::Children );
      v = resp->value()->Get( 3 )->value_as<Children>();
      REQUIRE( v );
      REQUIRE( v->value()->size() == 2 );
      REQUIRE( v->value()->Get( 0 )->string_view() == "key3"sv );
      REQUIRE( v->value()->Get( 1 )->string_view() == "key4"sv );
    }

    AND_WHEN( "Retrieving all keys" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{};
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( key1 ) ),
          CreateKeyValue( fb, fb.CreateString( key2 ) ),
          CreateKeyValue( fb, fb.CreateString( key3 ) ),
          CreateKeyValue( fb, fb.CreateString( key4 ) )
      };
      auto request = CreateRequest( fb, Action::Get, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 4 );

      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == key1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == "value"sv );

      REQUIRE( resp->value()->Get( 1 )->key()->string_view() == key2 );
      REQUIRE( resp->value()->Get( 1 )->value_type() == ValueVariant::Value );
      v = resp->value()->Get( 1 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == "value"sv );

      REQUIRE( resp->value()->Get( 2 )->key()->string_view() == key3 );
      REQUIRE( resp->value()->Get( 2 )->value_type() == ValueVariant::Value );
      v = resp->value()->Get( 2 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == "value"sv );

      REQUIRE( resp->value()->Get( 3 )->key()->string_view() == key4 );
      REQUIRE( resp->value()->Get( 3 )->value_type() == ValueVariant::Value );
      v = resp->value()->Get( 3 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == "value"sv );
    }

    AND_WHEN( "Removing the keys" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{};
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( key1 ) ),
          CreateKeyValue( fb, fb.CreateString( key2 ) ),
          CreateKeyValue( fb, fb.CreateString( key3 ) ),
          CreateKeyValue( fb, fb.CreateString( key4 ) )
      };

      auto request = CreateRequest( fb, Action::Delete, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Delete" );
      REQUIRE( isize != osize );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp );
      REQUIRE( resp->value() );
    }
  }
}

