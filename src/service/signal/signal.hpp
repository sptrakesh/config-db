//
// Created by Rakesh on 25/01/2022.
//

#pragma once

#include <memory>
#include <vector>
#include "nano/nano_signal_slot.hpp"

namespace spt::configdb::signal
{
  struct SignalMgr
  {
    using Bytes = std::vector<uint8_t>;
    using BytesPtr = std::shared_ptr<Bytes>;

    static SignalMgr& instance()
    {
      static SignalMgr m;
      return m;
    }

    void emit( BytesPtr bytes )
    {
      signal.fire( bytes );
    }

    template <auto MemberFunction, typename T>
    void connect( T& t )
    {
      signal.connect<MemberFunction, T>( t );
    }

    template <auto MemberFunction, typename T>
    void disconnect( T& t )
    {
      signal.disconnect<MemberFunction, T>( t );
    }

    ~SignalMgr()
    {
      signal.disconnect_all();
    }

    SignalMgr(const SignalMgr&) = delete;
    SignalMgr& operator=(const SignalMgr&) = delete;

  private:
    SignalMgr() = default;

    using NanoPolicy = Nano::TS_Policy_Safe<>;
    Nano::Signal<void(BytesPtr), NanoPolicy> signal;
  };
}