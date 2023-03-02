#include "labm8/cpp/app.h"
#include "programl/graph/format/YZDGraphBuilder.h"
#include "programl/proto/program_graph.pb.h"
#include "programl/util/stdin_fmt.h"
#include "programl/util/stdout_fmt.h"
#include "programl/version.h"

const char* usage =
    R"(Convert a ProgramGraph message to a YZDGraph (graph that contains only control and data flow).

Usage:

    graph2yzdgraph [--stdin_fmt={pb,pbtxt}]

The YZDGraph format is a subset of a ProgramGraph which contains only control and data flow)";

int main(int argc, char** argv) {
  gflags::SetVersionString(PROGRAML_VERSION);
  labm8::InitApp(&argc, &argv, usage);
  if (argc != 1) {
    std::cerr << usage;
    return 4;
  }

  programl::ProgramGraph graph;
  programl::util::ParseStdinOrDie(&graph);

  programl::graph::format::YZDGraphBuilder builder;
  programl::ProgramGraph yzdgraph = builder.Build(graph);
  programl::util::WriteStdout(yzdgraph);

  return 0;
}
