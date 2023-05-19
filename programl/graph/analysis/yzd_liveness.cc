#include "yzd_liveness.h"

#include <cassert>
#include <iostream>

#include "labm8/cpp/status_macros.h"

namespace yzd {
void YZDLiveness::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
                                         // gens/kills 算好； adjacencies也算好
  std::cout << "yzd_liveness: We do not rearange node idx!" << std::endl;
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

labm8::Status YZDLiveness::ParseProgramGraph_idx_reorganized() {
  // 需要把program_points 和 interested_points 给算好;
  // gens/kills 算好； adjacencies也算好;
  // idx_reorganized_map设置好
  std::cout << "yzd_liveness: We do rearange node idx!" << std::endl;
  NodeSet tmp_program_points, tmp_interested_points;
  for (const auto& edge : program_graph.edge()) {
    if (edge.flow() == programl::Edge::CONTROL) {
      tmp_program_points.insert(edge.source());
      tmp_program_points.insert(edge.target());
    } else if (edge.flow() == programl::Edge::DATA) {
      tmp_program_points.insert(edge.source());
      tmp_interested_points.insert(edge.target());
    }
  }
  node_idx_map.reserve(tmp_program_points.size() + tmp_interested_points.size());
  RETURN_IF_ERROR(RearangeNodeIdx(tmp_program_points));
  RETURN_IF_ERROR(RearangeNodeIdx(tmp_interested_points));

  for (const auto& edge : program_graph.edge()) {
    int new_source = -1, new_target = 1;
    if (edge.flow() == programl::Edge::CONTROL) {
      new_source = node_idx_map[edge.source()];
      new_target = node_idx_map[edge.target()];
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
    } else if (edge.flow() == programl::Edge::DATA) {
      new_source = node_idx_map[edge.source()];
      new_target = node_idx_map[edge.target()];
      if (program_graph.node(edge.source()).type() == programl::Node::INSTRUCTION) {  // def edge
        program_points.insert(new_source);
        interested_points.insert(new_target);
        kills[new_source].insert(new_target);
      } else {  // use edge
        assert((program_graph.node(edge.target()).type() == programl::Node::INSTRUCTION) &&
               "the target of this edge should be an instruction");
        interested_points.insert(new_source);
        program_points.insert(new_target);
        gens[new_target].insert(new_source);
      }
    }
  }
  return labm8::Status::OK;
}

labm8::Status YZDLiveness::ValidateWithPrograml() {
  RETURN_IF_ERROR(programl_liveness_analysis.Init());
  if (analysis_setting.sync_or_async == async) {
    RETURN_IF_ERROR(Init_async());
  } else {
    RETURN_IF_ERROR(Init_sync());
  }
  // 接下来应该就是对比最后一个iteration和programl的livein是不是一致了
  const std::vector<absl::flat_hash_set<int>> programl_result =
      programl_liveness_analysis.live_in_sets();
  const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
  int sim_count = 0;
  for (int node_idx = 0; node_idx < programl_result.size(); node_idx++) {
    int test_node = -1;
    if (analysis_setting.index_reorganized) {
      // 在节点重新编号了的情况下，idx_rearanged_map.keys()存着原始的program points
      if (!(node_idx_map.contains(node_idx))) {
        // 所以就是如果node_idx没有在原始的program points里，就跳过
        continue;
      }
      // 如果在，取它的新编号
      test_node = node_idx_map[node_idx];
    } else {
      test_node = node_idx;
    }
    const auto yzd_iter = yzd_last_result.find(test_node);

    // const auto yzd_iter = yzd_last_result.find(node_idx);
    if (!(yzd_iter == yzd_last_result.end())) {
      assert((program_graph.node(node_idx).type() == programl::Node::INSTRUCTION) &&
             "The intersected node should be an instruction node!");
      NodeSet programl_result_for_this_test_node;
      if (analysis_setting.index_reorganized) {
        // 需要remap的情形
        RETURN_IF_ERROR(GetRemapedNodeset(programl_result_for_this_test_node,
                                          programl_result[node_idx], node_idx_map));
      } else {
        // 不需要remap的情形
        programl_result_for_this_test_node = programl_result[node_idx];
      }
      if (!(yzd_iter->second == programl_result_for_this_test_node)) {
        return labm8::Status(labm8::error::ABORTED, "The validation did not pass!");
      } else {
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
  if (sim_count == GetNumProgramPoints()) {
    std::cout << "validation passed~" << std::endl;
  } else {
    std::cout << "validation **did not** passed!!!" << std::endl;
  }
  return labm8::Status::OK;
}

}  // namespace yzd
