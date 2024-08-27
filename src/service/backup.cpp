//
// Created by Rakesh on 22/06/2024.
//

#include "backup.hpp"
#include "../common/model/configuration.hpp"
#include "../log/NanoLog.hpp"
#include "../lib/db/storage.hpp"

#include <chrono>
#include <thread>

using spt::configdb::service::Backup;

void Backup::run()
{
  LOG_INFO << "Backup thread starting";
  const auto sleep = std::chrono::seconds{ model::Configuration::instance().storage.cleanExpiredKeysInterval };
  auto startDay = std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() ) };

  while ( !flag.load() )
  {
    if ( const auto day = std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() ) }; day > startDay )
    {
      db::backup( flag );
      startDay = day;
    }

    std::this_thread::sleep_for( sleep );
  }
}

void Backup::stop()
{
  LOG_INFO << "Stopping backup thread";
  flag.store( true );
}
