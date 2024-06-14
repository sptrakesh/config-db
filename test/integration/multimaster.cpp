//
// Created by Rakesh on 26/01/2022.
//

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <boost/process.hpp>

#include "../../src/api/impl/connection.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace spt::configdb::model;
using spt::configdb::api::impl::Connection;

namespace spt::configdb::itest::mmaster
{
  struct Cluster
  {
    static const Cluster& instance()
    {
      static Cluster c;
      return c;
    }

    ~Cluster()
    {
      boost::system::error_code ec;
      g.terminate( ec );
      if ( ec ) LOG_CRIT << "Error terminating processes group. " << ec.message();

      for ( auto&& c : children ) c.wait();
      auto path = std::filesystem::path{ "/tmp/configdb" };
      std::filesystem::remove_all( path, ec );
      if ( ec ) LOG_CRIT << "Error deleting directory tree. " << ec.message();
    }

  private:
    Cluster()
    {
      server1();
      server2();
      server3();
    }

    void server1()
    {
      if ( const auto dir = std::filesystem::path{ "/tmp/configdb/logs1" };
        !std::filesystem::exists( dir ) ) std::filesystem::create_directories( dir );

      auto config = R"({
  "logging": {
    "dir": "/tmp/configdb/logs1/",
    "level": "debug"
  },
  "services": {
    "http": "6027",
    "tcp": 2023,
    "notify": 2123
  },
  "storage": {
    "dbpath": "/tmp/configdb/server1",
    "encrypterInitialPoolSize": 1
  },
  "peers": [
    {
      "host": "localhost",
      "port": 2124
    },
    {
      "host": "localhost",
      "port": 2125
    }
  ],
  "threads": 2
})"s;
      auto file = "/tmp/configdb/server1.json"sv;
      auto os = std::ofstream{ file };
      os << config;
      os.close();
      start( std::filesystem::path{ "/tmp/configdb/server1/"sv }, file );
    }

    void server2()
    {
      if ( const auto dir = std::filesystem::path{ "/tmp/configdb/logs2" };
        !std::filesystem::exists( dir ) ) std::filesystem::create_directories( dir );

      auto config = R"({
  "logging": {
    "dir": "/tmp/configdb/logs2/",
    "level": "debug"
  },
  "services": {
    "http": "6028",
    "tcp": 2024,
    "notify": 2124
  },
  "storage": {
    "dbpath": "/tmp/configdb/server2",
    "encrypterInitialPoolSize": 1
  },
  "peers": [
    {
      "host": "localhost",
      "port": 2123
    },
    {
      "host": "localhost",
      "port": 2125
    }
  ],
  "threads": 2
})"s;
      auto file = "/tmp/configdb/server2.json"sv;
      auto os = std::ofstream{ file };
      os << config;
      os.close();
      start( std::filesystem::path{ "/tmp/configdb/server2/"sv }, file );
    }

    void server3()
    {
      if ( const auto dir = std::filesystem::path{ "/tmp/configdb/logs3" };
        !std::filesystem::exists( dir ) ) std::filesystem::create_directories( dir );

      auto config = R"({
  "logging": {
    "dir": "/tmp/configdb/logs3/",
    "level": "debug"
  },
  "services": {
    "http": "6029",
    "tcp": 2025,
    "notify": 2125
  },
  "storage": {
    "dbpath": "/tmp/configdb/server3",
    "encrypterInitialPoolSize": 1
  },
  "peers": [
    {
      "host": "localhost",
      "port": 2123
    },
    {
      "host": "localhost",
      "port": 2124
    }
  ],
  "threads": 2
})"s;
      auto file = "/tmp/configdb/server3.json"sv;
      auto os = std::ofstream{ file };
      os << config;
      os.close();
      start( std::filesystem::path{ "/tmp/configdb/server3/"sv }, file );
    }

    void start( const std::filesystem::path& dir, std::string_view path )
    {
      if ( !std::filesystem::exists( dir ) ) std::filesystem::create_directories( dir );
      if ( !std::filesystem::exists( cmd ) )
      {
        LOG_CRIT << "Executable " << cmd << " not found. CWD: " << std::filesystem::current_path().string();
        return;
      }
      auto command = std::string{};
      command.reserve( cmd.size() + 16 );
      command.append( cmd ).append( " -f ").append( path );
      children.emplace_back( cmd, "-f", std::string{ path }, g );
    }

    boost::process::group g;
    std::vector<boost::process::child> children;
    // Assuming current working directory is same as integration test executable location.
    std::string cmd{ "../../src/service/configdb"s };
  };

  void sleep()
  {
    std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
  }
}

