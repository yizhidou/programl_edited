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
  // 1. 获得图中root的个数/每个root对应的子图节点集/backedge的数量； 2. 获得第0个iteration的结果；

  // 检查num_point_interested和num_point_interested这两个数值得是大于0的
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
  // _top_order_map.reserve(GetNumProgramPoints());
  _top_order_list.reserve(GetNumProgramPoints());
  // std::cout << "The top order: " << std::endl;
  for (const int root_node : root_list) {
    std::pair<std::vector<int>, int> po_and_num_be =
        PostOrderAndNumBackEdgeFromOneRoot(adj, root_node);
    _num_be += po_and_num_be.second;
    const auto& po_list = po_and_num_be.first;
    for (int i = po_list.size() - 1; i > -1; i--) {
      // _top_order_map[po_list[i]] = _top_order_map.size();
      _top_order_list.push_back(po_list[i]);
      // std::cout << "node: " << po_list[i] << "; top order: " << _top_order_list.size() - 1
      //           << std::endl;
    }
    _root_subgraph[root_node] = NodeSet(po_list.begin(), po_list.end());
  }
  // NodeSet _top_order_set(_top_order_list.begin(), _top_order_list.end());
  // if (_top_order_set.size() !=
  //     GetNumProgramPoints()) {  // 就是一个图有多个root的情况，目前还没办法处理
  //   std::cout << "_top_order_map.size = " << _top_order_set.size()
  //             << "; num_pp = " << GetNumProgramPoints() << std::endl;
  //   return labm8::Status(labm8::error::FAILED_PRECONDITION,
  //                        "The graph has multiple root! We currently cannot handle this...");
  // }
  std::cout << TaskNameToStrTable[analysis_setting.task_name] << " num_be " << _num_be
            << std::endl;

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

