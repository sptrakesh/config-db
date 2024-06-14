//
// Created by Rakesh on 25/12/2021.
//

#include <catch2/catch_test_macros.hpp>

#include "../../src/api/api.h"
#include "../../src/log/NanoLog.h"
#include "../../src/common/model/request_generated.h"

using namespace spt::configdb::api;
using namespace std::string_view_literals;

SCENARIO( "Simple CRUD test suite", "[crud]" )
{
  GIVEN( "Connected to TCP Service" )
  {
    const auto key = "/key"sv;

    WHEN( "Setting a key-value pair" )
    {
      const auto status = set( key, "value"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Reading key" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value"sv );
    }

    AND_WHEN( "Updating value" )
    {
      const auto status = set( key, "value modified"sv );
      REQUIRE( status );
    }

    AND_WHEN( "Reading updated value" )
    {
      const auto value = get( key );
      REQUIRE( value );
      REQUIRE( *value == "value modified"sv );
    }

    AND_WHEN( "Deleting key" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }

    AND_WHEN( "Retrieving deleted key" )
    {
      const auto value = get( key );
      REQUIRE_FALSE( value );
    }
  }

  GIVEN( "A large JSON format value" )
  {
    auto key = "/key"sv;
    auto value = R"({
  "users": [{
    "userId": 1,
    "firstName": "Krish",
    "lastName": "Lee",
    "phoneNumber": "123456",
    "emailAddress": "krish.lee@learningcontainer.com"
  }, {
    "userId": 2,
    "firstName": "racks",
    "lastName": "jacson",
    "phoneNumber": "123456",
    "emailAddress": "racks.jacson@learningcontainer.com"
  }, {
    "userId": 3,
    "firstName": "denial",
    "lastName": "roast",
    "phoneNumber": "33333333",
    "emailAddress": "denial.roast@learningcontainer.com"
  }, {
    "userId": 4,
    "firstName": "devid",
    "lastName": "neo",
    "phoneNumber": "222222222",
    "emailAddress": "devid.neo@learningcontainer.com"
  }, {
    "userId": 5,
    "firstName": "jone",
    "lastName": "mac",
    "phoneNumber": "111111111",
    "emailAddress": "jone.mac@learningcontainer.com"
  }],
  "Employees": [{
    "userId": "krish",
    "jobTitle": "Developer",
    "firstName": "Krish",
    "lastName": "Lee",
    "employeeCode": "E1",
    "region": "CA",
    "phoneNumber": "123456",
    "emailAddress": "krish.lee@learningcontainer.com"
  }, {
    "userId": "devid",
    "jobTitle": "Developer",
    "firstName": "Devid",
    "lastName": "Rome",
    "employeeCode": "E2",
    "region": "CA",
    "phoneNumber": "1111111",
    "emailAddress": "devid.rome@learningcontainer.com"
  }, {
    "userId": "tin",
    "jobTitle": "Program Directory",
    "firstName": "tin",
    "lastName": "jonson",
    "employeeCode": "E3",
    "region": "CA",
    "phoneNumber": "2222222",
    "emailAddress": "tin.jonson@learningcontainer.com"
  }]
}
)"sv;

    WHEN( "Saving the key-value pair" )
    {
      const auto status = set( key, value );
      REQUIRE( status );
    }

    AND_WHEN( "Reading key" )
    {
      const auto resp = get( key );
      REQUIRE( resp );
      REQUIRE( *resp == value );
    }

    AND_WHEN( "Deleting key" )
    {
      const auto status = remove( key );
      REQUIRE( status );
    }
  }
}
