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
    GetGraph()->RunPass<DominatorsTree>();
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

/**
 * This function try to replace IsInstance with a constant.
 * If input of IsInstance is Nullptr then it replaced by zero constant.
 */
bool ObjectTypeCheckElimination::TryEliminateIsInstance(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::IsInstance);
    if (!inst->HasUsers()) {
        return false;
    }
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto ref = inst->GetDataFlowInput(0);
    // Null isn't instance of any class.
    if (ref->GetOpcode() == Opcode::NullPtr) {
        auto new_cnst = graph->FindOrCreateConstant(0);
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    auto is_instance = inst->CastToIsInstance();
    if (!graph->IsBytecodeOptimizer() && IsMember(ref, is_instance->GetTypeId(), is_instance)) {
        if (BoundsAnalysis::IsInstNotNull(ref, block)) {
            auto new_cnst = graph->FindOrCreateConstant(true);
            inst->ReplaceUsers(new_cnst);
            return true;
        }
    }
    auto tgt_klass = graph->GetRuntime()->GetClass(is_instance->GetMethod(), is_instance->GetTypeId());
    // If we can't resolve klass in runtime we must throw exception, so we check NullPtr after
    // But we can't change the IsInstance to Deoptimize, because we can resolve after compilation
    if (tgt_klass == nullptr) {
        return false;
    }
    auto ref_info = ref->GetObjectTypeInfo();
    if (ref_info) {
        auto ref_klass = ref_info.GetClass();
        bool result = graph->GetRuntime()->IsAssignableFrom(tgt_klass, ref_klass);
        // If ref can be null, IsInstance cannot be changed to true
        if (result) {
            if (graph->IsBytecodeOptimizer()) {
                // Cannot run BoundsAnalysis
                return false;
            }
            if (!BoundsAnalysis::IsInstNotNull(ref, block)) {
                return false;
            }
        }
        // If class of ref can be subclass of ref_klass, IsInstance cannot be changed to false
        if (!result && !ref_info.IsExact()) {
            return false;
        }
        auto new_cnst = graph->FindOrCreateConstant(result);
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

ObjectTypeCheckElimination::CheckCastEliminateType ObjectTypeCheckElimination::TryEliminateCheckCast(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::CheckCast);
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto ref = inst->GetDataFlowInput(0);
    // Null can be cast to every type.
    if (ref->GetOpcode() == Opcode::NullPtr) {
        inst->RemoveInputs();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        return CheckCastEliminateType::REDUNDANT;
    }
    auto check_cast = inst->CastToCheckCast();
    auto tgt_klass = graph->GetRuntime()->GetClass(check_cast->GetMethod(), check_cast->GetTypeId());
    auto ref_info = ref->GetObjectTypeInfo();
    // If we can't resolve klass in runtime we must throw exception, so we check NullPtr after
    // But we can't change the CheckCast to Deoptimize, because we can resolve after compilation
    if (tgt_klass != nullptr && ref_info) {
        auto ref_klass = ref_info.GetClass();
        bool result = graph->GetRuntime()->IsAssignableFrom(tgt_klass, ref_klass);
        if (result) {
            inst->RemoveInputs();
            block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
            return CheckCastEliminateType::REDUNDANT;
        }
        if (BoundsAnalysis::IsInstNotNull(ref, block) && ref_info.IsExact()) {
            return CheckCastEliminateType::MUST_THROW;
        }
    }
    if (IsMember(ref, check_cast->GetTypeId(), inst)) {
        inst->RemoveInputs();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        return CheckCastEliminateType::REDUNDANT;
    }
    return CheckCastEliminateType::INVALID;
}

// returns true if data flow input of inst is always member of class type_id when ref_user is executed
bool ObjectTypeCheckElimination::IsMember(Inst *inst, uint32_t type_id, Inst *ref_user)
{
    for (auto &user : inst->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst == ref_user || !user_inst->IsDominate(ref_user)) {
            continue;
        }
        bool success = false;
        switch (user_inst->GetOpcode()) {
            case Opcode::CheckCast:
                success = (user_inst->CastToCheckCast()->GetTypeId() == type_id);
                break;
            case Opcode::IsInstance:
                success = IsSuccessfulIsInstance(user_inst->CastToIsInstance(), type_id, ref_user);
                break;
            case Opcode::NullCheck:
                success = IsMember(user_inst, type_id, ref_user);
            default:
                break;
        }
        if (success) {
            return true;
        }
    }
    return false;
}

// returns true if is_instance has given type_id and evaluates to true at ref_user
bool ObjectTypeCheckElimination::IsSuccessfulIsInstance(IsInstanceInst *is_instance, uint32_t type_id, Inst *ref_user)
{
    ASSERT(is_instance->GetDataFlowInput(0) == ref_user->GetDataFlowInput(0));
    if (is_instance->GetTypeId() != type_id) {
        return false;
    }
    for (auto &user : is_instance->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst->GetOpcode() != Opcode::IfImm) {
            continue;
        }
        auto true_block = user_inst->CastToIfImm()->GetEdgeIfInputTrue();
        if (true_block->GetPredsBlocks().size() == 1 && true_block->IsDominate(ref_user->GetBasicBlock())) {
            return true;
        }
    }
    return false;
}

void ObjectTypeCheckElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}
}  // namespace panda::compiler
