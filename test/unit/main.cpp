//
// Created by Rakesh on 2019-05-16.
//

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <boost/json/src.hpp>

#include "../../src/lib/db/storage.h"
#include "../../src/log/NanoLog.h"

int main( int argc, char* argv[] )
{
  nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "config-db-test", false );
  spt::configdb::db::init();
  return Catch::Session().run( argc, argv );
}
