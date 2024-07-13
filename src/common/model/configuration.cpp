//
// Created by Rakesh on 12/01/2022.
//

#include "configuration.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>

#include <boost/json.hpp>

using spt::configdb::model::Configuration;

namespace
{
  namespace pconf
  {
    std::mutex mutex;

    struct Holder
    {
      static Holder& instance()
      {
        static Holder h;
        return h;
      }

      ~Holder() = default;
      Holder(const Holder&) = delete;
      Holder& operator=(const Holder&) = delete;

      Configuration* conf() { return cfg.get(); }
      void assign( Configuration* c ) { cfg.reset( c ); }

    private:
      std::unique_ptr<Configuration> cfg{ nullptr };
      Holder() = default;
    };
  }
}

const Configuration& Configuration::instance()
{
  auto& holder = pconf::Holder::instance();
  auto cfg = holder.conf();
  if ( !cfg )
  {
    auto lck = std::unique_lock{ pconf::mutex };
    holder.assign( new Configuration );
    cfg = holder.conf();
  }
  return *cfg;
}

Configuration::Configuration()
{
  if ( const auto value = std::getenv( "CONFIG_DB_THREADS" ); value != nullptr )
  {
    threads = static_cast<uint32_t>( std::strtol( value, nullptr, 10 ) );
  }
  if ( const auto value = std::getenv( "CONFIG_DB_ENABLE_CACHE" ); value != nullptr ) enableCache = std::strcmp( value, "true" ) == 0;

  if ( const auto value = std::getenv( "CONFIG_DB_ENCRYPTION_SALT" ); value != nullptr ) encryption.salt = value;
  if ( const auto value = std::getenv( "CONFIG_DB_ENCRYPTION_KEY" ); value != nullptr ) encryption.key = value;
  if ( const auto value = std::getenv( "CONFIG_DB_ENCRYPTION_IV" ); value != nullptr ) encryption.iv = value;
  if ( const auto value = std::getenv( "CONFIG_DB_ENCRYPTION_SECRET" ); value != nullptr ) encryption.secret = value;
  if ( const auto value = std::getenv( "CONFIG_DB_ENCRYPTION_ROUNDS" ); value != nullptr )
  {
    encryption.rounds = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_LOGGING_LEVEL" ); value != nullptr )
  {
    if ( std::strcmp( value, "debug" ) == 0 || std::strcmp( value, "info" ) == 0 ||
      std::strcmp( value, "warn" ) == 0 || std::strcmp( value, "critical" ) == 0 ) logging.level = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_LOGGING_DIR" ); value != nullptr ) logging.level = value;
  if ( const auto value = std::getenv( "CONFIG_DB_LOGGING_CONSOLE" ); value != nullptr ) logging.console = std::strcmp( value, "true" ) == 0;

  if ( const auto value = std::getenv( "CONFIG_DB_SERVICES_HTTP" ); value != nullptr ) services.http = value;
  if ( const auto value = std::getenv( "CONFIG_DB_SERVICES_TCP" ); value != nullptr )
  {
    services.tcp = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }
  if ( const auto value = std::getenv( "CONFIG_DB_SERVICES_NOTIFY" ); value != nullptr )
  {
    services.notify = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS0_HOST" ); value != nullptr && peers.empty() )
  {
    peers.emplace_back();
    peers.front().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS0_PORT" ); value != nullptr && peers.size() == 1 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS1_HOST" ); value != nullptr && peers.size() == 1 )
  {
    peers.emplace_back();
    peers.back().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS1_PORT" ); value != nullptr && peers.size() == 2 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS2_HOST" ); value != nullptr && peers.size() == 2 )
  {
    peers.emplace_back();
    peers.back().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS2_PORT" ); value != nullptr && peers.size() == 3 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS3_HOST" ); value != nullptr && peers.size() == 3 )
  {
    peers.emplace_back();
    peers.back().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS3_PORT" ); value != nullptr && peers.size() == 4 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS4_HOST" ); value != nullptr && peers.size() == 4 )
  {
    peers.emplace_back();
    peers.back().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS4_PORT" ); value != nullptr && peers.size() == 5 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS5_HOST" ); value != nullptr && peers.size() == 5 )
  {
    peers.emplace_back();
    peers.back().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS5_PORT" ); value != nullptr && peers.size() == 6 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_PEERS6_HOST" ); value != nullptr && peers.size() == 6 )
  {
    peers.emplace_back();
    peers.back().host = value;
  }
  if ( const auto value = std::getenv( "CONFIG_DB_PEERS6_PORT" ); value != nullptr && peers.size() == 7 )
  {
    peers.back().port = static_cast<int>( std::strtol( value, nullptr, 10 ) );
  }

  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_DBPATH" ); value != nullptr ) storage.dbpath = value;
  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_BACKUP_PATH" ); value != nullptr ) storage.backupPath = value;
  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_BLOCK_CACHE_SIZE_MB" ); value != nullptr )
  {
    storage.blockCacheSizeMb = static_cast<uint64_t>( std::strtoll( value, nullptr, 10 ) );
  }
  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_CLEAN_EXPIRED_KEYS_INTERVAL" ); value != nullptr )
  {
    storage.cleanExpiredKeysInterval = static_cast<uint32_t>( std::strtoll( value, nullptr, 10 ) );
  }
  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_ENCRYPTER_INITIAL_POOL_SIZE" ); value != nullptr )
  {
    storage.encrypterInitialPoolSize = static_cast<uint32_t>( std::strtoll( value, nullptr, 10 ) );
  }
  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_MAX_BACKUPS" ); value != nullptr )
  {
    storage.maxBackups = static_cast<uint32_t>( std::strtoll( value, nullptr, 10 ) );
  }
  if ( const auto value = std::getenv( "CONFIG_DB_STORAGE_USE_MUTEX" ); value != nullptr ) storage.useMutex = std::strcmp( value, "true" ) == 0;
}