NodeSet AnalysisBase::MeetOperation(const int iterIdx, const NodeSet& targetNodeList) {
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

labm8::Status AnalysisBase::RearangeNodeIdx(const NodeSet& node_set) {
  int start_idx = node_idx_map.size();
  int num_node = node_set.size();
  std::vector<int> tmp_old_node_list(node_set.begin(), node_set.end());
  std::sort(tmp_old_node_list.begin(), tmp_old_node_list.end());
  for (int old_node_idx = 0; old_node_idx < num_node; old_node_idx++) {
    int old_node = tmp_old_node_list[old_node_idx];
    if (node_idx_map.contains(old_node)) {
      std::cout << "The node should not be re-indexed more than once!" << std::endl
                << "the node is: " << old_node
                << ", and its type is: " << program_graph.node(old_node).type() << std::endl;
      return labm8::Status(
          labm8::error::ABORTED,
          "The node should not be re-indexed more than once! there must be sth wrong!");
    }
    int new_node = start_idx + old_node_idx;
    node_idx_map[old_node] = new_node;
  }
  return labm8::Status::OK;
}

labm8::Status AnalysisBase::Init_sync() {
  // 同步更新的版本 (worklist的版本)
  auto time_start = std::chrono::high_resolution_clock::now();
  // ParseProgramGraph();              // 需要把program_points 和 interested_points 给算好;
  // adjacencies也算好。这是一个纯虚函数。
  if (analysis_setting.index_reorganized) {
    RETURN_IF_ERROR(ParseProgramGraph_idx_reorganized());
  } else {
    ParseProgramGraph();
  }
  RETURN_IF_ERROR(InitSettings());  // 这个主要作用往stored_result_set里加初始的结果

  std::queue<WorklistItem> work_list;
  for (const int pp : _top_order_list) {
    // std::cout << "in the initialization of work_list, " << pp << " is going to be emplaced"
    //           << std::endl;
    // std::cout << "its order in topology is: " << _top_order_map[pp] << std::endl;
    work_list.emplace(1, pp);
  }
  // int continued_num = 0;
  int total_iter_num = 0;
  while (!work_list.empty()) {
    WorklistItem cur_item = work_list.front();
    work_list.pop();
    int cur_iter_idx = cur_item.iter_idx, cur_node_idx = cur_item.node_idx;
    total_iter_num = cur_iter_idx;
    // std::cout << "=======poped cur_node_idx: " << cur_node_idx
    //           << "; top order: " << _top_order_map[cur_node_idx] << std::endl;

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
      // meeted_nodeset = MeetOperation(cur_iter_idx, neighbor_nodes);  //
      // 这地方-1不-1结果都一样的吗？
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
    
    // if (analysis_setting.may_or_must == may) {
    //   if ((updated_bitvector.size() == 0) |
    //       !(updated_bitvector > stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]])) {
    //     std::cout << "may: should always extend!!!" << std::endl;
    //     abort();
    //   }
    //   std::cout << "cur_iter_idx: " << cur_iter_idx << ", cur_node_idx: " << cur_node_idx << ". diff = " << updated_bitvector - stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]] << std::endl;
    // } else {
    //   // must
    //   assert(
    //       (updated_bitvector.size() == 0) |
    //           !(updated_bitvector < stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]) &&
    //       "should always shrink!!!");
    //   if (!((cur_node_idx == 4) | (cur_node_idx == 149)) &&
    //       ((updated_bitvector.size() == 0) |
    //        !(updated_bitvector < stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]))) {
    //     std::cout << "must: should always shrink!!!" << std::endl;
    //     std::cout << "before update: "
    //               << stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]] << std::endl;
    //     std::cout << "after  update: " << updated_bitvector << std::endl;

    //     abort();
    //   }
    //   std::cout << "cur_iter_idx: " << cur_iter_idx << ", cur_node_idx: " << cur_node_idx << ". diff = " <<  stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]] - updated_bitvector << std::endl;
    // }

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
    // std::cout << "affected nodes are added to worklist: " << affected_list
    //           << ", and their iteration number would be " << cur_iter_idx + 1 << std::endl;
  }
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " num_pp " << GetNumProgramPoints() << std::endl;
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " num_ip " << GetNumInterestedPoints() << std::endl;
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " num_iteration(sync) " << total_iter_num << std::endl;
  auto time_end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start);
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " time_duration(sync) " << duration.count() << std::endl;
  return labm8::Status::OK;
}

labm8::Status AnalysisBase::Init_async() {
  // 异步更新的版本 (非worklist)
  auto time_start = std::chrono::high_resolution_clock::now();
  // ParseProgramGraph();              // 需要把program_points 和 interested_points 给算好;
  // adjacencies也算好。这是一个纯虚函数。
  if (analysis_setting.index_reorganized) {
    RETURN_IF_ERROR(ParseProgramGraph_idx_reorganized());
  } else {
    ParseProgramGraph();
  }
  RETURN_IF_ERROR(InitSettings());  // 这个主要作用往stored_result_set里加初始的结果
  bool flag = true;
  int cur_iter_idx = 0;
  while (flag) {
    cur_iter_idx++;
    flag = false;
    if (cur_iter_idx > analysis_setting.max_iteration) {
      return labm8::Status(labm8::error::FAILED_PRECONDITION,
                           "Failed to terminate in certain iterations!");
    }

    if (cur_iter_idx > result_pointers.size() - 1) {
      // std::cout << "~~~~~~~~~~~~~~~~~~~~iteration " << cur_iter_idx << "~~~~~~~~~~~~~~~~~"
      //           << std::endl;
      result_pointers.push_back(
          result_pointers.back());  // copy the result of the last iteration and push into result.
    }
    for (const int cur_node_idx : _top_order_list) {
      const auto& neighbor_nodes =
          (analysis_setting.direction == forward)
              ? adjacencies.control_reverse_adj_list[cur_node_idx]  // meet all predecessors
              : adjacencies.control_adj_list[cur_node_idx];         // meet all successors
      NodeSet meeted_nodeset;
      if (!neighbor_nodes.empty()) {
        meeted_nodeset = MeetOperation(cur_iter_idx, neighbor_nodes);
      } else {
        meeted_nodeset = stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]];
      }
      NodeSet temp = meeted_nodeset - kills[cur_node_idx];
      NodeSet updated_bitvector =
          gens[cur_node_idx] | temp;  // 这两句是真的有点奇怪，合并到一起就会提示有错
      if (updated_bitvector == stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]]) {
        continue;
      }
      // 就是结果有改变
      flag = true;
      if (analysis_setting.may_or_must == may) {
        if ((updated_bitvector.size() == 0) |
            !(updated_bitvector > stored_nodesets[result_pointers[cur_iter_idx][cur_node_idx]])) {
          std::cout << "may: should always extend!!!" << std::endl;
          abort();
        }
      } else {
        // must
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
      result_pointers[cur_iter_idx][cur_node_idx] = stored_nodesets.size() - 1;
    }
  }
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " num_pp " << GetNumProgramPoints() << std::endl;
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " num_ip " << GetNumInterestedPoints() << std::endl;
  std::cout << TaskNameToStrTable[analysis_setting.task_name] << " num_iteration(async) " << cur_iter_idx
            << std::endl;
  auto time_end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start);
  std::cout << TaskNameToStrTable[analysis_setting.task_name]
            << " time_duration(async) " << duration.count() << std::endl;
  return labm8::Status::OK;
}

