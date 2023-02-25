#include <iomanip>
#include <iostream>

#include "labm8/cpp/app.h"
#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
// #include "programl/graph/analysis/analysis.h"
#include "programl/graph/analysis/yzd_liveness.h"
#include "programl/graph/analysis/yzd_utils.h"
#include "programl/proto/program_graph.pb.h"
#include "programl/util/stdin_fmt.h"
#include "programl/util/stdout_fmt.h"
#include "programl/version.h"

using labm8::Status;
namespace error = labm8::error;

const char* usage = R"(Run a data-flow validate through programl on a program graph.
Usage:
    validate <max_iteration> < /path/to/program_graph.pb)";

int main(int argc, char** argv) {
  gflags::SetVersionString(PROGRAML_VERSION);
  labm8::InitApp(&argc, &argv, usage);

  if (argc != 2) {
    std::cerr << usage;
    return 4;
  }

  programl::ProgramGraph graph;
  programl::util::ParseStdinOrDie(&graph);

  int maxIteration = std::atoi(argv[1]);
  yzd::AnalysisSetting yzd_setting(yzd::TaskName::yzd_liveness, maxIteration);
  Status status = yzd::YZDLiveness(graph, yzd_setting).ValidateWithPrograml();
  if (!status.ok()) {
    LOG(ERROR) << status.error_message();
    return 4;
  }

  return 0;
}