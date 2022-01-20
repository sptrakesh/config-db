//
// Created by Rakesh on 29/12/2021.
//

#pragma once

#include <memory>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../common/contextholder.h"
#include "../common/model/request.h"
#if __has_include("../../log/NanoLog.h")
#include "../../log/NanoLog.h"
#else
#include <log/NanoLog.h>
#endif
#include "../../src/common/model/response_generated.h"

namespace spt::configdb::api::impl
{
  struct BaseConnection
  {
    BaseConnection() = default;
    virtual ~BaseConnection() = default;

    BaseConnection( const BaseConnection& ) = delete;
    BaseConnection& operator=( const BaseConnection& ) = delete;

    // CRUD
    const model::Response* list( std::string_view key );
    const model::Response* get( std::string_view key );
    const model::Response* set( const model::RequestData& data );
    const model::Response* remove( std::string_view key );
    const model::Response* move( const model::RequestData& data );
    const model::Response* ttl( std::string_view key );

    // Batch
    const model::Response* list( const std::vector<std::string_view>& keys );
    const model::Response* get( const std::vector<std::string_view>& keys );
    const model::Response* set( const std::vector<model::RequestData>& kvs );
    const model::Response* remove( const std::vector<std::string_view>& keys );
    const model::Response* move( const std::vector<model::RequestData>& kvs );
    const model::Response* ttl( const std::vector<std::string_view>& keys );

    // File import
    using ImportResponse = std::tuple<const model::Response*, std::size_t, uint32_t>;
    ImportResponse import( const std::string& file );

    bool valid() const { return status; }
    void invalid() { status = false; }

  protected:
    template <typename Socket>
    const model::Response* writeImpl( const flatbuffers::FlatBufferBuilder& fb, std::string_view context, Socket& s )
    {
      socket();
      if ( !valid() )
      {
        LOG_WARN << "Connection not valid.";
        return nullptr;
      }

      uint32_t n = fb.GetSize();
      std::ostream os{ &buffer };
      os.write( reinterpret_cast<const char*>( &n ), sizeof(n) );
      os.write( reinterpret_cast<const char*>( fb.GetBufferPointer() ), fb.GetSize() );

      const auto isize = boost::asio::write( s, buffer );
      buffer.consume( isize );

      auto osize = s.read_some( buffer.prepare( 256 ) );
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
        osize = s.read_some( buffer.prepare( 256 ) );
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

  private:
    virtual void socket() = 0;
    virtual const model::Response* write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) = 0;

    bool status{ true };
    boost::asio::streambuf buffer;
  };

  struct Connection : BaseConnection
  {
    Connection( std::string_view server, std::string_view port );
    virtual ~Connection();

  private:
    virtual void socket() override;
    virtual const model::Response* write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) override;

    boost::asio::ip::tcp::socket s{ spt::configdb::ContextHolder::instance().ioc };
    boost::asio::ip::tcp::resolver resolver{ spt::configdb::ContextHolder::instance().ioc };
  };

  struct SSLConnection : BaseConnection
  {
    SSLConnection( std::string_view server, std::string_view port );
    virtual ~SSLConnection();

  private:
    virtual void socket() override;
    virtual const model::Response* write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context ) override;

    using SecureSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    static boost::asio::ssl::context createContext();

    boost::asio::ssl::context ctx{ createContext() };
    SecureSocket s{ spt::configdb::ContextHolder::instance().ioc, ctx };
    boost::asio::ip::tcp::resolver resolver{ spt::configdb::ContextHolder::instance().ioc };
  };

  std::unique_ptr<BaseConnection> create();
}