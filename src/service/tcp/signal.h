//
// Created by Rakesh on 25/01/2022.
//

#pragma once

#include <memory>
#include <vector>
#include "nano/nano_signal_slot.hpp"

namespace spt::configdb::tcp
{
  struct SignalMgr
  {
    using Bytes = std::vector<uint8_t>;
    using BytesPtr = std::shared_ptr<Bytes>;
    //using Signal = boost::signals2::signal<void(BytesPtr)>;

    static SignalMgr& instance()
    {
      static SignalMgr m;
      return m;
    }

    void emit( BytesPtr bytes )
    {
      signal.fire( bytes );
      //signal( bytes );
    }

    /*
    boost::signals2::connection connect( const Signal::slot_type& subscriber )
    {
      return signal.connect( subscriber );
    }
     */

    template <auto mem_ptr, typename T>
    void connect( T& t )
    {
      signal.connect<mem_ptr, T>( t );
    }

    template <auto mem_ptr, typename T>
    void disconnect( T& t )
    {
      signal.disconnect<mem_ptr, T>( t );
    }

    ~SignalMgr()
    {
      signal.disconnect_all();
    }

    SignalMgr(const SignalMgr&) = delete;
    SignalMgr& operator=(const SignalMgr&) = delete;

  private:
    SignalMgr() {}

    //Signal signal;
    using NanoPolicy = Nano::TS_Policy_Safe<>;
    Nano::Signal<void(BytesPtr), NanoPolicy> signal;
  };
}