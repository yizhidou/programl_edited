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
// #pragma once
#include "programl/graph/analysis/analysis.h"

#include "programl/graph/analysis/datadep.h"
#include "programl/graph/analysis/dominance.h"
#include "programl/graph/analysis/liveness.h"
#include "programl/graph/analysis/reachability.h"
#include "programl/graph/analysis/subexpressions.h"
#include "programl/graph/analysis/yzd_liveness.h"
#include "programl/graph/analysis/yzd_reachability.h"
#include "programl/graph/analysis/yzd_dominance.h"
// #include "programl/graph/analysis/yzd_utils.h"

using labm8::Status;
namespace error = labm8::error;

namespace programl {
namespace graph {
namespace analysis {

template <typename T>
Status Run(const ProgramGraph& graph, ProgramGraphFeaturesList* featuresList) {
  T analysis(graph);
  return analysis.Run(featuresList);
}

Status RunAnalysis(const string& analysisName, const ProgramGraph& graph,
                   ProgramGraphFeaturesList* featuresList) {
  if (analysisName == "reachability") {
    return Run<ReachabilityAnalysis>(graph, featuresList);
  } else if (analysisName == "dominance") {
    return Run<DominanceAnalysis>(graph, featuresList);
  } else if (analysisName == "liveness") {
    return Run<LivenessAnalysis>(graph, featuresList);
  } else if (analysisName == "datadep") {
    return Run<DatadepAnalysis>(graph, featuresList);
  } else if (analysisName == "subexpressions") {
    return Run<DatadepAnalysis>(graph, featuresList);
  } else {
    return Status(error::Code::INVALID_ARGUMENT, "Invalid analysis: {}", analysisName);
  }
}

template <typename T>
Status YZDRun(yzd::AnalysisSetting setting, const ProgramGraph& graph,
              ResultsEveryIteration* resultsOfAllIterations, yzd::EdgeList* edgeListToSavePtr) {
  T analysis(graph, setting);
  return analysis.Run(resultsOfAllIterations, edgeListToSavePtr);
}

Status RunAnalysis(const string& analysisName, int maxIteration, bool if_sync,
                   bool if_idx_reorganized, const ProgramGraph& graph,
                   ResultsEveryIteration* resultsOfAllIterations,
                   yzd::EdgeList* edgeListToSavePtr) {
  // yzd::AnalysisSetting yzd_setting;
  yzd::SyncOrAsync sync_or_async = if_sync ? yzd::sync : yzd::async;
  if (analysisName == "yzd_liveness") {
    yzd::AnalysisSetting yzd_setting(yzd::TaskName::yzd_liveness, maxIteration, sync_or_async,
                                     if_idx_reorganized);
    return YZDRun<yzd::YZDLiveness>(yzd_setting, graph, resultsOfAllIterations, edgeListToSavePtr);
  } else if (analysisName == "yzd_dominance") {
    yzd::AnalysisSetting yzd_setting(yzd::TaskName::yzd_dominance, maxIteration, sync_or_async,
                                     if_idx_reorganized);
    return YZDRun<yzd::YZDDominance>(yzd_setting, graph, resultsOfAllIterations, edgeListToSavePtr);
  } else if (analysisName == "yzd_reachability") {
    yzd::AnalysisSetting yzd_setting(yzd::TaskName::yzd_reachability, maxIteration, sync_or_async,
                                     if_idx_reorganized);
    return YZDRun<yzd::YZDReachability>(yzd_setting, graph, resultsOfAllIterations, edgeListToSavePtr);
  } else {
    return Status(error::Code::INVALID_ARGUMENT, "Invalid analysis: {}", analysisName);
  }
}

}  // namespace analysis
}  // namespace graph
}  // namespace programl
