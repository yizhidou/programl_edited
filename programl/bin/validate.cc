#include <iomanip>
#include <iostream>
#include <unordered_map>

#include "labm8/cpp/app.h"
#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
// #include "programl/graph/analysis/analysis.h"
#include "programl/graph/analysis/yzd_liveness.h"
#include "programl/graph/analysis/yzd_dominance.h"
#include "programl/graph/analysis/yzd_reachability.h"
#include "programl/graph/analysis/yzd_utils.h"
#include "programl/proto/program_graph.pb.h"
#include "programl/util/stdin_fmt.h"
#include "programl/util/stdout_fmt.h"
#include "programl/version.h"

using labm8::Status;
namespace error = labm8::error;

const char* usage = R"(Run a data-flow validate through programl on a program graph.
Usage:
    validate <task_name[yzd_liveness, yzd_dominance]> <max_iteration> < /path/to/program_graph.pb)";

static std::unordered_map<std::string, yzd::TaskName> const taskname_table = {
    {"yzd_liveness", yzd::yzd_liveness}, 
    {"yzd_dominance", yzd::yzd_dominance},
    {"yzd_reachability", yzd::yzd_reachability}};

int main(int argc, char** argv) {
  gflags::SetVersionString(PROGRAML_VERSION);
  labm8::InitApp(&argc, &argv, usage);

  if (argc != 3) {
    std::cerr << usage;
    return 4;
  }

  programl::ProgramGraph graph;
  programl::util::ParseStdinOrDie(&graph);
  std::string task_name(argv[1]);

  int maxIteration = std::atoi(argv[2]);
  auto it = taskname_table.find(task_name);
  if (it == taskname_table.end()) {
    std::cerr << "unrecognized taskname! Currently available: yzd_liveness, yzd_dominance";
    return 4;
  }
  yzd::AnalysisSetting yzd_setting(it->second, maxIteration);
  Status status;
  if (task_name=="yzd_liveness"){status = yzd::YZDLiveness(graph, yzd_setting).ValidateWithPrograml();}
  else if (task_name=="yzd_dominance")
  {
    std::cout << "We are validating yzd_dominance~" << std::endl;
    status = yzd::YZDDominance(graph, yzd_setting).ValidateWithPrograml();
  }
  else if (task_name=="yzd_reachability")
  {
    std::cout << "We are validating yzd_reachability~" << std::endl;
    status = yzd::YZDReachability(graph, yzd_setting).ValidateWithPrograml();
  }
  
  
  
  if (!status.ok()) {
    LOG(ERROR) << status.error_message();
    return 4;
  }

  return 0;
}