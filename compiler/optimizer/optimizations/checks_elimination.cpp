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

#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/ir/graph_visitor.h"
#include "checks_elimination.h"

namespace panda::compiler {

bool ChecksElimination::RunImpl()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ChecksElimination";
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<ObjectTypePropagation>();

    VisitGraph();

    if (options.IsCompilerEnableReplacingChecksOnDeoptimization()) {
        if (!GetGraph()->IsOsrMode()) {
            ReplaceBoundsCheckToDeoptimizationBeforeLoop();
            ReplaceNullCheckToDeoptimizationBeforeLoop();
            MoveAnyTypeCheckOutOfLoop();
        }
        ReplaceBoundsCheckToDeoptimizationInLoop();

        ReplaceCheckMustThrowByUnconditionalDeoptimize();
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "ChecksElimination " << (IsApplied() ? "is" : "isn't") << " applied";
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ChecksElimination";
    return is_applied_;
}

void ChecksElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

void ChecksElimination::VisitNullCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NullCheck with id = " << inst->GetId();

    auto ref = inst->GetInput(0).GetInst();
    static_cast<ChecksElimination *>(v)->TryRemoveDominatedNullChecks(inst, ref);

    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NullCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NullCheck couldn't be deleted";
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NullCheck saved for further replacing on deoptimization";
        static_cast<ChecksElimination *>(v)->PushNewNullCheck(inst);
    }
}

void ChecksElimination::VisitDeoptimizeIf(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToDeoptimizeIf()->GetDeoptimizeType() != DeoptimizeType::NULL_CHECK) {
        return;
    }
    auto compare = inst->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    auto ref = compare->GetInput(0).GetInst();
    ASSERT(ref->GetType() == DataType::REFERENCE);
    ASSERT(compare->GetInput(1).GetInst()->GetOpcode() == Opcode::NullPtr);
    auto visitor = static_cast<ChecksElimination *>(v);
    if (visitor->TryRemoveCheckByBounds<Opcode::NullCheck>(inst, ref)) {
        visitor->SetApplied();
        return;
    }
    visitor->TryRemoveDominatedNullChecks(inst, ref);

    for (auto &user : ref->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst->GetOpcode() == Opcode::NullCheck) {
            if (inst->IsDominate(user_inst)) {
                COMPILER_LOG(DEBUG, CHECKS_ELIM)
                    << "DeoptimizeIf NULL_CHECK with id = " << inst->GetId() << " dominate on "
                    << "NullCheck with id = " << user_inst->GetId();
                visitor->ReplaceUsersAndRemoveCheck(user_inst, ref);
            } else if (user_inst->IsDominate(inst)) {
                COMPILER_LOG(DEBUG, CHECKS_ELIM)
                    << "Remove redundant DeoptimizeIf NULL_CHECK (id = " << inst->GetId() << ")";
                inst->RemoveInputs();
                inst->GetBasicBlock()->ReplaceInst(inst, visitor->GetGraph()->CreateInstNOP());
                visitor->SetApplied();
                return;
            }
        }
    }
}

void ChecksElimination::VisitNegativeCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NegativeCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NegativeCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NegativeCheck couldn't be deleted";
    }
}

void ChecksElimination::VisitZeroCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit ZeroCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::ZeroCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "ZeroCheck couldn't be deleted";
    }
}

void ChecksElimination::VisitRefTypeCheck(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<ChecksElimination *>(v);
    auto store_inst = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    // Case: a[i] = nullptr
    if (store_inst->GetOpcode() == Opcode::NullPtr) {
        visitor->ReplaceUsersAndRemoveCheck(inst, store_inst);
        return;
    }
    auto array_inst = inst->GetDataFlowInput(0);
    auto ref = inst->GetInput(1).GetInst();
    // Case:
    // a[1] = obj
    // a[2] = obj
    for (auto &user : ref->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst->GetOpcode() == Opcode::RefTypeCheck && user_inst != inst &&
            user_inst->GetDataFlowInput(0) == array_inst && user_inst->GetInput(1).GetInst() == ref &&
            inst->InSameBlockOrDominate(user_inst)) {
            ASSERT(inst->IsDominate(user_inst));
            visitor->ReplaceUsersAndRemoveCheck(user_inst, inst);
        }
    }
    visitor->GetGraph()->RunPass<ObjectTypePropagation>();
    auto array_type_info = array_inst->GetObjectTypeInfo();
    auto store_type_info = store_inst->GetObjectTypeInfo();
    if (array_type_info) {
        auto array_class = array_type_info.GetClass();
        auto store_class = (store_type_info) ? store_type_info.GetClass() : nullptr;
        if (visitor->GetGraph()->GetRuntime()->CheckStoreArray(array_class, store_class)) {
            visitor->ReplaceUsersAndRemoveCheck(inst, store_inst);
        }
    }
}

