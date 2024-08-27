//
// Created by Rakesh on 25/12/2021.
//

#include "../common/model/configuration.hpp"
#include "../common/model/request.h"
#include "../common/util/clara.hpp"
#include "../lib/db/storage.hpp"
#include "../log/NanoLog.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

int main( int argc, char const * const * argv )
{
  using namespace std::string_literals;
  using clara::Opt;
#ifdef __APPLE__
  std::string logLevel{ "debug" };
#else
  std::string logLevel{"info"};
#endif
  std::string dir{"/tmp/"};
  std::string config{};
  std::string file{};
  bool help = false;

  auto options = clara::Help(help) |
      Opt(config, "config.json")["-c"]["--conf"]("Service configuration file (optional)") |
      Opt(file, "file.txt")["-f"]["--file"]("File to bulk import data from") |
      Opt(logLevel, "info")["-l"]["--log-level"]("Log level to use [debug|info|warn|critical] (default info).") |
      Opt(dir, "/tmp/")["-o"]["--log-dir"]("Log directory (default /tmp/)");

  auto result = options.parse(clara::Args(argc, argv));
  if ( !result )
  {
    std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
    exit( 1 );
  }

  if ( help )
  {
    options.writeToStream( std::cout );
    exit( 0 );
  }

  if ( file.empty() )
  {
    std::cerr << "Missing file to import" << std::endl;
    options.writeToStream( std::cout );
    exit( 2 );
  }

  if ( logLevel == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( logLevel == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( logLevel == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( logLevel == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), dir, "seed-configdb", false );

  try
  {
    if ( !config.empty() ) spt::configdb::model::Configuration::loadFromFile( config );
    auto f = std::fstream{ file, std::ios_base::in };
    if ( ! f.is_open() )
    {
      std::cerr << "Error opening file " << file << std::endl;
      return 3;
    }

    auto kvs = std::vector<spt::configdb::model::RequestData>{};
    kvs.reserve( 64 );
    uint32_t total = 0;

    std::string line;
    while ( std::getline( f, line ) )
    {
      ++total;
      auto lv = std::string_view{ line };

      auto idx = lv.find( ' ', 0 );
      if ( idx == std::string_view::npos )
      {
        LOG_INFO << "Ignoring invalid line " << lv;
        continue;
      }
      auto end = idx;

      auto vidx = lv.find( ' ', end + 1 );
      while ( vidx != std::string_view::npos && lv.substr( end + 1, vidx - end - 1 ).empty() )
      {
        ++end;
        vidx = lv.find( ' ', end + 1 );
      }

      LOG_INFO << "Creating key: " << lv.substr( 0, idx ) << "; value: " << lv.substr( end + 1 );
      kvs.emplace_back( lv.substr( 0, idx ), lv.substr( end + 1 ) );
    }

    f.close();
    if ( spt::configdb::db::set( kvs ) )
    {
      std::cout << "Seeded " << kvs.size() << '/' << total << " records." << std::endl;
    }
    else
    {
      std::cerr << "Error seeding database." << std::endl;
      return 4;
    }
  }
  catch ( const std::exception& ex )
  {
    std::cerr << "Error seeding data. " << ex.what() << std::endl;
  }


  return EXIT_SUCCESS;
}
