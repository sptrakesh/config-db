//
// Created by Rakesh on 11/01/2022.
//

#pragma once

#include <atomic>

namespace spt::configdb::service
{
  struct ClearExpired
  {
    ClearExpired() = default;
    ~ClearExpired() = default;

    ClearExpired(const ClearExpired&) = delete;
    ClearExpired& operator=(const ClearExpired&) = delete;

    void run();
    void stop();

  private:
    std::atomic_bool flag{ false };
  };
}