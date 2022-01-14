//
// Created by Rakesh on 12/01/2022.
//

#include <catch2/catch.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "../../src/common/model/configuration.h"

namespace spt::configdb::unittest::pconf
{
  std::string file;
}

using spt::configdb::model::Configuration;
using namespace std::string_literals;
using namespace std::string_view_literals;
namespace pc = spt::configdb::unittest::pconf;

SCENARIO( "Configuration test", "config" )
{
  GIVEN( "A JSON representation of configuration" )
  {
    const auto json = R"({
  "encryption": {
    "salt": "asalt",
    "key": "akey",
    "iv": "aniv",
    "secret": "a secret",
    "rounds": 1000
  },
  "logging": {
    "level": "debug",
    "dir": "/tmp/",
    "console": false
  },
  "ssl": {
    "caCertificate": "/opt/spt/certs/ca.crt",
    "serverCertificate": "/opt/spt/certs/server.crt",
    "serverKey": "/opt/spt/certs/server.key",
    "enable": false
  },
  "services": {
    "http": "6006",
    "tcp": 2022
  },
  "storage": {
    "dbpath": "/tmp/config-db",
    "blockCacheSizeMb": 16
  },
  "threads": 8,
  "enableCache": true
})"sv;

    WHEN( "Writing the config to a file" )
    {
      auto now = std::chrono::high_resolution_clock::now();
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>( now.time_since_epoch() ).count();

      std::ostringstream ss;
      ss << "/tmp/" << ns << ".json";
      pc::file = ss.str();

      std::ofstream f( pc::file );
      f.write( json.data(), json.size() );
      f.close();

      auto p = std::filesystem::path{ pc::file };
      REQUIRE( std::filesystem::file_size( p ) > 0 );
    }

    AND_WHEN( "Creating configuration from file" )
    {
      Configuration::loadFromFile( pc::file );
      auto& cfg = Configuration::instance();

      REQUIRE( cfg.encryption.salt == "asalt"s );
      REQUIRE( cfg.encryption.key == "akey"s );
      REQUIRE( cfg.encryption.iv == "aniv"s );
      REQUIRE( cfg.encryption.secret == "a secret"s );
      REQUIRE( cfg.encryption.rounds == 1000 );

      REQUIRE( cfg.logging.level == "debug"s );
      REQUIRE( cfg.logging.dir == "/tmp/"s );
      REQUIRE_FALSE( cfg.logging.console );

      REQUIRE( cfg.ssl.caCertificate == "/opt/spt/certs/ca.crt"s );
      REQUIRE( cfg.ssl.certificate == "/opt/spt/certs/server.crt"s );
      REQUIRE( cfg.ssl.key == "/opt/spt/certs/server.key"s );
      REQUIRE_FALSE( cfg.ssl.enable );

      REQUIRE( cfg.services.http == "6006"s );
      REQUIRE( cfg.services.tcp == 2022 );

      REQUIRE( cfg.storage.dbpath == "/tmp/config-db"s );
      REQUIRE( cfg.storage.blockCacheSizeMb == 16 );

      REQUIRE( cfg.threads == 8 );
      REQUIRE( cfg.enableCache );
    }

    AND_WHEN( "Removing the file" )
    {
      auto p = std::filesystem::path{ pc::file };
      REQUIRE( std::filesystem::remove( p ) );
    }
  }
}
