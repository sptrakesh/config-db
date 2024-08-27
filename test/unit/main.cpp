//
// Created by Rakesh on 2019-05-16.
//

#include <catch2/catch_session.hpp>

#include "../../src/lib/db/storage.hpp"
#include "../../src/log/NanoLog.hpp"

int main( int argc, char* argv[] )
{
  nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "config-db-test", false );
  spt::configdb::db::init();
  return Catch::Session().run( argc, argv );
}
