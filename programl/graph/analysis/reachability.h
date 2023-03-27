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
#pragma once

#include "absl/container/flat_hash_set.h"
#include "labm8/cpp/status.h"
#include "programl/graph/analysis/data_flow_pass.h"
#include "programl/proto/program_graph.pb.h"

namespace programl {
namespace graph {
namespace analysis {

// Reachability analysis.
//
// Starting at instruction node `n`, label all instructions which are
// reachable from `n`.
class ReachabilityAnalysis : public RoodNodeDataFlowAnalysis {
 public:
  using RoodNodeDataFlowAnalysis::RoodNodeDataFlowAnalysis;

  virtual labm8::Status RunOne(int rootNode, ProgramGraphFeatures* features) override;

  virtual std::vector<int> GetEligibleRootNodes() override;

  virtual labm8::Status Init() override;
  labm8::Status CalculateResultFromRootNode(int rootNode);  // This is added by yzd for verifying
                                                            // her own calculation result
  absl::flat_hash_set<int> GetResultFromRootNode(){return _result_from_one_root;}

 private:
  absl::flat_hash_set<int> _result_from_one_root;
};

}  // namespace analysis
}  // namespace graph
}  // namespace programl