void ChecksElimination::VisitAnyTypeCheck(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<ChecksElimination *>(v);
    auto input_inst = inst->GetInput(0).GetInst();
    auto type = inst->CastToAnyTypeCheck()->GetAnyType();
    if (type == AnyBaseType::UNDEFINED_TYPE) {
        visitor->ReplaceUsersAndRemoveCheck(inst, input_inst);
        return;
    }
    auto language = visitor->GetGraph()->GetRuntime()->GetMethodSourceLanguage(visitor->GetGraph()->GetMethod());
    // from:
    //     2.any  CastValueToAnyType ANY_SUBTYPE v1 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v2, v3 -> (....)
    // to:
    //     2.any  CastValueToAnyType ANY_SUBTYPE v1 -> (...)
    if (input_inst->GetOpcode() == Opcode::CastValueToAnyType) {
        auto res = IsAnyTypeCanBeSubtypeOf(type, input_inst->CastToCastValueToAnyType()->GetAnyType(), language);
        if (!res) {
            return;
        }
        if (res.value()) {
            visitor->ReplaceUsersAndRemoveCheck(inst, input_inst);
        } else {
            visitor->PushNewCheckMustThrow(inst);
        }
        return;
    }
    // from:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v2, v3 -> (....)
    // to:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (...)
    if (input_inst->GetOpcode() == Opcode::AnyTypeCheck) {
        auto res = IsAnyTypeCanBeSubtypeOf(type, input_inst->CastToAnyTypeCheck()->GetAnyType(), language);
        if (!res) {
            return;
        }
        if (res.value()) {
            visitor->ReplaceUsersAndRemoveCheck(inst, input_inst);
        } else {
            visitor->PushNewCheckMustThrow(inst);
        }
        return;
    }
    // from:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v1, v3 -> (....)
    // to:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4,...)
    bool applied = false;
    for (auto &user : input_inst->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst == inst) {
            continue;
        }
        if (user_inst->GetOpcode() != Opcode::AnyTypeCheck) {
            continue;
        }
        if (!user_inst->IsDominate(inst)) {
            continue;
        }
        auto res = IsAnyTypeCanBeSubtypeOf(type, user_inst->CastToAnyTypeCheck()->GetAnyType(), language);
        if (!res) {
            continue;
        }
        applied = true;
        if (res.value()) {
            visitor->ReplaceUsersAndRemoveCheck(inst, user_inst);
        } else {
            visitor->PushNewCheckMustThrow(inst);
        }
        break;
    }
    if (!applied) {
        visitor->PushNewAnyTypeCheck(inst);
    }
}

void ChecksElimination::VisitBoundsCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit BoundsCheck with id = " << inst->GetId();
    auto block = inst->GetBasicBlock();
    auto len_array = inst->GetInput(0).GetInst();
    auto index = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);

    for (auto &user : index->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst != inst && user_inst->GetOpcode() == Opcode::BoundsCheck &&
            len_array == user_inst->GetInput(0).GetInst() && index == user_inst->GetInput(1).GetInst() &&
            inst->InSameBlockOrDominate(user_inst)) {
            ASSERT(inst->IsDominate(user_inst));
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "BoundsCheck with id = " << inst->GetId()
                                             << " dominate on BoundsCheck with id = " << user_inst->GetId();
            visitor->ReplaceUsersAndRemoveCheck(user_inst, inst);
        }
    }

    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto len_array_range = bri->FindBoundsRange(block, len_array);
    auto index_range = bri->FindBoundsRange(block, index);
    if (index_range.IsNotNegative() && (index_range.IsLess(len_array_range) || index_range.IsLess(len_array))) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Index of BoundsCheck have correct bounds";
        visitor->ReplaceUsersAndRemoveCheck(inst, index);
        return;
    }
    if (index_range.IsNegative() || index_range.IsMoreOrEqual(len_array_range)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM)
            << "BoundsCheck have incorrect bounds, saved for replace by unconditional deoptimize";
        visitor->PushNewCheckMustThrow(inst);
        return;
    }

    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "BoundsCheck saved for further replacing on deoptimization";
    auto loop = block->GetLoop();
    ASSERT(loop != nullptr);
    visitor->PushNewBoundsCheck(loop, len_array, index, inst);
}

