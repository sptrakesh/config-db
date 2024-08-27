//
// Created by Rakesh on 10/10/2020.
//

#include "cache.hpp"

spt::util::ValueCache& spt::util::getValueCache()
{
  static ValueCache cache;
  return cache;
}

