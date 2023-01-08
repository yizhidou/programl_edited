#pragma once
#include "programl/proto/program_graph.pb.h"
#include "yzd_analysis.h"

namespace programl {
class Liveness : public AnalysisBase {
 protected:
  virtual void ParseProgramGraph() override {
    // 其实主要就是parse prgramGraph啦。需要把program_points 和 interested_points; adjacencies也算好
    int interested_point_idx = 0;
    for (int pg_node_idx = 0; pg_node_idx < program_graph.node_size(); pg_node_idx++){
        if (program_graph.node()[pg_node_idx].type() == Node::INSTRUCTION){
            program_points.push_back(pg_node_idx);
        }else if (program_graph.node()[pg_node_idx].type() == Node::VARIABLE)   
        {
            interested_points.push_back(pg_node_idx);
            from_interested_points_to_bit_idx[pg_node_idx] = interested_point_idx;
            interested_point_idx += 1;
        }
    }

    for (const auto& pp: program_points){
        gens[pp] = BitVector(GetNumInterestedPoints(), 0);
        kills[pp] = BitVector(GetNumInterestedPoints(), 0);
    }

    for (int i = 0; i < program_graph.edge_size(); ++i) {
      const Edge& edge = program_graph.edge(i);
      if (edge.flow() == Edge::CONTROL) {
        adjacencies.control_adj_list[edge.source()].push_back(edge.target());
        adjacencies.control_reverse_adj_list[edge.target()].push_back(edge.source());
      } else if (edge.flow() == Edge::DATA) {
        // adjacencies.defs[edge.source()].push_back(edge.target());
        int def_idx = from_interested_points_to_bit_idx[edge.target()];
        kills[edge.source()][def_idx] = 1;
        if (program_graph.node()[edge.target()].type() ==
            Node::VARIABLE) {  // for getting rid of Node::CONSTANT
        //   adjacencies.uses[edge.target()].push_back(edge.source());
        int use_idx = from_interested_points_to_bit_idx[edge.source()];
        gens[edge.target()][use_idx] = 1;
        }
      }
    }
  }
};
}  // namespace programl