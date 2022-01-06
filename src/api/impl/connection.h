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
#include "../../src/common/model/response_generated.h"

namespace spt::configdb::api::impl
{
  struct Connection
  {
    Connection( std::string_view server, std::string_view port );
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    // CRUD
    const model::Response* list( std::string_view key );
    const model::Response* get( std::string_view key );
    const model::Response* set( std::string_view key, std::string_view value );
    const model::Response* remove( std::string_view key );
    const model::Response* move( std::string_view key, std::string_view value );

    // Batch
    const model::Response* list( const std::vector<std::string_view>& keys );
    const model::Response* get( const std::vector<std::string_view>& keys );

    using Pair = std::pair<std::string_view, std::string_view>;
    const model::Response* set( const std::vector<Pair>& kvs );

    const model::Response* remove( const std::vector<std::string_view>& keys );
    const model::Response* move( const std::vector<Pair>& kvs );

    // File import
    using ImportResponse = std::tuple<const model::Response*, std::size_t, uint32_t>;
    ImportResponse import( const std::string& file );

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