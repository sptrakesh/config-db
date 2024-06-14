//
// Created by Rakesh on 2019-05-16.
//

#include <catch2/catch_session.hpp>

#include "../../src/api/api.h"
#include "../../src/log/NanoLog.h"

int main( int argc, char* argv[] )
{
  nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "config-db-itest", false );
  spt::configdb::api::init( "localhost", "2022", false );
  return Catch::Session().run( argc, argv );
}