void ChecksElimination::VisitCheckCast(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->GetGraph()->RunPass<ObjectTypePropagation>();
    auto result = ObjectTypeCheckElimination::TryEliminateCheckCast(inst);
    if (result != ObjectTypeCheckElimination::CheckCastEliminateType::INVALID) {
        visitor->SetApplied();
        if (result == ObjectTypeCheckElimination::CheckCastEliminateType::MUST_THROW) {
            visitor->PushNewCheckMustThrow(inst);
        }
    }
}

void ChecksElimination::ReplaceUsersAndRemoveCheck(Inst *inst_del, Inst *inst_rep)
{
    auto block = inst_del->GetBasicBlock();
    auto graph = block->GetGraph();
    if (graph->IsOsrMode() && block->GetLoop() != inst_rep->GetBasicBlock()->GetLoop()) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Check couldn't be deleted, becouse in OSR mode we can't replace "
                                            "instructions with instructions from another loop";
        return;
    }
    inst_del->ReplaceUsers(inst_rep);
    inst_del->RemoveInputs();
    block->ReplaceInst(inst_del, graph->CreateInstNOP());
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Checks elimination delete " << GetOpcodeString(inst_del->GetOpcode())
                                     << " with id " << inst_del->GetId();
    graph->GetEventWriter().EventChecksElimination(GetOpcodeString(inst_del->GetOpcode()), inst_del->GetId(),
                                                   inst_del->GetPc());
    SetApplied();
}

bool ChecksElimination::IsInstIncOrDec(Inst *inst)
{
    return (inst->GetOpcode() == Opcode::Add || inst->GetOpcode() == Opcode::Sub) &&
           inst->GetInput(1).GetInst()->IsConst();
}

void ChecksElimination::InitItemForNewIndex(GroupedBoundsChecks *place, Inst *index, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    InstVector insts(GetGraph()->GetLocalAllocator()->Adapter());
    insts.push_back(inst);
    int64_t val = 0;
    Inst *parent_index = index;
    if (IsInstIncOrDec(index)) {
        val = static_cast<int64_t>(index->GetInput(1).GetInst()->CastToConstant()->GetIntValue());
        parent_index = index->GetInput(0).GetInst();
        if (index->GetOpcode() == Opcode::Sub) {
            val = -val;
        }
    }
    place->emplace(parent_index, std::make_tuple(insts, val, val));
}

