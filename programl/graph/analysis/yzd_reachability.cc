#include "yzd_reachability.h"

#include "labm8/cpp/status_macros.h"

namespace yzd {
void YZDReachability::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
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
labm8::Status YZDReachability::ValidateWithPrograml() {
  // std::cout << "we are good at line 25, validate.cc" << std::endl;
  RETURN_IF_ERROR(programl_reachability_analysis.Init());
  // std::cout << "we are good at line 27, validate.cc" << std::endl;
  RETURN_IF_ERROR(Init());
  // std::cout << "we are good at line 29, validate.cc" << std::endl;
  const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
  //   absl::flat_hash_map<int, yzd::NodeSet> programl_reachability_result;
  int sim_count = 0, diff_count = 0;
  // std::cout << "we are good at line 33, validate.cc" << std::endl;
  for (const auto& pp : programl_reachability_analysis.GetEligibleRootNodes()) {
    RETURN_IF_ERROR(programl_reachability_analysis.CalculateResultFromRootNode(pp));
    // std::cout << "we are good at line 36, validate.cc. current rootNode = " << pp << std::endl;
    NodeSet programl_result_for_this_root = programl_reachability_analysis.GetResultFromRootNode();

    const auto yzd_iter = yzd_last_result.find(pp);
    if (!(yzd_iter == yzd_last_result.end())) {
      assert((program_graph.node(pp).type() == programl::Node::INSTRUCTION) &&
             "The intersected node should be an instruction node!");
      if (!(yzd_iter->second == programl_result_for_this_root)) {
        std::cout << "inconsistency occurs! program_point is: " << pp << std::endl;
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
        sim_count++;
      }
    }
  }
  std::cout << "sim_count = " << sim_count << "; dif_count = " << diff_count << std::endl;
  return labm8::Status::OK;
}

}  // namespace yzd
