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

}  // namespace yzd
