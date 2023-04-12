// #pragma once
#include "yzd_analysis_base.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "labm8/cpp/logging.h"
#include "labm8/cpp/status_macros.h"
// #include "yzd_utils.h"
// #include "labm8/cpp/logging.h"

namespace yzd {

labm8::Status AnalysisBase::InitSettings() {
  // 这里应该check一下num_point_interested和num_point_interested这两个数值得是大于0的
  if ((interested_points.size() == 0) || (program_points.size() == 0)) {
    return labm8::Status(labm8::error::FAILED_PRECONDITION,
                         "The graph has either none program points or none interested points");
  }

  const auto& adj = analysis_setting.direction == forward ? adjacencies.control_adj_list
                                                          : adjacencies.control_reverse_adj_list;
  const auto& reverse_adj = analysis_setting.direction == forward
                                ? adjacencies.control_reverse_adj_list
                                : adjacencies.control_adj_list;
  std::vector<int> root_list = GetRootList(reverse_adj);
  // std::cout << "the number of pp is: " << GetNumProgramPoints() << std::endl;
  // std::cout << "the number of roots is: " << root_list.size() << std::endl;
  int num_nodes_in_po_list = 0;
  _top_order.reserve(GetNumProgramPoints());
  std::cout << "The top order: " << std::endl;
  for (const int root_node : root_list) {
    std::pair<std::vector<int>, int> po_and_num_be =
        PostOrderAndNumBackEdgeFromOneRoot(adj, root_node);
    _num_be += po_and_num_be.second;
    const auto& po_list = po_and_num_be.first;
    num_nodes_in_po_list += po_list.size();
    for (int i = po_list.size() - 1; i > -1; i--) {
      _top_order[po_list[i]] = _top_order.size();
      std::cout << "node: " << po_list[i] << "; top order: " << _top_order.size() - 1 << std::endl;
    }
    _root_subgraph[root_node] = NodeSet(po_list.begin(), po_list.end());
  }
  if (num_nodes_in_po_list !=
      GetNumProgramPoints()) {  // 就是一个图有多个root的情况，目前还没办法处理
    std::cout << "num_nodes_in_po_list = " << num_nodes_in_po_list
              << "; num_pp = " << GetNumProgramPoints() << std::endl;
    return labm8::Status(labm8::error::FAILED_PRECONDITION,
                         "The graph has multiple root! We currently cannot handle this...");
  }
  std::cout << "the number of back edges is: " << _num_be << std::endl;

  // add result of iteration 0 into result_pointers.
  absl::flat_hash_map<int, int> result_pointer_zero_iteration;
  result_pointer_zero_iteration.reserve(GetNumProgramPoints());
  if (analysis_setting.initialize_mode == allzeros) {
    stored_nodesets
        .emplace_back();  // 我不是很确定这个emplace_back()是不是就给vector加入了一个空的NodeSet?
    assert((stored_nodesets.size() == 1) &&
           "By now, there should be only one element in the stored_result set!");
    for (const int pp : program_points) {
      result_pointer_zero_iteration[pp] = 0;
    }

  } else if (analysis_setting.initialize_mode == allones) {
    for (auto iter = _root_subgraph.begin(); iter != _root_subgraph.end(); ++iter) {
      const auto& root_node = iter->first;
      const auto& subgraph_nodeset = iter->second;
      NodeSet initial_value_for_root = {root_node};
      stored_nodesets.push_back(initial_value_for_root);
      result_pointer_zero_iteration[root_node] = stored_nodesets.size() - 1;
      stored_nodesets.emplace_back(subgraph_nodeset);
      for (int node_in_subgraph : subgraph_nodeset) {
        if (node_in_subgraph == root_node) {
          continue;
        }
        result_pointer_zero_iteration[node_in_subgraph] = stored_nodesets.size() - 1;
      }
    }
    // the old version
    // for (const int& pp : program_points) {
    //   if (((adjacencies.control_reverse_adj_list[pp].size() == 0) &&
    //        analysis_setting.direction == forward) |
    //       ((adjacencies.control_adj_list[pp].size() == 0) &&
    //        analysis_setting.direction == backward)) {
    //     std::cout << "root node spotted! " << pp << std::endl;
    //     // root_nodes.emplace_back(pp);
    //     absl::flat_hash_set<int> initial_value_for_root = {pp};
    //     stored_nodesets.push_back(initial_value_for_root);
    //     result_pointer_zero_iteration[pp] = stored_nodesets.size() - 1;
    //     NodeSet subgraph_nodeset_from_root =
    //         SubgraphNodesFromRoot(pp, adjacencies, analysis_setting.direction);
    //     std::cout << "rootnode " << pp
    //               << "! corresponding subgraph nodes are: " << subgraph_nodeset_from_root
    //               << std::endl;
    //     stored_nodesets.emplace_back(subgraph_nodeset_from_root.begin(),
    //                                  subgraph_nodeset_from_root.end());
    //     for (int node_in_this_subgraph : subgraph_nodeset_from_root) {
    //       if (node_in_this_subgraph == pp) {
    //         continue;
    //       }
    //       result_pointer_zero_iteration[node_in_this_subgraph] = stored_nodesets.size() - 1;
    //     }
    //   }
    // }
    // for (const int& pp : program_points) {
    //   std::cout << "(yzd) the initial value of " << pp << " : "
    //             << stored_nodesets[result_pointer_zero_iteration[pp]] << std::endl;
    // }

  } else {
    return labm8::Status(labm8::error::UNIMPLEMENTED,
                         "Unrecognized initialization mode! Currently support: allzeros/ allones");
  }

  result_pointers.push_back(result_pointer_zero_iteration);
  return labm8::Status::OK;
}

NodeSet AnalysisBase::MeetOperation(const int iterIdx,
                                    const absl::flat_hash_set<int>& targetNodeList) {
  // assert((iterIdx == result_pointers.size()) && "Here!");
  // assert(true==false);
  // std::cout << "In meet, iterIdx = " << iterIdx
  //           << "; result_pointers.size() = " << result_pointers.size() << std::endl;
  if (targetNodeList.size() == 1) {
    const int t = *(targetNodeList.begin());
    return stored_nodesets[result_pointers[iterIdx][t]];
  }

  NodeSet meet_result = analysis_setting.may_or_must == may
                            ? NodeSet()
                            : NodeSet(interested_points.begin(), interested_points.end());
  for (const int t : targetNodeList) {
    // std::cout << "before meet: " << stored_nodesets[result_pointers[iterIdx][t]] << std::endl;
    if (analysis_setting.may_or_must == may) {
      meet_result |= stored_nodesets[result_pointers[iterIdx][t]];
    } else {
      meet_result &= stored_nodesets[result_pointers[iterIdx][t]];
    }
  }
  // std::cout << "after meet: " << meet_result << std::endl;
  return meet_result;
}

labm8::Status AnalysisBase::Init() {
  auto time_start = std::chrono::high_resolution_clock::now();
  ParseProgramGraph();              // 需要把program_points 和 interested_points 给算好;
                                    // adjacencies也算好。这是一个纯虚函数。
  RETURN_IF_ERROR(InitSettings());  // 这个主要作用往stored_result_set里加初始的结果

  // add all nodes in worklist
  std::cout << "till yzd_analysis_base.cc line 154, everything is fine" << std::endl;
  for (const int pp : program_points) {
    std::cout << "in the initialization of work_list, " << pp << " is going to be emplaced"
              << std::endl;
    std::cout << "its order in topology is: " << _top_order[pp] << std::endl;
    work_list.emplace(1, pp);
  }
  // int continued_num = 0;
  int total_iter_num = 0;
  while (!work_list.empty()) {
    // WorklistItem cur_item = work_list.front(); // 非优先级队列的情况
    WorklistItem cur_item = work_list.top();
    work_list.pop();
    int cur_iter_idx = cur_item.iter_idx, cur_node_idx = cur_item.node_idx;
    total_iter_num = cur_iter_idx;
    std::cout << "=======cur_node_idx: " << cur_node_idx
              << "; top order: " << _top_order[cur_node_idx] << std::endl;

    if (cur_iter_idx > analysis_setting.max_iteration) {
      return labm8::Status(labm8::error::FAILED_PRECONDITION,
                           "Failed to terminate in certain iterations!");
    }

    if (cur_iter_idx > result_pointers.size() - 1) {
      // std::cout << "~~~~~~~~~~~~~~~~~~~~iteration " << cur_iter_idx << "~~~~~~~~~~~~~~~~~"
      //           << std::endl;
      // PrintWorkList(work_list);
      result_pointers.push_back(
          result_pointers.back());  // copy the result of the last iteration and push into result.
    }
    const auto& neighbor_nodes =
        (analysis_setting.direction == forward)
            ? adjacencies.control_reverse_adj_list[cur_node_idx]  // meet all predecessors
            : adjacencies.control_adj_list[cur_node_idx];         // meet all successors
    // std::cout << " neighbors are: " << neighbor_nodes << std::endl;
    // for (const auto n : neighbor_nodes) {
    //   std::cout << n << ": " << stored_nodesets[result_pointers[cur_iter_idx - 1][n]] <<
    //   std::endl;
    // }

    NodeSet meeted_nodeset;
    if (!neighbor_nodes.empty()) {
      // std::cout << "cur_iter_idx - 1 = " << cur_iter_idx - 1
      //           << "; size of result_pointers is: " << result_pointers.size() << std::endl;
      meeted_nodeset =
          MeetOperation(cur_iter_idx - 1, neighbor_nodes);  // 这地方-1不-1结果都一样的吗？
    } else {
      meeted_nodeset = stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]];
    }

