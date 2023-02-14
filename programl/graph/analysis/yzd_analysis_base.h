#pragma once
#include <queue>
#include <string>
// #include <utility>
#include <vector>

#include "programl/proto/program_graph.pb.h"
#include "programl/proto/util.pb.h"
#include "yzd_utils.h"
// #include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
// #include "labm8/cpp/statusor.h"
#include "absl/container/flat_hash_map.h"

namespace yzd {

class AnalysisBase {
 private:
  // each element of this vector corresponds to the result of one iteration
  std::vector<absl::flat_hash_map<int, int>> result_pointers;
  std::vector<const NodeSet> stored_nodesets;
  std::queue<WorklistItem> work_list;

 protected:
  const programl::ProgramGraph& program_graph;
  Adjacencies adjacencies;

  NodeSet program_points;
  NodeSet interested_points;

  AnalysisSetting analysis_setting;
  absl::flat_hash_map<int, NodeSet> gens;
  absl::flat_hash_map<int, NodeSet> kills;

 private:
  labm8::Status InitSettings();

  NodeSet MeetOperation(const int iterIdx, const NodeSet& targetNodeList);

 public:
  explicit AnalysisBase(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : program_graph(pg), analysis_setting(s) {}

  labm8::Status Init();
  labm8::Status Run(programl::ResultsEveryIteration* resultsOfAllIterations);

  int GetNumIteration() const { return result_pointers.size(); }

  int GetNumProgramPoints() const { return program_points.size(); }

  int GetNumInterestedPoints() const { return interested_points.size(); }

  absl::flat_hash_map<int, NodeSet> GetLastIterationResult() const {
    absl::flat_hash_map<int, NodeSet> result;
    const absl::flat_hash_map<int, int>& last_result_pointer = result_pointers.back();
    for (const auto& item : last_result_pointer) {
      result[item.first] = stored_nodesets[item.second];
    }
    return result;
  }
  const std::vector<const NodeSet>& GetStoredNodeSets() const { return stored_nodesets; }
  const absl::flat_hash_map<int, NodeSet>& GetGens() const { return gens; }
  const absl::flat_hash_map<int, NodeSet>& GetKills() const { return kills; }

 protected:
  virtual void ParseProgramGraph() = 0;
};
}  // namespace yzd