SCENARIO( "Multi-master cluster test", "multi-master" )
{
  GIVEN( "A 3 node cluster of instances running in multi-master mode" )
  {
    using spt::configdb::itest::mmaster::sleep;
    spt::configdb::itest::mmaster::Cluster::instance();
    auto key = "/key1"sv;
    auto dest = "/key2"sv;
    auto value = "value"sv;
    auto updated = "value updated"sv;

    WHEN( "Creating a key on first server" )
    {
      std::cout << "Sleeping 20s to ensure cluster connections" << std::endl;
      std::this_thread::sleep_for( std::chrono::seconds{ 20 } );
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.set( RequestData{ key, value } );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp->value() );
      sleep();
    }

    AND_WHEN( "Reading key on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.get( key );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == value );
    }

    AND_WHEN( "Reading key on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.get( key );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == value );
    }

    AND_WHEN( "Updating key on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.set( RequestData{ key, updated } );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp->value() );
      sleep();
    }

    AND_WHEN( "Reading key on first server" )
    {
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.get( key );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == updated );
    }

    AND_WHEN( "Reading key on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.get( key );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == updated );
    }

    AND_WHEN( "Moving key on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.move( RequestData{ key, dest } );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp->value() );
      sleep();
    }

    AND_WHEN( "Reading dest key on first server" )
    {
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.get( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == updated );
    }

    AND_WHEN( "Reading dest key on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.get( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == updated );
    }

    AND_WHEN( "Deleting key on first server" )
    {
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.remove( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp->value() );
      sleep();
    }

    AND_WHEN( "Reading deleted key on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.get( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      auto v = resp->value()->Get( 0 )->value_as<Success>();
      REQUIRE_FALSE( v->value() );
    }

    AND_WHEN( "Reading deleted key on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.get( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      auto v = resp->value()->Get( 0 )->value_as<Success>();
      REQUIRE_FALSE( v->value() );
    }

    AND_WHEN( "Setting cached key on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.set( RequestData{ key, value, RequestData::Options{ 600u, false, true } } );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp->value() );
      sleep();
    }

    AND_WHEN( "Reading key on first server" )
    {
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.get( key );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == value );
    }

    AND_WHEN( "Reading key on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.get( key );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Value );
      auto v = resp->value()->Get( 0 )->value_as<Value>();
      REQUIRE( v->value()->string_view() == value );
    }

    AND_WHEN( "Listing root on first server" )
    {
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.list( "/"sv );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      CHECK_FALSE( resp->value()->Get( 0 )->value_as<Success>()->value() );
    }

    AND_WHEN( "Listing root on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.list( "/"sv );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      CHECK_FALSE( resp->value()->Get( 0 )->value_as<Success>()->value() );
    }

    AND_WHEN( "Listing root on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.list( "/"sv );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      CHECK_FALSE( resp->value()->Get( 0 )->value_as<Success>()->value() );
    }

    AND_WHEN( "Deleting key on third server" )
    {
      auto c = Connection{ "localhost"sv, "2025"sv };
      auto response = c.remove( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::Success );
      auto resp = response->value_as<Success>();
      REQUIRE( resp->value() );
      sleep();
    }

    AND_WHEN( "Reading deleted key on first server" )
    {
      auto c = Connection{ "localhost"sv, "2023"sv };
      auto response = c.get( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      auto v = resp->value()->Get( 0 )->value_as<Success>();
      REQUIRE_FALSE( v->value() );
    }

    AND_WHEN( "Reading deleted key on second server" )
    {
      auto c = Connection{ "localhost"sv, "2024"sv };
      auto response = c.get( dest );
      REQUIRE( response );
      REQUIRE( response->value_type() == ResultVariant::KeyValueResults );
      auto resp = response->value_as<KeyValueResults>();
      REQUIRE( resp->value()->size() == 1 );
      REQUIRE( resp->value()->Get( 0 )->value_type() == ValueVariant::Success );
      auto v = resp->value()->Get( 0 )->value_as<Success>();
      REQUIRE_FALSE( v->value() );
    }
  }
}
