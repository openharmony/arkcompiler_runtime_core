/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_HYBRID_STRINGS_OPTIMIZATION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_HYBRID_STRINGS_OPTIMIZATION_H

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"
#include "optimizer/optimizations/string_builder_utils.h"
#include "compiler_options.h"

namespace ark::compiler {

class PANDA_PUBLIC_API HybridStringsOptimization : public Optimization {
public:
    explicit HybridStringsOptimization(Graph *graph);

    NO_MOVE_SEMANTIC(HybridStringsOptimization);
    NO_COPY_SEMANTIC(HybridStringsOptimization);
    ~HybridStringsOptimization() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerHybridStringsOptimization() && !GetGraph()->IsOsrMode();
    }

    const char *GetPassName() const override
    {
        return "HybridStringsOptimization";
    }

private:
    struct ConcatenationMatch {
        ConcatenationMatch(Graph *graph) : concatInsts {graph->GetLocalAllocator()->Adapter()} {}

        bool IsValid() const;
        ArenaVector<Inst *> concatInsts;
        size_t width {1};
        bool requireFlattening {false};
        bool usedByConcatOnly {true};
        bool usedInTryCatch {false};
    };

    void MatchConcatenationExpression(Inst *inst, ConcatenationMatch &match, Marker visited, Marker maybeDeoptimized);
    void ReplaceEachWithStringBuilder(const ConcatenationMatch &match);
    void Cleanup(const ConcatenationMatch &match);
    bool HasUserRequireFlattening(Inst *inst);
    void FixBrokenSaveStates(Inst *source, Inst *target);
    void RemoveFromSaveStateInputs(Inst *inst, bool doMark = false);
    void RemoveFromAllExceptPhiInputs(Inst *inst);
    bool ManuallyInlineMethods();
    void MarkMaybeDeoptimizedInstructions(Marker maybeDeoptimized);
    void MarkMaybeDeoptimizedInstruction(Inst *inst, Marker visited, Marker maybeDeoptimized);
    void MarkMaybeDeoptimizedConcatStringsInputs(Inst *inst, Marker visited, Marker maybeDeoptimized);

    bool isApplied_ {false};
    SaveStateBridgesBuilder ssb_ {};
    ArenaVector<InputDesc> inputDescriptors_;
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_HYBRID_STRINGS_OPTIMIZATION_H
