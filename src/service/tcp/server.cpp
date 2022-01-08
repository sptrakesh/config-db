//
// Created by Rakesh on 25/12/2021.
//

#include "server.h"
#include "../common/contextholder.h"
#include "../common/log/NanoLog.h"
#include "../common/model/request_generated.h"
#include "../common/model/response_generated.h"
#include "../lib/db/crud.h"

#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

using SecureSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

namespace spt::configdb::tcp::coroutine
{
  template <typename Socket>
  boost::asio::awaitable<void> write( Socket& socket, const flatbuffers::FlatBufferBuilder& fb )
  {
    auto size = fb.GetSize();
    std::vector<boost::asio::const_buffer> buffers;
    buffers.reserve( 2 );
    buffers.emplace_back( &size, sizeof(size) );
    buffers.emplace_back( fb.GetBufferPointer(), size );
    boost::system::error_code ec;
    co_await boost::asio::async_write( socket, buffers,
        boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
    if ( ec ) LOG_WARN << "Error writing to socket. " << ec.message();
    else LOG_DEBUG << "Finished writing buffer of size " << int(size) << " + " << int(sizeof(size)) << " bytes";
  }

  template <typename Socket>
  boost::asio::awaitable<void> get( Socket& socket, const model::Request* request )
  {
    std::vector<std::string_view> keys;
    for ( auto&& kv : *request->data() ) keys.emplace_back( kv->key()->string_view() );

    auto value = db::get( keys );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vec = std::vector<flatbuffers::Offset<model::KeyValueResult>>{};

    if ( !value.empty() )
    {
      vec.reserve( value.size() );
      for ( auto&& [k, v] : value )
      {
        if ( v )
        {
          auto vt = model::CreateValue( fb, fb.CreateString( *v ) );
          vec.push_back( model::CreateKeyValueResult(
              fb, fb.CreateString( k ), model::ValueVariant::Value, vt.Union() ) );
        }
        else
        {
          auto vt = model::CreateSuccess( fb, false );
          vec.push_back( model::CreateKeyValueResult(
              fb, fb.CreateString( k ), model::ValueVariant::Success, vt.Union() ) );
        }
      }
    }

    auto rv = model::CreateKeyValueResults( fb, fb.CreateVector( vec ) );
    auto r = model::CreateResponse( fb, model::ResultVariant::KeyValueResults, rv.Union() );
    fb.Finish( r );

    co_await write( socket, fb );
  }

  template <typename Socket>
  boost::asio::awaitable<void> list( Socket& socket, const model::Request* request )
  {
    std::vector<std::string_view> keys;
    for ( auto&& kv : *request->data() ) keys.emplace_back( kv->key()->string_view() );

    auto value = db::list( keys );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vec = std::vector<flatbuffers::Offset<model::KeyValueResult>>{};

    if ( !value.empty() )
    {
      vec.reserve( value.size() );
      for ( auto&& [k, v] : value )
      {
        if ( v )
        {
          auto vt = model::CreateChildren( fb, fb.CreateVectorOfStrings( *v ) );
          vec.push_back( model::CreateKeyValueResult(
              fb, fb.CreateString( k ), model::ValueVariant::Children, vt.Union() ) );
        }
        else
        {
          auto vt = model::CreateSuccess( fb, false );
          vec.push_back( model::CreateKeyValueResult(
              fb, fb.CreateString( k ), model::ValueVariant::Success, vt.Union() ) );
        }
      }
    }

    auto rv = model::CreateKeyValueResults( fb, fb.CreateVector( vec ) );
    auto r = model::CreateResponse( fb, model::ResultVariant::KeyValueResults, rv.Union() );
    fb.Finish( r );

    co_await write( socket, fb );
  }

  template <typename Socket>
  boost::asio::awaitable<void> put( Socket& socket, const model::Request* request )
  {
    std::vector<db::Pair> pairs;
    for ( auto&& kv : *request->data() ) pairs.emplace_back( kv->key()->string_view(), kv->value()->string_view() );

    auto value = db::set( pairs );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vt = model::CreateSuccess( fb, value );
    auto r = model::CreateResponse( fb, model::ResultVariant::Success, vt.Union() );
    fb.Finish( r );
    co_await write( socket, fb );
  }

  template <typename Socket>
  boost::asio::awaitable<void> remove( Socket& socket, const model::Request* request )
  {
    std::vector<std::string_view> keys;
    for ( auto&& kv : *request->data() ) keys.emplace_back( kv->key()->string_view() );

    auto value = db::remove( keys );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vt = model::CreateSuccess( fb, value );
    auto r = model::CreateResponse( fb, model::ResultVariant::Success, vt.Union() );
    fb.Finish( r );
    co_await write( socket, fb );
  }

  template <typename Socket>
  boost::asio::awaitable<void> move( Socket& socket, const model::Request* request )
  {
    std::vector<db::Pair> pairs;
    for ( auto&& kv : *request->data() ) pairs.emplace_back( kv->key()->string_view(), kv->value()->string_view() );

    auto value = db::move( pairs );
    auto fb = flatbuffers::FlatBufferBuilder{};
    auto vt = model::CreateSuccess( fb, value );
    auto r = model::CreateResponse( fb, model::ResultVariant::Success, vt.Union() );
    fb.Finish( r );
    co_await write( socket, fb );
  }

  template <typename Socket>
  boost::asio::awaitable<void> process( Socket& socket, const model::Request* request )
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
    case model::Action::Move:
      co_await move( socket, request );
      break;
    }
  }

  template <typename Socket>
  boost::asio::awaitable<void> respond( Socket& socket )
  {
    using namespace std::string_view_literals;

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

    const auto brokenPipe = []( const boost::system::error_code& ec )
    {
      static const std::string msg{ "Broken pipe" };
      return boost::algorithm::starts_with( ec.message(), msg );
    };

    boost::system::error_code ec;
    std::size_t osize = co_await socket.async_read_some( boost::asio::buffer( data ), boost::asio::use_awaitable );
    const auto docSize = documentSize( osize );

    // echo, noop, ping etc.
    if ( docSize < 5 )
    {
      co_await boost::asio::async_write( socket, boost::asio::buffer( data, docSize ),
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec && !brokenPipe( ec ) )
      {
        LOG_WARN << "Error writing to socket. " << ec.message();
      }
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

    LOG_DEBUG << "Read " << int(osize) << " bytes, total size " << int(docSize);
    while ( docSize < maxBytes && read != docSize )
    {
      osize = co_await socket.async_read_some( boost::asio::buffer( data ), boost::asio::use_awaitable );
      rbuf.insert( rbuf.end(), data, data + osize );
      read += osize;
    }

    auto verifier = flatbuffers::Verifier( rbuf.data(), rbuf.size() );
    auto ok = model::VerifyRequestBuffer( verifier );
    if ( !ok )
    {
      LOG_WARN << "Invalid request buffer";
      co_await boost::asio::async_write( socket, boost::asio::buffer( rbuf.data(), rbuf.size() ),
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec && !brokenPipe( ec ) ) LOG_WARN << "Error writing to socket. " << ec.message();
      co_return;
    }

    auto request = model::GetRequest( rbuf.data() );
    co_await process( socket, request );
  }

  boost::asio::ssl::context createSSLContext()
  {
    auto ctx = boost::asio::ssl::context( boost::asio::ssl::context::tlsv13_server );
    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::single_dh_use );

#ifdef __APPLE__
    ctx.load_verify_file( "../../../certs/ca.crt" );
    ctx.use_certificate_file( "../../../certs/server.crt", boost::asio::ssl::context::pem );
    ctx.use_private_key_file( "../../../certs/server.key", boost::asio::ssl::context::pem );
#else
    ctx.load_verify_file( "/opt/spt/certs/ca.crt" );
    ctx.use_certificate_file( "/opt/spt/certs/server.crt", boost::asio::ssl::context::pem );
    ctx.use_private_key_file( "/opt/spt/certs/server.key", boost::asio::ssl::context::pem );
#endif
    ctx.set_verify_mode( boost::asio::ssl::verify_peer | boost::asio::ssl::verify_fail_if_no_peer_cert );

    return ctx;
  }

  boost::asio::awaitable<void> serveWithSSL( boost::asio::ip::tcp::socket s )
  {
    try
    {
      auto ctx = createSSLContext();
      auto socket = SecureSocket{ std::move( s ), ctx };
      boost::system::error_code ec;
      co_await socket.async_handshake( boost::asio::ssl::stream_base::server,
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec )
      {
        LOG_WARN << "Handshake error. " << ec.message();
        socket.next_layer().close( ec );
        if ( ec ) LOG_WARN << "Error closing socket. " << ec.message();
        co_return;
      }

      for (;;)
      {
        co_await respond( socket );
      }
    }
    catch ( const std::exception& e )
    {
      static const auto eof{ "stream truncated" };
      if ( !boost::algorithm::starts_with( e.what(), eof ) ) LOG_WARN << "Exception servicing request " << e.what();
    }
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

  boost::asio::awaitable<void> listener( int port, bool ssl )
  {
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor( executor,
        { boost::asio::ip::tcp::v4(), static_cast<boost::asio::ip::port_type>( port ) } );
    for (;;)
    {
      if ( ContextHolder::instance().ioc.stopped() ) break;
      boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept( boost::asio::use_awaitable );
      if ( ssl )
      {
        boost::asio::co_spawn( executor, serveWithSSL( std::move(socket) ), boost::asio::detached );
      }
      else
      {
        boost::asio::co_spawn( executor, serve( std::move(socket) ), boost::asio::detached );
      }
    }
  }
}

int spt::configdb::tcp::start( int port, bool ssl )
{
  namespace net = boost::asio;
  boost::asio::co_spawn( ContextHolder::instance().ioc, coroutine::listener( port, ssl ), boost::asio::detached );
  LOG_INFO << "TCP service started on port " << port;

  ContextHolder::instance().ioc.run();
  LOG_INFO << "TCP service stopped";

  return 0;
}

