//
// Created by Rakesh on 25/12/2021.
//

#include <catch2/catch.hpp>

#include <boost/asio/io_context.hpp>

#include "connection.h"
#include "../../src/common/log/NanoLog.h"
#include "../../src/common/model/request_generated.h"

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
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( "/key" ), fb.CreateString( "value" ) )
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

    AND_WHEN( "Reading key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( "/key" ) )
      };
      auto request = CreateRequest( fb, Action::Get, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/key"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == "value"sv );
    }

    AND_WHEN( "Updating value" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( "/key" ), fb.CreateString( "value modified" ) )
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

    AND_WHEN( "Reading updated value" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( "/key" ) )
      };
      auto request = CreateRequest( fb, Action::Get, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/key"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == "value modified"sv );
    }

    AND_WHEN( "Deleting key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( "/key" ) )
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

    AND_WHEN( "Retrieving deleted key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( "/key" ) )
      };
      auto request = CreateRequest( fb, Action::Get, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get deleted" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == "/key"sv );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      auto v = resp->value()->Get( 0 )->value_as<Success>();
      REQUIRE_FALSE( v->value() );
    }

    AND_WHEN( "Sending noop message" )
    {
      auto size = c.noop();
      REQUIRE( size == 4 );
    }
  }

  GIVEN( "A large JSON format value" )
  {
    spt::configdb::itest::tcp::Connection c{ ioc };
    auto key = "/key"sv;
    auto value = R"({
  "users": [{
    "userId": 1,
    "firstName": "Krish",
    "lastName": "Lee",
    "phoneNumber": "123456",
    "emailAddress": "krish.lee@learningcontainer.com"
  }, {
    "userId": 2,
    "firstName": "racks",
    "lastName": "jacson",
    "phoneNumber": "123456",
    "emailAddress": "racks.jacson@learningcontainer.com"
  }, {
    "userId": 3,
    "firstName": "denial",
    "lastName": "roast",
    "phoneNumber": "33333333",
    "emailAddress": "denial.roast@learningcontainer.com"
  }, {
    "userId": 4,
    "firstName": "devid",
    "lastName": "neo",
    "phoneNumber": "222222222",
    "emailAddress": "devid.neo@learningcontainer.com"
  }, {
    "userId": 5,
    "firstName": "jone",
    "lastName": "mac",
    "phoneNumber": "111111111",
    "emailAddress": "jone.mac@learningcontainer.com"
  }],
  "Employees": [{
    "userId": "krish",
    "jobTitle": "Developer",
    "firstName": "Krish",
    "lastName": "Lee",
    "employeeCode": "E1",
    "region": "CA",
    "phoneNumber": "123456",
    "emailAddress": "krish.lee@learningcontainer.com"
  }, {
    "userId": "devid",
    "jobTitle": "Developer",
    "firstName": "Devid",
    "lastName": "Rome",
    "employeeCode": "E2",
    "region": "CA",
    "phoneNumber": "1111111",
    "emailAddress": "devid.rome@learningcontainer.com"
  }, {
    "userId": "tin",
    "jobTitle": "Program Directory",
    "firstName": "tin",
    "lastName": "jonson",
    "employeeCode": "E3",
    "region": "CA",
    "phoneNumber": "2222222",
    "emailAddress": "tin.jonson@learningcontainer.com"
  }]
}
)"sv;

    WHEN( "Saving the key-value pair" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{};
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( key ), fb.CreateString( value ) )
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

    AND_WHEN( "Reading key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{};
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( key ) )
      };
      auto request = CreateRequest( fb, Action::Get, fb.CreateVector( kvs ) );
      fb.Finish( request );

      const auto [response, isize, osize] = c.write( fb, "Get" );
      REQUIRE( isize != osize );
      REQUIRE( response );

      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp );
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->key()->string_view() == key );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v );
      REQUIRE( v->value()->string_view() == value );
    }

    AND_WHEN( "Deleting key" )
    {
      auto fb = flatbuffers::FlatBufferBuilder{ 256 };
      auto kvs = std::vector<flatbuffers::Offset<KeyValue>>{
          CreateKeyValue( fb, fb.CreateString( key ) )
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
