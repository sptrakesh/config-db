//
// Created by Rakesh on 2019-05-16.
//

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "../../src/lib/db/crud.h"
#include "../../src/common/log/NanoLog.h"

int main( int argc, char* argv[] )
{
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "config-db-test", false );
  spt::configdb::db::init();
  return Catch::Session().run( argc, argv );
}
