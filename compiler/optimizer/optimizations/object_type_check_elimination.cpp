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

#include "object_type_check_elimination.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"

namespace panda::compiler {
bool ObjectTypeCheckElimination::RunImpl()
{
    GetGraph()->RunPass<ObjectTypePropagation>();
    VisitGraph();
    ReplaceCheckMustThrowByUnconditionalDeoptimize();
    return IsApplied();
}

void ObjectTypeCheckElimination::VisitIsInstance(GraphVisitor *visitor, Inst *inst)
{
    if (TryEliminateIsInstance(inst)) {
        static_cast<ObjectTypeCheckElimination *>(visitor)->SetApplied();
    }
}

void ObjectTypeCheckElimination::VisitCheckCast(GraphVisitor *visitor, Inst *inst)
{
    auto result = TryEliminateCheckCast(inst);
    if (result != CheckCastEliminateType::INVALID) {
        auto opt = static_cast<ObjectTypeCheckElimination *>(visitor);
        opt->SetApplied();
        if (result == CheckCastEliminateType::MUST_THROW) {
            opt->PushNewCheckMustThrow(inst);
        }
    }
}

void ObjectTypeCheckElimination::ReplaceCheckMustThrowByUnconditionalDeoptimize()
{
    for (auto &inst : checks_must_throw_) {
        auto block = inst->GetBasicBlock();
        if (block != nullptr) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << "Replace check with id = " << inst->GetId() << " by uncondition deoptimize";
            block->ReplaceInstByDeoptimize(inst);
            SetApplied();
        }
    }
}

/// This function try to replace IsInstance with a constant.
/// If input of IsInstance is Nullptr then it replaced by zero constant.
bool ObjectTypeCheckElimination::TryEliminateIsInstance(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::IsInstance);
    if (!inst->HasUsers()) {
        return false;
    }
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto ref = inst->GetDataFlowInput(0);
    auto is_instance = inst->CastToIsInstance();
    auto tgt_klass = graph->GetRuntime()->GetClass(is_instance->GetMethod(), is_instance->GetTypeId());
    // If we can't resolve klass in runtime we must throw exception, so we check NullPtr after
    // But we can't change the IsInstance to Deoptimize, because we can resolve after compilation
    if (tgt_klass == nullptr) {
        // If we can't replace IsInstance, we should reset ObjectTypeInfo for input
        ref->SetObjectTypeInfo(ObjectTypeInfo());
        return false;
    }

    // Null isn't instance of any class.
    if (ref->GetOpcode() == Opcode::NullPtr) {
        auto new_cnst = graph->FindOrCreateConstant(0);
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    auto ref_info = ref->GetObjectTypeInfo();
    if (ref_info) {
        auto ref_klass = ref_info.GetClass();
        bool result = graph->GetRuntime()->IsAssignableFrom(tgt_klass, ref_klass);
        auto new_cnst = graph->FindOrCreateConstant(result);
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    // If we can't replace IsInstance, we should reset ObjectTypeInfo for input
    ref->SetObjectTypeInfo(ObjectTypeInfo());
    return false;
}

ObjectTypeCheckElimination::CheckCastEliminateType ObjectTypeCheckElimination::TryEliminateCheckCast(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::CheckCast);
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto ref = inst->GetDataFlowInput(0);
    auto check_cast = inst->CastToCheckCast();
    auto tgt_klass = graph->GetRuntime()->GetClass(check_cast->GetMethod(), check_cast->GetTypeId());
    // If we can't resolve klass in runtime we must throw exception, so we check NullPtr after
    // But we can't change the CheckCast to Deoptimize, because we can resolve after compilation
    if (tgt_klass == nullptr) {
        // If we can't replace CheckCast, we should reset ObjectTypeInfo for input.
        ref->SetObjectTypeInfo(ObjectTypeInfo());
        return CheckCastEliminateType::INVALID;
    }
    // Null can be cast to every type.
    if (ref->GetOpcode() == Opcode::NullPtr) {
        inst->RemoveInputs();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        return CheckCastEliminateType::REDUNDANT;
    }
    auto ref_info = ref->GetObjectTypeInfo();
    if (ref_info) {
        auto ref_klass = ref_info.GetClass();
        bool result = graph->GetRuntime()->IsAssignableFrom(tgt_klass, ref_klass);
        if (result) {
            inst->RemoveInputs();
            block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
            return CheckCastEliminateType::REDUNDANT;
        }
        return CheckCastEliminateType::MUST_THROW;
    }
    // If we can't replace CheckCast, we should reset ObjectTypeInfo for input.
    ref->SetObjectTypeInfo(ObjectTypeInfo());
    return CheckCastEliminateType::INVALID;
}

void ObjectTypeCheckElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}
}  // namespace panda::compiler
