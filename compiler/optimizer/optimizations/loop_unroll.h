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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNROLL_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNROLL_H_

#include "optimizer/optimizations/loop_transform.h"
#include "compiler_options.h"

namespace panda::compiler {
/**
 * Loop unroll optimization
 * @param inst_limit - the maximum number of loop instructions after its unrolling
 * @param unroll_factor - the number of loop body copies including the original one
 */
class LoopUnroll : public LoopTransform<LoopExitPoint::LOOP_EXIT_BACKEDGE> {
    struct UnrollParams {
        uint32_t unroll_factor;
        uint32_t cloneable_insts;
        bool has_call;
    };

public:
    LoopUnroll(Graph *graph, uint32_t inst_limit, uint32_t unroll_factor)
        : LoopTransform(graph), INST_LIMIT(inst_limit), UNROLL_FACTOR(unroll_factor)
    {
    }

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerLoopUnroll();
    }

    const char *GetPassName() const override
    {
        return "LoopUnroll";
    }

    void InvalidateAnalyses() override;

private:
    bool TransformLoop(Loop *loop) override;
    UnrollParams GetUnrollParams(Loop *loop);
    void FixCompareInst(const CountableLoopInfo &loop_info, BasicBlock *header, uint32_t unroll_factor);
    Inst *CreateNewTestInst(const CountableLoopInfo &loop_info, Inst *const_inst, Inst *pre_header_cmp);
    bool HasPreHeaderCompare(Loop *loop, const CountableLoopInfo &loop_info);

private:
    const uint32_t INST_LIMIT {0};
    const uint32_t UNROLL_FACTOR {0};
    bool is_applied_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOOP_UNROLL_H_
