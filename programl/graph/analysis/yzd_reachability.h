#pragma once

#include "labm8/cpp/status.h"
#include "programl/graph/analysis/reachability.h"
#include "programl/graph/analysis/yzd_analysis_base.h"

namespace yzd {
class YZDReachability : public AnalysisBase {
 public:
  YZDReachability(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : AnalysisBase(pg, s), programl_reachability_analysis(pg) {}
  labm8::Status ValidateWithPrograml();

 protected:
  virtual void ParseProgramGraph() override;
  virtual labm8::Status ParseProgramGraph_idx_reorganized() override;

 private:
  programl::graph::analysis::ReachabilityAnalysis programl_reachability_analysis;
};
}  // namespace yzd