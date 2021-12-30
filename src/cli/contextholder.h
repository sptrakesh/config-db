//
// Created by Rakesh on 10/10/2020.
//

#pragma once

#include <thread>
#include <boost/asio/io_context.hpp>

namespace spt::configdb
{
  struct ContextHolder
  {
    ~ContextHolder() = default;

    ContextHolder( const ContextHolder& ) = delete;
    ContextHolder& operator=( const ContextHolder& ) = delete;

    static ContextHolder& instance()
    {
      static ContextHolder holder;
      return holder;
    }

    boost::asio::io_context ioc{ static_cast<int>( std::thread::hardware_concurrency() ) };

  private:
    ContextHolder() = default;
  };
}
