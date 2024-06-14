//
// Created by Rakesh on 14/06/2024.
//

#include <boost/json/parse.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>

using std::operator ""s;
using std::operator ""sv;

SCENARIO( "HTTP server test suite", "http" )
{
  GIVEN( "HTTP server on localhost:6026" )
  {
    WHEN( "Setting a key-value pair" )
    {
      auto r = cpr::Put( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::Body{ "value" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE },
        cpr::Header{ { "Content-Type", "text/plain" } } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "code" ) );
      REQUIRE( doc["code"].is_int64() );
      CHECK( doc["code"].as_int64() == 200 );
      REQUIRE( doc.contains( "cause" ) );
      REQUIRE( doc["cause"].is_string() );
      CHECK( doc["cause"].as_string() == "Ok" );
    }

    AND_WHEN( "Listing the test parent path" )
    {
      auto r = cpr::Get( cpr::Url{ "http://localhost:6026/list/test" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "children" ) );
      REQUIRE( doc["children"].is_array() );
      REQUIRE( doc["children"].as_array().size() == 1 );
      REQUIRE( doc["children"].as_array()[0].is_string() );
      CHECK( doc["children"].as_array()[0].as_string() == "key" );
    }

    AND_WHEN( "Listing the root path" )
    {
      auto r = cpr::Get( cpr::Url{ "http://localhost:6026/list/" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "children" ) );
      REQUIRE( doc["children"].is_array() );
      REQUIRE( doc["children"].as_array().size() == 1 );
      REQUIRE( doc["children"].as_array()[0].is_string() );
      CHECK( doc["children"].as_array()[0].as_string() == "test" );
    }

    AND_WHEN( "Retrieving the key" )
    {
      auto r = cpr::Get( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "key" ) );
      REQUIRE( doc["key"].is_string() );
      CHECK( doc["key"].as_string() == "/test/key" );
      REQUIRE( doc.contains( "value" ) );
      REQUIRE( doc["value"].is_string() );
      CHECK( doc["value"].as_string() == "value" );
    }

    AND_WHEN( "Deleting the key" )
    {
      auto r = cpr::Delete( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "code" ) );
      REQUIRE( doc["code"].is_int64() );
      CHECK( doc["code"].as_int64() == 200 );
      REQUIRE( doc.contains( "cause" ) );
      REQUIRE( doc["cause"].is_string() );
      CHECK( doc["cause"].as_string() == "Ok" );
    }

    AND_WHEN( "Creating a cache value without TTL" )
    {
      auto r = cpr::Put( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::Body{ "value" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE },
        cpr::Header{ { "Content-Type", "text/plain" }, { "x-config-db-cache", "true" } } );
      REQUIRE( r.status_code == 412 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "code" ) );
      REQUIRE( doc["code"].is_int64() );
      CHECK( doc["code"].as_int64() == 412 );
      REQUIRE( doc.contains( "cause" ) );
      REQUIRE( doc["cause"].is_string() );
      CHECK( doc["cause"].as_string() == "Unable to save" );
    }

    AND_WHEN( "Creating a cache value" )
    {
      auto r = cpr::Put( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::Body{ "value" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE },
        cpr::Header{ { "Content-Type", "text/plain" },
          { "x-config-db-cache", "true" }, { "x-config-db-ttl", "60" } } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "code" ) );
      REQUIRE( doc["code"].is_int64() );
      CHECK( doc["code"].as_int64() == 200 );
      REQUIRE( doc.contains( "cause" ) );
      REQUIRE( doc["cause"].is_string() );
      CHECK( doc["cause"].as_string() == "Ok" );
    }

    AND_WHEN( "Listing the root path" )
    {
      auto r = cpr::Get( cpr::Url{ "http://localhost:6026/list/" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 404 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "code" ) );
      REQUIRE( doc["code"].is_int64() );
      REQUIRE( doc["code"].as_int64() == 404 );
      REQUIRE( doc.contains( "cause" ) );
      REQUIRE( doc["cause"].is_string() );
      CHECK( doc["cause"].as_string() == "Not found" );
    }

    AND_WHEN( "Retrieving the key" )
    {
      auto r = cpr::Get( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "key" ) );
      REQUIRE( doc["key"].is_string() );
      CHECK( doc["key"].as_string() == "/test/key" );
      REQUIRE( doc.contains( "value" ) );
      REQUIRE( doc["value"].is_string() );
      CHECK( doc["value"].as_string() == "value" );
    }

    AND_WHEN( "Deleting the key" )
    {
      auto r = cpr::Delete( cpr::Url{ "http://localhost:6026/key/test/key" },
        cpr::HttpVersion{ cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE } );
      REQUIRE( r.status_code == 200 );

      boost::system::error_code ec;
      auto resp = boost::json::parse( r.text, ec );
      REQUIRE_FALSE( ec );
      REQUIRE( resp.is_object() );

      auto& doc = resp.as_object();
      REQUIRE( doc.contains( "code" ) );
      REQUIRE( doc["code"].is_int64() );
      CHECK( doc["code"].as_int64() == 200 );
      REQUIRE( doc.contains( "cause" ) );
      REQUIRE( doc["cause"].is_string() );
      CHECK( doc["cause"].as_string() == "Ok" );
    }
  }
}