void ChecksElimination::PushNewBoundsCheck(Loop *loop, Inst *len_array, Inst *index, Inst *inst)
{
    ASSERT(loop != nullptr && len_array != nullptr && index != nullptr && inst != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    if (bounds_checks_.find(loop) == bounds_checks_.end()) {
        auto it1 = bounds_checks_.emplace(loop, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it1.second);
        auto it2 = it1.first->second.emplace(len_array, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it2.second);
        InitItemForNewIndex(&it2.first->second, index, inst);
    } else if (bounds_checks_.at(loop).find(len_array) == bounds_checks_.at(loop).end()) {
        auto it1 = bounds_checks_.at(loop).emplace(len_array, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it1.second);
        InitItemForNewIndex(&it1.first->second, index, inst);
    } else if (auto &len_a_bc = bounds_checks_.at(loop).at(len_array); len_a_bc.find(index) == len_a_bc.end()) {
        if (!IsInstIncOrDec(index) || len_a_bc.find(index->GetInput(0).GetInst()) == len_a_bc.end()) {
            auto parent_index = index;
            if (IsInstIncOrDec(index)) {
                parent_index = index->GetInput(0).GetInst();
            }
            InitItemForNewIndex(&len_a_bc, parent_index, inst);
        } else {
            auto val = static_cast<int64_t>(index->GetInput(1).GetInst()->CastToConstant()->GetIntValue());
            auto &item = len_a_bc.at(index->GetInput(0).GetInst());
            std::get<0>(item).push_back(inst);
            if (index->GetOpcode() == Opcode::Add && val > 0 && val > std::get<1>(item)) {
                std::get<1>(item) = val;
            } else if (index->GetOpcode() == Opcode::Add && val < 0 && val < std::get<2U>(item)) {
                std::get<2U>(item) = val;
            } else if (index->GetOpcode() == Opcode::Sub && val > 0 && (-val) < std::get<2U>(item)) {
                std::get<2U>(item) = -val;
            } else if (index->GetOpcode() == Opcode::Sub && val < 0 && (-val) > std::get<1>(item)) {
                std::get<1>(item) = -val;
            }
        }
    } else {
        ASSERT(!IsInstIncOrDec(index));
        auto &item = bounds_checks_.at(loop).at(len_array).at(index);
        std::get<0>(item).push_back(inst);
        if (std::get<1>(item) < 0) {
            std::get<1>(item) = 0;
        }
        if (std::get<2U>(item) > 0) {
            std::get<2U>(item) = 0;
        }
    }
}

void ChecksElimination::TryRemoveDominatedNullChecks(Inst *inst, Inst *ref)
{
    for (auto &user : ref->GetUsers()) {
        auto user_inst = user.GetInst();
        if (((user_inst->GetOpcode() == Opcode::IsInstance && !user_inst->CastToIsInstance()->GetOmitNullCheck()) ||
             (user_inst->GetOpcode() == Opcode::CheckCast && !user_inst->CastToCheckCast()->GetOmitNullCheck())) &&
            inst->IsDominate(user_inst)) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << "NullCheck with id = " << inst->GetId() << " dominate on " << GetOpcodeString(user_inst->GetOpcode())
                << " with id = " << user_inst->GetId();
            if (user_inst->GetOpcode() == Opcode::IsInstance) {
                user_inst->CastToIsInstance()->SetOmitNullCheck(true);
            } else {
                user_inst->CastToCheckCast()->SetOmitNullCheck(true);
            }
            SetApplied();
        }
    }
}

template <Opcode opc>
void ChecksElimination::TryRemoveDominatedChecks(Inst *inst)
{
    for (auto &user : inst->GetInput(0).GetInst()->GetUsers()) {
        auto user_inst = user.GetInst();
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (user_inst->GetOpcode() == opc && user_inst != inst && user_inst->GetType() == inst->GetType() &&
            inst->InSameBlockOrDominate(user_inst)) {
            ASSERT(inst->IsDominate(user_inst));
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                // NOLINTNEXTLINE(readability-magic-numbers)
                << GetOpcodeString(opc) << " with id = " << inst->GetId() << " dominate on " << GetOpcodeString(opc)
                << " with id = " << user_inst->GetId();
            ReplaceUsersAndRemoveCheck(user_inst, inst);
        }
    }
}

// Remove consecutive checks: NullCheck -> NullCheck -> NullCheck
template <Opcode opc>
void ChecksElimination::TryRemoveConsecutiveChecks(Inst *inst)
{
    auto end = inst->GetUsers().end();
    for (auto user = inst->GetUsers().begin(); user != end;) {
        auto user_inst = (*user).GetInst();
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (user_inst->GetOpcode() == opc) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Remove consecutive " << GetOpcodeString(opc);
            ReplaceUsersAndRemoveCheck(user_inst, inst);
            // Start iteration from beginning, because the new successors may be added.
            user = inst->GetUsers().begin();
            end = inst->GetUsers().end();
        } else {
            ++user;
        }
    }
}

