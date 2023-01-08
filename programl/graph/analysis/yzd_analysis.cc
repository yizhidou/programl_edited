#pragma once
#include "yzd_analysis.h"

namespace programl {

void AnalysisBase::InitSettings() {
  // 这里应该check一下num_point_interested和num_point_interested这两个数值得是大于0的

  // add result of iteration 0 into result_pointers.
  if (analysis_setting.initialize_mode == "AllZeros") {
    // auto emplaced = stored_result_set.emplace(GetNumInterestedPoints(), 0);
    // result_pointers.emplace_back(GetNumProgramPoints(), &(*emplaced.first));
    stored_result_set.emplace_back(GetNumInterestedPoints(), 0);
    result_pointers.emplace_back(GetNumProgramPoints(), &stored_result_set.back());
  } else if (analysis_setting.initialize_mode == "AllOnes") {
    // auto emplaced = stored_result_set.emplace(GetNumInterestedPoints(), 1);
    // result_pointers.emplace_back(GetNumInterestedPoints(), &(*emplaced.first));
    stored_result_set.emplace_back(GetNumInterestedPoints(), 1);
    result_pointers.emplace_back(GetNumProgramPoints(), &stored_result_set.back());
  }
}

BitVector AnalysisBase::MeetBitVectors(const int iterIdx, const int sourceNodeIdx,
                           const std::vector<int>& targetNodeList) {
    if (targetNodeList.empty()) {
      // TODO exception handling
    }
    if (iterIdx > (result_pointers.size() - 1)) {
      // TODO exception handling
    }

    BitVector meet_result = analysis_setting.may_or_must == "may"
                                ? BitVector(GetNumInterestedPoints(), 0)
                                : BitVector(GetNumInterestedPoints(), 1);

    for (auto t : targetNodeList) {
      if (analysis_setting.may_or_must == "may") {
        meet_result |= *(result_pointers[iterIdx][t]);
      } else {
        meet_result &= *(result_pointers[iterIdx][t]);
      }
    }

    return meet_result;
  }

void AnalysisBase::IterativeAlgorithm() {
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

      BitVector meeted_bitvector;
      if (analysis_setting.forward_or_backward == "forward") {  // meet all predecessors
        if (adjacencies.control_reverse_adj_list[cur_node_idx].empty()) {
          continue;
        }
        meeted_bitvector = MeetBitVectors(cur_iter_idx - 1, cur_node_idx,
                                          adjacencies.control_reverse_adj_list[cur_node_idx]);

      } else {  // meet all successors
        if (adjacencies.control_adj_list[cur_node_idx].empty()) {
          continue;
        }
        meeted_bitvector = MeetBitVectors(cur_iter_idx - 1, cur_node_idx,
                                          adjacencies.control_adj_list[cur_node_idx]);
      }

      BitVector updated_bitvector = gens[cur_node_idx] | (meeted_bitvector - kills[cur_node_idx]);
      if (updated_bitvector == *(result_pointers[cur_iter_idx][cur_node_idx])) {
        continue;
      }
      // if it changes, update the result
      // auto inserted = stored_result_set.insert(updated_bitvector);
      // result_pointers[cur_iter_idx][cur_node_idx] = &(*inserted.first);
      stored_result_set.push_back(updated_bitvector);
      result_pointers[cur_iter_idx][cur_node_idx] = &(stored_result_set.back());

      // add predecessors/successors to worklist
      std::vector<int>& affected_list = analysis_setting.forward_or_backward == "forward"
                                            ? adjacencies.control_adj_list[cur_node_idx]
                                            : adjacencies.control_reverse_adj_list[cur_node_idx];

      for (auto affted_node : affected_list) {
        work_list.emplace(cur_iter_idx + 1, affted_node);
      }
    }
  }

}  // namespace programl
