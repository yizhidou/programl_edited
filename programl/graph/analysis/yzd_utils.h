#pragma once
#include <iostream>
#include <queue>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "labm8/cpp/status.h"
#include "labm8/cpp/statusor.h"

namespace yzd {
using NodeSet = absl::flat_hash_set<int>;

enum EdgeType { control_edge, gen_edge, kill_edge, unset_edge };
struct EdgeItem {
  int source_node;
  int target_node;
  enum EdgeType edge_type;
  EdgeItem(int sourceNode, int targetNode)
      : source_node(sourceNode), target_node(targetNode), edge_type(unset_edge) {}
  EdgeItem(int sourceNode, int targetNode, enum EdgeType edgeType)
      : source_node(sourceNode), target_node(targetNode), edge_type(edgeType) {}
};
using EdgeList = std::vector<EdgeItem>;

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

labm8::Status GetRemapedNodeset(NodeSet& remapped_nodeset, const NodeSet& origin_nodeset,
                                const absl::flat_hash_map<int, int>& map);

void PrintWorkList(const std::queue<WorklistItem>& workList);

enum TaskName { yzd_liveness, yzd_dominance, yzd_reachability };
enum Direction { forward, backward };
enum MayOrMust { may, must };
enum InitializeMode { allzeros, allones };
enum SyncOrAsync { sync, async };

extern std::unordered_map<TaskName, std::string> TaskNameToStrTable;

struct AnalysisSetting {
  TaskName task_name;
  int max_iteration;
  Direction direction;
  MayOrMust may_or_must;
  InitializeMode initialize_mode;
  SyncOrAsync sync_or_async;
  bool index_reorganized;

  AnalysisSetting(const TaskName taskName, int maxIteration, const SyncOrAsync syncOrAsync,
                  bool if_idx_reorganized)
      : task_name(taskName),
        max_iteration(maxIteration),
        sync_or_async(syncOrAsync),
        index_reorganized(if_idx_reorganized) {
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
      case yzd_reachability:
        return std::string("Reachability Analysis (yzd)");
        break;
      case yzd_dominance:
        return std::string("Dominance Analysis (yzd)");
        break;
      default:
        return std::string("Unrecognized (yzd)");
        break;
    }
  }
};

NodeSet SubgraphNodesFromRoot(const int& rootNode, const Adjacencies& adjacencies,
                              const Direction& direction);

absl::flat_hash_map<int, int> NodeListToOrderMap(const std::vector<int>& node_list,
                                                 bool if_reverse);

std::vector<int> GetRootList(const absl::flat_hash_map<int, absl::flat_hash_set<int>>& reverse_adj);

std::pair<std::vector<int>, int> PostOrderAndNumBackEdgeFromOneRoot(
    const absl::flat_hash_map<int, absl::flat_hash_set<int>>& adj, const int rootnode);

}  // namespace yzd