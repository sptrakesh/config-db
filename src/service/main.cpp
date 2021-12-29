//
// Created by Rakesh on 25/12/2021.
//

#include "contextholder.h"
#include "http/server.h"
#include "tcp/server.h"
#include "../lib/db/storage.h"
#include "../lib/log/NanoLog.h"
#include "../lib/util/clara.h"

#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio/signal_set.hpp>

int main( int argc, char const * const * argv )
{
  using clara::Opt;
  uint32_t threads = std::thread::hardware_concurrency();
#if __APPLE__
  std::string httpPort{ "6006" };
  int tcpPort{ 2022 };
#else
  std::string httpPort{ "6000" };
  int tcpPort{ 2020 };
#endif
  std::string logLevel{"info"};
  std::string dir{"logs/"};
  bool help = false;
  bool console = false;

  auto options = clara::Help(help) |
      Opt(console, "true")["-c"]["--console"]("Log to console (default false)") |
      Opt(httpPort, "6000")["-p"]["--http-port"]("Port on which to listen for http/2 traffic (default 6000)") |
      Opt(tcpPort, "2020")["-t"]["--tcp-port"]("Port on which to listen for tcp traffic (default 2020)") |
      Opt(threads, "8")["-n"]["--threads"]("Number of server threads to spawn (default system)") |
      Opt(logLevel, "info")["-l"]["--log-level"]("Log level to use [debug|info|warn|critical] (default info).") |
      Opt(dir, "logs/")["-o"]["--dir"]("Log directory (default logs/)");

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

  std::cout << "Starting server with options\n" <<
    "console: " << console << '\n' <<
    "http-port: " << httpPort << "\n" <<
    "tcp-port: " << tcpPort << "\n" <<
    "threads: " << threads << "\n" <<
    "logLevel: " << logLevel << '\n' <<
    "dir: " << dir << '\n';

  if ( logLevel == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( logLevel == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( logLevel == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( logLevel == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), dir, "config-db", console );

  spt::configdb::db::init();

  std::vector<std::thread> v;
  v.reserve( threads );
  v.emplace_back( [httpPort, threads]{ spt::configdb::http::start( httpPort, threads ); } );
  v.emplace_back( [tcpPort]{ spt::configdb::tcp::start( tcpPort ); } );

  boost::asio::signal_set signals( spt::configdb::ContextHolder::instance().ioc, SIGINT, SIGTERM );
  signals.async_wait( [&](auto const&, int ) { spt::configdb::ContextHolder::instance().ioc.stop(); } );

  const auto run = []
  {
    for (;;)
    {
      try
      {
        spt::configdb::ContextHolder::instance().ioc.run();
        break;
      }
      catch ( std::exception& e )
      {
        LOG_CRIT << "Unhandled exception " << e.what();
        spt::configdb::ContextHolder::instance().ioc.run();
      }
    }
  };

  for( auto i = std::thread::hardware_concurrency() - 2; i > 0; --i )
  {
    v.emplace_back( [&run] { run(); } );
  }

  run();

  for ( auto&& t : v )
  {
    if ( t.joinable() ) t.join();
  }
}

