//
// Created by Rakesh on 29/12/2021.
//

#include "connection.h"
#include "../lib/log/NanoLog.h"
#include "../lib/model/request_generated.h"

#include <boost/asio/connect.hpp>

namespace spt::configdb::client::pconnection
{
  std::string server{};
  std::string port{};
}

void spt::configdb::client::init( std::string_view server, std::string_view port )
{
  pconnection::server.append( server.data(), server.size() );
  pconnection::port.append( port.data(), port.size() );
}

auto spt::configdb::client::create() -> std::unique_ptr<Connection>
{
  return std::make_unique<Connection>( pconnection::server, pconnection::port );
}

using spt::configdb::client::Connection;

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

auto Connection::list( std::string_view k ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{ 64 };
  auto key = fb.CreateString( k );
  auto value = fb.CreateString( "" );
  auto request = model::CreateRequest( fb, model::Action::List, key, value );
  fb.Finish( request );

  return write( fb, "List" );
}

auto Connection::get( std::string_view k ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto key = fb.CreateString( k );
  auto value = fb.CreateString( "" );
  auto request = model::CreateRequest( fb, model::Action::Get, key, value );
  fb.Finish( request );

  return write( fb, "Get" );
}

auto Connection::set( std::string_view k, std::string_view v ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto key = fb.CreateString( k );
  auto value = fb.CreateString( v );
  auto request = model::CreateRequest( fb, model::Action::Put, key, value );
  fb.Finish( request );

  return write( fb, "Set" );
}

auto Connection::remove( std::string_view k ) -> const model::Response*
{
  auto fb = flatbuffers::FlatBufferBuilder{};
  auto key = fb.CreateString( k );
  auto value = fb.CreateString( "" );
  auto request = model::CreateRequest( fb, model::Action::Delete, key, value );
  fb.Finish( request );

  return write( fb, "Delete" );
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
  LOG_INFO << "Read " << int(read) << " bytes, total size " << int(len);

  auto i = 0;
  while ( read < ( len + sizeof(len) ) )
  {
    LOG_INFO << "Iteration " << ++i;
    osize = s.receive( buffer.prepare( 256 ) );
    buffer.commit( osize );
    read += osize;
  }

  LOG_INFO << "Read " << int(read) << " bytes, total size " << int(len);
  const auto d1 = reinterpret_cast<const uint8_t*>( buffer.data().data() );
  auto verifier = flatbuffers::Verifier(  d1 + sizeof(len), len );
  auto ok = model::VerifyResponseBuffer( verifier );
  buffer.consume( buffer.size() );
  if ( !ok )
  {
    LOG_WARN << "Invalid buffer for " << context;
    return nullptr;
  }

  return model::GetResponse( d1 + sizeof(len) );
}
