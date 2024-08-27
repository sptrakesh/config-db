//
// Created by Rakesh on 25/12/2021.
//

#include "client/client.hpp"
#include "../api/api.hpp"
#include "../common/util/clara.hpp"
#include "../log/NanoLog.hpp"

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio/signal_set.hpp>

int main( int argc, char const * const * argv )
{
  using namespace std::string_literals;
  using clara::Opt;
  std::string server{ "localhost" };
#ifdef __APPLE__
  std::string port{ "2022" };
  std::string logLevel{ "debug" };
#else
  std::string port{ "2020" };
  std::string logLevel{"info"};
#endif
  std::string dir{"/tmp/"};
  std::string action{};
  std::string key{};
  std::string value{};
  std::string file{};
  bool ssl = false;
  bool help = false;

  auto options = clara::Help(help) |
      Opt(server, "localhost")["-s"]["--server"]("Server to connect to (default localhost).") |
      Opt(port, "2020")["-p"]["--port"]("TCP port for the server (default 2020)") |
      Opt(ssl)["-t"]["--with-ssl"]("Use SSL to connect to service (default false)") |
      Opt(file, "file.txt")["-f"]["--file"]("File to bulk import data from") |
      Opt(action, "set")["-a"]["--action"]("Action to perform against the database.") |
      Opt(key, "/key")["-k"]["--key"]("Key or path to apply action to") |
      Opt(value, "value")["-v"]["--value"]("Value to set.  Only applies to 'set' action") |
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

  if ( logLevel == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( logLevel == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( logLevel == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( logLevel == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), dir, "configdb-cli", false );

  boost::asio::signal_set signals( spt::configdb::ContextHolder::instance().ioc, SIGINT, SIGTERM );
  signals.async_wait( [&](auto const&, int ) { spt::configdb::ContextHolder::instance().ioc.stop(); } );

  std::vector<std::thread> v;
  v.reserve( 1 );
  v.emplace_back( []{ spt::configdb::ContextHolder::instance().ioc.run(); } );
  int response = EXIT_SUCCESS;

  try
  {
    spt::configdb::api::init( server, port, ssl );
    if ( file.empty() )
    {
      if ( action.empty() )
      {
        std::cout << "Action not specified" << '\n';
        options.writeToStream( std::cout );
        exit( 1 );
      }

      const std::vector<std::string> actions{ "delete"s ,"get"s, "list"s, "move"s, "set"s };
      if ( std::find( std::begin( actions ), std::end( actions ), action ) == std::end( actions ) )
      {
        std::cout << "Unsupported action" << '\n';
        std::cout << "Valid values get|set|move|list|delete" << '\n';
        exit( 1 );
      }

      if ( key.empty() )
      {
        std::cout << "Key not specified" << '\n';
        options.writeToStream( std::cout );
        exit( 1 );
      }

      if ( action == "set"s && value.empty() )
      {
        std::cout << "Value not specified" << '\n';
        options.writeToStream( std::cout );
        exit( 1 );
      }

      if ( action == "move"s && value.empty() )
      {
        std::cout << "Destination key not specified" << '\n';
        options.writeToStream( std::cout );
        exit( 1 );
      }

      if ( action == "get"s ) response = spt::configdb::client::get( server, port, key, ssl );
      else if ( action == "list"s ) response = spt::configdb::client::list( server, port, key, ssl );
      else if ( action == "set"s ) response = spt::configdb::client::set( server, port, key, value, ssl );
      else if ( action == "move"s ) response = spt::configdb::client::move( server, port, key, value, ssl );
      else if ( action == "delete"s ) response = spt::configdb::client::remove( server, port, key, ssl );
    }
    else
    {
      response = spt::configdb::client::import( server, port, file, ssl );
    }
  }
  catch ( const std::exception& ex )
  {
    std::cerr << "Error communicating with server. " << ex.what() << '\n';
  }

  spt::configdb::ContextHolder::instance().ioc.stop();

  for ( auto&& t : v )
  {
    if ( t.joinable() ) t.join();
  }

  return response;
}
