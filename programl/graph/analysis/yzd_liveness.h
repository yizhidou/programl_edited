#include "yzd_analysis_base.h"

namespace yzd {
class YZDLiveness : public AnalysisBase {
 public:
  YZDLiveness(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : AnalysisBase(pg, s) {
        ParseProgramGraph();
        
      }

 protected:
  virtual void ParseProgramGraph() override;
};
}  // namespace yzd