    NodeSet temp = meeted_nodeset - kills[cur_node_idx];
    // std::cout << "tmp: " << temp << std::endl;

    // if (analysis_setting.task_name == yzd_dominance) {
    // assert((temp == meeted_nodeset) && "Here, temp should be equal to meeted_nodeset");
    // if (temp != meeted_nodeset) {
    //   std::cout << "Here, temp should be equal to meeted_nodeset" << std::endl;
    //   std::abort();
    // } else {
    // std::cout << "substract an empty set does not affect the result!" << std::endl;
    // }
    // }

    NodeSet updated_bitvector =
        gens[cur_node_idx] | temp;  // 这两句是真的有点奇怪，合并到一起就会提示有错
    if (updated_bitvector == stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]) {
      // std::cout << "not update, so continue! cur_bv = " << updated_bitvector << std::endl;
      continue;
    }
    // assert((!temp.contains(cur_node_idx)) &&
    //        "This should not be true! or it is the mistake of operator|");
    // if (!temp.contains(cur_node_idx)){
    //   std::cout << "If updated_bv !=  or it is the mistake of operator|" << std::endl;
    //   std::cout << "temp = " << temp << std::endl;
    //   std::abort();
    // }
    if (analysis_setting.may_or_must == may) {
      assert((updated_bitvector.size() > 0) &&
             (updated_bitvector > stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]) &&
             "should always extend!!!");
      if ((updated_bitvector.size() == 0) |
          !(updated_bitvector > stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]])) {
        std::cout << "may: should always extend!!!" << std::endl;
        abort();
      }
    } else {
      // must
      assert(
          (updated_bitvector.size() == 0) |
              !(updated_bitvector < stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]) &&
          "should always shrink!!!");
      if (!((cur_node_idx == 4) | (cur_node_idx == 149)) &&
          ((updated_bitvector.size() == 0) |
           !(updated_bitvector < stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]))) {
        std::cout << "must: should always shrink!!!" << std::endl;
        std::cout << "before update: "
                  << stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]] << std::endl;
        std::cout << "after  update: " << updated_bitvector << std::endl;

        abort();
      }
    }

    stored_nodesets.push_back(updated_bitvector);
    // std::cout << "before update: " <<
    // stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]
    //           << std::endl;
    // const auto added =
    //     updated_bitvector - stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]];
    // std::cout << "the new added are: " << added << std::endl;
    // result_pointers[cur_iter_idx][cur_node_idx] = stored_nodesets.size() - 1;
    // std::cout << "after update: " << stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]
    //           << std::endl;

    // add predecessors/successors to worklist
    // std::cout << "update! before update: "
    //           << stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]] << std::endl;
    // std::cout << "update! after  update: " << updated_bitvector << std::endl;
    result_pointers[cur_iter_idx][cur_node_idx] = stored_nodesets.size() - 1;
    const auto& affected_list =
        analysis_setting.direction == forward
            ? adjacencies.control_adj_list[cur_node_idx]           // add all successors
            : adjacencies.control_reverse_adj_list[cur_node_idx];  // add all predecessors

    for (auto affted_node : affected_list) {
      work_list.emplace(cur_iter_idx + 1, affted_node);
    }
    // std::cout << "affected nodes are added to worklist: " << affected_list << std::endl;
  }
  if (analysis_setting.task_name == yzd::yzd_liveness) {
    std::cout << "num_iteration_liveness " << total_iter_num << std::endl;
  } else if (analysis_setting.task_name == yzd::yzd_dominance) {
    std::cout << "num_iteration_dominance " << total_iter_num << std::endl;
  }
  auto time_end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start);
  std::cout << "Time taken by function: " << duration.count() << " microseconds" << std::endl;
  return labm8::Status::OK;
}

labm8::Status AnalysisBase::Run(programl::ResultsEveryIteration* resultsOfAllIterations) {
  RETURN_IF_ERROR(Init());

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
      const NodeSet& tmp_nodeset = stored_nodesets[it.second];
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