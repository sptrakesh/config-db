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
      Encryption(const Encryption&) = delete;
      Encryption& operator=(const Encryption&) = delete;

      std::string salt{ "jJ1LFPN2kl8P34saLd4rGe/UWncig04" };
      std::string key{ "mQtFU2PCvbqFhoM4XnB8yQMDETnSUaW" };
      std::string iv{ "Mr+xbc4TRKDrnCCrzE4t/7+ORrCfs6i" };
      std::string secret{ "TpSBvWY35C1sqURL9JCy6sKRtScKvCPTTQUZUE/vrfQ=" };
      int rounds = 1000000;
    };

    struct Logging
    {
      Logging() = default;
      ~Logging() = default;
      Logging(const Logging&) = delete;
      Logging& operator=(const Logging&) = delete;

      std::string level{ "info" };
      std::string dir{ "logs/" };
      bool console = false;
    };

    static const Configuration& instance()
    {
      static Configuration c;
      return c;
    }

    ~Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    Encryption encryption;
    Logging logging;
    uint32_t threads = std::thread::hardware_concurrency();
    bool enableCache{ false };

  private:
    Configuration() = default;
  };
}