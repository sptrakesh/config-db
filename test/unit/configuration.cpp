//
// Created by Rakesh on 12/01/2022.
//

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "../../src/common/model/configuration.hpp"

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
      Configuration::reset();
    }
  }
}

/*
SCENARIO( "Serialisation test" )
{
  GIVEN( "A set of strings" )
  {
    std::set<std::string, std::less<>> names;
    for ( auto i = 0; i < 10000; ++i )
    {
      std::string name;
      name.reserve( 12 );
      name.append( "name" ).append( std::to_string( i ) );
      names.insert( name );
    }

    WHEN( "Comparing flatbuffers to boost serialization" )
    {
      const auto tofbuf = [&names]
      {
        auto st = std::chrono::high_resolution_clock::now();
        auto fb = flatbuffers::FlatBufferBuilder{ 128 };
        auto children = std::vector<flatbuffers::Offset<flatbuffers::String>>{};
        children.reserve( names.size() );
        for ( auto&& n : names ) children.push_back( fb.CreateString( n ) );

        auto nc = fb.CreateVector( children );
        auto n = spt::configdb::model::CreateNode( fb, nc );
        fb.Finish( n );

        std::string buf{ reinterpret_cast<const char*>( fb.GetBufferPointer() ), fb.GetSize() };
        std::ostringstream ss;
        ss << buf;
        ss.str();

        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return std::tuple{ ss.str(), delta.count() };
      };

      const auto fromfbuf = [&names]( std::string_view str )
      {
        auto st = std::chrono::high_resolution_clock::now();

        const auto d = reinterpret_cast<const uint8_t*>( str.data() );
        auto response = spt::configdb::model::GetNode( d );

        auto vec = std::vector<std::string>{};
        vec.reserve( response->children()->size() + 1 );
        for ( auto&& item : *response->children() ) vec.push_back( item->str() );
        REQUIRE( vec.size() == names.size() );
        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return delta.count();
      };

      const auto tocppser = [&names]()
      {
        auto st = std::chrono::high_resolution_clock::now();
        auto buf = buffer{};
        store( buf, names );
        auto str = std::string{ reinterpret_cast<const char*>( buf.data() ), buf.size() };

        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return std::tuple{ str, delta.count() };
      };

      const auto fromcppser = [&names]( std::string_view str )
      {
        auto st = std::chrono::high_resolution_clock::now();
        auto par = parser{};
        par.init( reinterpret_cast<const uint8_t*>( str.data() ), str.size() );
        std::set<std::string> n;
        parse( par, n );
        REQUIRE( n.size() == names.size() );
        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return delta.count();
      };

      std::vector<uint64_t> tofbufs;
      tofbufs.reserve( 100 );
      std::vector<uint64_t> fromfbufs;
      fromfbufs.reserve( 100 );
      std::vector<uint64_t> tocppsers;
      tocppsers.reserve( 100 );
      std::vector<uint64_t> fromcppsers;
      fromcppsers.reserve( 100 );

      for ( auto i = 0; i < 100; ++i )
      {
        const auto [str,tfbuft] = tofbuf();
        tofbufs.push_back( tfbuft );
        const auto ffbuft = fromfbuf( str );
        fromfbufs.push_back( ffbuft );

        const auto [bstr,tcppser] = tocppser();
        tocppsers.push_back( tcppser );
        const auto fcppser = fromcppser( bstr );
        fromcppsers.push_back( fcppser );
      }

      std::cout << "Average time to produce Flatbuffer from set " << std::reduce( tofbufs.begin(), tofbufs.end() ) / tofbufs.size() << std::endl;
      std::cout << "Average time to consume set from Flatbuffer " << std::reduce( fromfbufs.begin(), fromfbufs.end() ) / fromfbufs.size() << std::endl;
      std::cout << "Average time to produce cppser serialisation from set " << std::reduce( fromcppsers.begin(), fromcppsers.end() ) / fromcppsers.size() << std::endl;
      std::cout << "Average time to consume set from cppser serialisation " << std::reduce( tocppsers.begin(), tocppsers.end() ) / tocppsers.size() << std::endl;
    }
  }

  GIVEN( "A vector of strings" )
  {
    std::vector<std::string> names;
    for ( auto i = 0; i < 10000; ++i )
    {
      std::string name;
      name.reserve( 12 );
      name.append( "name" ).append( std::to_string( i ) );
      names.push_back( name );
    }

    WHEN( "Comparing flatbuffers to boost serialization" )
    {
      const auto tofbuf = [&names]
      {
        auto st = std::chrono::high_resolution_clock::now();
        auto fb = flatbuffers::FlatBufferBuilder{ 128 };
        auto children = std::vector<flatbuffers::Offset<flatbuffers::String>>{};
        children.reserve( names.size() );
        for ( auto&& n : names ) children.push_back( fb.CreateString( n ) );

        auto nc = fb.CreateVector( children );
        auto n = spt::configdb::model::CreateNode( fb, nc );
        fb.Finish( n );

        std::string buf{ reinterpret_cast<const char*>( fb.GetBufferPointer() ), fb.GetSize() };
        std::ostringstream ss;
        ss << buf;
        ss.str();

        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return std::tuple{ ss.str(), delta.count() };
      };

      const auto fromfbuf = [&names]( std::string_view str )
      {
        auto st = std::chrono::high_resolution_clock::now();

        const auto d = reinterpret_cast<const uint8_t*>( str.data() );
        auto response = spt::configdb::model::GetNode( d );

        auto vec = std::vector<std::string>{};
        vec.reserve( response->children()->size() + 1 );
        for ( auto&& item : *response->children() ) vec.push_back( item->str() );
        REQUIRE( vec.size() == names.size() );
        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return delta.count();
      };

      const auto tocppser = [&names]()
      {
        auto st = std::chrono::high_resolution_clock::now();
        auto buf = buffer{};
        store( buf, names );
        auto str = std::string{ reinterpret_cast<const char*>( buf.data() ), buf.size() };

        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return std::tuple{ str, delta.count() };
      };

      const auto fromcppser = [&names]( std::string_view str )
      {
        auto st = std::chrono::high_resolution_clock::now();
        auto par = parser{};
        par.init( reinterpret_cast<const uint8_t*>( str.data() ), str.size() );
        std::vector<std::string> n;
        parse( par, n );
        REQUIRE( n.size() == names.size() );
        auto et = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return delta.count();
      };

      std::vector<uint64_t> tofbufs;
      tofbufs.reserve( 100 );
      std::vector<uint64_t> fromfbufs;
      fromfbufs.reserve( 100 );
      std::vector<uint64_t> tocppsers;
      tocppsers.reserve( 100 );
      std::vector<uint64_t> fromcppsers;
      fromcppsers.reserve( 100 );

      for ( auto i = 0; i < 100; ++i )
      {
        const auto [str,tfbuft] = tofbuf();
        tofbufs.push_back( tfbuft );
        const auto ffbuft = fromfbuf( str );
        fromfbufs.push_back( ffbuft );

        const auto [bstr,tcppser] = tocppser();
        tocppsers.push_back( tcppser );
        const auto fcppser = fromcppser( bstr );
        fromcppsers.push_back( fcppser );
      }

      std::cout << "Average time to produce Flatbuffer from vector " << std::reduce( tofbufs.begin(), tofbufs.end() ) / tofbufs.size() << std::endl;
      std::cout << "Average time to consume vector from Flatbuffer " << std::reduce( fromfbufs.begin(), fromfbufs.end() ) / fromfbufs.size() << std::endl;
      std::cout << "Average time to produce cppser serialisation from vetor " << std::reduce( fromcppsers.begin(), fromcppsers.end() ) / fromcppsers.size() << std::endl;
      std::cout << "Average time to consume vector from cppser serialisation " << std::reduce( tocppsers.begin(), tocppsers.end() ) / tocppsers.size() << std::endl;
    }
  }
}
*/