template <Opcode opc>
bool ChecksElimination::TryRemoveCheckByBounds(Inst *inst, Inst *input)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(opc == Opcode::ZeroCheck || opc == Opcode::NegativeCheck || opc == Opcode::NullCheck);
    ASSERT(inst->GetOpcode() == opc || (inst->GetOpcode() == Opcode::DeoptimizeIf && opc == Opcode::NullCheck));

    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(block, input);
    bool result = false;
    // NOLINTNEXTLINE(readability-magic-numbers, readability-braces-around-statements, bugprone-branch-clone)
    if constexpr (opc == Opcode::ZeroCheck) {
        result = range.IsLess(BoundsRange(0)) || range.IsMore(BoundsRange(0));
    } else if constexpr (opc == Opcode::NullCheck) {  // NOLINT
        result = range.IsMore(BoundsRange(0));
    } else if constexpr (opc == Opcode::NegativeCheck) {  // NOLINT
        result = range.IsNotNegative();
    }
    if (result) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << GetOpcodeString(opc) << " have correct bounds";
        ReplaceUsersAndRemoveCheck(inst, input);
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers, readability-braces-around-statements)
        if constexpr (opc == Opcode::ZeroCheck || opc == Opcode::NullCheck) {
            result = range.IsEqual(BoundsRange(0));
        } else if constexpr (opc == Opcode::NegativeCheck) {  // NOLINT
            result = range.IsNegative();
        }
        if (result) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                // NOLINTNEXTLINE(readability-magic-numbers)
                << GetOpcodeString(opc) << " must throw, saved for replace by unconditional deoptimize";
            PushNewCheckMustThrow(inst);
        }
    }
    return result;
}

template <Opcode opc>
bool ChecksElimination::TryRemoveCheck(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(opc == Opcode::ZeroCheck || opc == Opcode::NegativeCheck || opc == Opcode::NullCheck);
    ASSERT(inst->GetOpcode() == opc);

    // NOLINTNEXTLINE(readability-magic-numbers)
    TryRemoveDominatedChecks<opc>(inst);
    // NOLINTNEXTLINE(readability-magic-numbers)
    TryRemoveConsecutiveChecks<opc>(inst);

    auto input = inst->GetInput(0).GetInst();
    // NOLINTNEXTLINE(readability-magic-numbers)
    return TryRemoveCheckByBounds<opc>(inst, input);
}

Inst *ChecksElimination::FindSaveState(Loop *loop)
{
    auto block = loop->GetPreHeader();
    while (block != nullptr) {
        for (const auto &inst : block->InstsSafeReverse()) {
            if (inst->GetOpcode() == Opcode::SaveStateDeoptimize || inst->GetOpcode() == Opcode::SaveState) {
                return inst;
            }
        }
        block = block->GetDominator();
    }
    return nullptr;
}

void ChecksElimination::InsertDeoptimization(ConditionCode cc, Inst *left, int64_t val, Inst *right, Inst *ss,
                                             Inst *insert_after)
{
    auto cnst = GetGraph()->FindOrCreateConstant(val);
    auto block = insert_after->GetBasicBlock();
    Inst *new_left = nullptr;
    if (val == 0) {
        new_left = left;
    } else {
        new_left = GetGraph()->CreateInstAdd();
        new_left->SetType(left->GetType());
        new_left->SetInput(0, left);
        new_left->SetInput(1, cnst);
        block->InsertAfter(new_left, insert_after);
    }
    auto deopt_comp = GetGraph()->CreateInstCompare();
    deopt_comp->SetType(DataType::BOOL);
    deopt_comp->SetOperandsType(DataType::INT32);
    deopt_comp->SetCc(cc);
    deopt_comp->SetInput(0, new_left);
    deopt_comp->SetInput(1, right);
    auto deopt = GetGraph()->CreateInstDeoptimizeIf();
    deopt->SetDeoptimizeType(DeoptimizeType::BOUNDS_CHECK);
    deopt->SetInput(0, deopt_comp);
    deopt->SetInput(1, ss);
    deopt->SetPc(ss->GetPc());
    if (val == 0) {
        block->InsertAfter(deopt_comp, insert_after);
    } else {
        block->InsertAfter(deopt_comp, new_left);
    }
    block->InsertAfter(deopt, deopt_comp);
}

