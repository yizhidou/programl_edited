#include "labm8/cpp/status.h"
#include "labm8/cpp/status_macros.h"
#include "liveness.h"
#include "yzd_analysis_base.h"

namespace yzd {
class YZDLiveness : public AnalysisBase {
 public:
  YZDLiveness(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : AnalysisBase(pg, s), programl_liveness_analysis(pg) {}

  labm8::Status ValidateWithPrograml();

 protected:
  virtual void ParseProgramGraph() override;

 private:
  programl::graph::analysis::LivenessAnalysis programl_liveness_analysis;
};
}  // namespace yzd