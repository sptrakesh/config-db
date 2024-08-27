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
      [[nodiscard]] const char* what() const noexcept override
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
    static void loadOpenSSL();
    void initContext();
    void cleanOpenSSL();
    static void printError();

    EVP_CIPHER_CTX* encryptingContext = nullptr;
    EVP_CIPHER_CTX* decryptingContext = nullptr;

    std::string secret;
    std::string key;
    std::string iv;
    std::string salt;

    bool refreshEncryptContext = false;
    bool refreshDecryptContext = false;
  };
}