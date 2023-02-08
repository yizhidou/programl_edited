// #pragma once
#include "yzd_analysis_base.h"

#include <cassert>
#include <iostream>
// #include "yzd_utils.h"
// #include "labm8/cpp/logging.h"

namespace yzd {

labm8::Status AnalysisBase::InitSettings() {
  // 这里应该check一下num_point_interested和num_point_interested这两个数值得是大于0的
  if ((interested_points.size() == 0) || (program_points.size() == 0)) {
    return labm8::Status(labm8::error::FAILED_PRECONDITION,
                         "The graph has either none program points or none interested points");
  }

  // add result of iteration 0 into result_pointers.
  absl::flat_hash_map<int, int> result_pointer_zero_iteration;
  result_pointer_zero_iteration.reserve(GetNumProgramPoints());
  if (analysis_setting.initialize_mode == allzeros) {
    stored_nodesets
        .emplace_back();  // 我不是很确定这个emplace_back()是不是就给vector加入了一个空的NodeSet?
  } else if (analysis_setting.initialize_mode == allones) {
    stored_nodesets.emplace_back(interested_points.begin(), interested_points.end());
  }
  assert((stored_nodesets.size() == 1) &&
         "By now, there should be only one element in the stored_result set!");
  for (const int pp : program_points) {
    result_pointer_zero_iteration[pp] = 0;
    // std::cout << "result_pointer_zero_iteration of " << pp << " is "
    //           << result_pointer_zero_iteration[pp] << std::endl;
  }
  result_pointers.push_back(result_pointer_zero_iteration);
  return labm8::Status::OK;
}

NodeSet AnalysisBase::MeetOperation(const int iterIdx,
                                    const absl::flat_hash_set<int>& targetNodeList) {
  assert(iterIdx < result_pointers.size());
  NodeSet meet_result = analysis_setting.may_or_must == may
                            ? NodeSet()
                            : NodeSet(interested_points.begin(), interested_points.end());

  for (const int t : targetNodeList) {
    if (analysis_setting.may_or_must == may) {
      meet_result |= stored_nodesets[result_pointers[iterIdx][t]];
    } else {
    }
  }

  return meet_result;
}

labm8::Status AnalysisBase::Run(programl::ResultsEveryIteration* resultsOfAllIterations) {
  ParseProgramGraph();  // 需要把program_points 和 interested_points 给算好;
                        // adjacencies也算好。这是一个纯虚函数。
  labm8::Status init_setting_status =
      InitSettings();  // 这个主要作用往stored_result_set里加初始的结果
  if (!init_setting_status.ok()) {
    return init_setting_status;
  }

  // add all nodes in worklist
  for (const int pp : program_points) {
    work_list.emplace(1, pp);
  }
  std::cout << "till line 69, worklist size: " << work_list.size() << std::endl;
  while (!work_list.empty()) {
    WorklistItem cur_item = work_list.front();
    work_list.pop();
    int cur_iter_idx = cur_item.iter_idx, cur_node_idx = cur_item.node_idx;
    if (cur_iter_idx > analysis_setting.max_iteration) {
      return labm8::Status(labm8::error::FAILED_PRECONDITION,
                           "Failed to terminate liveness computation in certain steps");
    }

    if (cur_iter_idx > result_pointers.size() - 1) {
      std::cout << "till line 80, cur_iter_idx: " << cur_iter_idx << std::endl;
      result_pointers.push_back(
          result_pointers.back());  // copy the result of the last iteration and push into result.
    }

    // std::cout << "till line 85, cur_node_idx: " << cur_node_idx << std::endl;
    // assert((result_pointers[cur_iter_idx].contains(cur_node_idx)) &&
    //        "line 86, cur_node_idx should be in that hash map");
    // NodeSet* tmp_ptr = result_pointers[cur_iter_idx][cur_node_idx];
    // std::cout << "till line 89, tmp_ptr:" << tmp_ptr
    //           << "; &stored_nodesets.back():" << &stored_nodesets.back()
    //           << "; stored_nodesets.size(): " << stored_nodesets.size() << std::endl;
    NodeSet meeted_nodeset = stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]];
    std::cout << "till line 93, cur_node_idx: " << cur_node_idx << std::endl;
    const auto& neighbor_nodes =
        (analysis_setting.direction == forward)
            ? adjacencies.control_reverse_adj_list[cur_node_idx]  // meet all predecessors
            : adjacencies.control_adj_list[cur_node_idx];         // meet all successors
    if (!neighbor_nodes.empty()) {
      meeted_nodeset = MeetOperation(cur_iter_idx - 1, neighbor_nodes);
    }

    NodeSet temp = meeted_nodeset - kills[cur_node_idx];
    NodeSet updated_bitvector =
        gens[cur_node_idx] | (temp);  // 这两句是真的有点奇怪，合并到一起就会提示有错
    if (updated_bitvector == stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]) {
      continue;
    }
    // if it changes, update the result
    stored_nodesets.push_back(updated_bitvector);
    result_pointers[cur_iter_idx][cur_node_idx] = stored_nodesets.size() - 1;

    // add predecessors/successors to worklist
    const auto& affected_list =
        analysis_setting.direction == forward
            ? adjacencies.control_adj_list[cur_node_idx]           // add all successors
            : adjacencies.control_reverse_adj_list[cur_node_idx];  // add all predecessors

    for (auto affted_node : affected_list) {
      work_list.emplace(cur_iter_idx + 1, affted_node);
    }
  }

  // 接下来就是把result_pointers写到resultsOfAllIterations里
  resultsOfAllIterations->set_task_name(analysis_setting.TaskNameToString());

  programl::Int64List program_points_message, interested_points_message;
  program_points_message.mutable_value()->Add(program_points.begin(), program_points.end());
  *(resultsOfAllIterations->mutable_program_points()) = program_points_message;  // 这样写应该对吧？
  interested_points_message.mutable_value()->Add(interested_points.begin(),
                                                 interested_points.end());
  *(resultsOfAllIterations->mutable_interested_points()) = interested_points_message;

  for (int iter_idx = 0; iter_idx < result_pointers.size(); iter_idx++) {
    const absl::flat_hash_map<int, int>& result_pointer = result_pointers[iter_idx];
    programl::ResultOneIteration result_one_iteration_message;
    for (const auto& it : result_pointer) {
      // *(result_one_iteration.mutable_result_one_iteration())[it.first] = *(it.second);
      const NodeSet & tmp_nodeset = stored_nodesets[it.second];
      programl::Int64List nodeset_message;
      nodeset_message.mutable_value()->Add(
          tmp_nodeset.begin(),
          tmp_nodeset.end());  // 这个不确定能不能work，因为it.second是set其实。不行的话就换下面
      // for (const int node : *it.second) {
      //   nodeset_message.add_value(node);
      // }
      (*result_one_iteration_message.mutable_result_map())[it.first] = nodeset_message;
    }

    *resultsOfAllIterations->add_results_every_iteration() = result_one_iteration_message;
  }
  return labm8::Status::OK;
}

}  // namespace yzd
