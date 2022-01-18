//
// Created by Rakesh on 01/01/2022.
//

#pragma once

#include <string>
#include <thread>

namespace spt::configdb::model
{
  struct Configuration
  {
    struct Encryption
    {
      Encryption() = default;
      ~Encryption() = default;
      Encryption(Encryption&&) = default;
      Encryption& operator=(Encryption&&) = default;
      Encryption(const Encryption&) = delete;
      Encryption& operator=(const Encryption&) = delete;

      // Salt to use when initialising the OpenSSL encryption context.  Must be 32 chars
      std::string salt{ "jJ1LFPN2kl8P34saLd4rGe/UWncig04" };
      // Key to use when initialising OpenSSL encryption context.  Must be 32 chars
      std::string key{ "mQtFU2PCvbqFhoM4XnB8yQMDETnSUaW" };
      // Initialisation vector to use when initialising OpenSSL encryption context.  Must be 32 chars
      std::string iv{ "Mr+xbc4TRKDrnCCrzE4t/7+ORrCfs6i" };
      // The secret to use for AES encryption
      std::string secret{ "TpSBvWY35C1sqURL9JCy6sKRtScKvCPTTQUZUE/vrfQ=" };
      // Number of rounds to use
      int rounds = 1000000;
    };

    struct Logging
    {
      Logging() = default;
      ~Logging() = default;
      Logging(Logging&&) = default;
      Logging& operator=(Logging&&) = default;
      Logging(const Logging&) = delete;
      Logging& operator=(const Logging&) = delete;

      // Log level to use.  One of debug|info|warn|critical
      std::string level{ "info" };
      // The directory to store log files in.  Should end with trailing '/'
      std::string dir{ "logs/" };
      // Echo logs to std::out
      bool console = false;
    };

    struct SSL
    {
      SSL() = default;
      ~SSL() = default;
      SSL(SSL&&) = default;
      SSL& operator=(SSL&&) = default;
      SSL(const SSL&) = delete;
      SSL& operator=(const SSL&) = delete;

#ifdef __APPLE__
      // The CA certificate file to use
      std::string caCertificate{ "../../../certs/ca.crt" };
      // The server (or client) certificate in PEM format.
      std::string certificate{ "../../../certs/server.crt" };
      // The server (or client) key in PEM format.
      std::string key{ "../../../certs/server.key" };
#else
      // The CA certificate file to use
      std::string caCertificate{ "/opt/spt/certs/ca.crt" };
      // The server (or client) certificate in PEM format.
      std::string certificate{ "/opt/spt/certs/server.crt" };
      // The server (or client) key in PEM format.
      std::string key{ "/opt/spt/certs/server.key" };
#endif

      bool enable{ false };
    };

    struct Services
    {
      Services() = default;
      ~Services() = default;
      Services(Services&&) = default;
      Services& operator=(Services&&) = default;
      Services(const Services&) = delete;
      Services& operator=(const Services&) = delete;

#ifdef __APPLE__
      std::string http{ "6006" };
      int tcp{ 2022 };
#else
      std::string http{ "6000" };
      int tcp{ 2020 };
#endif
    };

    struct Storage
    {
      Storage() = default;
      ~Storage() = default;
      Storage(Storage&&) = default;
      Storage& operator=(Storage&&) = default;
      Storage(const Storage&) = delete;
      Storage& operator=(const Storage&) = delete;

#ifdef __APPLE__
      // Path under which the database files are to be stored
      std::string dbpath{ "/tmp/config-db" };
#else
      // Path under which the database files are to be stored
      std::string dbpath{ "/opt/spt/data/config-db" };
#endif
      // Block size passed to rocksdb::ColumnFamilyOptions::OptimizeForPointLookup
      uint64_t blockCacheSizeMb{ 8u };
    };

    static const Configuration& instance();
    static void loadFromFile( const std::string& file );
    // For testing only
    static void reset();

    ~Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    Encryption encryption{};
    Logging logging{};
    SSL ssl{};
    Services services{};
    Storage storage{};
    uint32_t threads = std::thread::hardware_concurrency();
    bool enableCache{ false };

  private:
    Configuration() = default;
  };
}