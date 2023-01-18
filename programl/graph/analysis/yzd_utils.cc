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

SparseBitVector operator|(SparseBitVector& lhs, SparseBitVector& rhs){
  SparseBitVector result(lhs);
  result.merge(rhs);
  return result;
}

SparseBitVector operator|=(SparseBitVector& lhs, SparseBitVector& rhs){
  lhs.merge(rhs);
  return lhs;
}

SparseBitVector operator&(const SparseBitVector& lhs, const SparseBitVector& rhs){
  SparseBitVector result;
  for (const auto& item : lhs){
    if (rhs.contains(item)){
      result.insert(item);
    }
  }
  return result;
}

SparseBitVector& operator&=(SparseBitVector& lhs, const SparseBitVector& rhs){
  for (const auto& item : lhs){
    if (!rhs.contains(item)){
      lhs.erase(item);
    }
  }
  return lhs;
}

SparseBitVector operator-(const SparseBitVector& lhs, const SparseBitVector& rhs){
  SparseBitVector result;
  for (const auto& item : lhs){
    if (rhs.contains(item)){
      result.erase(item);
    }
  }
  return result;
}


struct AnalysisSetting {
  std::string forward_or_backward;
  std::string may_or_must;
  std::string initialize_mode;  // 这几个感觉应该换成enum
  int max_iteration;
  AnalysisSetting(const std::string& forwardBackward, const std::string& mayMust,
                  const std::string intializeMode, int maxIteration)
      : forward_or_backward(forwardBackward),
        may_or_must(mayMust),
        initialize_mode(intializeMode),
        max_iteration(maxIteration) {}
  AnalysisSetting(const AnalysisSetting& other) {
    forward_or_backward = other.forward_or_backward;
    may_or_must = other.may_or_must;
    max_iteration = other.max_iteration;
  }
};
} // namespace yzd