Inst *ChecksElimination::InsertDeoptimization(ConditionCode cc, Inst *left, Inst *right, Inst *ss, Inst *insert_after)
{
    auto deopt_comp = GetGraph()->CreateInstCompare();
    deopt_comp->SetType(DataType::BOOL);
    deopt_comp->SetOperandsType(left->GetType());
    deopt_comp->SetCc(cc);
    deopt_comp->SetInput(0, left);
    deopt_comp->SetInput(1, right);
    auto deopt = GetGraph()->CreateInstDeoptimizeIf();
    deopt->SetDeoptimizeType(DeoptimizeType::NULL_CHECK);
    deopt->SetInput(0, deopt_comp);
    deopt->SetInput(1, ss);
    deopt->SetPc(ss->GetPc());
    auto block = insert_after->GetBasicBlock();
    block->InsertAfter(deopt_comp, insert_after);
    block->InsertAfter(deopt, deopt_comp);
    return deopt;
}

std::optional<LoopInfo> ChecksElimination::FindLoopInfo(Loop *loop)
{
    Inst *ss = FindSaveState(loop);
    if (ss == nullptr) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "SaveState isn't founded";
        return std::nullopt;
    }
    auto loop_parser = CountableLoopParser(*loop);
    if (auto loop_info = loop_parser.Parse()) {
        auto loop_info_value = loop_info.value();
        if (loop_info_value.normalized_cc == CC_NE) {
            return std::nullopt;
        }
        ASSERT(loop_info_value.index->GetOpcode() == Opcode::Phi);
        if (loop_info_value.update->GetOpcode() == Opcode::Add) {
            return std::make_tuple(ss, loop_info_value.init, loop_info_value.test, loop_info_value.index,
                                   loop_info_value.normalized_cc == CC_LE ? CC_GE : CC_GT);
        }
        ASSERT(loop_info_value.update->GetOpcode() == Opcode::Sub);
        return std::make_tuple(ss, loop_info_value.test, loop_info_value.init, loop_info_value.index,
                               loop_info_value.normalized_cc);
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Not countable loop isn't supported";
    return std::nullopt;
}

Inst *ChecksElimination::InsertNewLenArray(Inst *len_array, Inst *ss)
{
    auto result_len_array = len_array;
    if (len_array->GetOpcode() == Opcode::LenArray && !len_array->IsDominate(ss)) {
        auto null_check = len_array->GetInput(0).GetInst();
        auto ref = len_array->GetDataFlowInput(null_check);
        if (ref->IsDominate(ss)) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build new NullCheck and LenArray before loop";
            // Build deopt.nullcheck + lenarray before loop
            auto new_len_array = len_array->Clone(GetGraph());
            new_len_array->SetInput(0, ref);
            auto block = ss->GetBasicBlock();
            block->InsertAfter(new_len_array, ss);
            result_len_array = new_len_array;
            auto null_ptr = GetGraph()->GetOrCreateNullPtr();
            auto deopt = InsertDeoptimization(ConditionCode::CC_EQ, ref, null_ptr, ss, ss);
            ChecksElimination::VisitDeoptimizeIf(this, deopt);
        }
    }
    return result_len_array;
}

