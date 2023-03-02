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
  //   absl::flat_hash_set<int> recorded_node_idx;
  for (int node_idx = 0; node_idx < total_program_graph.node_size(); node_idx++) {
    // if (recorded_node_idx.contains(node_idx)) {
    //   continue;
    // }
    const Node& cur_node = total_program_graph.node(node_idx);
    if (cur_node.type() != Node::TYPE)  // INSTRUCTION or VARIABLE or TYPE
    {
      Node* newNode = result_YZDGraph.add_node();
      newNode->set_text(cur_node.text());
      newNode->set_function(cur_node.function());
      *newNode->mutable_features() = cur_node.features();
    }
  }
  for (int edge_idx = 0; edge_idx < total_program_graph.edge_size(); edge_idx++) {
    const Edge& cur_edge = total_program_graph.edge(edge_idx);
    if (cur_edge.flow() == programl::Edge::CONTROL || cur_edge.flow() == programl::Edge::DATA) {
      Edge* newEdge = result_YZDGraph.add_edge();
      newEdge->set_flow(cur_edge.flow());
      newEdge->set_source(cur_edge.source());
      newEdge->set_target(cur_edge.target());
      *newEdge->mutable_features() = cur_edge.features();
    }
  }
  *result_YZDGraph.mutable_function() = total_program_graph.function();
  return result_YZDGraph;
}

// void _addNode2Graph(absl::flat_hash_set<int>& recorded_nodes, int candidate_node_idx,
//                     const Node& candidate_node, ProgramGraph& target_pg) {
//   if (recorded_nodes.contains(candidate_node_idx)) {
//     Node* newNode = target_pg.add_node();
//     newNode->set_text(candidate_node.text());
//     newNode->set_function(candidate_node.function());
//     *newNode->mutable_features() = candidate_node.features();
//   } else {
//     recorded_nodes.insert(candidate_node_idx);
//   }
// }

// void _addEdge2Graph(absl::flat_hash_set<int> recorded_nodes, const Edge& candidate_edge,
//                     ProgramGraph& target_pg) {
//   assert(recorded_nodes.contains(candidate_edge.source()) &&
//          recorded_nodes.contains(candidate_edge.target()));
//   Edge* newEdge;

//   switch (candidate_edge.flow()) {
//     case Edge::CONTROL:
//       newEdge->set_source(candidate_edge.source());
//       newEdge->set_target(candidate_edge.target());
//       newEdge->set_flow(Edge::CONTROL);
//       break;
//     case Edge::DATA:
//       newEdge->set_source(candidate_edge.source());
//       newEdge->set_target(candidate_edge.target());
//       newEdge->set_flow(Edge::DATA);
//       break;
//     case Edge::CALL:
//       break;
//     default:
//       LOG(FATAL) << "unreachable";
//   }
// }

}  // namespace format
}  // namespace graph
}  // namespace programl
