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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REDUNDANTLOOPELIMINATIONS_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REDUNDANTLOOPELIMINATIONS_H_

#include "optimizer/pass.h"
#include "optimizer/optimizations/loop_transform.h"

namespace panda::compiler {
class RedundantLoopElimination : public LoopTransform<LoopExitPoint::ALL_LOOP> {
public:
    explicit RedundantLoopElimination(Graph *graph) : LoopTransform(graph) {}

    NO_MOVE_SEMANTIC(RedundantLoopElimination);
    NO_COPY_SEMANTIC(RedundantLoopElimination);
    ~RedundantLoopElimination() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "RedundantLoopElimination";
    }

    bool IsApplied() const
    {
        return is_applied_;
    }

    void InvalidateAnalyses() override;

private:
    bool TransformLoop(Loop *loop) override;
    BasicBlock *IsRedundant(Loop *loop) const;
    void DeleteLoop(Loop *loop, BasicBlock *outside_succ) const;

    bool is_applied_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REDUNDANTLOOPELIMINATIONS_H_
