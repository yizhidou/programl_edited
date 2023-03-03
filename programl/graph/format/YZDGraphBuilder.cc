#pragma once
#include "programl/graph/format/YZDGraphBuilder.h"

#include <cassert>

#include "absl/container/flat_hash_set.h"
#include "labm8/cpp/logging.h"
#include "programl/graph/features.h"

namespace programl {
namespace graph {
namespace format {
ProgramGraph YZDGraphBuilder::Build(const ProgramGraph& total_program_graph) {
  ProgramGraph result_YZDGraph;
  absl::flat_hash_set<int> recorded_node_idx;
  // for (int node_idx = 0; node_idx < total_program_graph.node_size(); node_idx++) {
  //   if (recorded_node_idx.contains(node_idx)) {
  //     continue;
  //   }
  //   const Node& cur_node = total_program_graph.node(node_idx);
  //   if (cur_node.type() != Node::TYPE)  // INSTRUCTION or VARIABLE or TYPE
  //   {
  //     Node* newNode = result_YZDGraph.add_node();
  //     newNode->set_text(cur_node.text());
  //     newNode->set_function(cur_node.function());
  //     *newNode->mutable_features() = cur_node.features();
  //   }
  // }
  int num_pp = 0, num_control_edge = 0;
  for (int edge_idx = 0; edge_idx < total_program_graph.edge_size(); edge_idx++) {
    const Edge& cur_edge = total_program_graph.edge(edge_idx);
    if (cur_edge.flow() == programl::Edge::CONTROL) {
      // if (cur_edge.flow() == programl::Edge::CONTROL || cur_edge.flow() == programl::Edge::DATA)
      // {

      // if (!recorded_node_idx.contains(cur_edge.source())) {
      //   const Node& source_node = total_program_graph.node(cur_edge.source());
      //   Node* newNode = result_YZDGraph.add_node();
      //   newNode->set_text(source_node.text());
      //   newNode->set_function(source_node.function());
      //   *newNode->mutable_features() = source_node.features();
      //   num_pp++;
      // }
      // if (!recorded_node_idx.contains(cur_edge.target())) {
      //   const Node& target_node = total_program_graph.node(cur_edge.target());
      //   Node* newNode = result_YZDGraph.add_node();
      //   newNode->set_text(target_node.text());
      //   newNode->set_function(target_node.function());
      //   *newNode->mutable_features() = target_node.features();
      //   num_pp++;
      // }

      Edge* newEdge = result_YZDGraph.add_edge();
      newEdge->set_flow(cur_edge.flow());
      newEdge->set_source(cur_edge.source());
      newEdge->set_target(cur_edge.target());
      *newEdge->mutable_features() = cur_edge.features();
      num_control_edge++;
    }
  }
  *result_YZDGraph.mutable_function() = total_program_graph.function();
  // std::cout << "num_program_point: " << num_pp << std::endl;
  // std::cout << "num_control_edge: " << num_control_edge << std::endl;
  return result_YZDGraph;
}

}  // namespace format
}  // namespace graph
}  // namespace programl
