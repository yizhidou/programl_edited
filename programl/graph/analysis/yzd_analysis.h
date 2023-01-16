#pragma once
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "programl/proto/program_graph.pb.h"
#include "programl/proto/util.pb.h"
#include "yzd_utils.h"
#include "labm8/cpp/logging.h"

namespace programl {

class AnalysisBase {
 private:
  std::vector<std::vector<const BitVector*>> result_pointers;
  std::vector<BitVector> stored_result_set;
  std::queue<WorklistItem> work_list;
  int num_iteration = 0;

 protected:
  const ProgramGraph& program_graph;
  Adjacencies adjacencies;

  std::vector<int> program_points;
  std::vector<int> interested_points;
  std::map<int, int> from_interested_points_to_bit_idx;


  AnalysisSetting analysis_setting;
  std::map<int, BitVector> gens;
  std::map<int, BitVector> kills;

 private:
  void InitSettings();

  BitVector MeetBitVectors(const int iterIdx, const int sourceNodeIdx,
                           const std::vector<int>& targetNodeList);

 public:
  explicit AnalysisBase(const ProgramGraph& pg, const AnalysisSetting& s)
      : program_graph(pg), analysis_setting(s) {
    ParseProgramGraph();  // 需要把program_points 和 interested_points 给算好; adjacencies也算好
    InitSettings();       // 这个主要作用往stored_result_set里加初始的结果
    CalculateGenKill();   // 这个就是把gens/kills算出来呗
  }

  void Run(ResultsEveryIteration* results);

  int GetNumIteration() const { return num_iteration; }

  int GetNumProgramPoints() const { return program_points.size(); }

  int GetNumInterestedPoints() const { return interested_points.size(); }

 protected:
  virtual void CalculateGenKill();

  virtual void ParseProgramGraph();

};
}  // namespace programl
