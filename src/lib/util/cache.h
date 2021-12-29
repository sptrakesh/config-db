//
// Created by Rakesh on 2019-05-20.
//

#pragma once

#include "ExpirationCache.h"

namespace spt::util
{
  using ValueCache = ExpirationCache<std::string, std::string, 60>;
  ValueCache& getValueCache();
}
