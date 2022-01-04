//
// Created by Rakesh on 29/12/2021.
//

#pragma once

#include <memory>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../common/contextholder.h"
#include "../../src/lib/model/response_generated.h"

namespace spt::configdb::api::impl
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

    const model::Response* mlist( const std::vector<std::string_view>& keys );
    const model::Response* mget( const std::vector<std::string_view>& keys );

    using Pair = std::pair<std::string_view, std::string_view>;
    const model::Response* mset( const std::vector<Pair>& kvs );

    const model::Response* mremove( const std::vector<std::string_view>& keys );

    bool valid() const { return status; }
    void invalid() { status = false; }

  private:
    boost::asio::ip::tcp::socket& socket();
    const model::Response* write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context );

    boost::asio::ip::tcp::socket s{ spt::configdb::ContextHolder::instance().ioc };
    boost::asio::ip::tcp::resolver resolver{ spt::configdb::ContextHolder::instance().ioc };
    boost::asio::streambuf buffer;
    bool status{ true };
  };

  std::unique_ptr<Connection> create();
}