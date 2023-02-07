#pragma once
#include <queue>
#include <string>
// #include <utility>
#include <vector>

#include "yzd_utils.h"
#include "programl/proto/program_graph.pb.h"
#include "programl/proto/util.pb.h"
#include "labm8/cpp/logging.h"
#include "labm8/cpp/status.h"
// #include "labm8/cpp/statusor.h"
#include "absl/container/flat_hash_map.h"

namespace yzd {

class AnalysisBase {
 private:
  // each element of this vector corresponds to the result of one iteration 
  std::vector<absl::flat_hash_map<int, NodeSet*>> result_pointers;
  std::vector<NodeSet> stored_nodesets;
  std::queue<WorklistItem> work_list;
  int num_iteration = 0;

 protected:
  const programl::ProgramGraph& program_graph;
  Adjacencies adjacencies;

  NodeSet program_points;
  NodeSet interested_points;
  std::map<int, int> from_interested_points_to_bit_idx;


  AnalysisSetting analysis_setting;
  std::map<int, NodeSet> gens;
  std::map<int, NodeSet> kills;

 private:
  labm8::Status InitSettings();

  NodeSet MeetOperation(const int iterIdx, const NodeSet& targetNodeList);
  

 public:
  explicit AnalysisBase(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : program_graph(pg), analysis_setting(s) {}

  labm8::Status Run(programl::ResultsEveryIteration* resultsOfAllIterations);

  int GetNumIteration() const { return num_iteration; }

  int GetNumProgramPoints() const { return program_points.size(); }

  int GetNumInterestedPoints() const { return interested_points.size(); }

 protected:
  virtual void ParseProgramGraph() = 0;

};
}  // namespace programl
