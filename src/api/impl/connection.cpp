//
// Created by Rakesh on 29/12/2021.
//

#include "api.h"
#include "connection.h"
#include "../common/log/NanoLog.h"
#include "../common/model/request_generated.h"

#include <fstream>
#include <mutex>
#include <boost/asio/connect.hpp>

using spt::configdb::api::impl::Connection;

namespace spt::configdb::api::pconnection
{
  std::mutex mutex{};
  std::string server{};
  std::string port{};
}

void spt::configdb::api::init( std::string_view server, std::string_view port )
{
  auto lock = std::unique_lock<std::mutex>( pconnection::mutex );
  if ( pconnection::server.empty() )
  {
    pconnection::server.append( server.data(), server.size() );
    pconnection::port.append( port.data(), port.size() );
  }
  else
  {
    LOG_CRIT << "API init called multiple times.";
  }
}

auto spt::configdb::api::impl::create() -> std::unique_ptr<Connection>
{
  return std::make_unique<Connection>( pconnection::server, pconnection::port );
}

Connection::Connection( std::string_view server, std::string_view port )
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

Connection::~Connection()
{
  if ( s.next_layer().is_open() )
  {
    boost::system::error_code ec;
    s.next_layer().close( ec );
    if ( ec ) LOG_CRIT << "Error closing socket. " << ec.message();
  }
}

boost::asio::ssl::context Connection::createContext()
{
  auto ctx = boost::asio::ssl::context( boost::asio::ssl::context::tlsv12_client );

#ifdef __APPLE__
  ctx.load_verify_file( "../../../certs/ca.crt" );
  ctx.use_certificate_file( "../../../certs/client.crt", boost::asio::ssl::context::pem );
  ctx.use_private_key_file( "../../../certs/client.key", boost::asio::ssl::context::pem );
#else
  ctx.load_verify_file( "/opt/spt/certs/ca.crt" );
  ctx.use_certificate_file( "/opt/spt/certs/client.crt", boost::asio::ssl::context::pem );
  ctx.use_private_key_file( "/opt/spt/certs/client.key", boost::asio::ssl::context::pem );
#endif

  ctx.set_options( boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::single_dh_use );
  ctx.set_verify_mode( boost::asio::ssl::verify_peer );
  return ctx;
}

auto Connection::list( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::List, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "List" );
}

auto Connection::get( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::Get, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Get" );
}

auto Connection::set( std::string_view key, std::string_view value ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{
    model::CreateKeyValue( fb, fb.CreateString( key ), fb.CreateString( value ) ) };
  auto request = CreateRequest( fb, model::Action::Put, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Set" );
}

auto Connection::remove( std::string_view key ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{ model::CreateKeyValue( fb, fb.CreateString( key ) ) };
  auto request = CreateRequest( fb, model::Action::Delete, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Delete" );
}

auto Connection::move( std::string_view key, std::string_view dest ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{
      model::CreateKeyValue( fb, fb.CreateString( key ), fb.CreateString( dest ) ) };
  auto request = CreateRequest( fb, model::Action::Move, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "Move" );
}

auto Connection::list( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::List, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MList" );
}

auto Connection::get( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::Get, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MGet" );
}

auto Connection::set( const std::vector<Pair>& kvs ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( kvs.size() );
  for ( auto&& [key, value] : kvs ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ), fb.CreateString( value ) ) );
  auto request = CreateRequest( fb, model::Action::Put, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MSet" );
}

auto Connection::remove( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::Delete, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MDelete" );
}

auto Connection::move( const std::vector<Pair>& kvs ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( kvs.size() );
  for ( auto&& [key, value] : kvs ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ), fb.CreateString( value ) ) );
  auto request = CreateRequest( fb, model::Action::Move, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MMove" );
}

auto Connection::import( const std::string& file ) -> ImportResponse
{
  auto f = std::fstream{ file };
  if ( ! f.is_open() )
  {
    LOG_WARN << "Error opening file " << file;
    return { nullptr, 0, 0 };
  }

  uint32_t count = 0;
  auto lines = std::vector<std::string>{};
  lines.reserve( 64 );

  auto kvs = std::vector<spt::configdb::api::Pair>{};
  kvs.reserve( lines.size() );

  std::string line;
  while ( std::getline( f, line ) )
  {
    ++count;
    lines.push_back( line );
    auto lv = std::string_view{ lines.back() };

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

    LOG_INFO << "Creating key: " << lv.substr( 0, idx ) << "; value: " << lv.substr( end + 1 );
    kvs.emplace_back( lv.substr( 0, idx ), lv.substr( end + 1 ) );
  }

  f.close();

  return { set( kvs ), lines.size(), count };
}

boost::asio::ip::tcp::socket& Connection::socket()
{
  if ( ! s.next_layer().is_open() )
  {
    LOG_DEBUG << "Re-opening closed connection.";
    boost::system::error_code ec;
    boost::asio::connect( s.next_layer(), resolver.resolve( pconnection::server, pconnection::port ), ec );
    if ( ec )
    {
      LOG_WARN << "Error opening socket connection. " << ec.message();
      invalid();
      return s.next_layer();
    }
    boost::asio::socket_base::keep_alive option( true );
    s.next_layer().set_option( option );
  }

  return s.next_layer();
}

auto Connection::write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) -> const model::Response*
{
  if ( !valid() )
  {
    LOG_WARN << "Connection not valid.";
    return nullptr;
  }

  auto n = fb.GetSize();
  std::ostream os{ &buffer };
  os.write( reinterpret_cast<const char*>( &n ), sizeof(flatbuffers::uoffset_t) );
  os.write( reinterpret_cast<const char*>( fb.GetBufferPointer() ), fb.GetSize() );

  const auto isize = s.next_layer().send( buffer.data() );
  buffer.consume( isize );

  auto osize = s.next_layer().receive( buffer.prepare( 256 ) );
  buffer.commit( osize );
  std::size_t read = osize;

  if ( read < 5 )
  {
    LOG_WARN << "Invalid short response for " << context;
    return nullptr;
  }

  const auto d = reinterpret_cast<const uint8_t*>( buffer.data().data() );
  uint32_t len;
  memcpy( &len, d, sizeof(len) );
  LOG_DEBUG << "Read " << int(read) << " bytes, total size " << int(len);

  auto i = 0;
  while ( read < ( len + sizeof(len) ) )
  {
    LOG_DEBUG << "Iteration " << ++i;
    osize = s.next_layer().receive( buffer.prepare( 256 ) );
    buffer.commit( osize );
    read += osize;
  }

  LOG_DEBUG << "Read " << int(read) << " bytes, total size " << int(len);
  const auto d1 = reinterpret_cast<const uint8_t*>( buffer.data().data() );
  auto verifier = flatbuffers::Verifier(  d1 + sizeof(len), len );
  auto ok = model::VerifyResponseBuffer( verifier );
  buffer.consume( buffer.size() );

  if ( !ok )
  {
    LOG_WARN << "Invalid buffer for " << context;
    invalid();
    return nullptr;
  }

  return model::GetResponse( d1 + sizeof(len) );
}
