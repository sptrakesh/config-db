//
// Created by Rakesh on 25/01/2022.
//

#include "server.h"
#include "../common/contextholder.h"
#include "../common/model/request_generated.h"
#include "../common/model/request.h"
#include "../lib/db/storage.h"
#include "../log/NanoLog.h"

#include <memory>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace spt::configdb::tcp::plistener
{
  boost::asio::ssl::context createContext()
  {
    auto ctx = boost::asio::ssl::context( boost::asio::ssl::context::tlsv13_client );

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

  using Socket = boost::asio::ip::tcp::socket;
  using SecureSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

  template <typename SocketType>
  struct Listener
  {
    Listener( SocketType s, std::string_view host, std::string_view port )
        : socket{ std::move( s ) },
        host{ host.data(), host.size() }, port{ port.data(), port.size() }
    {
    }

    ~Listener()
    {
      close( socket );
    }

    boost::asio::awaitable<void> run()
    {
      auto& ch = ContextHolder::instance();

      const auto waitTillConnected = [&ch, this]() -> boost::asio::awaitable<void>
      {
        auto connected = co_await connect( socket );
        while ( !connected && !ch.ioc.stopped() ) connected = co_await connect( socket );
        LOG_INFO << "Connected to notification service on host: " << host << ", port: " << port;
      };

      endpoints = resolver.resolve( host, port );
      co_await waitTillConnected();

      while ( !ch.ioc.stopped() )
      {
        co_await read();
        if ( !isOpen( socket ) ) co_await waitTillConnected();
      }
    }

  private:
    boost::asio::awaitable<bool> connect( Socket& s )
    {
      boost::system::error_code ec;
      co_await boost::asio::async_connect( s, endpoints,
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec ) co_return false;
      co_return true;
    }

    boost::asio::awaitable<bool> connect( SecureSocket& s )
    {
      boost::system::error_code ec;
      co_await boost::asio::async_connect( s.lowest_layer(), endpoints,
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec ) co_return false;

      co_await s.async_handshake( boost::asio::ssl::stream_base::client,
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec )
      {
        LOG_WARN << "SSL error connecting to notification service on host: " <<
          host << ", port: " << port << ". " << ec.message();
        co_return false;
      }

      co_return true;
    }

    void put( const model::Request* request )
    {
      std::vector<model::RequestData> pairs;
      for ( auto&& kv : *request->data() )
      {
        auto opts = model::RequestData::Options{
            kv->options()->expiration_in_seconds(), false };
        pairs.emplace_back( kv->key()->string_view(), kv->value()->string_view(), opts );
      }

      auto value = db::set( pairs );
      if ( !value ) LOG_WARN << "Error setting " << int(pairs.size()) << " key-value pairs";
      else LOG_INFO << "Set " << int(pairs.size()) << " key-value pairs";
    }

    void remove( const model::Request* request )
    {
      std::vector<std::string_view> keys;
      for ( auto&& kv : *request->data() ) keys.emplace_back( kv->key()->string_view() );

      auto value = db::remove( keys );
      if ( !value ) LOG_WARN << "Error removing " << int(keys.size()) << " keys";
      else LOG_INFO << "Removed " << int(keys.size()) << " keys";
    }

    void move( const model::Request* request )
    {
      std::vector<model::RequestData> pairs;
      for ( auto&& kv : *request->data() )
      {
        auto opts = model::RequestData::Options{ kv->options()->expiration_in_seconds(), false };
        pairs.emplace_back( kv->key()->string_view(), kv->value()->string_view(), opts );
      }

      auto value = db::move( pairs );
      if ( !value ) LOG_WARN << "Error moving " << int(pairs.size()) << " keys";
      else LOG_INFO << "Moved " << int(pairs.size()) << " keys";
    }

    void process( const model::Request* request )
    {
      switch ( request->action() )
      {
      case model::Action::Get:
        LOG_CRIT << "Invalid Get action in notification";
        break;
      case model::Action::List:
        LOG_CRIT << "Invalid List action in notification";
        break;
      case model::Action::Put:
        put( request );
        break;
      case model::Action::Delete:
        remove( request );
        break;
      case model::Action::Move:
        move( request );
        break;
      case model::Action::TTL:
        LOG_CRIT << "Invalid TTL action in notification";
        break;
      }
    }

    boost::asio::awaitable<void> read()
    {
      static constexpr int bufSize = 128;
      static constexpr auto maxBytes = 8 * 1024 * 1024;
      uint8_t data[bufSize];

      const auto documentSize = [&data]( std::size_t length )
      {
        if ( length < 5 ) return length;

        const auto d = const_cast<uint8_t*>( data );
        uint32_t len;
        memcpy( &len, d, sizeof(len) );
        return std::size_t( len + sizeof(len) );
      };

      boost::system::error_code ec;
      LOG_DEBUG << "Reading notification from host: " << host << ", port: " << port;
      std::size_t osize = co_await socket.async_read_some( boost::asio::buffer( data ),
          boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
      if ( ec )
      {
        static const auto eof{ "End of file" };
        if ( !boost::algorithm::starts_with( ec.message(), eof ) )
        {
          LOG_WARN << "Error reading from host: " << host << ", port: " << port << ". " << ec.message();
        }
        close( socket );
        co_return;
      }
      const auto docSize = documentSize( osize );

      // echo, noop, ping etc.
      if ( docSize < 5 ) co_return;

      if ( docSize <= bufSize )
      {
        auto request = model::GetRequest( data + sizeof(uint32_t) );
        process( request );
        co_return;
      }

      auto read = osize;
      std::vector<uint8_t> rbuf;
      rbuf.reserve( docSize - sizeof(uint32_t) );
      rbuf.insert( rbuf.end(), data + sizeof(uint32_t), data + osize );

      while ( docSize < maxBytes && read != docSize )
      {
        osize = co_await socket.async_read_some( boost::asio::buffer( data ),
            boost::asio::redirect_error( boost::asio::use_awaitable, ec ) );
        if ( ec )
        {
          LOG_WARN << "Error reading from host: " << host << ", port: " << port << ". " << ec.message();
          close( socket );
          co_return;
        }
        rbuf.insert( rbuf.end(), data, data + osize );
        read += osize;
      }

      auto request = model::GetRequest( rbuf.data() );
      process( request );
    }

    bool isOpen( Socket& s ) { return s.is_open(); }
    bool isOpen( SecureSocket& s ) { return s.next_layer().is_open(); }

    void close( Socket& s )
    {
      if ( s.is_open() )
      {
        boost::system::error_code ec;
        s.close( ec );
        if ( ec ) LOG_CRIT << "Error closing socket. " << ec.message();
      }
    }

    void close( SecureSocket& s )
    {
      if ( s.next_layer().is_open() )
      {
        boost::system::error_code ec;
        s.next_layer().close( ec );
        if ( ec ) LOG_CRIT << "Error closing socket. " << ec.message();
      }
    }

    boost::asio::ip::tcp::resolver resolver{ ContextHolder::instance().ioc };
    boost::asio::ip::tcp::resolver::results_type endpoints;
    SocketType socket;
    std::string host;
    std::string port;
  };

  std::unique_ptr<Listener<Socket>> listener( std::string_view host, std::string_view port )
  {
    Socket s{ ContextHolder::instance().ioc };
    return std::make_unique<Listener<boost::asio::ip::tcp::socket>>( std::move( s ), host, port );
  }

  std::unique_ptr<Listener<SecureSocket>> listenerWithSSL(
      std::string_view host, std::string_view port, boost::asio::ssl::context& ctx )
  {
    SecureSocket s{ ContextHolder::instance().ioc, ctx };
    return std::make_unique<Listener<SecureSocket>>( std::move( s ), host, port );
  }

  boost::asio::awaitable<void> listen( std::string host, std::string port, bool ssl )
  {
    auto executor = co_await boost::asio::this_coro::executor;

    if ( ssl )
    {
      boost::asio::ssl::context ctx = createContext();
      auto ptr = listenerWithSSL( host, port, ctx );
      boost::asio::co_spawn( executor, ptr->run(), boost::asio::detached );
    }
    else
    {
      auto ptr = listener( host, port );
      boost::asio::co_spawn( executor, ptr->run(), boost::asio::detached );
    }
  }
}

int spt::configdb::tcp::listen( std::string_view host, std::string_view port, bool ssl )
{
  boost::asio::co_spawn( ContextHolder::instance().ioc,
      plistener::listen( std::string{ host }, std::string{ port }, ssl ),
      boost::asio::detached );
  LOG_INFO << "TCP notification listener client started on port " << port;
  return 0;
}