labm8::Status AnalysisBase::Run(programl::ResultsEveryIteration* resultsOfAllIterations,
                                EdgeList* edge_list_to_save) {
  if (analysis_setting.sync_or_async == async) {
    RETURN_IF_ERROR(Init_async());
  } else {
    RETURN_IF_ERROR(Init_sync());
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
  if (edge_list_to_save == nullptr) {
    return labm8::Status::OK;
  }
  // 接下来就是把相应的邻接矩阵信息存下来
  int num_node_of_this_graph = 0;
  if (analysis_setting.task_name == yzd_liveness) {
    // 第一行存的是 (num_pp, num_ip, unset_edge)
    edge_list_to_save->emplace_back(GetNumProgramPoints(), GetNumInterestedPoints());
    // 然后开始存control edge, gen edge, kill edge
    for (int pp = 0; pp < GetNumProgramPoints(); pp++) {
      for (const int& target_pp : adjacencies.control_reverse_adj_list[pp]) {
        edge_list_to_save->emplace_back(pp, target_pp, control_edge);
      }
      for (const int& gen_ip : gens[pp]) {
        edge_list_to_save->emplace_back(pp, gen_ip, gen_edge);
      }
      for (const int& kill_ip : kills[pp]) {
        edge_list_to_save->emplace_back(pp, kill_ip, kill_edge);
      }
    }
  } else if (analysis_setting.task_name == yzd_dominance) {
    // 第一行存的是 (num_pp, -1, unset)
    edge_list_to_save->emplace_back(GetNumProgramPoints(), -1);
    // 然后开始存control edge
    for (int pp = 0; pp < GetNumProgramPoints(); pp++) {
      for (const int& target_pp : adjacencies.control_adj_list[pp]) {
        edge_list_to_save->emplace_back(pp, target_pp);
      }
    }
  } else if (analysis_setting.task_name == yzd_reachability) {
    // 第一行存的是 (num_pp, -1, unset)
    edge_list_to_save->emplace_back(GetNumProgramPoints(), -1);
    // 然后开始存control edge
    for (int pp = 0; pp < GetNumProgramPoints(); pp++) {
      for (const int& target_pp : adjacencies.control_reverse_adj_list[pp]) {
        edge_list_to_save->emplace_back(pp, target_pp);
      }
    }
  } else {
    return labm8::Status(labm8::error::FAILED_PRECONDITION,
                         "Failed to recognize task_name! currently available: yzd_dominance");
  }

  return labm8::Status::OK;
}

}  // namespace yzd
