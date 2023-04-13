#pragma once
#include <iostream>
#include <queue>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "labm8/cpp/status.h"
#include "labm8/cpp/statusor.h"

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

enum TaskName { yzd_liveness, yzd_dominance, yzd_reachability };
enum Direction { forward, backward };
enum MayOrMust { may, must };
enum InitializeMode { allzeros, allones };
enum SyncOrAsync {sync, async};

struct AnalysisSetting {
  TaskName task_name;
  Direction direction;
  MayOrMust may_or_must;
  InitializeMode initialize_mode;
  SyncOrAsync sync_or_async;
  int max_iteration;
  AnalysisSetting(const TaskName taskName, int maxIteration, const SyncOrAsync syncOrAsync)
      : task_name(taskName), max_iteration(maxIteration), sync_or_async(syncOrAsync) {
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
        break;
      case yzd_reachability:
        direction = backward;
        may_or_must = may;
        initialize_mode = allzeros;
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

NodeSet SubgraphNodesFromRoot(const int& rootNode, const Adjacencies& adjacencies,
                              const Direction& direction);


absl::flat_hash_map<int, int> NodeListToOrderMap(const std::vector<int>& node_list,
                                                 bool if_reverse);

std::vector<int> GetRootList(
    const absl::flat_hash_map<int, absl::flat_hash_set<int>>& reverse_adj);

std::pair<std::vector<int>, int> PostOrderAndNumBackEdgeFromOneRoot(
    const absl::flat_hash_map<int, absl::flat_hash_set<int>>& adj, const int rootnode);
}  // namespace yzd