//
// Created by Rakesh on 25/01/2022.
//

#include "server.hpp"
#include "signal/signal.hpp"
#include "../common/contextholder.hpp"
#include "../common/model/configuration.hpp"
#include "../common/util/concurrentqueue.h"
#include "../log/NanoLog.hpp"

#include <functional>
#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

using SecureSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

namespace spt::configdb::tcp::pnotifier
{
  struct Listener
  {
    Listener()
    {
      std::ostringstream ss;
      ss << "Connecting listener " << this;
      LOG_DEBUG << ss.str();
      signal::SignalMgr::instance().connect<&Listener::notify>( *this );
    }

    ~Listener()
    {
      std::ostringstream ss;
      ss << "Disconnecting listener " << this;
      LOG_DEBUG << ss.str();
      signal::SignalMgr::instance().disconnect<&Listener::notify>( *this );
    }

    void notify( signal::SignalMgr::BytesPtr b )
    {
      std::ostringstream ss;
      ss << "Received notification of size " << b->size() << ' ' << this;
      LOG_DEBUG << ss.str();
      queue.enqueue( b );
    }

    moodycamel::ConcurrentQueue<signal::SignalMgr::BytesPtr> queue;
    std::string ping{ "ping" };
    using Time = std::chrono::time_point<std::chrono::system_clock>;
    Time time = std::chrono::system_clock::now();
  };

  template <typename Socket>
  boost::asio::awaitable<void> respond( Socket& socket, Listener& listener )
  {
    const auto brokenPipe = []( const boost::system::error_code& ec )
    {
      static const std::string msg{ "Broken pipe" };
      return boost::algorithm::starts_with( ec.message(), msg );
    };

    signal::SignalMgr::BytesPtr bytes;
    if ( !listener.queue.try_dequeue( bytes ) )
    {
      auto now = std::chrono::system_clock::now();
      auto diff = std::chrono::duration_cast<std::chrono::seconds>( now - listener.time ).count();

      if ( diff > 15 )
      {
        boost::system::error_code ec;
        co_await boost::asio::async_write( socket,
            boost::asio::buffer( listener.ping.data(), listener.ping.size() ),
            boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
        if ( ec && !brokenPipe( ec ) ) LOG_WARN << "Error writing to socket. " << ec.message();
        listener.time = now;
      }
      co_return;
    }

    uint32_t size = bytes->size();
    std::vector<boost::asio::const_buffer> buffers;
    buffers.reserve( 2 );
    buffers.emplace_back( &size, sizeof(size) );
    buffers.emplace_back( bytes->data(), size );
    boost::system::error_code ec;
    co_await boost::asio::async_write( socket, buffers,
        boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
    if ( ec && !brokenPipe( ec ) ) LOG_WARN << "Error writing to socket. " << ec.message();
    else LOG_DEBUG << "Write notification of size " << int(bytes->size()) << " + " << int(sizeof(size)) << " bytes";
  }

  boost::asio::ssl::context createSSLContext()
  {
    auto& conf = model::Configuration::instance();
    auto ctx = boost::asio::ssl::context( boost::asio::ssl::context::tlsv13_server );
    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::single_dh_use );

    ctx.load_verify_file( conf.ssl.caCertificate );
    ctx.use_certificate_file( conf.ssl.certificate, boost::asio::ssl::context::pem );
    ctx.use_private_key_file( conf.ssl.key, boost::asio::ssl::context::pem );
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

      auto listener = Listener{};
      for (;;)
      {
        co_await respond( socket, listener );
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
      auto listener = Listener{};
      for (;;)
      {
        co_await respond( socket, listener );
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
    boost::asio::ip::tcp::acceptor acceptor{ executor,
        { boost::asio::ip::tcp::v4(), static_cast<boost::asio::ip::port_type>( port ) } };
    for (;;)
    {
      if ( ContextHolder::instance().ioc.stopped() ) break;
      boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept( boost::asio::use_awaitable );
      LOG_INFO << "Received socket connection from " << socket.remote_endpoint().address().to_string();
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

int spt::configdb::tcp::notifier( int port, bool ssl )
{
  boost::asio::co_spawn( ContextHolder::instance().ioc, pnotifier::listener( port, ssl ), boost::asio::detached );
  LOG_INFO << "TCP notifier service started on port " << port;
  return 0;
}

