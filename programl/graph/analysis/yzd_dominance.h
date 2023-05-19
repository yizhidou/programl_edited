#pragma once

#include "programl/graph/analysis/dominance.h"
#include "labm8/cpp/status.h"
#include "programl/graph/analysis/yzd_analysis_base.h"

namespace yzd {
class YZDDominance : public AnalysisBase {
 public:
  YZDDominance(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : AnalysisBase(pg, s), programl_dominance_analysis(pg) {}
  
  labm8::Status ValidateWithPrograml();

 protected:
  virtual void ParseProgramGraph() override;
  virtual labm8::Status ParseProgramGraph_idx_reorganized() override;

 private:
  programl::graph::analysis::DominanceAnalysis programl_dominance_analysis;
};

}  // namespace yzd