void Configuration::loadFromFile( const std::string& file )
{
  auto path = std::filesystem::path{ file };
  auto length = std::filesystem::file_size( path );
  std::string s;
  s.reserve( length );
  auto fs = std::ifstream{ file };
  s.assign( ( std::istreambuf_iterator<char>( fs ) ), ( std::istreambuf_iterator<char>() ) );
  fs.close();

  auto cfg = new Configuration;
  const auto jv = boost::json::parse( s );
  auto& obj = jv.as_object();

  auto v = obj.if_contains( "threads" );
  if ( v ) cfg->threads = static_cast<uint32_t>( v->as_int64() );

  v = obj.if_contains( "enableCache" );
  if ( v ) cfg->enableCache = v->as_bool();

  v = obj.if_contains( "encryption" );
  if ( v )
  {
    auto enc = v->as_object();

    v = enc.if_contains( "salt" );
    if ( v ) cfg->encryption.salt = v->as_string();

    v = enc.if_contains( "key" );
    if ( v ) cfg->encryption.key = v->as_string();

    v = enc.if_contains( "iv" );
    if ( v ) cfg->encryption.iv = v->as_string();

    v = enc.if_contains( "secret" );
    if ( v ) cfg->encryption.secret = v->as_string();

    v = enc.if_contains( "rounds" );
    if ( v ) cfg->encryption.rounds = static_cast<int>( v->as_int64() );
  }

  v = obj.if_contains( "logging" );
  if ( v )
  {
    auto log = v->as_object();

    v = log.if_contains( "level" );
    if ( v ) cfg->logging.level = v->as_string();

    v = log.if_contains( "dir" );
    if ( v ) cfg->logging.dir = v->as_string();

    v = log.if_contains( "console" );
    if ( v ) cfg->logging.console = v->as_bool();
  }

  v = obj.if_contains( "ssl" );
  if ( v )
  {
    auto ssl = v->as_object();

    v = ssl.if_contains( "caCertificate" );
    if ( v ) cfg->ssl.caCertificate = v->as_string();

    v = ssl.if_contains( "serverCertificate" );
    if ( v ) cfg->ssl.certificate = v->as_string();

    v = ssl.if_contains( "serverKey" );
    if ( v ) cfg->ssl.key = v->as_string();

    v = ssl.if_contains( "enable" );
    if ( v ) cfg->ssl.enable = v->as_bool();
  }

  v = obj.if_contains( "services" );
  if ( v )
  {
    auto svcs = v->as_object();

    v = svcs.if_contains( "http" );
    if ( v ) cfg->services.http = v->as_string();

    v = svcs.if_contains( "tcp" );
    if ( v ) cfg->services.tcp = static_cast<int>( v->as_int64() );

    v = svcs.if_contains( "notify" );
    if ( v ) cfg->services.notify = static_cast<int>( v->as_int64() );
  }

  v = obj.if_contains( "storage" );
  if ( v )
  {
    auto storage = v->as_object();

    v = storage.if_contains( "dbpath" );
    if ( v ) cfg->storage.dbpath = v->as_string();

    v = storage.if_contains( "backupPath" );
    if ( v ) cfg->storage.backupPath = v->as_string();

    v = storage.if_contains( "blockCacheSizeMb" );
    if ( v ) cfg->storage.blockCacheSizeMb = v->as_int64();

    v = storage.if_contains( "cleanExpiredKeysInterval" );
    if ( v ) cfg->storage.cleanExpiredKeysInterval = static_cast<uint32_t>( v->as_int64() );

    v = storage.if_contains( "encrypterInitialPoolSize" );
    if ( v ) cfg->storage.encrypterInitialPoolSize = static_cast<uint32_t>( v->as_int64() );

    v = storage.if_contains( "maxBackups" );
    if ( v ) cfg->storage.maxBackups = static_cast<uint32_t>( v->as_int64() );

    v = storage.if_contains( "useMutex" );
    if ( v ) cfg->storage.useMutex = v->as_bool();
  }

  v = obj.if_contains( "peers" );
  if ( v )
  {
    auto peers = v->as_array();
    cfg->peers.reserve( peers.size() );
    for ( auto&& e : peers )
    {
      auto o = e.as_object();
      auto peer = Peer{};

      auto h = o.if_contains( "host" );
      if ( h ) peer.host = h->as_string();

      h = o.if_contains( "port" );
      if ( h ) peer.port = static_cast<int>( h->as_int64() );

      if ( !peer.host.empty() && peer.port > 0 ) cfg->peers.push_back( std::move( peer ) );
    }
  }

  auto& holder = pconf::Holder::instance();
  auto lck = std::unique_lock{ pconf::mutex };
  holder.assign( cfg );
}

void Configuration::reset()
{
  auto lck = std::unique_lock{ pconf::mutex };
  pconf::Holder::instance().assign( new Configuration );
}