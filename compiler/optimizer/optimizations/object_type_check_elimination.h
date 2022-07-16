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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_OBJECT_TYPE_CHECK_ELIMINATION_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_OBJECT_TYPE_CHECK_ELIMINATION_H_

#include "optimizer/ir/graph.h"
#include "compiler_logger.h"
#include "optimizer/pass.h"
#include "deoptimize_elimination.h"
#include "optimizer/ir/graph_visitor.h"

namespace panda::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ObjectTypeCheckElimination : public Optimization, public GraphVisitor {
    using GraphVisitor::GraphVisitor;
    using Optimization::Optimization;

public:
    explicit ObjectTypeCheckElimination(Graph *graph)
        : Optimization(graph), checks_must_throw_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(ObjectTypeCheckElimination);
    NO_COPY_SEMANTIC(ObjectTypeCheckElimination);
    ~ObjectTypeCheckElimination() override = default;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "ObjectTypeCheckElimination";
    }

    bool IsApplied() const
    {
        return is_applied_;
    }

    void InvalidateAnalyses() override;

    /// This function try to replace IsInstance with a constant.
    /// If input of IsInstance is Nullptr then it replaced by zero constant.
    static bool TryEliminateIsInstance(Inst *inst);

    enum CheckCastEliminateType { REDUNDANT, MUST_THROW, INVALID };

    static CheckCastEliminateType TryEliminateCheckCast(Inst *inst);

    static void VisitIsInstance(GraphVisitor *visitor, Inst *inst);
    static void VisitCheckCast(GraphVisitor *visitor, Inst *inst);

#include "optimizer/ir/visitor.inc"
private:
    void PushNewCheckMustThrow(Inst *inst)
    {
        checks_must_throw_.push_back(inst);
    }

    void ReplaceCheckMustThrowByUnconditionalDeoptimize();

    void SetApplied()
    {
        is_applied_ = true;
    }

private:
    bool is_applied_ {false};
    InstVector checks_must_throw_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_OBJECT_TYPE_CHECK_ELIMINATION_H_
