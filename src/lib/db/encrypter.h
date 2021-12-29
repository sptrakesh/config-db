//
// Created by Rakesh on 24/12/2021.
//

#pragma once

#include <string>
#include <string_view>

#include <openssl/evp.h>

namespace spt::configdb::db
{
  struct Encrypter
  {
    struct SSLException : std::exception
    {
      const char* what() const noexcept override
      {
        return "Error initialising OpenSSL";
      }
    };

    Encrypter( std::string_view cryptKey );
    ~Encrypter();

    Encrypter(const Encrypter&) = delete;
    Encrypter& operator=(const Encrypter&) = delete;

    std::string encrypt( std::string_view value );
    std::string decrypt( std::string_view value );

    // For pooling
    [[nodiscard]] bool valid() const { return true; }

  private:
    const unsigned char* encrypt( const char* value, int length, int& outlength );
    char* decrypt( const unsigned char* value, int length, int& outlength );

    static void loadOpenSSL();
    void initContext();
    void cleanOpenSSL();
    static void printError();

    std::string key;

    EVP_CIPHER_CTX* encryptingContext = nullptr;
    EVP_CIPHER_CTX* decryptingContext = nullptr;
    bool refreshEncryptContext = false;
    bool refreshDecryptContext = false;
  };
}