//
// Created by Rakesh on 22/06/2024.
//

#pragma once

#include <atomic>

namespace spt::configdb::service
{
  struct Backup
  {
    Backup() = default;
    ~Backup() = default;

    Backup(const Backup&) = delete;
    Backup& operator=(const Backup&) = delete;

    void run();
    void stop();

  private:
    std::atomic_bool flag{ false };
  };
}
