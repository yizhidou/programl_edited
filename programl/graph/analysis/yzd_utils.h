#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include <cstddef>
#include <unordered_set>
#include "absl/container/flat_hash_set.h"

namespace yzd{
using SparseBitVector = absl::flat_hash_set<int>;

struct WorklistItem
{
  int iter_idx;
  int node_idx;
};

struct Adjacencies
{
  absl::flat_hash_map<int, absl::flat_hash_set<int>> control_adj_list;
  absl::flat_hash_map<int, absl::flat_hash_set<int>> control_reverse_adj_list;
};

SparseBitVector operator|(SparseBitVector& lhs, SparseBitVector& rhs);
SparseBitVector operator|=(SparseBitVector& lhs, SparseBitVector& rhs);

SparseBitVector operator&(const SparseBitVector& lhs, const SparseBitVector& rhs);
SparseBitVector& operator&=(SparseBitVector& lhs, const SparseBitVector& rhs);

SparseBitVector operator-(const SparseBitVector& lhs, const SparseBitVector& rhs);


struct AnalysisSetting {
  std::string task_name;
  std::string forward_or_backward;
  std::string may_or_must;
  std::string initialize_mode;
  int max_iteration;
  AnalysisSetting(const std::string& forwardBackward, const std::string& mayMust,
                  const std::string intializeMode, int maxIteration);
  AnalysisSetting(const AnalysisSetting& other);
};
} //namespace yzd