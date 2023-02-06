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
#include <iomanip>
#include <iostream>
#include <cstdlib>

#include "labm8/cpp/app.h"
#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
#include "programl/graph/analysis/analysis.h"
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

    analyze <analysis> <max_iteration> < /path/to/program_graph.pb)";

int main(int argc, char** argv) {
  gflags::SetVersionString(PROGRAML_VERSION);
  labm8::InitApp(&argc, &argv, usage);

  programl::ProgramGraph graph;
  programl::util::ParseStdinOrDie(&graph);

  Status status;
  if ((argv[1] == "reachability") || (argv[1] == "dominance") || (argv[1] == "liveness") ||
      (argv[1] == "datadep" || (argv[1] == "subexpressions"))) {
    if (argc != 2) {
      std::cerr << usage;
      return 4;
    }

    programl::ProgramGraphFeaturesList featuresList;
    status = programl::graph::analysis::RunAnalysis(argv[1], graph, &featuresList);
    if (!status.ok()) {
      LOG(ERROR) << status.error_message();
      return 4;
    }
    programl::util::WriteStdout(featuresList);
  } else {
    if (argc != 3) {
    std::cerr << usage_yzd;
    return 4;
  }
    programl::ResultsEveryIteration resultsEveryIterationMessage;
    int maxIteration = std::atoi(argv[2]);
    status = programl::graph::analysis::RunAnalysis(argv[1], maxIteration, graph, &resultsEveryIterationMessage);
    if (!status.ok()) {
      LOG(ERROR) << status.error_message();
      return 4;
    }
    programl::util::WriteStdout(resultsEveryIterationMessage);
  }

  return 0;
}
