//
// Created by Rakesh on 29/12/2021.
//

#include "api.h"
#include "connection.h"
#include "../common/log/NanoLog.h"
#include "../lib/model/request_generated.h"

#include <mutex>
#include <boost/asio/connect.hpp>

using spt::configdb::api::impl::Connection;

namespace spt::configdb::api::pconnection
{
  std::mutex mutex;
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

auto Connection::mlist( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::List, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MList" );
}

auto Connection::mget( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::Get, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MGet" );
}

auto Connection::mset( const std::vector<Pair>& kvs ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( kvs.size() );
  for ( auto&& [key, value] : kvs ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ), fb.CreateString( value ) ) );
  auto request = CreateRequest( fb, model::Action::Put, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MSet" );
}

auto Connection::mremove( const std::vector<std::string_view>& keys ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto vec = std::vector<flatbuffers::Offset<model::KeyValue>>{};
  vec.reserve( keys.size() );
  for ( auto&& key : keys ) vec.push_back( model::CreateKeyValue( fb, fb.CreateString( key ) ) );
  auto request = CreateRequest( fb, model::Action::Delete, fb.CreateVector( vec ) );
  fb.Finish( request );

  return write( fb, "MDelete" );
}

boost::asio::ip::tcp::socket& Connection::socket()
{
  if ( ! s.is_open() )
  {
    LOG_DEBUG << "Re-opening closed connection.";
    boost::system::error_code ec;
    boost::asio::connect( s, resolver.resolve( pconnection::server, pconnection::port ), ec );
    if ( ec )
    {
      LOG_WARN << "Error opening socket connection. " << ec.message();
      invalid();
      return s;
    }
    boost::asio::socket_base::keep_alive option( true );
    s.set_option( option );
  }

  return s;
}

auto Connection::write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) -> const model::Response*
{
  auto n = fb.GetSize();
  std::ostream os{ &buffer };
  os.write( reinterpret_cast<const char*>( &n ), sizeof(flatbuffers::uoffset_t) );
  os.write( reinterpret_cast<const char*>( fb.GetBufferPointer() ), fb.GetSize() );

  const auto isize = s.send( buffer.data() );
  buffer.consume( isize );

  auto osize = s.receive( buffer.prepare( 256 ) );
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
    osize = s.receive( buffer.prepare( 256 ) );
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
