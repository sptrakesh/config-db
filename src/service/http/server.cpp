//
// Created by Rakesh on 25/12/2021.
//

#include "server.h"
#include "../common/contextholder.h"
#include "../common/model/configuration.h"
#include "../lib/db/storage.h"
#include "../log/NanoLog.h"

#include <charconv>
#include <boost/algorithm/string/replace.hpp>
#include <boost/json.hpp>
#include <nghttp2/asio_http2_server.h>

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace spt::configdb::http::endpoints
{
  void write( int code, std::string json,
      const nghttp2::asio_http2::server::response& res )
  {
    auto headers = nghttp2::asio_http2::header_map{
        { "content-type", { "application/json; charset=utf-8", false } },
        { "content-length", { std::to_string( json.size() ), false } }
    };
    res.write_head( code, std::move( headers ) );
    res.end( std::move( json ) );
  }

  void get( const nghttp2::asio_http2::server::request &req,
      const nghttp2::asio_http2::server::response &res )
  {
    using boost::json::object;

    const auto key = boost::algorithm::replace_first_copy( req.uri().path, "/key"sv, ""sv );
    auto value = db::get( key );
    if ( value )
    {
      auto obj = object{};
      obj["key"] = key;
      obj["value"] = *value;
      write( 200, boost::json::serialize( obj ), res );
    }
    else
    {
      LOG_WARN << "Key " << key << " not found";
      write( 404, R"({"code": 404, "cause": "Not found"})"s, res );
    }
  }

  void put( const nghttp2::asio_http2::server::request &req,
      const nghttp2::asio_http2::server::response &res )
  {
    auto body = std::make_shared<std::string>();
    const auto iter = req.header().find( "content-length"s );
    if ( iter == req.header().end() )
    {
      body->reserve( 2048 );
    }
    else
    {
      uint32_t length{};
      auto [ptr, ec] { std::from_chars( iter->second.value.data(), iter->second.value.data() + iter->second.value.size(), length ) };
      if ( ec == std::errc() ) body->reserve( length );
      else
      {
        LOG_WARN << "Invalid content-length: " << iter->second.value;
        body->reserve( 2048 );
      }
    }

    req.on_data([body, &req, &res](const uint8_t* chars, std::size_t size)
    {
      if ( size )
      {
        body->append( reinterpret_cast<const char*>( chars ), size );
        return;
      }

      if ( body->empty() )
      {
        return write( 400, R"({"code": 200, "cause": "Ok"})"s, res );
      }

      const auto key = boost::algorithm::replace_first_copy( req.uri().path, "/key"sv, ""sv );

      auto opts = model::RequestData::Options{};
      auto iter = req.header().find( "x-config-db-if-not-exists"s );
      if ( iter != req.header().end() ) opts.ifNotExists = iter->second.value == "true"s;
      iter = req.header().find( "x-config-db-ttl"s );
      if ( iter != req.header().end() )
      {
        auto sv = std::string_view{ iter->second.value };
        uint64_t ttl;
        auto [p, ec] = std::from_chars( sv.data(), sv.data() + sv.size(), ttl );
        if ( ec != std::errc() )
        {
          LOG_WARN << "Invalid TTL: " << sv;
        } else opts.expirationInSeconds = ttl;
      }

      auto result = db::set( model::RequestData{ key, *body, opts } );
      if ( result )
      {
        return write( 200, R"({"code": 200, "cause": "Ok"})"s, res );
      }

      LOG_WARN << "Error saving " << key;
      write( 412, R"({"code": 412, "cause": "Unable to save"})"s, res );
    } );
  }

  void remove( const nghttp2::asio_http2::server::request &req,
      const nghttp2::asio_http2::server::response &res )
  {
    const auto key = boost::algorithm::replace_first_copy( req.uri().path, "/key"sv, ""sv );
    auto result = db::remove( key );
    if ( result )
    {
      write( 200, R"({"code": 200, "cause": "Ok"})", res );
    }
    else
    {
      LOG_WARN << "Key " << key << " not found";
      write( 404, R"({"code": 404, "cause": "Not found"})"s, res );
    }
  }

  void list( const nghttp2::asio_http2::server::request &req,
      const nghttp2::asio_http2::server::response &res )
  {
    using boost::json::array;
    using boost::json::object;

    const auto key = boost::algorithm::replace_first_copy( req.uri().path, "/list"sv, ""sv );
    auto value = db::list( key );
    if ( value )
    {
      auto arr = array{};
      arr.reserve( value->size() );
      for ( auto&& v : *value ) arr.emplace_back( v );
      auto obj = object{};
      obj["children"] = arr;
      write( 200, boost::json::serialize( obj ), res );
    }
    else
    {
      LOG_WARN << "Key " << key << " not found";
      write( 404, R"({"code": 404, "cause": "Not found"})"s, res );
    }
  }

  void setup( nghttp2::asio_http2::server::http2& server )
  {
    server.handle( "/key/"s, []( const nghttp2::asio_http2::server::request &req,
        const nghttp2::asio_http2::server::response &res )
    {
      if ( req.method() == "GET"s ) return get( req, res );
      else if ( req.method() == "PUT"s ) return put( req, res );
      else if ( req.method() == "DELETE"s ) return remove( req, res );

      res.write_head( 405 );
      res.end( R"({"code": 405, "cause": "Method not supported"})" );
    } );

    server.handle( "/list/"s, []( const nghttp2::asio_http2::server::request &req,
        const nghttp2::asio_http2::server::response &res )
    {
      if ( req.method() == "GET"s ) return list( req, res );
      res.write_head( 405 );
      res.end( R"({"code": 405, "cause": "Method not supported"})" );
    } );

    server.handle( "/"s, []( const nghttp2::asio_http2::server::request &req,
        const nghttp2::asio_http2::server::response &res )
    {
      if ( req.uri().path != "/"s )
      {
        res.write_head( 404 );
        res.end( R"({"code": 404, "cause": "Not found"})" );
      }

      if ( req.method() == "GET"s )
      {
        res.write_head( 200 );
        res.end( R"({"status": "ok"})" );
      }
      else
      {
        res.write_head( 405 );
        res.end( R"({"code": 405, "cause": "Method not supported"})" );
      }
    } );
  }

  int startWithSSL( const std::string& port, int threads )
  {
    auto& conf = model::Configuration::instance();
    boost::asio::ssl::context tls{ boost::asio::ssl::context::tlsv12_server };
    tls.load_verify_file( conf.ssl.caCertificate );
    tls.use_certificate_file( conf.ssl.certificate, boost::asio::ssl::context::pem );
    tls.use_private_key_file( conf.ssl.key, boost::asio::ssl::context::pem );

    boost::system::error_code ec;
    nghttp2::asio_http2::server::configure_tls_context_easy( ec, tls );

    nghttp2::asio_http2::server::http2 server;
    server.num_threads( threads );

    endpoints::setup( server );

    LOG_INFO << "HTTP service starting on port " << port;
    if ( server.listen_and_serve( ec, tls, "0.0.0.0"s, port, true ) ) {
      LOG_CRIT << "error: " << ec.message();
      return 1;
    }

    ContextHolder::instance().ioc.run();
    LOG_INFO << "Stopping server";
    server.stop();
    server.join();

    return 0;
  }
}

int spt::configdb::http::start( const std::string& port, int threads, bool ssl )
{
  if ( ssl ) return endpoints::startWithSSL( port, threads );

  boost::system::error_code ec;
  nghttp2::asio_http2::server::http2 server;
  server.num_threads( threads );

  endpoints::setup( server );

  if ( server.listen_and_serve( ec, "0.0.0.0"s, port, true ) ) {
    LOG_CRIT << "error: " << ec.message();
    return 1;
  }

  ContextHolder::instance().ioc.run();
  LOG_INFO << "Stopping server";
  server.stop();
  server.join();

  return 0;
}

