//
// Created by Rakesh on 29/12/2021.
//

#include "api.h"
#include "connection.h"
#include "../common/model/request_generated.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>
#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>

using spt::configdb::api::impl::BaseConnection;
using spt::configdb::api::impl::Connection;
using spt::configdb::api::impl::SSLConnection;

namespace spt::configdb::api::pconnection
{
  struct ApiSettings
  {
    static const ApiSettings& instance()
    {
      static ApiSettings s;
      return s;
    }

    std::mutex mutex{};
    std::string server{};
    std::string port{};
    boost::asio::io_context* ioc{ nullptr };
    bool ssl{ false };

    ~ApiSettings() = default;
    ApiSettings(const ApiSettings&) = delete;
    ApiSettings& operator=(const ApiSettings&) = delete;

  private:
    ApiSettings() = default;
  };
}

void spt::configdb::api::init( std::string_view server, std::string_view port, bool ssl,
    boost::asio::io_context& ioc )
{
  auto& s = const_cast<pconnection::ApiSettings&>( pconnection::ApiSettings::instance() );
  auto lock = std::unique_lock<std::mutex>( s.mutex );
  if ( s.server.empty() )
  {
    s.server.append( server.data(), server.size() );
    s.port.append( port.data(), port.size() );
    s.ioc = &ioc;
    s.ssl = ssl;
  }
  else
  {
    LOG_CRIT << "API init called multiple times.";
  }
}

auto spt::configdb::api::impl::create() -> std::unique_ptr<BaseConnection>
{
  auto& s = pconnection::ApiSettings::instance();
  if ( s.ssl )
  {
    return std::make_unique<SSLConnection>( s.server, s.port );
  }
  return std::make_unique<Connection>( s.server, s.port );
}

Connection::Connection( std::string_view server, std::string_view port ) :
  s{ *pconnection::ApiSettings::instance().ioc }, resolver{ *pconnection::ApiSettings::instance().ioc }
{
  boost::asio::connect( s, resolver.resolve( server, port ) );
}

Connection::~Connection()
{
  if ( s.is_open() )
  {
    boost::system::error_code ec;
    s.close( ec );
    if ( ec ) LOG_CRIT << "Error closing socket. " << ec.message();
  }
}

SSLConnection::SSLConnection( std::string_view server, std::string_view port ) : BaseConnection(),
  s{ *pconnection::ApiSettings::instance().ioc, ctx }, resolver{ *pconnection::ApiSettings::instance().ioc }
{
  s.set_verify_mode( boost::asio::ssl::verify_none );
  auto endpoints = resolver.resolve( server, port );
  boost::asio::connect( s.lowest_layer(), endpoints );
  boost::system::error_code ec;
  s.handshake( boost::asio::ssl::stream_base::client, ec );
  if ( ec )
  {
    LOG_CRIT << "Error during SSL handshake. " << ec.message();
    invalid();
  }
}

SSLConnection::~SSLConnection()
{
  if ( s.next_layer().is_open() )
  {
    boost::system::error_code ec;
    s.next_layer().close( ec );
    if ( ec ) LOG_CRIT << "Error closing socket. " << ec.message();
  }
}

boost::asio::ssl::context SSLConnection::createContext()
{
  auto ctx = boost::asio::ssl::context( boost::asio::ssl::context::tlsv13_client );

#ifdef __APPLE__
  auto searchPaths = std::vector<std::string>{ "../../../certs",
      "/usr/local/spt/certs", "/opt/spt/certs",
      "/usr/local/config-db", "/usr/local/configdb",
      "/opt/config-db", "/opt/configdb" };
  for ( auto&& path : searchPaths )
  {
    auto file = std::string{};
    file.reserve( 64 );
    file.append( path ).append( "/ca.crt" );
    auto p = std::filesystem::path{ file };
    if ( std::filesystem::exists( p ) )
    {
      LOG_INFO << "Loading certificates from " << path;
      ctx.load_verify_file( file );

      file.clear();
      file.append( path ).append( "/client.crt" );
      ctx.use_certificate_file( file, boost::asio::ssl::context::pem );

      file.clear();
      file.append( path ).append( "/client.key" );
      ctx.use_private_key_file( file, boost::asio::ssl::context::pem );
    }
  }
#else
  ctx.load_verify_file( "/opt/spt/certs/ca.crt" );
  ctx.use_certificate_file( "/opt/spt/certs/client.crt", boost::asio::ssl::context::pem );
  ctx.use_private_key_file( "/opt/spt/certs/client.key", boost::asio::ssl::context::pem );
#endif

  ctx.set_options( boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::single_dh_use );
  ctx.set_verify_mode( boost::asio::ssl::verify_peer );
  return ctx;
}

auto BaseConnection::list( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::List, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "List" );
}

auto BaseConnection::get( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::Get, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Get" );
}

