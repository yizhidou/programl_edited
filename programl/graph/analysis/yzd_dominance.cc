#include "yzd_dominance.h"

#include <algorithm>
#include <cassert>

#include "labm8/cpp/status_macros.h"

namespace yzd {
void YZDDominance::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
                                          // gens/kills 算好； adjacencies也算好
  std::cout << "yzd_dominance: We do not rearange node idx!" << std::endl;
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

labm8::Status YZDDominance::ParseProgramGraph_idx_reorganized() {
  // 需要把program_points 和 interested_points 给算好;
  // gens/kills 算好； adjacencies也算好
  std::cout << "yzd_dominance: We do rearange node idx!" << std::endl;
  NodeSet tmp_program_points;
  for (const auto& cur_edge : program_graph.edge()) {
    if (cur_edge.flow() == programl::Edge::CONTROL) {
      tmp_program_points.insert(cur_edge.source());
      tmp_program_points.insert(cur_edge.target());
    }
  }
  node_idx_map.reserve(tmp_program_points.size());
  RETURN_IF_ERROR(RearangeNodeIdx(tmp_program_points));

  for (const auto& cur_edge : program_graph.edge()) {
    int new_source = -1, new_target = -1;
    if (cur_edge.flow() == programl::Edge::CONTROL) {
      new_source = node_idx_map[cur_edge.source()];
      new_target = node_idx_map[cur_edge.target()];

      adjacencies.control_adj_list[new_source].insert(new_target);
      adjacencies.control_reverse_adj_list[new_target].insert(new_source);
      if (!adjacencies.control_adj_list.contains(new_target)) {
        adjacencies.control_adj_list[new_target] = {};
      }
      if (!adjacencies.control_reverse_adj_list.contains(new_source)) {
        adjacencies.control_reverse_adj_list[new_source] = {};
      }
      program_points.insert(new_source);
      program_points.insert(new_target);
      interested_points.insert(new_source);
      interested_points.insert(new_target);
      gens[new_source].insert(new_source);
      gens[new_target].insert(new_target);
      kills[new_source];  // 其实不是很确定是不是这样就initialize了一个empty的NodeSet
      kills[new_target];
    }
  }
  return labm8::Status::OK;
}

labm8::Status YZDDominance::ValidateWithPrograml() {
  RETURN_IF_ERROR(programl_dominance_analysis.Init());
  // const auto programl_eligiable_roots = programl_dominance_analysis.GetEligibleRootNodes();
  // std::cout << "programl eligiable roots include: ";
  // for (const auto& item : programl_eligiable_roots){
  //   std::cout << item << ", ";
  // }
  // std::cout << std::endl;
  std::cout << "We have entered yzd_dominance:ValidateWithPrograml~" << std::endl;
  if (analysis_setting.sync_or_async == async) {
    RETURN_IF_ERROR(Init_async());
  } else {
    RETURN_IF_ERROR(Init_sync());
  }
  const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
  absl::flat_hash_map<int, yzd::NodeSet> programl_dominators;
  int programl_dataflow_step_count;
  int sim_count = 0;

  // 以下这些是为了测试111
  // absl::flat_hash_map<int, yzd::NodeSet> programl_dominators_for_test;
  // int programl_dataflow_step_count_for_test;
  // for (const auto& pp : programl_dominance_analysis.GetEligibleRootNodes()) {
  //   RETURN_IF_ERROR(programl_dominance_analysis.ComputeDominators(
  //       pp, &programl_dataflow_step_count_for_test, &programl_dominators_for_test));
  //   std::cout << pp << " : result from pro(test): " << programl_dominators_for_test[pp]
  //             << std::endl;
  // }
  // 测试内容结束111

  for (const auto& eligible_pp_from_programl : programl_dominance_analysis.GetEligibleRootNodes()) {
    RETURN_IF_ERROR(programl_dominance_analysis.ComputeDominators(
        eligible_pp_from_programl, &programl_dataflow_step_count, &programl_dominators));
    // std::cout << "we are here in line 37 yzd_dominance.cc~~~ pp idx: " << pp << std::endl;
    int test_node = -1;
    NodeSet programl_result_for_this_test_node;
    if (analysis_setting.index_reorganized) {
      if (!(node_idx_map.contains(eligible_pp_from_programl))) {
        continue;
      }
      test_node = node_idx_map[eligible_pp_from_programl];
      RETURN_IF_ERROR(GetRemapedNodeset(programl_result_for_this_test_node,
                                        programl_dominators[eligible_pp_from_programl],
                                        node_idx_map));
    } else {
      test_node = eligible_pp_from_programl;
      programl_result_for_this_test_node = programl_dominators[eligible_pp_from_programl];
    }
    const auto yzd_iter = yzd_last_result.find(test_node);
    if (yzd_iter != yzd_last_result.end()) {
      assert(
          (program_graph.node(eligible_pp_from_programl).type() == programl::Node::INSTRUCTION) &&
          "The intersected node should be an instruction node!");
      // if (!(yzd_iter->second == programl_dominators[pp])) {
      if (!(yzd_iter->second == programl_result_for_this_test_node)) {
        std::cout << "inconsistency occurs! program_point is: " << test_node << std::endl;
        std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        std::cout << "result from pro: " << programl_result_for_this_test_node << std::endl;
        std::cout << "diff = " << yzd_iter->second - programl_result_for_this_test_node
                  << std::endl;
        continue;
        return labm8::Status(labm8::error::ABORTED, "The validation did not pass!");
      } else {
        std::cout << "sim for pp: " << test_node << std::endl;
        std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        std::cout << "result from pro: " << programl_result_for_this_test_node << std::endl;
        sim_count++;
      }
    }
  }
  std::cout << "sim_count = " << sim_count << std::endl;
  return labm8::Status::OK;
}
}  // namespace yzd