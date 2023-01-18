#include <cassert>

#include "yzd_analysis.h"

namespace yzd

{
class YZDLiveness : public yzd::AnalysisBase {
 protected:
  virtual void ParseProgramGraph()
      override {  // 需要把program_points 和 interested_points 给算好; adjacencies也算好
    for (const auto& edge : program_graph.edge()) {
      if (edge.flow() == programl::Edge::CONTROL) {
        adjacencies.control_adj_list[edge.source()].insert(edge.target());
        adjacencies.control_reverse_adj_list[edge.target()].insert(edge.source());
        program_points.insert(edge.source());
        program_points.insert(edge.target());
      } else if (edge.flow() == programl::Edge::DATA) {
        if (program_graph.node(edge.source()).type() == programl::Node::INSTRUCTION) {  // def edge
          assert((program_graph.node(edge.target()).type() == programl::Node::VARIABLE) &&
                 "The target of this DataEdge should be Variable node!");
          program_points.insert(edge.source());
          interested_points.insert(edge.target());
          kills[edge.source()].insert(edge.target());
        } else if (program_graph.node(edge.source()).type() ==
                   programl::Node::VARIABLE) {  // use edge
          assert((program_graph.node(edge.target()).type() == programl::Node::INSTRUCTION) &&
                 "The target of this DataEdge should be Instruction node!");
          interested_points.insert(edge.source());
          program_points.insert(edge.target());
          gens[edge.target()].insert(edge.source());
        } else {
          assert((program_graph.node(edge.source()).type() == programl::Node::CONSTANT) &&
                 (program_graph.node(edge.source()).type() == programl::Node::INSTRUCTION) &&
                 "I suppose there should not be the fourth way of generating DataEdge...");
        }
      }
    }
  }
};
}  // namespace yzd