auto BaseConnection::set( const model::RequestData& data ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto opts = model::CreateOptions( fb, data.options.ifNotExists, data.options.expirationInSeconds, data.options.cache );
  auto vec = std::vector{
    model::CreateKeyValue( fb, fb.CreateString( data.key ), fb.CreateString( data.value ), opts ) };
  auto request = model::CreateRequest( fb, model::Action::Put, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Set" );
}

auto BaseConnection::remove( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::Delete, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Delete" );
}

auto BaseConnection::move( const model::RequestData& data ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto opts = model::CreateOptions( fb, data.options.ifNotExists, data.options.expirationInSeconds, data.options.cache );
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{
      model::CreateKeyValue( fb, fb.CreateString( data.key ), fb.CreateString( data.value ), opts ) };
  auto request = CreateRequest( fb, model::Action::Move, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Move" );
}

auto BaseConnection::list( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::List, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MList" );
}

auto BaseConnection::ttl( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::TTL, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "TTL" );
}

auto BaseConnection::get( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::Get, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MGet" );
}

auto BaseConnection::set( const std::vector<model::RequestData>& kvs ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( kvs.size() );
  for ( auto&& data : kvs )
  {
    auto opts = model::CreateOptions( fb, data.options.ifNotExists, data.options.expirationInSeconds, data.options.cache );
    vec.push_back( model::CreateKeyValue( fb, fb.CreateString( data.key ), fb.CreateString( data.value ), opts ) );
  }
  auto request = CreateRequest( fb, model::Action::Put, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MSet" );
}

auto BaseConnection::remove( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::Delete, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MDelete" );
}

auto BaseConnection::move( const std::vector<model::RequestData>& kvs ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( kvs.size() );
  for ( auto&& data : kvs )
  {
    auto opts = model::CreateOptions( fb, data.options.ifNotExists, data.options.expirationInSeconds, data.options.cache );
    vec.push_back( model::CreateKeyValue( fb, fb.CreateString( data.key ), fb.CreateString( data.value ), opts ) );
  }
  auto request = CreateRequest( fb, model::Action::Move, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MMove" );
}

auto BaseConnection::ttl( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::TTL, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MTTL" );
}

auto BaseConnection::import( const std::string& file ) -> ImportResponse
{
  auto f = std::fstream{ file, std::ios_base::in };
  if ( ! f.is_open() )
  {
    LOG_WARN << "Error opening file " << file;
    return { nullptr, 0, 0 };
  }

  uint32_t count = 0;
  uint32_t total = 0;

  auto kvs = std::vector<model::RequestData>{};
  kvs.reserve( 64 );

  std::string line;
  while ( std::getline( f, line ) )
  {
    ++total;
    auto lv = std::string_view{ line };

    auto idx = lv.find( ' ', 0 );
    if ( idx == std::string_view::npos )
    {
      LOG_WARN << "Ignoring invalid line " << lv;
      continue;
    }
    auto end = idx;

    auto vidx = lv.find( ' ', end + 1 );
    while ( vidx != std::string_view::npos && lv.substr( end + 1, vidx - end - 1 ).empty() )
    {
      ++end;
      vidx = lv.find( ' ', end + 1 );
    }

    ++count;
    LOG_INFO << "Creating key: " << lv.substr( 0, idx ) << "; value: " << lv.substr( end + 1 );
    kvs.emplace_back( lv.substr( 0, idx ), lv.substr( end + 1 ) );
  }

  f.close();

  return { set( kvs ), count, total };
}

void Connection::socket()
{
  if ( s.is_open() ) return;

  LOG_DEBUG << "Re-opening closed connection.";
  auto& settings = pconnection::ApiSettings::instance();
  boost::system::error_code ec;
  boost::asio::connect( s, resolver.resolve( settings.server, settings.port ), ec );
  if ( ec )
  {
    LOG_WARN << "Error opening socket connection. " << ec.message();
    invalid();
    return;
  }
  boost::asio::socket_base::keep_alive option( true );
  s.set_option( option );
}

void SSLConnection::socket()
{
  if ( s.next_layer().is_open() ) return;

  LOG_DEBUG << "Re-opening closed connection.";
  auto& settings = pconnection::ApiSettings::instance();
  boost::system::error_code ec;
  boost::asio::connect( s.next_layer(), resolver.resolve( settings.server, settings.port ), ec );
  if ( ec )
  {
    LOG_WARN << "Error opening socket connection. " << ec.message();
    invalid();
    return;
  }
  boost::asio::socket_base::keep_alive option( true );
  s.next_layer().set_option( option );
}

auto Connection::write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) -> const model::Response*
{
  return writeImpl( fb, context, s );
}

auto SSLConnection::write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) -> const model::Response*
{
  return writeImpl( fb, context, s );
}