bool ChecksElimination::TryInsertDeoptimization(LoopInfo loop_info, Inst *len_array, int64_t max_add, int64_t max_sub)
{
    auto [ss, lower, upper, phi_index, cc] = loop_info;
    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto loop = phi_index->GetBasicBlock()->GetLoop();
    auto len_array_range = bri->FindBoundsRange(loop->GetPreHeader(), len_array);
    auto upper_range = bri->FindBoundsRange(loop->GetHeader(), upper).Add(BoundsRange(max_add));
    auto lower_range = bri->FindBoundsRange(loop->GetHeader(), lower).Add(BoundsRange(max_sub));
    auto result_len_array = (len_array == upper ? len_array : InsertNewLenArray(len_array, ss));
    bool insert_new_len_array =
        len_array != result_len_array ||
        (len_array == upper && len_array->GetOpcode() == Opcode::LenArray && ss->IsDominate(len_array));

    bool need_upper_deopt = !((cc != CC_LE && cc != CC_GE && len_array == upper) ||
                              upper_range.IsLess(len_array_range) || upper_range.IsLess(len_array));
    bool need_lower_deopt = !lower_range.IsNotNegative();
    bool insert_upper_deopt =
        (result_len_array->IsDominate(ss) || insert_new_len_array) && (upper->IsDominate(ss) || len_array == upper);
    bool insert_lower_deopt = lower->IsDominate(ss);
    if (need_upper_deopt && need_lower_deopt && insert_upper_deopt && insert_lower_deopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for lower and upper value";
        // Create deoptimize if upper >=(>) result_len_array
        InsertDeoptimization(cc, upper, max_add, result_len_array, ss, insert_new_len_array ? result_len_array : ss);
        // Create deoptimize if lower < 0
        auto zero_const = GetGraph()->FindOrCreateConstant(0);
        InsertDeoptimization(ConditionCode::CC_LT, lower, max_sub, zero_const, ss, ss);
        return true;
    }
    if (need_upper_deopt && insert_upper_deopt && !need_lower_deopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for upper value";
        // Create deoptimize if upper >=(>) result_len_array
        InsertDeoptimization(cc, upper, max_add, result_len_array, ss, insert_new_len_array ? result_len_array : ss);
        return true;
    }
    if (need_lower_deopt && insert_lower_deopt && !need_upper_deopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for lower value";
        // Create deoptimize if lower < 0
        auto zero_const = GetGraph()->FindOrCreateConstant(0);
        InsertDeoptimization(ConditionCode::CC_LT, lower, max_sub, zero_const, ss, ss);
        return true;
    }
    return !need_lower_deopt && !need_upper_deopt;
}

void ChecksElimination::ProcessingGroupBoundsCheck(GroupedBoundsChecks *index_boundschecks, LoopInfo loop_info,
                                                   Inst *len_array)
{
    constexpr auto IMM_3 = 3;
    auto phi_index = std::get<IMM_3>(loop_info);
    if (index_boundschecks->find(phi_index) == index_boundschecks->end()) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Loop index isn't founded for this group";
        return;
    }
    auto &[insts_to_delete, max_add, max_sub] = index_boundschecks->at(phi_index);
    ASSERT(!insts_to_delete.empty());
    if (TryInsertDeoptimization(loop_info, len_array, max_add, max_sub)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Delete group of BoundsChecks";
        // Delete bounds checks instructions
        for (const auto &inst : insts_to_delete) {
            ReplaceUsersAndRemoveCheck(inst, inst->GetInput(1).GetInst());
            if (index_boundschecks->find(inst->GetInput(1).GetInst()) != index_boundschecks->end()) {
                index_boundschecks->erase(inst->GetInput(1).GetInst());
            }
        }
    }
}

void ChecksElimination::ProcessingLoop(Loop *loop, ArenaUnorderedMap<Inst *, GroupedBoundsChecks> *lenarr_index_checks)
{
    auto loop_info = FindLoopInfo(loop);
    if (loop_info == std::nullopt) {
        return;
    }
    for (auto &[len_array, index_boundschecks] : *lenarr_index_checks) {
        ProcessingGroupBoundsCheck(&index_boundschecks, loop_info.value(), len_array);
    }
}

void ChecksElimination::ReplaceBoundsCheckToDeoptimizationBeforeLoop()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ReplaceBoundsCheckToDeoptimizationBeforeLoop";
    for (auto &[loop, lenarr_index_checks] : bounds_checks_) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Processing loop with id = " << loop->GetId();
        if (loop->IsRoot()) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Skip root loop";
            continue;
        }
        ProcessingLoop(loop, &lenarr_index_checks);
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ReplaceBoundsCheckToDeoptimizationBeforeLoop";
}

