#include "yzd_dominance.h"

#include <algorithm>
#include <cassert>

#include "labm8/cpp/status_macros.h"

namespace yzd {
void YZDDominance::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
                                          // gens/kills 算好； adjacencies也算好
  for (int edge_idx = 0; edge_idx < program_graph.edge_size(); edge_idx++) {
    const programl::Edge& cur_edge = program_graph.edge(edge_idx);
    if (cur_edge.flow() == programl::Edge::CONTROL) {
      adjacencies.control_adj_list[cur_edge.source()].insert(cur_edge.target());
      adjacencies.control_reverse_adj_list[cur_edge.target()].insert(cur_edge.source());
      if (!adjacencies.control_adj_list.contains(cur_edge.target())) {
        adjacencies.control_adj_list[cur_edge.target()] = {};
      }
      if (!adjacencies.control_reverse_adj_list.contains(cur_edge.source())) {
        adjacencies.control_reverse_adj_list[cur_edge.source()] = {};
      }
      program_points.insert(cur_edge.source());
      program_points.insert(cur_edge.target());
      interested_points.insert(cur_edge.source());
      interested_points.insert(cur_edge.target());
      gens[cur_edge.source()].insert(cur_edge.source());
      gens[cur_edge.target()].insert(cur_edge.target());
      kills[cur_edge.source()];  // 其实不是很确定是不是这样就initialize了一个empty的NodeSet
      kills[cur_edge.target()];
    }
  }
}

void YZDDominance::ParseProgramGraph_new() {
  // 需要把program_points 和 interested_points 给算好;
  // gens/kills 算好； adjacencies也算好
  absl::flat_hash_map<int, int> old_node_to_new_node_map;
  absl::flat_hash_map<int, NodeSet> tmp_control_adj_list;
  absl::flat_hash_map<int, NodeSet> tmp_control_reverse_adj_list;
  for (int edge_idx = 0; edge_idx < program_graph.edge_size(); edge_idx++) {
    const programl::Edge& cur_edge = program_graph.edge(edge_idx);
    if (cur_edge.flow() == programl::Edge::CONTROL) {
      if (!tmp_control_adj_list.contains(cur_edge.target())) {
        tmp_control_adj_list[cur_edge.target()] = {};
      }
      if (!tmp_control_reverse_adj_list.contains(cur_edge.source())) {
        tmp_control_reverse_adj_list[cur_edge.source()] = {};
      }
      tmp_control_adj_list[cur_edge.source()].insert(cur_edge.target());
      tmp_control_reverse_adj_list[cur_edge.target()].insert(cur_edge.source());
    }
  }
  int num_node = tmp_control_adj_list.size();
  std::vector<int> old_nodes_list;
  old_nodes_list.reserve(num_node);
  for (const auto& item : tmp_control_adj_list) {
    old_nodes_list.push_back(item.first);
  }
  std::sort(old_nodes_list.begin(), old_nodes_list.end());

  for (int new_node_idx = 0; new_node_idx < num_node; new_node_idx++) {
    old_node_to_new_node_map[old_nodes_list[new_node_idx]] = new_node_idx;
    // old_nodes_list[new_node_idx] is the old node idx in the new position.
    program_points.insert(new_node_idx);
    interested_points.insert(new_node_idx);
    gens[new_node_idx].insert(new_node_idx);
    kills[new_node_idx];
  }
  for (const int& old_node : old_nodes_list) {
    const NodeSet& old_adj_list = tmp_control_adj_list[old_node];
    const NodeSet& old_adj_reverse_list = tmp_control_reverse_adj_list[old_node];
    int new_node = old_node_to_new_node_map[old_node];
    adjacencies.control_adj_list[new_node] = {};
    adjacencies.control_reverse_adj_list[new_node] = {};
    for (const int& old_adj_node : old_adj_list) {
      int new_adj_node = old_node_to_new_node_map[old_adj_node];
      adjacencies.control_adj_list[new_node].insert(new_adj_node);
    }
    for (const int& old_adj_reverse_node : old_adj_reverse_list) {
      int new_adj_reverse_node = old_node_to_new_node_map[old_adj_reverse_node];
      adjacencies.control_reverse_adj_list[new_node].insert(old_adj_reverse_node);
    }
  }
}

labm8::Status YZDDominance::ValidateWithPrograml() {
  RETURN_IF_ERROR(programl_dominance_analysis.Init());
  // const auto programl_eligiable_roots = programl_dominance_analysis.GetEligibleRootNodes();
  // std::cout << "programl eligiable roots include: ";
  // for (const auto& item : programl_eligiable_roots){
  //   std::cout << item << ", ";
  // }
  // std::cout << std::endl;
  if (analysis_setting.sync_or_async == async) {
    RETURN_IF_ERROR(Init_async());
  } else {
    RETURN_IF_ERROR(Init_sync());
  }
  const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
  absl::flat_hash_map<int, yzd::NodeSet> programl_dominators;
  int programl_dataflow_step_count;
  int sim_count = 0;
  // std::cout << "we are here in line 33 yzd_dominance.cc~~~ " << std::endl;
  // for (const int pp : program_points) {
  // for (int pp = 1; pp < program_graph.node_size(); pp++) {
  // 这个是为了模仿data_flow_pass.cc line170 的
  // GetInstructionsInFunctionsNodeIndices里的逻辑(虽然我没懂为啥0是rootnode)

  // if (!(program_graph.node(pp).type() == programl::Node::INSTRUCTION)) {
  //   continue;
  // }

  // 以下这些是为了测试111
  absl::flat_hash_map<int, yzd::NodeSet> programl_dominators_for_test;
  int programl_dataflow_step_count_for_test;
  for (const auto& pp : programl_dominance_analysis.GetEligibleRootNodes()) {
    RETURN_IF_ERROR(programl_dominance_analysis.ComputeDominators(
        pp, &programl_dataflow_step_count_for_test, &programl_dominators_for_test));
    std::cout << pp << " : result from pro(test): " << programl_dominators_for_test[pp]
              << std::endl;
  }
  // 测试内容结束111

  for (const auto& pp : programl_dominance_analysis.GetEligibleRootNodes()) {
    RETURN_IF_ERROR(programl_dominance_analysis.ComputeDominators(pp, &programl_dataflow_step_count,
                                                                  &programl_dominators));
    // std::cout << "we are here in line 37 yzd_dominance.cc~~~ pp idx: " << pp << std::endl;
    const auto yzd_iter = yzd_last_result.find(pp);
    if (!(yzd_iter == yzd_last_result.end())) {
      assert((program_graph.node(pp).type() == programl::Node::INSTRUCTION) &&
             "The intersected node should be an instruction node!");
      if (!(yzd_iter->second == programl_dominators[pp])) {
        std::cout << "inconsistency occurs! program_point is: " << pp << std::endl;
        std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        std::cout << "result from pro: " << programl_dominators[pp] << std::endl;
        std::cout << "diff = " << yzd_iter->second - programl_dominators[pp] << std::endl;
        continue;
        return labm8::Status(labm8::error::ABORTED, "The validation did not pass!");
      } else {
        std::cout << "sim for pp: " << pp << std::endl;
        std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        std::cout << "result from pro: " << programl_dominators[pp] << std::endl;
        sim_count++;
      }
    }
  }
  std::cout << "sim_count = " << sim_count << std::endl;
  return labm8::Status::OK;
}
}  // namespace yzd