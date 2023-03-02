#pragma once
#include <vector>

#include "programl/proto/program_graph.pb.h"

namespace programl {
namespace graph {
namespace format {

class YZDGraphBuilder {
 public:
  YZDGraphBuilder(){};

  // Construct a YZDGraph from the given ProGraML graph.
  [[nodiscard]] ProgramGraph Build(const ProgramGraph& total_program_graph);

};

}  // namespace format
}  // namespace graph
}  // namespace programl