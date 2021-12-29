//
// Created by Rakesh on 28/12/2021.
//

#include "connection.h"
#include "../../src/lib/log/NanoLog.h"
#include "../../src/lib/model/request_generated.h"

#include <boost/asio/connect.hpp>
#include <flatbuffers/minireflect.h>

using spt::configdb::itest::tcp::Connection;
using namespace std::string_view_literals;

Connection::Connection( boost::asio::io_context& ioc ) : s{ ioc }, resolver{ ioc }
{
  boost::asio::connect( s, resolver.resolve( "localhost", "2022" ) );
}

Connection::~Connection()
{
  if ( s.is_open() ) s.close();
}

auto Connection::write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) -> Tuple
{
  uint8_t bytes[4];
  auto n = fb.GetSize();
  bytes[0] = (n >> 24) & 0xFF;
  bytes[1] = (n >> 16) & 0xFF;
  bytes[2] = (n >> 8) & 0xFF;
  bytes[3] = n & 0xFF;

  std::ostream os{ &buffer };
  os.write( reinterpret_cast<const char*>( bytes ), 4 );
  os.write( reinterpret_cast<const char*>( fb.GetBufferPointer() ), fb.GetSize() );

  const auto isize = s.send( buffer.data() );
  buffer.consume( isize );

  auto osize = s.receive( buffer.prepare( 256 ) );
  buffer.commit( osize );
  std::size_t read = osize;

  if ( read < 5 )
  {
    LOG_WARN << "Invalid short response";
    return { nullptr, isize, read };
  }

  const auto d = reinterpret_cast<const uint8_t*>( buffer.data().data() );
  uint32_t len;
  memcpy( &len, d, sizeof(len) );

  auto i = 0;
  while ( read < ( len + sizeof(len) ) )
  {
    LOG_INFO << "Iteration " << ++i;
    osize = s.receive( buffer.prepare( 256 ) );
    buffer.commit( osize );
    read += osize;
  }

  auto verifier = flatbuffers::Verifier(  d + sizeof(len), len );
  auto ok = model::VerifyResponseBuffer( verifier );
  buffer.consume( buffer.size() );
  if ( !ok )
  {
    LOG_WARN << "Invalid buffer";
    return { nullptr, isize, read };
  }
  LOG_INFO << context << ' ' << flatbuffers::FlatBufferToString( d + sizeof(len), model::ResponseTypeTable() );
  return { model::GetResponse( d + sizeof(len) ), isize, read };
}

std::size_t Connection::noop()
{
  auto message = "noop"sv;
  std::ostream os{ &buffer };
  os.write( message.data(), message.size() );

  const auto isize = s.send( buffer.data() );
  buffer.consume( isize );

  auto osize = s.receive( buffer.prepare( 8 ) );
  buffer.commit( osize );
  return osize;
}


