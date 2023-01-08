#pragma once

#include "yzd_utils.h"

#include <bitset>
#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>


BitVector operator|(const BitVector& lhs, const BitVector& rhs){
  if (lhs.size() != rhs.size()){
    std::cout << "length unmatch error!" << std::endl;
    //TODO: exception handling
  }
  BitVector result;
  result.reserve(lhs.size());
  for (int i = 0; i < lhs.size(); i++) {
      result.emplace_back(lhs[i] || rhs[i]);
    }
  return result;
}

BitVector& operator|=(BitVector& lhs, const BitVector& rhs){
  if (lhs.size() != rhs.size()){
    std::cout << "length unmatch error!" << std::endl;
    //TODO: exception handling
  }
  for (int i = 0; i < lhs.size(); i++){
    lhs[i] = lhs[i] || rhs[i];
  }
  return lhs;
}

BitVector operator&(const BitVector& lhs, const BitVector& rhs){
  if (lhs.size() != rhs.size()){
    std::cout << "length unmatch error!" << std::endl;
  }
  BitVector result;
  result.reserve(lhs.size());
  for (int i = 0; i < lhs.size(); i++) {
      result.emplace_back(lhs[i] && rhs[i]);
    }
  return result;
}

BitVector& operator&=(BitVector& lhs, const BitVector& rhs){
  if (lhs.size() != rhs.size()){
    std::cout << "length unmatch error!" << std::endl;
    //TODO: exception handling
  }
  for (int i = 0; i < lhs.size(); i++){
    lhs[i] = lhs[i] && rhs[i];
  }
  return lhs;
}

BitVector operator-(const BitVector& lhs, const BitVector& rhs){
  if (lhs.size() != rhs.size()){
    std::cout << "length unmatch error!" << std::endl;
  }
  BitVector result;
  result.reserve(lhs.size());
  for (int i = 0; i < lhs.size(); i++) {
      result.emplace_back((bool)((lhs[i] - rhs[i]) > 0));
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
