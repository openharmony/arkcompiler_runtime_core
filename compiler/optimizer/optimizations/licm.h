/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_H_

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"

namespace panda::compiler {
class Licm : public Optimization {
public:
    Licm(Graph *graph, uint32_t hoist_limit);
    NO_MOVE_SEMANTIC(Licm);
    NO_COPY_SEMANTIC(Licm);
    ~Licm() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerLicm();
    }

    const char *GetPassName() const override
    {
        return "LICM";
    }

    void InvalidateAnalyses() override;

    bool IsBlockLoopExit(BasicBlock *block);

private:
    bool IsLoopVisited(const Loop &loop) const;
    void VisitLoop(Loop *loop);
    bool IsInstHoistable(Inst *inst);
    void LoopSearchDFS(Loop *loop);
    bool InstDominatesLoopExits(Inst *inst);
    bool InstInputDominatesPreheader(Inst *inst);

private:
    const uint32_t HOIST_LIMIT {0};
    uint32_t hoisted_inst_count_ {0};
    Marker marker_loop_exit_ {UNDEF_MARKER};
    Marker marker_hoist_inst_ {UNDEF_MARKER};
    InstVector hoistable_instructions_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_H_
