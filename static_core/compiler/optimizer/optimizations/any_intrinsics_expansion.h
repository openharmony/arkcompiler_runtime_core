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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_ANYINTRINSICSEXPANSION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_ANYINTRINSICSEXPANSION_H

#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class AnyIntrinsicsExpansion : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit AnyIntrinsicsExpansion(Graph *graph) : Optimization(graph) {}

    NO_MOVE_SEMANTIC(AnyIntrinsicsExpansion);
    NO_COPY_SEMANTIC(AnyIntrinsicsExpansion);
    ~AnyIntrinsicsExpansion() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "AnyIntrinsicsExpansion";
    }

    bool IsEnable() const override
    {
        return !GetGraph()->IsAotMode() && g_options.IsCompilerOptimizeAnyBytecodes();
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    bool IsApplied() const
    {
        return isApplied_;
    }

    void SetApplied()
    {
        isApplied_ = true;
    }

    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    void HandleAnyLdbyname(IntrinsicInst *inst);
    void HandleAnyStbyname(IntrinsicInst *inst);

    Inst *CreateLoadClassWithGuard(Inst *inst, Inst *objInst, RuntimeInterface::ClassPtr cls);
    Inst *BoxValue(Inst *inst, Inst *val);
    void CallSetter(IntrinsicInst *inst, Inst *val, Inst *obj, RuntimeInterface::MethodPtr setter);

    SmallVector<Inst *, 8U> toRemove_;
    bool isApplied_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_ANYINTRINSICSEXPANSION_H
