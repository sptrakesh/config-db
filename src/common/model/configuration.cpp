//
// Created by Rakesh on 12/01/2022.
//

#include "configuration.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>

#include <boost/json.hpp>

using spt::configdb::model::Configuration;

namespace spt::configdb::model::pconf
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
  if ( v ) cfg->threads = v->as_int64();

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
    if ( v ) cfg->encryption.rounds = v->as_int64();
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
    if ( v ) cfg->services.tcp = v->as_int64();

    v = svcs.if_contains( "notify" );
    if ( v ) cfg->services.notify = v->as_int64();
  }

  v = obj.if_contains( "storage" );
  if ( v )
  {
    auto storage = v->as_object();

    v = storage.if_contains( "dbpath" );
    if ( v ) cfg->storage.dbpath = v->as_string();

    v = storage.if_contains( "blockCacheSizeMb" );
    if ( v ) cfg->storage.blockCacheSizeMb = v->as_int64();

    v = storage.if_contains( "cleanExpiredKeysInterval" );
    if ( v ) cfg->storage.cleanExpiredKeysInterval = v->as_int64();

    v = storage.if_contains( "encrypterInitialPoolSize" );
    if ( v ) cfg->storage.encrypterInitialPoolSize = v->as_int64();
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
      if ( h ) peer.port = h->as_int64();

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