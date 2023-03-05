#pragma once
#include <iostream>
#include <queue>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

namespace yzd {
using NodeSet = absl::flat_hash_set<int>;

struct WorklistItem {
  int iter_idx;
  int node_idx;
  WorklistItem(int iterIdx, int nodeIdx) : iter_idx(iterIdx), node_idx(nodeIdx) {}
};

struct Adjacencies {
  absl::flat_hash_map<int, absl::flat_hash_set<int>> control_adj_list;
  absl::flat_hash_map<int, absl::flat_hash_set<int>> control_reverse_adj_list;
};

NodeSet operator|(const NodeSet& lhs, const NodeSet& rhs);
NodeSet& operator|=(NodeSet& lhs, const NodeSet& rhs);
NodeSet operator&(const NodeSet& lhs, const NodeSet& rhs);
NodeSet& operator&=(NodeSet& lhs, const NodeSet& rhs);
NodeSet operator-(const NodeSet& lhs, const NodeSet& rhs);
bool operator==(const NodeSet& ns, const std::vector<int>& vi);
bool operator>(const NodeSet& lhs, const NodeSet& rhs);
bool operator<(const NodeSet& lhs, const NodeSet& rhs);

std::ostream& operator<<(std::ostream& os, const NodeSet& nodeSet);
std::ostream& operator<<(std::ostream& os, const std::vector<int>& intVec);
// std::ostream& operator<<(std::ostream& os, const std::queue<WorklistItem>& workList);
void PrintWorkList(const std::queue<WorklistItem>& workList);

enum TaskName { yzd_liveness, yzd_dominance };
enum Direction { forward, backward };
enum MayOrMust { may, must };
enum InitializeMode { allzeros, allones };

struct AnalysisSetting {
  TaskName task_name;
  Direction direction;
  MayOrMust may_or_must;
  InitializeMode initialize_mode;
  int max_iteration;
  AnalysisSetting(const TaskName taskName, int maxIteration)
      : task_name(taskName), max_iteration(maxIteration) {
    switch (task_name) {
      case yzd_liveness:
        direction = backward;
        may_or_must = may;
        initialize_mode = allzeros;
        break;
      case yzd_dominance:
        direction = forward;
        may_or_must = must;
        initialize_mode = allones;
      default:
        break;
    }
  }

  std::string TaskNameToString() {
    switch (task_name) {
      case yzd_liveness:
        return std::string("Liveness Analysis (yzd)");
        break;

      default:
        break;
    }
  }
};
}  // namespace yzd