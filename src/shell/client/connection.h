//
// Created by Rakesh on 29/12/2021.
//

#pragma once

#include <memory>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "contextholder.h"
#include "../../src/lib/model/response_generated.h"

namespace spt::configdb::client
{
  struct Connection
  {
    Connection( std::string_view server, std::string_view port );
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    const model::Response* list( std::string_view key );
    const model::Response* get( std::string_view key );
    const model::Response* set( std::string_view key, std::string_view value );
    const model::Response* remove( std::string_view key );

    bool valid() const { return true; }

  private:
    boost::asio::ip::tcp::socket& socket();
    const model::Response* write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context );

    boost::asio::ip::tcp::socket s{ spt::configdb::ContextHolder::instance().ioc };
    boost::asio::ip::tcp::resolver resolver{ spt::configdb::ContextHolder::instance().ioc };
    boost::asio::streambuf buffer;
  };

  void init( std::string_view server, std::string_view port );
  std::unique_ptr<Connection> create();
}