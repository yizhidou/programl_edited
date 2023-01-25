#pragma once
#include <cassert>
#include "yzd_analysis.h"
// #include "yzd_utils.h"
#include "labm8/cpp/logging.h"


namespace yzd {

void AnalysisBase::InitSettings() {
  // 这里应该check一下num_point_interested和num_point_interested这两个数值得是大于0的

  // add result of iteration 0 into result_pointers.
  if (analysis_setting.initialize_mode == allzeros) {
    // auto emplaced = stored_result_set.emplace(GetNumInterestedPoints(), 0);
    // result_pointers.emplace_back(GetNumProgramPoints(), &(*emplaced.first));
    stored_nodesets.emplace_back();
    assert((stored_nodesets.size() == 1) && "By now, there should be only one element in the stored_result set!");
    result_pointers.emplace_back(GetNumProgramPoints(), &stored_nodesets.back());
  } else if (analysis_setting.initialize_mode == allones) {
    // auto emplaced = stored_result_set.emplace(GetNumInterestedPoints(), 1);
    // result_pointers.emplace_back(GetNumInterestedPoints(), &(*emplaced.first));
    stored_nodesets.emplace_back(interested_points.begin(), interested_points.end());
    assert((stored_nodesets.size() == 1) && "By now, there should be only one element in the stored_result set!");
    result_pointers.emplace_back(GetNumProgramPoints(), &stored_nodesets.back());
  }
}

NodeSet AnalysisBase::MeetOperation(const int iterIdx, const absl::flat_hash_set<int>& targetNodeList) {
    if (targetNodeList.empty()) {
      // TODO exception handling
    }
    if (iterIdx > (result_pointers.size() - 1)) {
      // TODO exception handling
    }

    NodeSet meet_result = analysis_setting.may_or_must == may
                                ? NodeSet()
                                : NodeSet(interested_points.begin(), interested_points.end());

    for (auto t : targetNodeList) {
      if (analysis_setting.may_or_must == may) {
        meet_result |= *(result_pointers[iterIdx][t]);
      } else {
        meet_result &= *(result_pointers[iterIdx][t]);
      }
    }

    return meet_result;
  }

void AnalysisBase::Run(programl::ResultsEveryIteration* resultsOfAllIterations) {
    // add all nodes in worklist
    for (int pp : program_points) {
      work_list.emplace(1, pp);
    }
    
    while (!work_list.empty()) {
      WorklistItem cur_item = work_list.front();
      work_list.pop();
      int cur_iter_idx = cur_item.iter_idx, cur_node_idx = cur_item.node_idx;

      if (cur_iter_idx >= result_pointers.size()) {
        result_pointers.push_back(
            result_pointers.back());  // copy the result of the last iteration and push into result.
      }

      NodeSet meeted_bitvector;
      if (analysis_setting.direction == forward) {  // meet all predecessors
        if (adjacencies.control_reverse_adj_list[cur_node_idx].empty()) {
          continue;
        }
        meeted_bitvector = MeetOperation(cur_iter_idx - 1, adjacencies.control_reverse_adj_list[cur_node_idx]);

      } else {  // meet all successors
        if (adjacencies.control_adj_list[cur_node_idx].empty()) {
          continue;
        }
        meeted_bitvector = MeetOperation(cur_iter_idx - 1, adjacencies.control_adj_list[cur_node_idx]);
      }

      NodeSet temp = meeted_bitvector - kills[cur_node_idx];
      NodeSet updated_bitvector = gens[cur_node_idx] | (temp);  // 这两句是真的有点奇怪，合并到一起就会提示有错
      if (updated_bitvector == *(result_pointers[cur_iter_idx][cur_node_idx])) {
        continue;
      }
      // if it changes, update the result
      // auto inserted = stored_result_set.insert(updated_bitvector);
      // result_pointers[cur_iter_idx][cur_node_idx] = &(*inserted.first);
      stored_nodesets.push_back(updated_bitvector);
      result_pointers[cur_iter_idx][cur_node_idx] = &(stored_nodesets.back());

      // add predecessors/successors to worklist
      absl::flat_hash_set<int>& affected_list = analysis_setting.direction == forward
                                            ? adjacencies.control_adj_list[cur_node_idx]
                                            : adjacencies.control_reverse_adj_list[cur_node_idx];

      for (auto affted_node : affected_list) {
        work_list.emplace(cur_iter_idx + 1, affted_node);
      }
    }

    //接下来就是把result_pointers写到resultsOfAllIterations里
    //先写program_points和interested_points, 还有task_name
    resultsOfAllIterations->set_task_name(analysis_setting.TaskNameToString());

    programl::Int64List program_points_list, interested_points_list;
    program_points_list.mutable_value()->Add(program_points.begin(), program_points.end());
    *(resultsOfAllIterations->mutable_program_points()) = program_points_list; // 这样写应该对吧？ should be double check!

    interested_points_list.mutable_value()->Add(interested_points.begin(), interested_points.end());
    *(resultsOfAllIterations->mutable_interested_points()) = interested_points_list;
    
    for (int iter_idx = 0; iter_idx < result_pointers.size(); iter_idx ++){
      const absl::flat_hash_map<int, NodeSet*>& result_pointers_one_iteration = result_pointers[iter_idx];
      programl::ResultOneIteration result_one_iteration_message;
      for(const auto& it: result_pointers_one_iteration){
        // *(result_one_iteration.mutable_result_one_iteration())[it.first] = *(it.second);
        programl::Int64List tmp; 
        tmp.mutable_value()->Add(it.second->begin(), it.second->end());
        (*result_one_iteration_message.mutable_result_map())[it.first] = tmp; //这个地方我怀疑不太对，因为原本存的是指针来着
      }
      programl::ResultOneIteration* added_one_iteration_result_ptr = resultsOfAllIterations->add_results_every_iteration();
      *added_one_iteration_result_ptr = result_one_iteration_message;
      

    }
  }

}  // namespace programl
