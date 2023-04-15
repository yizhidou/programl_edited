#include "yzd_utils.h"
// #include "labm8/cpp/status.h"
// #include <bitset>
// #include <cstddef>
// #include <iostream>
// #include <queue>
// #include <string>
// #include <utility>
// #include <vector>

namespace yzd {

std::unordered_map<TaskName, std::string> TaskNameToStrTable = {
    {yzd::yzd_liveness, "yzd_liveness"},
    {yzd::yzd_dominance, "yzd_dominance"},
    {yzd::yzd_reachability, "yzd_reachability"}};

NodeSet operator|(const NodeSet& lhs, const NodeSet& rhs) {
  NodeSet result(lhs);
  for (const auto& i : rhs) {
    result.insert(i);
  }
  return result;
}

NodeSet& operator|=(NodeSet& lhs, const NodeSet& rhs) {
  for (const auto& i : rhs) {
    lhs.insert(i);
  }
  return lhs;
}

NodeSet operator&(const NodeSet& lhs, const NodeSet& rhs) {
  NodeSet result;
  for (const auto& item : lhs) {
    if (rhs.contains(item)) {
      result.insert(item);
    }
  }
  return result;
}

NodeSet& operator&=(NodeSet& lhs, const NodeSet& rhs) {
  for (const auto& item : lhs) {
    if (!rhs.contains(item)) {
      lhs.erase(item);
    }
  }
  return lhs;
}

NodeSet operator-(const NodeSet& lhs, const NodeSet& rhs) {
  NodeSet result(lhs);
  for (const auto& item : lhs) {
    if (rhs.contains(item)) {
      result.erase(item);
    }
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const NodeSet& nodeSet) {
  if (nodeSet.size() == 0) {
    os << "[]";
    return os;
  }
  os << "[";
  for (int node_idx : nodeSet) {
    os << " " << node_idx;
  }
  os << "]";
  return os;
}

void PrintWorkList(const std::queue<WorklistItem>& workList) {
  std::queue<WorklistItem> tmp_wl(workList);
  std::cout << "work_list: [";
  int reserved_iter_idx = -1;
  while (!tmp_wl.empty()) {
    const auto cur_item = tmp_wl.front();
    if (reserved_iter_idx == -1) {
      reserved_iter_idx = cur_item.iter_idx;
    } else {
      assert((cur_item.iter_idx == reserved_iter_idx) && "iter_idx should be all the same.");
    }
    std::cout << " " << cur_item.node_idx;
    tmp_wl.pop();
  }
  std::cout << " ]: " << reserved_iter_idx << std::endl;
}

bool operator==(const NodeSet& ns, const std::vector<int>& vi) {
  NodeSet tmp_ns(vi.begin(), vi.end());
  return ns == tmp_ns;
}

bool operator>(const NodeSet& lhs, const NodeSet& rhs) {
  for (const auto& item : rhs) {
    if (!lhs.contains(item)) {
      return false;
    }
  }
  return lhs.size() > rhs.size();
}

bool operator<(const NodeSet& lhs, const NodeSet& rhs) {
  if (lhs.size() == rhs.size()) {
    return false;
  }
  return !(lhs > rhs);
}

std::ostream& operator<<(std::ostream& os, const std::vector<int>& intVec) {
  if (intVec.size() == 0) {
    os << "[]";
    return os;
  }
  os << "[";
  for (int t : intVec) {
    os << " " << t;
  }
  os << "]";
  return os;
}

NodeSet SubgraphNodesFromRoot(const int& rootNode, const Adjacencies& adjacencies,
                              const Direction& direction) {
  NodeSet result_nodeset;
  std::vector<int> stack;
  stack.emplace_back(rootNode);
  const absl::flat_hash_map<int, NodeSet>* adj;
  // auto t = adjacencies.control_adj_list[0];

  if (direction == forward) {
    adj = &(adjacencies.control_adj_list);
  } else {
    adj = &(adjacencies.control_reverse_adj_list);
  }
  while (!stack.empty()) {
    int cur_node = stack.back();
    result_nodeset.insert(cur_node);
    stack.pop_back();
    const auto search = adj->find(cur_node);
    if (search != adj->end()) {
      const auto& predecessors = search->second;
      for (const auto& pred : predecessors) {
        if (result_nodeset.contains(pred)) {
          continue;
        }
        stack.emplace_back(pred);
      }
    }
  }
  return result_nodeset;
}

NodeSet SubgraphNodesFromRoot(const int& rootNode, const Adjacencies& adjacencies,
                              const Direction& direction);

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
  // std::cout << "At the begining of GetRootList, the size of reverse_adj is: " << reverse_adj.size()
  //           << std::endl;
  for (auto iter = reverse_adj.begin(); iter != reverse_adj.end(); ++iter) {
    if (iter->second.size() == 0) {
      root_list.emplace_back(iter->first);
    }
  }
  // std::cout << "in GetRootList, roots are: " << root_list << std::endl;
  return root_list;
}

std::pair<std::vector<int>, int> PostOrderAndNumBackEdgeFromOneRoot(
    const absl::flat_hash_map<int, absl::flat_hash_set<int>>& adj, const int rootnode) {
  absl::flat_hash_map<int, int> color_map;
  int num_back_edge = 0;
  std::vector<int> post_order_list;
  color_map.reserve(adj.size());
  for (auto iter = adj.begin(); iter != adj.end(); ++iter) {
    color_map[iter->first] = -1;  // white
  }
  std::function<void(const int&)> dfs;
  dfs = [&](const int& start_node) -> void {
    color_map[start_node] = 0;  // grey
    const auto adj_iter = adj.find(start_node);
    if (adj_iter != adj.end()) {
      const auto& adj_nodes = adj_iter->second;
      // std::cout << "the number of its children is: " << adj_nodes.size() << std::endl;
      for (const auto child_node : adj_nodes) {
        if (color_map[child_node] == -1) {  // white
          // std::cout << "child " << child_node
          //           << " has not been visited, so we are going to visit it!" << std::endl;
          dfs(child_node);
        } else if (color_map[child_node] == 0) {  // grey
          num_back_edge++;
          // std::cout << "Back edge spotted! (" << start_node << ", " << child_node << ")"
                    // << std::endl;
        }
      }
      color_map[start_node] = 1;  // black
      post_order_list.push_back(start_node);
      // std::cout << "node " << start_node << " has been visted!" << std::endl;
    }
  };
  dfs(rootnode);
  return std::make_pair(post_order_list, num_back_edge);
}

}  // namespace yzd
