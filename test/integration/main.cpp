//
// Created by Rakesh on 2019-05-16.
//

#include <catch2/catch_session.hpp>

#include "../../src/api/api.hpp"
#include "../../src/log/NanoLog.hpp"

int main( int argc, char* argv[] )
{
  nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "config-db-itest", false );
  spt::configdb::api::init( "localhost", "2022", false );
  return Catch::Session().run( argc, argv );
}
