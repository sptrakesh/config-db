//
// Created by Rakesh on 25/12/2021.
//

#include "contextholder.h"
#include "server.h"
#include "../lib/db/storage.h"
#include "../lib/log/NanoLog.h"
#include "../lib/model/request_generated.h"
#include "../lib/model/response_generated.h"

#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

using boost::asio::use_awaitable;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

namespace spt::configdb::tcp::coroutine
{
  boost::asio::awaitable<void> write( boost::asio::ip::tcp::socket& socket,
      const flatbuffers::FlatBufferBuilder& fb )
  {
    auto size = fb.GetSize();
    std::vector<boost::asio::const_buffer> buffers;
    buffers.emplace_back( &size, sizeof(size) );
    buffers.emplace_back( fb.GetBufferPointer(), size );
    co_await boost::asio::async_write( socket, buffers, use_awaitable );
  }

  boost::asio::awaitable<void> get( boost::asio::ip::tcp::socket& socket,
      const model::Request* request )
  {
    auto value = db::get( request->key()->string_view() );
    auto fb = flatbuffers::FlatBufferBuilder{};
    if ( value )
    {
      auto v = fb.CreateString( *value );
      auto vt = model::CreateValue( fb, v );
      auto r = model::CreateResponse( fb, model::ResponseValue::Value, vt.Union() );
      fb.Finish( r );
    }
    else
    {
      auto msg = fb.CreateString( "Key not found" );
      auto vt = model::CreateError( fb, true, msg );
      auto r = model::CreateResponse( fb, model::ResponseValue::Error, vt.Union() );
      fb.Finish( r );
    }

    co_await write( socket, fb );
  }

  boost::asio::awaitable<void> list( boost::asio::ip::tcp::socket& socket,
      const model::Request* request )
  {
    auto value = db::list( request->key()->string_view() );
    auto fb = flatbuffers::FlatBufferBuilder{};
    if ( value )
    {
      auto vt = model::CreateChildren( fb, fb.CreateVectorOfStrings( *value ) );
      auto r = model::CreateResponse( fb, model::ResponseValue::Children, vt.Union() );
      fb.Finish( r );
    }
    else
    {
      auto msg = fb.CreateString( "Path not found" );
      auto vt = model::CreateError( fb, true, msg );
      auto r = model::CreateResponse( fb, model::ResponseValue::Error, vt.Union() );
      fb.Finish( r );
    }

    co_await write( socket, fb );
  }

  boost::asio::awaitable<void> put( boost::asio::ip::tcp::socket& socket,
      const model::Request* request )
  {
    auto value = db::set( request->key()->string_view(), request->value()->string_view() );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vt = model::CreateSuccess( fb, value );
    auto r = model::CreateResponse( fb, model::ResponseValue::Success, vt.Union() );
    fb.Finish( r );
    co_await write( socket, fb );
  }

  boost::asio::awaitable<void> remove( boost::asio::ip::tcp::socket& socket,
      const model::Request* request )
  {
    auto value = db::remove( request->key()->string_view() );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vt = model::CreateSuccess( fb, value );
    auto r = model::CreateResponse( fb, model::ResponseValue::Success, vt.Union() );
    fb.Finish( r );
    co_await write( socket, fb );
  }

  boost::asio::awaitable<void> process( boost::asio::ip::tcp::socket& socket,
      const model::Request* request )
  {
    switch ( request->action() )
    {
    case model::Action::Get:
      co_await get( socket, request );
      break;
    case model::Action::List:
      co_await list( socket, request );
      break;
    case model::Action::Put:
      co_await put( socket, request );
      break;
    case model::Action::Delete:
      co_await remove( socket, request );
      break;
    }
  }

  boost::asio::awaitable<void> respond( boost::asio::ip::tcp::socket& socket )
  {
    static constexpr int bufSize = 128;
    static constexpr auto maxBytes = 8 * 1024 * 1024;
    uint8_t data[bufSize];

    const auto documentSize = [&data]( std::size_t length )
    {
      if ( length < 5 ) return length;

      const auto d = reinterpret_cast<const uint8_t*>( data );
      uint32_t len;
      memcpy( &len, d, sizeof(len) );
      return std::size_t( len + sizeof(len) );
    };

    std::size_t osize = co_await socket.async_read_some( boost::asio::buffer( data ), use_awaitable );
    const auto docSize = documentSize( osize );

    // echo, noop, ping etc.
    if ( docSize < 5 )
    {
      co_await boost::asio::async_write( socket, boost::asio::buffer( data, docSize ), use_awaitable );
      co_return;
    }

    if ( docSize <= bufSize )
    {
      auto request = model::GetRequest( data + sizeof(uint32_t) );
      co_await process( socket, request );
      co_return;
    }

    auto read = osize;
    std::vector<uint8_t> rbuf;
    rbuf.reserve( docSize - sizeof(uint32_t) );
    rbuf.insert( rbuf.end(), data + sizeof(uint32_t), data + osize );

    LOG_INFO << "Read " << int(osize) << " bytes, total size " << int(docSize);
    while ( docSize < maxBytes && read != docSize )
    {
      osize = co_await socket.async_read_some( boost::asio::buffer( data ), use_awaitable );
      rbuf.insert( rbuf.end(), data, data + osize );
      read += osize;
    }

    auto verifier = flatbuffers::Verifier( rbuf.data(), rbuf.size() );
    auto ok = model::VerifyRequestBuffer( verifier );
    if ( !ok )
    {
      LOG_WARN << "Invalid request buffer";
      co_await boost::asio::async_write( socket, boost::asio::buffer( rbuf.data(), rbuf.size() ), use_awaitable );
      co_return;
    }

    auto request = model::GetRequest( rbuf.data() );
    co_await process( socket, request );
  }

  boost::asio::awaitable<void> serve( boost::asio::ip::tcp::socket socket )
  {
    try
    {
      for (;;)
      {
        co_await respond( socket );
      }
    }
    catch ( const std::exception& e )
    {
      static const auto eof{ "End of file" };
      if ( !boost::algorithm::starts_with( e.what(), eof ) ) LOG_WARN << "Exception servicing request " << e.what();
    }
  }

  boost::asio::awaitable<void> listener( int port )
  {
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor( executor,
        { boost::asio::ip::tcp::v4(), static_cast<boost::asio::ip::port_type>( port ) } );
    for (;;)
    {
      if ( ContextHolder::instance().ioc.stopped() ) break;
      boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept( use_awaitable );
      boost::asio::co_spawn( executor, serve( std::move(socket) ), boost::asio::detached );
    }
  }
}

int spt::configdb::tcp::start( int port )
{
  namespace net = boost::asio;
  boost::asio::co_spawn( ContextHolder::instance().ioc, coroutine::listener( port ), boost::asio::detached );
  LOG_INFO << "TCP service started";

  ContextHolder::instance().ioc.run();
  LOG_INFO << "TCP service stopped";

  return 0;
}

