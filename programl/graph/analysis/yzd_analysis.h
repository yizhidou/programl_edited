#pragma once
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "programl/proto/program_graph.pb.h"
#include "programl/proto/util.pb.h"
#include "yzd_utils.h"
#include "labm8/cpp/logging.h"
#include "absl/container/flat_hash_map.h"

namespace yzd {

class AnalysisBase {
 private:
  // std::vector<std::vector<const SparseBitVector*>> result_pointers;
  std::vector<absl::flat_hash_map<int, SparseBitVector*>> result_pointers;
  std::vector<SparseBitVector> stored_result_set;
  std::queue<WorklistItem> work_list;
  int num_iteration = 0;

 protected:
  const programl::ProgramGraph& program_graph;
  Adjacencies adjacencies;

  absl::flat_hash_set<int> program_points;
  absl::flat_hash_set<int> interested_points;
  std::map<int, int> from_interested_points_to_bit_idx;


  AnalysisSetting analysis_setting;
  std::map<int, SparseBitVector> gens;
  std::map<int, SparseBitVector> kills;

 private:
  void InitSettings();

  SparseBitVector MeetBitVectors(const int iterIdx, const absl::flat_hash_set<int>& targetNodeList);
  

 public:
  explicit AnalysisBase(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : program_graph(pg), analysis_setting(s) {
    ParseProgramGraph();  // 需要把program_points 和 interested_points 给算好; adjacencies也算好
    InitSettings();       // 这个主要作用往stored_result_set里加初始的结果
  }

  void Run(programl::ResultsEveryIteration* resultsOfAllIterations);

  int GetNumIteration() const { return num_iteration; }

  int GetNumProgramPoints() const { return program_points.size(); }

  int GetNumInterestedPoints() const { return interested_points.size(); }

 protected:
  virtual void ParseProgramGraph();

};
}  // namespace programl
