/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SIMPLIFY_STRING_BUILDER_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SIMPLIFY_STRING_BUILDER_H_

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace ark::compiler {
class SimplifyStringBuilder : public Optimization {
public:
    explicit SimplifyStringBuilder(Graph *graph);

    NO_MOVE_SEMANTIC(SimplifyStringBuilder);
    NO_COPY_SEMANTIC(SimplifyStringBuilder);
    ~SimplifyStringBuilder() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerSimplifyStringBuilder();
    }

    const char *GetPassName() const override
    {
        return "SimplifyStringBuilder";
    }
    void InvalidateAnalyses() override;

private:
    bool IsMethodStringBuilderConstructorWithStringArg(Inst *inst);
    bool IsMethodStringBuilderToString(Inst *inst);
    InstIter SkipToStringBuilderConstructor(InstIter begin, InstIter end);
    void VisitBlock(BasicBlock *block);

private:
    constexpr static size_t CONSTRUCTOR_WITH_STRING_ARG_TOTAL_ARGS_NUM = 3;
    bool isApplied_ {false};
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_SIMPLIFY_STRING_BUILDER_H_
