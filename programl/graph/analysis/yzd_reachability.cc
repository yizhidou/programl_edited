#include "yzd_reachability.h"

#include "labm8/cpp/status_macros.h"

namespace yzd {
void YZDReachability::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
                                             // gens/kills 算好； adjacencies也算好
  std::cout << "yzd_reachability: We do not rearange node idx!" << std::endl;
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

labm8::Status YZDReachability::ParseProgramGraph_idx_reorganized() {
  // 需要把program_points 和 interested_points 给算好;
  // gens/kills 算好； adjacencies也算好
  // std::cout << "yzd_reachability: We do rearange node idx!" << std::endl;
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

  // std::cout << "all pp are: " << program_points << std::endl;
  // std::cout << "and the map is: " << std::endl;
  // for (auto iter = node_idx_map.begin(); iter != node_idx_map.end(); ++iter) {
  //   std::cout << iter->first << ": " << iter->second << std::endl;
  // }
  // std::cout << "till yzd_reachability.cc line 76 all good" << std::endl;
  return labm8::Status::OK;
}

labm8::Status YZDReachability::ValidateWithPrograml() {
  // std::cout << "we are good at line 25, validate.cc" << std::endl;
  RETURN_IF_ERROR(programl_reachability_analysis.Init());
  // std::cout << "we are good at line 27, validate.cc" << std::endl;
  if (analysis_setting.sync_or_async == async) {
    RETURN_IF_ERROR(Init_async());
  } else {
    RETURN_IF_ERROR(Init_sync());
  }
  // std::cout << "we are good at line 29, validate.cc" << std::endl;
  const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
  //   absl::flat_hash_map<int, yzd::NodeSet> programl_reachability_result;
  int sim_count = 0, diff_count = 0;

  for (const auto& eligible_pp_from_programl :
       programl_reachability_analysis.GetEligibleRootNodes()) {
    RETURN_IF_ERROR(
        programl_reachability_analysis.CalculateResultFromRootNode(eligible_pp_from_programl));
    // std::cout << "we are good at line 36, validate.cc. current rootNode = " << pp << std::endl;
    NodeSet programl_result_for_this_root;
    int test_node = -1;
    if (analysis_setting.index_reorganized) {
      if (!node_idx_map.contains(eligible_pp_from_programl)) {
        continue;
      }
      test_node = node_idx_map[eligible_pp_from_programl];
      RETURN_IF_ERROR(GetRemapedNodeset(programl_result_for_this_root,
                                        programl_reachability_analysis.GetResultFromRootNode(),
                                        node_idx_map));
    } else {
      test_node = eligible_pp_from_programl;
      programl_result_for_this_root = programl_reachability_analysis.GetResultFromRootNode();
    }

    const auto yzd_iter = yzd_last_result.find(test_node);
    if (!(yzd_iter == yzd_last_result.end())) {
      assert(
          (program_graph.node(eligible_pp_from_programl).type() == programl::Node::INSTRUCTION) &&
          "The intersected node should be an instruction node!");
      if (!(yzd_iter->second == programl_result_for_this_root)) {
        std::cout << "inconsistency occurs! program_point is: " << eligible_pp_from_programl
                  << std::endl;
        std::cout << "corresponding test_node is: " << test_node << std::endl;
        std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        std::cout << "result from pro: " << programl_result_for_this_root << std::endl;
        // std::cout << "diff = " << yzd_iter->second - programl_dominators[pp] << std::endl;
        // continue;
        diff_count++;
        return labm8::Status(labm8::error::ABORTED, "The validation did not pass!");
      } else {
        // std::cout << "sim for pp: " << pp << std::endl;
        // std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        // std::cout << "result from pro: " << programl_result_for_this_root << std::endl;
        // below is for test
        // std::cout << "result for node " << eligible_pp_from_programl << " is the same~"
        //           << std::endl;
        // if (eligible_pp_from_programl == 4) {
        //   std::cout << "specifically check node 4:" << std::endl;
        //   std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        //   std::cout << "result from pro: " << programl_result_for_this_root << std::endl;
        // }
        // test ends
        sim_count++;
      }
    }
  }
  std::cout << "sim_count = " << sim_count << "; dif_count = " << diff_count << std::endl;
  return labm8::Status::OK;
}

}  // namespace yzd
