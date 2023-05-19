#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include "labm8/cpp/app.h"
#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
// #include "programl/graph/analysis/analysis.h"
#include "programl/graph/analysis/yzd_dominance.h"
#include "programl/graph/analysis/yzd_liveness.h"
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
    validate <task_name[yzd_liveness, yzd_dominance]> <max_iteration> <sync_or_async> < /path/to/program_graph.pb)";

DEFINE_int32(max_iteration, 500, "max iteration that allowed");
DEFINE_string(program_graph_sourcepath, "unset", "program graph source path");
DEFINE_bool(sync, false,
            "if this --sync flag is set to be true, then the message update would be synchronous. "
            "(default: false)");
DEFINE_bool(idx_reorganized, true,
            "if this --idx_reorganized flag is set to be true, then the node idex would be "
            "reorganized, (default: true)");

static std::unordered_map<std::string, yzd::TaskName> const taskname_table = {
    {"yzd_liveness", yzd::yzd_liveness},
    {"yzd_dominance", yzd::yzd_dominance},
    {"yzd_reachability", yzd::yzd_reachability}};

static std::unordered_map<std::string, yzd::SyncOrAsync> const sync_or_async_table = {
    {"sync", yzd::sync}, {"async", yzd::async}};

int main(int argc, char** argv) {
  gflags::SetVersionString(PROGRAML_VERSION);
  labm8::InitApp(&argc, &argv, usage);

  if (argc != 2) {
    std::cerr << usage;
    return 4;
  }

  programl::ProgramGraph graph;
  // programl::util::ParseStdinOrDie(&graph);
  std::string task_name_str(argv[1]);
  // std::string sync_or_async_str(argv[3]);
  std::ifstream pg_output_stream(FLAGS_program_graph_sourcepath);
  if (pg_output_stream.is_open()) {
    if (!graph.ParseFromIstream(&pg_output_stream)) {
      LOG(ERROR) << "Failed to parse binary protocol buffer from "
                 << FLAGS_program_graph_sourcepath;
      return 4;
    }
    pg_output_stream.close();
  } else {
    LOG(ERROR) << "Failed to open program graph source path!";
    return 4;
  }

  // int maxIteration = std::atoi(argv[2]);
  auto it_t = taskname_table.find(task_name_str);
  if (it_t == taskname_table.end()) {
    std::cerr << "unrecognized taskname! Currently available: yzd_liveness, yzd_dominance";
    return 4;
  }
  // auto it_s = sync_or_async_table.find(sync_or_async_str);
  // if (it_s == sync_or_async_table.end()) {
  //   std::cerr << "unrecognized sync_or_async indicator! Currently available: sync (worklist), "
  //                "async (not worklist)";
  //   return 4;
  // }
  // bool if_idx_reorganized = false;
  yzd::SyncOrAsync sync_or_async = FLAGS_sync ? yzd::sync : yzd::async;
  yzd::AnalysisSetting yzd_setting(it_t->second, FLAGS_max_iteration, sync_or_async,
                                   FLAGS_idx_reorganized); 
  Status status;
  if (task_name_str == "yzd_liveness") {
    status = yzd::YZDLiveness(graph, yzd_setting).ValidateWithPrograml();
  } else if (task_name_str == "yzd_dominance") {
    std::cout << "We are validating yzd_dominance~" << std::endl;
    status = yzd::YZDDominance(graph, yzd_setting).ValidateWithPrograml();
  } else if (task_name_str == "yzd_reachability") {
    std::cout << "We are validating yzd_reachability~" << std::endl;
    status = yzd::YZDReachability(graph, yzd_setting).ValidateWithPrograml();
  }

  if (!status.ok()) {
    LOG(ERROR) << status.error_message();
    return 4;
  }

  return 0;
}