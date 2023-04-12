#include "yzd_liveness.h"

#include <cassert>
#include <iostream>

namespace yzd {
void YZDLiveness::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
                                         // gens/kills 算好； adjacencies也算好
  int control_edge_count = 0, data_edge_count = 0, non_empty_gens = 0, non_empty_kills = 0;
  for (const auto& edge : program_graph.edge()) {
    if (edge.flow() == programl::Edge::CONTROL) {
      control_edge_count++;

      adjacencies.control_adj_list[edge.source()].insert(edge.target());
      adjacencies.control_reverse_adj_list[edge.target()].insert(edge.source());
      if (!adjacencies.control_adj_list.contains(edge.target())) {
        adjacencies.control_adj_list[edge.target()] = {};
      }
      if (!adjacencies.control_reverse_adj_list.contains(edge.source())) {
        adjacencies.control_reverse_adj_list[edge.source()] = {};
      }
      program_points.insert(edge.source());
      program_points.insert(edge.target());
    } else if (edge.flow() == programl::Edge::DATA) {
      data_edge_count++;

      // if (program_graph.node(edge.source()).type() == programl::Node::INSTRUCTION) {  // def edge
      //   assert((program_graph.node(edge.target()).type() == programl::Node::VARIABLE) &&
      //          "The target of this DataEdge should be Variable node!");
      //   program_points.insert(edge.source());
      //   interested_points.insert(edge.target());
      //   kills[edge.source()].insert(edge.target());
      // } else if (program_graph.node(edge.source()).type() ==
      //            programl::Node::VARIABLE) {  // use edge
      //   assert((program_graph.node(edge.target()).type() == programl::Node::INSTRUCTION) &&
      //          "The target of this DataEdge should be Instruction node!");
      //   interested_points.insert(edge.source());
      //   program_points.insert(edge.target());
      //   gens[edge.target()].insert(edge.source());
      // }
      // else {
      //   assert((program_graph.node(edge.source()).type() == programl::Node::CONSTANT) &&
      //          (program_graph.node(edge.target()).type() == programl::Node::INSTRUCTION) &&
      //          "I suppose there should not be the fourth way of generating DataEdge...");
      // }

      // modified for test
      if (program_graph.node(edge.source()).type() == programl::Node::INSTRUCTION) {  // def edge
        program_points.insert(edge.source());
        interested_points.insert(edge.target());
        kills[edge.source()].insert(edge.target());
      } else {  // use edge
        assert((program_graph.node(edge.target()).type() == programl::Node::INSTRUCTION) &&
               "the target of this edge should be an instruction");
        interested_points.insert(edge.source());
        program_points.insert(edge.target());
        gens[edge.target()].insert(edge.source());
      }
    }
  }

  std::cout << "num_control_edge " << control_edge_count << std::endl;
  std::cout << "num_data_edge " << data_edge_count << std::endl;
  std::cout << "num_program_points " << program_points.size() << std::endl;
  std::cout << "num_interested_points_liveness " << interested_points.size() << std::endl;
  for (const auto pp : program_points) {
    if (!(gens[pp].size() == 0)) {
      non_empty_gens++;
    }
    if (!(kills[pp].size() == 0)) {
      non_empty_kills++;
    }
  }
  std::cout << "num_none_empty_gens_liveness " << non_empty_gens << std::endl;
  std::cout << "num_none_empty_kills_liveness " << non_empty_kills << std::endl;
}

labm8::Status YZDLiveness::ValidateWithPrograml() {
  RETURN_IF_ERROR(programl_liveness_analysis.Init());
  RETURN_IF_ERROR(Init());
  // 接下来应该就是对比最后一个iteration和programl的livein是不是一致了
  const std::vector<absl::flat_hash_set<int>> programl_result =
      programl_liveness_analysis.live_in_sets();
  const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
  int sim_count = 0;
  for (int node_idx = 0; node_idx < programl_result.size(); node_idx++) {
    const auto yzd_iter = yzd_last_result.find(node_idx);
    if (!(yzd_iter == yzd_last_result.end())) {
      assert((program_graph.node(node_idx).type() == programl::Node::INSTRUCTION) &&
             "The intersected node should be an instruction node!");
      if (!(yzd_iter->second == programl_result[node_idx])) {
        return labm8::Status(labm8::error::ABORTED, "The validation did not pass!");
      }
      else{
        // std::cout << "program point: " << node_idx << std::endl;
        // std::cout << "result from yzd: " << yzd_iter->second << std::endl;
        // std::cout << "result from pro: " << programl_result[node_idx] << std::endl;
        sim_count++;
      }
    }
    //   if (!(yzd_iter == yzd_last_result.end()) && !(yzd_iter->second ==
    //   programl_result[node_idx])) {
    //     // diff_count++;
    //     assert((program_graph.node(node_idx).type() == programl::Node::INSTRUCTION) &&
    //            "The intersected node should be an instruction node!");
    //     return labm8::Status(labm8::error::ABORTED, "The validation did not pass!");
    //   }
  }
  std::cout << "sim_count = " << sim_count << std::endl;
  return labm8::Status::OK;
}

}  // namespace yzd