void ChecksElimination::ReplaceNullCheckToDeoptimizationBeforeLoop()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ReplaceNullCheckToDeoptimizationBeforeLoop";
    for (auto inst : null_checks_) {
        auto block = inst->GetBasicBlock();
        if (block == nullptr) {
            continue;
        }
        auto loop = block->GetLoop();
        if (loop->IsRoot() || loop->GetHeader()->IsOsrEntry() || loop->IsIrreducible()) {
            continue;
        }
        // Find save state
        Inst *ss = FindSaveState(loop);
        if (ss == nullptr) {
            continue;
        }
        auto ref = inst->GetInput(0).GetInst();
        if (ref->IsDominate(ss)) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << "Replace NullCheck with id = " << inst->GetId() << " on deoptimization before loop";
            auto null_ptr = GetGraph()->GetOrCreateNullPtr();
            InsertDeoptimization(ConditionCode::CC_EQ, ref, null_ptr, ss, ss);
            ReplaceUsersAndRemoveCheck(inst, ref);
        }
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ReplaceNullCheckToDeoptimizationBeforeLoop";
}

void ChecksElimination::MoveAnyTypeCheckOutOfLoop()
{
    for (auto inst : any_type_checks_) {
        auto block = inst->GetBasicBlock();
        ASSERT(block != nullptr);
        auto loop = block->GetLoop();
        if (loop->IsRoot() || loop->GetHeader()->IsOsrEntry() || loop->IsIrreducible()) {
            continue;
        }
        // Find save state
        Inst *ss = FindSaveState(loop);
        if (ss == nullptr) {
            continue;
        }
        auto input = inst->GetInput(0).GetInst();
        if (!input->IsDominate(ss)) {
            continue;
        }
        bool is_hot_pass = true;
        for (auto back_edge : loop->GetBackEdges()) {
            if (!block->IsDominate(back_edge)) {
                is_hot_pass = false;
                break;
            }
        }
        // This check is necessary in order not to take AnyTypeCheck out of slowpath.
        if (is_hot_pass) {
            block->EraseInst(inst);
            ss->GetBasicBlock()->InsertAfter(inst, ss);
            inst->SetInput(1, ss);
            inst->SetPc(ss->GetPc());
            SetApplied();
        }
    }
}

Inst *ChecksElimination::FindSaveState(const InstVector &insts_to_delete)
{
    for (auto bounds_check : insts_to_delete) {
        bool is_dominate = true;
        for (auto bounds_check_test : insts_to_delete) {
            if (bounds_check == bounds_check_test) {
                continue;
            }
            is_dominate &= bounds_check->IsDominate(bounds_check_test);
        }
        if (is_dominate) {
            constexpr auto IMM_2 = 2;
            return bounds_check->GetInput(IMM_2).GetInst();
        }
    }
    return nullptr;
}

void ChecksElimination::ReplaceBoundsCheckToDeoptimizationInLoop()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ReplaceBoundsCheckToDeoptimizationInLoop";
    for (auto &item : bounds_checks_) {
        for (auto &[len_array, index_boundschecks_map] : item.second) {
            for (auto &[index, it] : index_boundschecks_map) {
                auto [insts_to_delete, max, min] = it;
                constexpr auto MIN_BOUNDSCHECKS_NUM = 2;
                if (insts_to_delete.size() <= MIN_BOUNDSCHECKS_NUM) {
                    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Skip small group of BoundsChecks";
                    continue;
                }
                // Try to replace more than 2 bounds checks to deoptimization in loop
                auto save_state = FindSaveState(insts_to_delete);
                if (save_state == nullptr) {
                    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "SaveState isn't founded";
                    continue;
                }
                COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Replace group of BoundsChecks on deoptimization in loop";
                auto insert_after = len_array->IsDominate(save_state) ? save_state : len_array;
                // Create deoptimize if max_index >= len_array
                InsertDeoptimization(ConditionCode::CC_GE, index, max, len_array, save_state, insert_after);
                // Create deoptimize if min_index < 0
                auto zero_const = GetGraph()->FindOrCreateConstant(0);
                InsertDeoptimization(ConditionCode::CC_LT, index, min, zero_const, save_state, insert_after);
                for (const auto &inst : insts_to_delete) {
                    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Delete group of BoundsChecks";
                    ReplaceUsersAndRemoveCheck(inst, inst->GetInput(1).GetInst());
                }
            }
        }
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ReplaceBoundsCheckToDeoptimizationInLoop";
}

void ChecksElimination::ReplaceCheckMustThrowByUnconditionalDeoptimize()
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

}  // namespace panda::compiler
