//
// Created by Rakesh on 24/12/2021.
//

#include <catch2/catch.hpp>
#include "../../src/lib/db/encrypter.h"

namespace test::encrypt
{
  std::string encrypted;
}

SCENARIO( "Encryption/decryption test suite" )
{
  GIVEN( "Encrypter with an encryption key" )
  {
    using spt::configdb::db::Encrypter;
    auto encrypter = Encrypter{ "unit test secret" };
    const auto text = std::string{ "Unit test string to be encrypted and decrypted" };

    WHEN( "Encrypting a string" )
    {
      test::encrypt::encrypted = encrypter.encrypt( text );
      REQUIRE_FALSE( test::encrypt::encrypted.empty() );
    }

    AND_THEN( "Decrypting encrypted string gives back original string" )
    {
      const auto dec = encrypter.decrypt( test::encrypt::encrypted );
      REQUIRE( dec.size() == text.size() );
      REQUIRE( dec == text );
    }
  }
}

