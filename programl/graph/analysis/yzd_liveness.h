#include "labm8/cpp/status.h"
#include "labm8/cpp/status_macros.h"
#include "liveness.h"
#include "yzd_analysis_base.h"

namespace yzd {
class YZDLiveness : public AnalysisBase {
 public:
  YZDLiveness(const programl::ProgramGraph& pg, const AnalysisSetting& s)
      : AnalysisBase(pg, s), programl_liveness_analysis(pg) {}

  labm8::Status RunAndValidate() {
    RETURN_IF_ERROR(programl_liveness_analysis.Init());
    RETURN_IF_ERROR(Init());
    // 接下来应该就是对比最后一个yzd最后一个iteration和livein是不是一致了
    const std::vector<absl::flat_hash_set<int>> programl_result =
        programl_liveness_analysis.live_in_sets();
    const absl::flat_hash_map<int, NodeSet> yzd_last_result = GetLastIterationResult();
    for (int node_idx = 0; node_idx < programl_result.size(); node_idx++) {
      const auto yzd_iter = yzd_last_result.find(node_idx);
      if (yzd_iter == yzd_last_result.end()) {
        assert((program_graph.node(node_idx).type() != programl::Node::INSTRUCTION) &&
               "If the node contains in LiveInSet but not in program points, then it should not be "
               "an INSTRUCTION node~");
      } else {
        // check 这俩对应的NodeSet一样
        // auto tmp = yzd_last_result.find(node_idx)
        if (!(yzd_iter->second == programl_result[node_idx])) {
          // 这就是俩不一样
          std::cout << "NOT THE SAME! node_idx: " << node_idx << std::endl;
          // 还要去实现一个NodeSet的打印
          std::cout << "from yzd: " << yzd_iter->second << std::endl;
          std::cout << "from programl: " << programl_result[node_idx] << std::endl;
          return labm8::Status(labm8::error::FAILED_PRECONDITION,
                         "Results from the two are not exactly the same!!!");
        }
      }
    }
  }

 protected:
  virtual void ParseProgramGraph() override;

 private:
  programl::graph::analysis::LivenessAnalysis programl_liveness_analysis;
};
}  // namespace yzd