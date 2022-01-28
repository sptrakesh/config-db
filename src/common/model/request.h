//
// Created by Rakesh on 09/01/2022.
//

#pragma once

#include <string>
#include <string_view>

namespace spt::configdb::model
{
  struct RequestData
  {
    struct Options
    {
      Options() = default;
      explicit Options( bool ine ) : ifNotExists{ ine } {}
      explicit Options( uint32_t eis ) : expirationInSeconds{ eis } {}
      Options( uint32_t eis, bool ine ) : expirationInSeconds{ eis }, ifNotExists{ ine } {}

      ~Options() = default;
      Options(Options&&) = default;
      Options& operator=(Options&&) = default;
      Options(const Options&) = default;
      Options& operator=(const Options&) = default;

      // Set TTL value for the key.
      uint32_t expirationInSeconds{ 0 };
      // Store only if specified key does not exist
      bool ifNotExists{ false };
    };

    RequestData( std::string_view k, std::string_view v ) : key{ k }, value{ v } {}
    RequestData( std::string_view k, std::string_view v, Options opts ) :
        options{ opts }, key{ k }, value{ v } {}

    ~RequestData() = default;
    RequestData(RequestData&&) = default;
    RequestData& operator=(RequestData&&) = default;
    RequestData(const RequestData&) = default;
    RequestData& operator=(const RequestData&) = default;

    Options options{};
    std::string key;
    // For move operation specify the destination key
    std::string value;
  };
}