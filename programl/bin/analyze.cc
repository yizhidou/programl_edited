// Copyright 2019-2020 the ProGraML authors.
//
// Contact Chris Cummins <chrisc.101@gmail.com>.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <getopt.h>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "labm8/cpp/app.h"
#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
#include "programl/graph/analysis/analysis.h"
#include "programl/graph/analysis/yzd_liveness.h"
#include "programl/graph/analysis/yzd_utils.h"
#include "programl/proto/program_graph.pb.h"
#include "programl/util/stdin_fmt.h"
#include "programl/util/stdout_fmt.h"
#include "programl/version.h"

using labm8::Status;
namespace error = labm8::error;

const char* usage = R"(Run a data-flow analysis on a program graph.

Usage:

    analyze <analysis> < /path/to/program_graph.pb)";

const char* usage_yzd = R"(Run a yzd data-flow analysis on a program graph.

Usage:

    analyze <analysis> --stdin_fmt=pb <max_iteration> < /path/to/program_graph.pb)";

// DEFINE_string(task_name, "", "task name");
DEFINE_int32(max_iteration, 500, "max iteration that allowed");
DEFINE_string(program_graph_sourcepath, "unset", "program graph source path");
DEFINE_string(edge_list_savepath, "unset", "edge list save path");
DEFINE_string(result_savepath, "unset", "analysis result save path");
DEFINE_bool(sync, false,
            "if this --sync flag is set to be true, then the message update would be synchronous. "
            "(default: false)");
DEFINE_bool(new_line_sep_edges, false,
            "if this --sync flag is set to be true, then the edges will be separated by new lines. "
            "(default: false)");
DEFINE_bool(idx_reorganized, true,
            "if this --idx_reorganized flag is set to be true, then the node idex would be "
            "reorganized, (default: true)");
int main(int argc, char** argv) {
  gflags::SetVersionString(PROGRAML_VERSION);
  labm8::InitApp(&argc, &argv, usage);

  if (argc < 2) {
    std::cerr << "at least one argument (task name) needed!";
    return 4;
  }
  std::string task_name(argv[1]);
  if (FLAGS_program_graph_sourcepath == "unset") {
    std::cerr << "should set a program graph source file with flag --program_graph_sourcepath";
    return 4;
  }
  programl::ProgramGraph graph;
  // edit begin
  // programl::util::ParseStdinOrDie(&graph);
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
  // edit end
  Status status;
  if ((task_name == "reachability") || (task_name == "dominance") || (task_name == "liveness") ||
      (task_name == "datadep" || (task_name == "subexpressions"))) {
    if (argc != 2) {
      std::cerr << usage;
      return 4;
    }

    programl::ProgramGraphFeaturesList featuresList;
    status = programl::graph::analysis::RunAnalysis(task_name, graph, &featuresList);
    if (!status.ok()) {
      LOG(ERROR) << status.error_message();
      return 4;
    }
    programl::util::WriteStdout(featuresList);
  } else {
    if (argc != 2) {
      std::cerr << usage_yzd;
      return 4;
    }
    programl::ResultsEveryIteration resultsEveryIterationMessage;
    yzd::EdgeList edgeList;
    // int maxIteration = std::atoi(argv[2]);
    // std::cout << "FLAGS_idx_reorganized = " << FLAGS_idx_reorganized << std::endl;
    status = programl::graph::analysis::RunAnalysis(task_name, FLAGS_max_iteration, FLAGS_sync,
                                                    FLAGS_idx_reorganized, graph,
                                                    &resultsEveryIterationMessage, &edgeList);
    // yzd::AnalysisSetting yzd_setting(yzd::TaskName::yzd_liveness, maxIteration);
    // status = yzd::YZDLiveness(graph, yzd_setting).ValidateWithPrograml();
    if (!status.ok()) {
      LOG(ERROR) << status.error_message();
      return 4;
    }

    // edge results record
    std::string edge_result_str = "";
    std::string edge_sep = FLAGS_new_line_sep_edges ? "\n" : " ";
    if (task_name == "yzd_dominance" || task_name == "yzd_reachability") {
      for (const auto& edge : edgeList) {
        edge_result_str += (std::to_string(edge.source_node) + " " +
                            std::to_string(edge.target_node) + edge_sep);
      }

    } else if (task_name == "yzd_liveness") {
      for (const auto& edge : edgeList) {
        edge_result_str +=
            (std::to_string(edge.source_node) + " " + std::to_string(edge.target_node) + " " +
             std::to_string(edge.edge_type) + edge_sep);
      }
    }
    if (FLAGS_edge_list_savepath != "unset") {
      std::ofstream file(FLAGS_edge_list_savepath);
      file << edge_result_str;
      file.close();
    }
    std::size_t edge_size_in_byte = edge_result_str.size() * sizeof(char);
    std::cout << task_name << " edge_size_in_byte: " << edge_size_in_byte << std::endl;
    std::cout << edge_result_str;

    // result trace record
    if (FLAGS_result_savepath != "unset") {
      std::fstream result_record_stream(FLAGS_result_savepath, std::ios::out | std::ios::binary);
      resultsEveryIterationMessage.SerializeToOstream(&result_record_stream);
      result_record_stream.close();
    }
    // programl::util::WriteStdout(resultsEveryIterationMessage);
    std::ostream cout_stream_result(std::cout.rdbuf());
    resultsEveryIterationMessage.SerializeToOstream(&cout_stream_result);
  }

  return 0;
}
