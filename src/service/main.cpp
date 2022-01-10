//
// Created by Rakesh on 25/12/2021.
//

#include "http/server.h"
#include "tcp/server.h"
#include "../common/contextholder.h"
#include "../common/log/NanoLog.h"
#include "../common/util/clara.h"
#include "../lib/db/storage.h"
#include "../lib/model/configuration.h"

#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio/signal_set.hpp>

int main( int argc, char const * const * argv )
{
  using clara::Opt;
  auto& conf = spt::configdb::model::Configuration::instance();
#if __APPLE__
  std::string httpPort{ "6006" };
  int tcpPort{ 2022 };
#else
  std::string httpPort{ "6000" };
  int tcpPort{ 2020 };
#endif
  bool ssl = false;
  bool help = false;

  auto options = clara::Help(help) |
      Opt(conf.logging.console, "true")["-c"]["--console"]("Log to console (default false)") |
      Opt(httpPort, "6000")["-p"]["--http-port"]("Port on which to listen for http/2 traffic (default 6000)") |
      Opt(tcpPort, "2020")["-t"]["--tcp-port"]("Port on which to listen for tcp traffic (default 2020)") |
      Opt(ssl, "true")["-s"]["--with-ssl"]("Enable SSL wrappers for services (default false)") |
      Opt(conf.threads, "8")["-n"]["--threads"]("Number of server threads to spawn (default system)") |
      Opt(conf.encryption.secret, "AESEncryptionKey")["-e"]["--encryption-secret"]("Secret to use to encrypt values (default internal)") |
      Opt(conf.enableCache, "true")["-x"]["--enable-cache"]("Enable temporary cache for key values (default false)") |
      Opt(conf.logging.level, "info")["-l"]["--log-level"]("Log level to use [debug|info|warn|critical] (default info).") |
      Opt(conf.logging.dir, "logs/")["-o"]["--log-dir"]("Log directory (default logs/)");

  auto result = options.parse( clara::Args( argc, argv ) );
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
    "console: " << conf.logging.console << '\n' <<
    "http-port: " << httpPort << "\n" <<
    "tcp-port: " << tcpPort << "\n" <<
    "threads: " << conf.threads << "\n" <<
    "logLevel: " << conf.logging.level << '\n' <<
    "dir: " << conf.logging.dir << '\n';

  if ( conf.logging.level == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( conf.logging.level == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( conf.logging.level == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( conf.logging.level == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), conf.logging.dir, "config-db", conf.logging.console );

  spt::configdb::db::init();

  std::vector<std::thread> v;
  v.reserve( conf.threads );
  v.emplace_back( [httpPort, &conf, ssl]{ spt::configdb::http::start( httpPort, conf.threads, ssl ); } );
  v.emplace_back( [tcpPort, ssl]{ spt::configdb::tcp::start( tcpPort, ssl ); } );

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

  for( auto i = conf.threads - 2; i > 0; --i )
  {
    v.emplace_back( [&run] { run(); } );
  }

  run();

  for ( auto&& t : v )
  {
    if ( t.joinable() ) t.join();
  }
}

