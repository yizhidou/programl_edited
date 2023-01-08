#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include <cstddef>
#include <unordered_set>

typedef std::vector<bool> BitVector;

struct WorklistItem
{
  int iter_idx;
  int node_idx;
};

struct Adjacencies
{
  std::vector<std::vector<int>> control_adj_list;
  std::vector<std::vector<int>> control_reverse_adj_list;
  // std::vector<std::vector<int>> defs;
  // std::vector<std::vector<int>> uses;
};



BitVector operator|(const BitVector& lhs, const BitVector& rhs);
BitVector& operator|=(BitVector& lhs, const BitVector& rhs);

BitVector operator&(const BitVector& lhs, const BitVector& rhs);
BitVector& operator&=(BitVector& lhs, const BitVector& rhs);

BitVector operator-(const BitVector& lhs, const BitVector& rhs);

struct AnalysisSetting {
  std::string forward_or_backward;
  std::string may_or_must;
  std::string initialize_mode;
  int max_iteration;
  AnalysisSetting(const std::string& forwardBackward, const std::string& mayMust,
                  const std::string intializeMode, int maxIteration);
  AnalysisSetting(const AnalysisSetting& other);
};
