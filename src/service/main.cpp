//
// Created by Rakesh on 25/12/2021.
//

#include "clearexpired.h"
#include "http/server.h"
#include "tcp/server.h"
#include "../common/contextholder.h"
#include "../common/model/configuration.h"
#include "../common/util/clara.h"
#include "../common/util/split.h"
#include "../lib/db/storage.h"
#include "../log/NanoLog.h"

#include <iostream>
#include <thread>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/json/src.hpp>

void loadConfig( int argc, char const * const * argv )
{
  using clara::Opt;
  auto& conf = const_cast<spt::configdb::model::Configuration&>( spt::configdb::model::Configuration::instance() );
  std::string file{};
  std::string peers{};
  bool help = false;

  auto options = clara::Help(help) |
      Opt(file, "/tmp/configdb.json")["-f"]["--config-file"]("The JSON configuration file to load.  All other options are ignored.") |
      Opt(peers, "confdb1:2020,confdb2:2020")["-z"]["--peers"]("Comma separated list of peers to listen to for notifications.") |
      Opt(conf.logging.console)["-c"]["--console"]("Log to console (default false)") |
      Opt(conf.services.http, "6000")["-p"]["--http-port"]("Port on which to listen for http/2 traffic.  Specify 0 to disable (default 6020)") |
      Opt(conf.services.tcp, "2020")["-t"]["--tcp-port"]("Port on which to listen for tcp traffic (default 2020)") |
      Opt(conf.services.notify, "2120")["-b"]["--notify-port"]("Port on which to publish notifications (default 2120)") |
      Opt(conf.ssl.enable)["-s"]["--with-ssl"]("Enable SSL wrappers for services (default false)") |
      Opt(conf.threads, "8")["-n"]["--threads"]("Number of server threads to spawn (default system)") |
      Opt(conf.encryption.secret, "AESEncryptionKey")["-e"]["--encryption-secret"]("Secret to use to encrypt values (default internal)") |
      Opt(conf.enableCache)["-x"]["--enable-cache"]("Enable temporary cache for key values (default false)") |
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

  if ( !file.empty() )
  {
    std::cout << "Loading configuration file " << file << std::endl;
    spt::configdb::model::Configuration::loadFromFile( file );
  }

  if ( !peers.empty() )
  {
    auto p = spt::util::split( peers );
    conf.peers.reserve( p.size() );
    for ( auto&& pv : p )
    {
      auto parts = spt::util::split( pv, 2, ":" );
      if ( parts.size() == 2 )
      {
        conf.peers.emplace_back( parts[0], boost::lexical_cast<int>( parts[1] ) );
      }
    }
  }
}

int main( int argc, char const * const * argv )
{
  using namespace std::string_literals;
  loadConfig( argc, argv );
  auto& conf = spt::configdb::model::Configuration::instance();

  std::cout << "Starting server with options\n" << std::boolalpha <<
    "console: " << conf.logging.console << '\n' <<
    "ssl: " << conf.ssl.enable << '\n' <<
    "http-port: " << conf.services.http << "\n" <<
    "tcp-port: " << conf.services.tcp << "\n" <<
    "threads: " << conf.threads << "\n" <<
    "logLevel: " << conf.logging.level << '\n' <<
    "dir: " << conf.logging.dir << '\n';

  if ( conf.logging.level == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( conf.logging.level == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( conf.logging.level == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( conf.logging.level == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), conf.logging.dir, "config-db", conf.logging.console );

  spt::configdb::db::init();
  auto cleaner = spt::configdb::service::ClearExpired{};
  spt::configdb::tcp::start( conf.services.tcp, conf.ssl.enable );

  std::vector<std::thread> v;
  v.reserve( conf.threads + 2 );
  if ( conf.services.http != "0"s )
  {
    v.emplace_back( [&conf]{ spt::configdb::http::start( conf.services.http, conf.threads, conf.ssl.enable ); } );
  }
  v.emplace_back( [&cleaner]{ cleaner.run(); } );

  if ( !conf.peers.empty() )
  {
    spt::configdb::tcp::notifier( conf.services.notify, conf.ssl.enable );

    for ( auto&& p : conf.peers )
    {
      spt::configdb::tcp::listen( p.host, std::to_string( p.port ), conf.ssl.enable );
    }
  }

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

  for( auto i = conf.threads; i > 0; --i )
  {
    v.emplace_back( [&run] { run(); } );
  }

  run();
  cleaner.stop();

  for ( auto&& t : v ) if ( t.joinable() ) t.join();
}

