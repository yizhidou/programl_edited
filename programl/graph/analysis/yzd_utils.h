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

NodeSet NodeListToNodeSet(const std::vector<int>& node_list) {
  NodeSet result_nodeset(node_list.begin(), node_list.end());
  return result_nodeset;
}

absl::flat_hash_map<int, int> NodeListToOrderMap(const std::vector<int>& node_list,
                                                 bool if_reverse) {
  absl::flat_hash_map<int, int> result_map;
  int num_node = node_list.size();
  result_map.reserve(num_node);

  if (if_reverse) {
    for (int idx = 0; idx < num_node; idx++) {
      result_map[node_list[idx]] = num_node - idx;
    }
  } else {
    for (int idx = 0; idx < num_node; idx++) {
      result_map[node_list[idx]] = idx;
    }
  }
  return result_map;
}

std::vector<int> GetRootList(
    const absl::flat_hash_map<int, absl::flat_hash_set<int>>& reverse_adj) {
  std::vector<int> root_list;
  for (auto iter = reverse_adj.begin(); iter != reverse_adj.end(); ++iter) {
    if (iter->second.size() == 0) {
      root_list.emplace_back(iter->first);
    }
  }
  return root_list;
}

std::pair<std::vector<int>, int> PostOrderAndNumBackEdge(
    const absl::flat_hash_map<int, absl::flat_hash_set<int>>& adj, const int rootnode) {
  absl::flat_hash_map<int, int> color_map;
  int num_back_edge = 0;
  std::vector<int> post_order_list;
  color_map.reserve(adj.size());
  for (auto iter = adj.begin(); iter != adj.end(); ++iter) {
    color_map[iter->first] = -1;  // white
  }
  std::function<void(int)> dfs;
  dfs = [&](int start_node) -> void {
    color_map[start_node] = 0;  // grey
    const auto adj_iter = adj.find(start_node);
    if (adj_iter != adj.end()) {
      const auto& adj_nodes = adj_iter->second;
      for (const auto child_node : adj_nodes) {
        if (color_map[child_node] == -1) {  // white
          dfs(start_node);
        } else if (color_map[child_node] == 0) {  // grey
          num_back_edge++;
        }
      }
      color_map[start_node] = 1;  // black
      post_order_list.push_back(start_node);
    }
  };
  return std::make_pair(post_order_list, num_back_edge);
}
}  // namespace yzd