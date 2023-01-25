#pragma once

#include "yzd_utils.h"
// #include "labm8/cpp/status.h"
#include <bitset>
#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace yzd{

NodeSet operator|(NodeSet& lhs, NodeSet& rhs){
  NodeSet result(lhs);
  result.merge(rhs);
  return result;
}

NodeSet operator|=(NodeSet& lhs, NodeSet& rhs){
  lhs.merge(rhs);
  return lhs;
}

NodeSet operator&(const NodeSet& lhs, const NodeSet& rhs){
  NodeSet result;
  for (const auto& item : lhs){
    if (rhs.contains(item)){
      result.insert(item);
    }
  }
  return result;
}

NodeSet& operator&=(NodeSet& lhs, const NodeSet& rhs){
  for (const auto& item : lhs){
    if (!rhs.contains(item)){
      lhs.erase(item);
    }
  }
  return lhs;
}

NodeSet operator-(const NodeSet& lhs, const NodeSet& rhs){
  NodeSet result;
  for (const auto& item : lhs){
    if (rhs.contains(item)){
      result.erase(item);
    }
  }
  return result;
}
} // namespace yzd