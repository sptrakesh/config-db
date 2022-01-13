//
// Created by Rakesh on 24/12/2021.
//

#include "encrypter.h"
#include "model/configuration.h"
#include "../common/log/NanoLog.h"

#include <chrono>
#include <openssl/err.h>

using spt::configdb::db::Encrypter;

Encrypter::Encrypter( std::string_view cryptKey ) : key{ cryptKey.data(), cryptKey.size() }
{
  loadOpenSSL();
  initContext();
}

Encrypter::~Encrypter()
{
  cleanOpenSSL();
}

std::string Encrypter::encrypt( std::string_view value )
{
  auto const execute = [this, value]() -> std::string
  {
    int outlen = value.size();
    std::string str( outlen + EVP_CIPHER_CTX_block_size( encryptingContext ), '\0' );
    auto* outbuf = reinterpret_cast<unsigned char*>( str.data() );

    if ( refreshEncryptContext )
    {
      EVP_EncryptInit_ex( encryptingContext, nullptr, nullptr, nullptr, nullptr );
    }
    else
    {
      refreshEncryptContext = true;
    }

    if ( ! EVP_EncryptUpdate( encryptingContext, outbuf, &outlen,
        reinterpret_cast<const unsigned char *>( value.data() ), outlen ) )
    {
      LOG_WARN << "EVP_EncryptUpdate() failed!";
      printError();
      return {};
    }

    int templen;
    if ( ! EVP_EncryptFinal_ex( encryptingContext, outbuf + outlen, &templen ) )
    {
      LOG_WARN << "EVP_EncryptFinal() failed!";
      printError();
      return nullptr;
    }

    outlen += templen;
    return std::string{ reinterpret_cast<const char*>( outbuf ), static_cast<std::size_t>( outlen ) };
  };

  if ( value.empty() ) return {};

  auto start = std::chrono::high_resolution_clock::now();
  auto result = execute();
  auto finish = std::chrono::high_resolution_clock::now();
  LOG_DEBUG << "Encryption of value of length " << int( value.size() ) <<
    " took " << std::chrono::duration_cast<std::chrono::nanoseconds>( finish - start ).count() <<
    " nanoseconds";
  return result;
}

std::string Encrypter::decrypt( std::string_view sec )
{
  if ( sec.empty() ) return {};

  auto const execute = [this, sec]() -> std::string
  {
    int outlen = sec.size();
    std::string str( outlen, '\0' );
    auto* outbuf = reinterpret_cast<unsigned char *>( str.data() );

    if ( refreshDecryptContext )
    {
      EVP_DecryptInit_ex( decryptingContext, nullptr, nullptr, nullptr, nullptr );
    }
    else
    {
      refreshDecryptContext = true;
    }

    auto* input = reinterpret_cast<const unsigned char*>( sec.data() );
    if ( ! EVP_DecryptUpdate( decryptingContext, outbuf, &outlen, input, outlen ) )
    {
      LOG_WARN << "EVP_DecryptUpdate() failed!";
      printError();
      return nullptr;
    }

    int templen;
    if ( ! EVP_DecryptFinal_ex( decryptingContext, outbuf + outlen, &templen ) )
    {
      LOG_WARN << "EVP_DecryptFinal() failed!";
      printError();
      return nullptr;
    }

    outlen += templen;
    return std::string{ reinterpret_cast<char*>( outbuf ), static_cast<std::size_t>( outlen ) };
  };

  auto start = std::chrono::high_resolution_clock::now();
  auto result = execute();
  auto finish = std::chrono::high_resolution_clock::now();
  LOG_DEBUG << "Decryption of value of length " << int( sec.size() ) <<
    " took " << std::chrono::duration_cast<std::chrono::nanoseconds>( finish - start ).count() <<
    " nanoseconds";
  return result;
}

void Encrypter::loadOpenSSL()
{
  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
}

void Encrypter::initContext()
{
  constexpr static const char* scheme = "aes-256-cbc";
  auto& conf = model::Configuration::instance();

  auto aesk = std::string{ conf.encryption.key };
  auto aesi = std::string{ conf.encryption.iv };

  auto* salt = reinterpret_cast<const unsigned char*>( conf.encryption.salt.data() );
  auto* aes_key = reinterpret_cast<unsigned char*>( aesk.data() );
  auto* aes_iv = reinterpret_cast<unsigned char*>( aesi.data() );

  const EVP_CIPHER *cipher = EVP_get_cipherbyname( scheme );
  if ( ! cipher )
  {
    LOG_WARN << "Cannot get cipher with name {" << scheme << "}";
    printError();
    throw SSLException{};
  }

  EVP_BytesToKey(
      cipher,  // Cryptographic mode
      EVP_sha512(),         // SHA512
      salt,               // a fuzzifier
      reinterpret_cast<const unsigned char *>( key.c_str() ),
      int( key.size() + 1 ),
      conf.encryption.rounds,             // more rounds
      aes_key, aes_iv );   // return buffers

  encryptingContext = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init( encryptingContext );

  EVP_EncryptInit_ex( encryptingContext, cipher, nullptr, aes_key, aes_iv );

  decryptingContext = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init( decryptingContext );
  EVP_DecryptInit_ex( decryptingContext, cipher, nullptr, aes_key, aes_iv );
}

void Encrypter::cleanOpenSSL()
{
  EVP_CIPHER_CTX_free( encryptingContext );
  EVP_CIPHER_CTX_free( decryptingContext );
  EVP_cleanup();
  ERR_free_strings();
}

void Encrypter::printError()
{
  unsigned long errorCode;
  while( ( errorCode = ERR_get_error() ) > 0 )
  {
    char *errorMessage = ERR_error_string( errorCode, nullptr );
    LOG_WARN << "Error code: {" << int(errorCode) << "} message: {"
      << errorMessage << "}";
  }
}
