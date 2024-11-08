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

#include "programl/graph/analysis/reachability.h"

#include <queue>
#include <utility>
#include <vector>

#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
#include "programl/graph/features.h"

using labm8::Status;
using std::vector;
namespace error = labm8::error;

namespace programl {
namespace graph {
namespace analysis {

Status ReachabilityAnalysis::Init() {
  ComputeAdjacencies({.control = true});
  return Status::OK;
}

vector<int> ReachabilityAnalysis::GetEligibleRootNodes() {
  return GetInstructionsInFunctionsNodeIndices(graph());
}

Status ReachabilityAnalysis::RunOne(int rootNode, ProgramGraphFeatures* features) {
  vector<bool> visited(graph().node_size(), false);

  int dataFlowStepCount = 0;
  std::queue<std::pair<int, int>> q;
  q.push({rootNode, 1});

  const vector<vector<int>>& cfg = adjacencies().control;
  DCHECK(cfg.size() == graph().node_size()) << "CFG size: " << cfg.size() << " != "
                                            << " graph size: " << graph().node_size();

  int activeNodeCount = 0;
  while (!q.empty()) {
    int current = q.front().first;
    dataFlowStepCount = q.front().second;
    q.pop();

    visited[current] = true;
    ++activeNodeCount;

    for (int neighbour : cfg[current]) {
      if (!visited[neighbour]) {
        q.push({neighbour, dataFlowStepCount + 1});
      }
    }
  }

  // Set the node features.
  Feature falseFeature = CreateFeature(0);
  Feature trueFeature = CreateFeature(1);

  for (int i = 0; i < graph().node_size(); ++i) {
    AddNodeFeature(features, "data_flow_root_node", i == rootNode ? trueFeature : falseFeature);
    AddNodeFeature(features, "data_flow_value", visited[i] ? trueFeature : falseFeature);
  }

  SetFeature(features->mutable_features(), "data_flow_step_count",
             CreateFeature(dataFlowStepCount));
  SetFeature(features->mutable_features(), "data_flow_active_node_count",
             CreateFeature(activeNodeCount));

  return Status::OK;
}

labm8::Status ReachabilityAnalysis::CalculateResultFromRootNode(const int rootNode) {  // this is added by yzd for verification
    // std::cout << "we are good at line 89, reachability.cc" << std::endl;
    _result_from_one_root.clear();
    // std::cout << "we are good at line 91, reachability.cc" << std::endl;
    // vector<bool> visited(graph().node_size(), false);

    int dataFlowStepCount = 0;
    std::queue<std::pair<int, int>> q;
    q.push({rootNode, 1});

    const vector<vector<int>>& cfg = adjacencies().control;
    DCHECK(cfg.size() == graph().node_size()) << "CFG size: " << cfg.size() << " != "
                                              << " graph size: " << graph().node_size();

    int activeNodeCount = 0;
    while (!q.empty()) {
      int current = q.front().first;
      dataFlowStepCount = q.front().second;
      q.pop();

      // visited[current] = true;
      _result_from_one_root.insert(current);
      ++activeNodeCount;

      for (int neighbour : cfg[current]) {
        // if (!visited[neighbour]) {
        if (_result_from_one_root.find(neighbour) == _result_from_one_root.end()){
          q.push({neighbour, dataFlowStepCount + 1});
        }
      }
    }
    return Status::OK;
  }

}  // namespace analysis
}  // namespace graph
}  // namespace programl
