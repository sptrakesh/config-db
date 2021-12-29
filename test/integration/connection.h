//
// Created by Rakesh on 28/12/2021.
//

#pragma once

#include <tuple>
#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../../src/lib/model/response_generated.h"

namespace spt::configdb::itest::tcp
{
  struct Connection
  {
    Connection( boost::asio::io_context& ioc );
    ~Connection();

    Connection( const Connection& ) = delete;
    Connection& operator=( const Connection& ) = delete;

    using Tuple = std::tuple<const model::Response*, std::size_t, std::size_t>;
    Tuple write( const flatbuffers::FlatBufferBuilder& fb, std::string_view context );

    std::size_t noop();

  private:
    boost::asio::ip::tcp::socket s;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::streambuf buffer;
  };
}
