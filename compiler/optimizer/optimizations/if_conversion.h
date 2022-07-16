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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_IF_CONVERSION_H_
#define COMPILER_OPTIMIZER_ANALYSIS_IF_CONVERSION_H_

#include "optimizer/ir/graph.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
class BasicBlock;
class Graph;

class IfConversion : public Optimization {
public:
    explicit IfConversion(Graph *graph, uint32_t limit = 2) : Optimization(graph), limit_(limit) {}

    NO_MOVE_SEMANTIC(IfConversion);
    NO_COPY_SEMANTIC(IfConversion);
    ~IfConversion() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerIfConversion();
    }

    const char *GetPassName() const override
    {
        return "IfConversion";
    }

    void InvalidateAnalyses() override;

private:
    uint32_t limit_;
    bool TryTriangle(BasicBlock *bb);
    bool TryDiamond(BasicBlock *bb);
    static bool IsConvertable(BasicBlock *bb, uint32_t *inst_count);
    static bool IsPhisAllowed(BasicBlock *bb, BasicBlock *pred1, BasicBlock *pred2, uint32_t *phi_count);
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_IF_CONVERSION_H_
