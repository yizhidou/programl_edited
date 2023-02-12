#include "yzd_liveness.h"

#include <cassert>
#include <iostream>

namespace yzd {
void YZDLiveness::ParseProgramGraph() {  // 需要把program_points 和 interested_points 给算好;
                                         // gens/kills 算好； adjacencies也算好
  int edge_count = 0, control_edge_count = 0, data_edge_count = 0, non_empty_gens = 0,
      non_empty_kills = 0;
  for (const auto& edge : program_graph.edge()) {
    edge_count++;
    if (edge.flow() == programl::Edge::CONTROL) {
      control_edge_count++;

      adjacencies.control_adj_list[edge.source()].insert(edge.target());
      adjacencies.control_reverse_adj_list[edge.target()].insert(edge.source());
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
        assert((program_graph.node(edge.target()).type() == programl::Node::INSTRUCTION) && "the target of this edge should be an instruction");
        interested_points.insert(edge.source());
        program_points.insert(edge.target());
        gens[edge.target()].insert(edge.source());
      }
    }
  }
  std::cout << "total edge_count is: " << edge_count << std::endl;
  std::cout << "total node_count is: " << program_graph.node_size() << std::endl;
  std::cout << "control edge_count is: " << control_edge_count << std::endl;
  std::cout << "(yzd) data edge_count is: " << data_edge_count << std::endl;
  std::cout << "num of program points is: " << program_points.size() << std::endl;
  std::cout << "num of interested points is: " << interested_points.size() << std::endl;
  for (const auto pp : program_points) {
    if (!(gens[pp].size() == 0)) {
      non_empty_gens++;
    }
    if (!(kills[pp].size() == 0)) {
      non_empty_kills++;
    }
  }
  std::cout << "num of non_empty gens and kills are: " << non_empty_gens << " ; " << non_empty_kills
            << std::endl;
}
}  // namespace yzd
