//
// Created by Rakesh on 11/01/2022.
//

#include "clearexpired.hpp"
#include "../common/model/configuration.hpp"
#include "../log/NanoLog.hpp"
#include "../lib/db/storage.hpp"

#include <thread>

using spt::configdb::service::ClearExpired;

void ClearExpired::run()
{
  LOG_INFO << "Expired key clearer thread starting";
  const auto sleep = std::chrono::seconds{ model::Configuration::instance().storage.cleanExpiredKeysInterval };

  while ( !flag.load() )
  {
    db::clearExpired( flag );
    std::this_thread::sleep_for( sleep );
  }
}

void ClearExpired::stop()
{
  LOG_INFO << "Stopping expired key clearer thread";
  flag.store( true );
}