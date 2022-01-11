//
// Created by Rakesh on 09/01/2022.
//

#pragma once

#include <string_view>

namespace spt::configdb::model
{
  struct RequestData
  {
    struct Options
    {
      Options() = default;
      ~Options() = default;
      Options(Options&&) = default;
      Options& operator=(Options&&) = default;
      Options(const Options&) = default;
      Options& operator=(const Options&) = default;

      // Store only if specified key does not exist
      bool ifNotExists{ false };
      // Set TTL value for the key.
      uint32_t expirationInSeconds{ 0 };
    };

    RequestData( std::string_view k, std::string_view v ) : key{ k }, value{ v } {}
    RequestData( std::string_view k, std::string_view v, Options opts ) :
        key{ k }, value{ v }, options{ opts } {}

    ~RequestData() = default;
    RequestData(RequestData&&) = default;
    RequestData& operator=(RequestData&&) = default;

    RequestData(const RequestData&) = delete;
    RequestData& operator=(const RequestData&) = delete;

    std::string_view key;
    // For move operation specify the destination key
    std::string_view value;
    Options options{};
  };
}