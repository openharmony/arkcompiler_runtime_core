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
#include "checks_elimination.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/analysis.h"

namespace panda::compiler {

bool ChecksElimination::RunImpl()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ChecksElimination";
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<ObjectTypePropagation>();

    VisitGraph();

    if (OPTIONS.IsCompilerEnableReplacingChecksOnDeoptimization()) {
        if (!GetGraph()->IsOsrMode()) {
            ReplaceBoundsCheckToDeoptimizationBeforeLoop();
            MoveCheckOutOfLoop();
        }
        ReplaceBoundsCheckToDeoptimizationInLoop();

        ReplaceCheckMustThrowByUnconditionalDeoptimize();
    }
    if (IsLoopDeleted() && GetGraph()->IsOsrMode()) {
        CleanupGraphSaveStateOSR(GetGraph());
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "ChecksElimination " << (IsApplied() ? "is" : "isn't") << " applied";
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ChecksElimination";
    return is_applied_;
}

void ChecksElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    // Already "LoopAnalyzer" was ran in "CleanupGraphSaveStateOSR"
    // in case (IsLoopDeleted() && GetGraph()->IsOsrMode())
    if (!(IsLoopDeleted() && GetGraph()->IsOsrMode())) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

template <bool CHECK_FULL_DOM>
void ChecksElimination::VisitNullCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NullCheck with id = " << inst->GetId();

    auto ref = inst->GetInput(0).GetInst();
    static_cast<ChecksElimination *>(v)->TryRemoveDominatedNullChecks(inst, ref);

    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NullCheck, CHECK_FULL_DOM>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NullCheck couldn't be deleted";
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NullCheck saved for further replacing on deoptimization";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitNegativeCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NegativeCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NegativeCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NegativeCheck couldn't be deleted";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitNotPositiveCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NotPositiveCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NotPositiveCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NotPositiveCheck couldn't be deleted";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitZeroCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit ZeroCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::ZeroCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "ZeroCheck couldn't be deleted";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
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
    visitor->TryRemoveDominatedChecks<Opcode::RefTypeCheck>(inst, [array_inst, ref](Inst *user_inst) {
        return user_inst->GetDataFlowInput(0) == array_inst && user_inst->GetInput(1) == ref;
    });
    visitor->GetGraph()->RunPass<ObjectTypePropagation>();
    auto array_type_info = array_inst->GetObjectTypeInfo();
    if (array_type_info && array_type_info.IsExact()) {
        auto store_type_info = store_inst->GetObjectTypeInfo();
        auto array_class = array_type_info.GetClass();
        auto store_class = (store_type_info) ? store_type_info.GetClass() : nullptr;
        if (visitor->GetGraph()->GetRuntime()->CheckStoreArray(array_class, store_class)) {
            visitor->ReplaceUsersAndRemoveCheck(inst, store_inst);
        }
    }
}

bool ChecksElimination::TryToEliminateAnyTypeCheck(Inst *inst, Inst *inst_to_replace, AnyBaseType type,
                                                   AnyBaseType prev_type)
{
    auto language = GetGraph()->GetRuntime()->GetMethodSourceLanguage(GetGraph()->GetMethod());
    auto allowed_type = inst->CastToAnyTypeCheck()->GetAllowedInputType();
    profiling::AnyInputType prev_allowed_type;
    if (inst_to_replace->GetOpcode() == Opcode::AnyTypeCheck) {
        prev_allowed_type = inst_to_replace->CastToAnyTypeCheck()->GetAllowedInputType();
    } else {
        prev_allowed_type = inst_to_replace->CastToCastValueToAnyType()->GetAllowedInputType();
    }
    auto res = IsAnyTypeCanBeSubtypeOf(language, type, prev_type, allowed_type, prev_allowed_type);
    if (!res) {
        return false;
    }
    if (res.value()) {
        ReplaceUsersAndRemoveCheck(inst, inst_to_replace);
    } else {
        PushNewCheckMustThrow(inst);
    }
    return true;
}

void ChecksElimination::UpdateHclassChecks(Inst *inst)
{
    // AnyTypeCheck HEAP_OBJECT => LoadObject Class => LoadObject Hclass => HclassCheck
    bool all_users_inlined = false;
    for (auto &user : inst->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst->IsCall()) {
            if (user_inst->CastToCallDynamic()->IsInlined()) {
                all_users_inlined = true;
            } else {
                return;
            }
        } else if (user_inst->GetOpcode() == Opcode::LoadObject) {
            if (user_inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_CLASS) {
                continue;
            }
            ASSERT(user_inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_METHOD);
            if (!IsInlinedCallLoadMethod(user_inst)) {
                return;
            }
            all_users_inlined = true;
        } else {
            return;
        }
    }
    if (!all_users_inlined) {
        return;
    }
    for (auto &user : inst->GetUsers()) {
        auto user_inst = user.GetInst();
        auto check = GetHclassCheckFromLoads(user_inst);
        if (!check.has_value()) {
            continue;
        }
        check.value()->CastToHclassCheck()->SetCheckFunctionIsNotClassConstructor(false);
        SetApplied();
    }
}

std::optional<Inst *> ChecksElimination::GetHclassCheckFromLoads(Inst *load_class)
{
    if (load_class->GetOpcode() != Opcode::LoadObject ||
        load_class->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_CLASS) {
        return std::nullopt;
    }
    for (auto &users_load : load_class->GetUsers()) {
        auto load_hclass = users_load.GetInst();
        if (load_hclass->GetOpcode() != Opcode::LoadObject ||
            load_hclass->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_HCLASS) {
            continue;
        }
        for (auto &users_hclass : load_hclass->GetUsers()) {
            auto hclass_check = users_hclass.GetInst();
            if (hclass_check->GetOpcode() == Opcode::HclassCheck) {
                return hclass_check;
            }
        }
    }
    return std::nullopt;
}

bool ChecksElimination::IsInlinedCallLoadMethod(Inst *inst)
{
    bool is_method = inst->GetOpcode() == Opcode::LoadObject &&
                     inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_METHOD;
    if (!is_method) {
        return false;
    }
    for (auto &method_user : inst->GetUsers()) {
        auto compare = method_user.GetInst()->CastToCompare();
        bool is_compare = compare->GetOpcode() == Opcode::Compare && compare->GetCc() == ConditionCode::CC_NE &&
                          compare->GetInputType(0) == DataType::POINTER;
        if (!is_compare) {
            return false;
        }
        auto deoptimize_if = compare->GetFirstUser()->GetInst()->CastToDeoptimizeIf();
        if (deoptimize_if->GetDeoptimizeType() != DeoptimizeType::INLINE_DYN) {
            return false;
        }
    }
    return true;
}

void ChecksElimination::TryRemoveDominatedHclassCheck(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::HclassCheck);
    // AnyTypeCheck HEAP_OBJECT => LoadObject Class => LoadObject Hclass => HclassCheck
    auto object = inst->GetInput(0).GetInst()->GetInput(0).GetInst()->GetInput(0).GetInst();

    for (auto &user : object->GetUsers()) {
        auto user_inst = user.GetInst();
        auto hclass_check = GetHclassCheckFromLoads(user_inst);
        if (hclass_check.has_value() && hclass_check.value() != inst && inst->IsDominate(hclass_check.value())) {
            ASSERT(inst->IsDominate(hclass_check.value()));
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << GetOpcodeString(Opcode::HclassCheck) << " with id = " << inst->GetId() << " dominate on "
                << GetOpcodeString(Opcode::HclassCheck) << " with id = " << hclass_check.value()->GetId();
            auto block = hclass_check.value()->GetBasicBlock();
            auto graph = block->GetGraph();
            hclass_check.value()->RemoveInputs();
            // We don't should update because it's possible to delete one flag in check, when all DynCall is inlined
            inst->CastToHclassCheck()->ExtendFlags(hclass_check.value());
            block->ReplaceInst(hclass_check.value(), graph->CreateInstNOP());
            SetApplied();
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
    // from:
    //     2.any  CastValueToAnyType ANY_SUBTYPE v1 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v2, v3 -> (....)
    // to:
    //     2.any  CastValueToAnyType ANY_SUBTYPE v1 -> (...)
    if (input_inst->GetOpcode() == Opcode::CastValueToAnyType) {
        visitor->TryToEliminateAnyTypeCheck(inst, input_inst, type,
                                            input_inst->CastToCastValueToAnyType()->GetAnyType());
        return;
    }
    // from:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v2, v3 -> (....)
    // to:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (...)
    if (input_inst->GetOpcode() == Opcode::AnyTypeCheck) {
        visitor->TryToEliminateAnyTypeCheck(inst, input_inst, type, input_inst->CastToAnyTypeCheck()->GetAnyType());
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
        if (!inst->IsDominate(user_inst)) {
            continue;
        }

        if (visitor->TryToEliminateAnyTypeCheck(user_inst, inst, user_inst->CastToAnyTypeCheck()->GetAnyType(), type)) {
            applied = true;
        }
    }
    if (!applied) {
        visitor->PushNewCheckForMoveOutOfLoop(inst);
    }
    // from:
    //    39.any  AnyTypeCheck ECMASCRIPT_HEAP_OBJECT_TYPE v37, v38 -> (v65, v40)
    //    40.ref  LoadObject 4294967288 Class v39 -> (v41)
    //    41.ref  LoadObject 4294967287 Hclass v40
    //    42.     HclassCheck [IsFunc, IsNotClassConstr] v41, v38
    //      ...METHOD IS INLINED ...
    // to:
    //    39.any  AnyTypeCheck ECMASCRIPT_HEAP_OBJECT_TYPE v37, v38 -> (v65, v40)
    //    40.ref  LoadObject 4294967288 Class v39 -> (v41)
    //    41.ref  LoadObject 4294967287 Hclass v40
    //    42.     HclassCheck [IsFunc] v41, v38
    //      ...METHOD IS INLINED ...
    if (TryIsDynHeapObject(type)) {
        visitor->UpdateHclassChecks(inst);
    }
}

void ChecksElimination::VisitHclassCheck(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedHclassCheck(inst);
    visitor->PushNewCheckForMoveOutOfLoop(inst);
}

void ChecksElimination::VisitBoundsCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit BoundsCheck with id = " << inst->GetId();
    auto block = inst->GetBasicBlock();
    auto len_array = inst->GetInput(0).GetInst();
    auto index = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);

    visitor->TryRemoveDominatedChecks<Opcode::BoundsCheck>(inst, [len_array, index](Inst *user_inst) {
        return len_array == user_inst->GetInput(0) && index == user_inst->GetInput(1);
    });

    if (index->GetType() == DataType::UINT64) {
        return;
    }
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto len_array_range = bri->FindBoundsRange(block, len_array);
    auto index_range = bri->FindBoundsRange(block, index);
    auto correct_upper = index_range.IsLess(len_array_range) || index_range.IsLess(len_array);
    auto correct_lower = index_range.IsNotNegative();
    if (correct_upper && correct_lower) {
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
    auto loop = GetLoopForBoundsCheck(block, len_array, index);
    visitor->PushNewBoundsCheck(loop, len_array, index, inst, !correct_upper, !correct_lower);
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
        return;
    }
    if (inst->CastToCheckCast()->GetOmitNullCheck()) {
        return;
    }
    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto input_range = bri->FindBoundsRange(block, inst->GetInput(0).GetInst());
    if (input_range.IsMore(BoundsRange(0))) {
        visitor->SetApplied();
        inst->CastToCheckCast()->SetOmitNullCheck(true);
    }
}

void ChecksElimination::VisitIsInstance(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToIsInstance()->GetOmitNullCheck()) {
        return;
    }
    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto input_range = bri->FindBoundsRange(block, inst->GetInput(0).GetInst());
    if (input_range.IsMore(BoundsRange(0))) {
        auto visitor = static_cast<ChecksElimination *>(v);
        visitor->SetApplied();
        inst->CastToIsInstance()->SetOmitNullCheck(true);
    }
}

void ChecksElimination::VisitAddOverflowCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetType() == DataType::INT32);
    auto input1 = inst->GetInput(0).GetInst();
    auto input2 = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedChecks<Opcode::AddOverflowCheck>(inst, [input1, input2](Inst *user_inst) {
        return (user_inst->GetInput(0) == input1 && user_inst->GetInput(1) == input2) ||
               (user_inst->GetInput(0) == input2 && user_inst->GetInput(1) == input1);
    });
    visitor->TryOptimizeOverflowCheck<Opcode::AddOverflowCheck>(inst);
}

void ChecksElimination::VisitSubOverflowCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetType() == DataType::INT32);
    auto input1 = inst->GetInput(0).GetInst();
    auto input2 = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedChecks<Opcode::SubOverflowCheck>(inst, [input1, input2](Inst *user_inst) {
        return (user_inst->GetInput(0) == input1 && user_inst->GetInput(1) == input2);
    });
    visitor->TryOptimizeOverflowCheck<Opcode::SubOverflowCheck>(inst);
}

void ChecksElimination::VisitNegOverflowAndZeroCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetType() == DataType::INT32);
    auto input1 = inst->GetInput(0).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedChecks<Opcode::NegOverflowAndZeroCheck>(
        inst, [input1](Inst *user_inst) { return (user_inst->GetInput(0) == input1); });
    visitor->TryOptimizeOverflowCheck<Opcode::NegOverflowAndZeroCheck>(inst);
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
    return inst->IsAddSub() && inst->GetInput(1).GetInst()->IsConst();
}

int64_t ChecksElimination::GetInc(Inst *inst)
{
    ASSERT(IsInstIncOrDec(inst));
    auto val = static_cast<int64_t>(inst->GetInput(1).GetInst()->CastToConstant()->GetIntValue());
    if (inst->IsSub()) {
        val = -val;
    }
    return val;
}

Loop *ChecksElimination::GetLoopForBoundsCheck(BasicBlock *block, Inst *len_array, Inst *index)
{
    auto parent_index = IsInstIncOrDec(index) ? index->GetInput(0).GetInst() : index;
    auto index_block = parent_index->GetBasicBlock();
    ASSERT(index_block != nullptr);
    auto index_loop = index_block->GetLoop();
    if (auto loop_info = CountableLoopParser(*index_loop).Parse()) {
        auto input = len_array;
        if (len_array->GetOpcode() == Opcode::LenArray) {
            // new len_array can be inserted
            input = len_array->GetDataFlowInput(0);
        }
        if (loop_info->index == parent_index && input->GetBasicBlock()->IsDominate(index_block)) {
            ASSERT(index_block == index_loop->GetHeader());
            return index_loop;
        }
    }
    return block->GetLoop();
}

void ChecksElimination::InitItemForNewIndex(GroupedBoundsChecks *place, Inst *index, Inst *inst, bool check_upper,
                                            bool check_lower)
{
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    InstVector insts(GetGraph()->GetLocalAllocator()->Adapter());
    insts.push_back(inst);
    int64_t val = 0;
    Inst *parent_index = index;
    if (IsInstIncOrDec(index)) {
        val = GetInc(index);
        parent_index = index->GetInput(0).GetInst();
    } else if (index->IsConst()) {
        val = static_cast<int64_t>(index->CastToConstant()->GetIntValue());
        parent_index = nullptr;
    }
    auto max_val = check_upper ? val : std::numeric_limits<int64_t>::min();
    auto min_val = check_lower ? val : std::numeric_limits<int64_t>::max();
    place->emplace(parent_index, std::make_tuple(insts, max_val, min_val));
}

void ChecksElimination::PushNewBoundsCheck(Loop *loop, Inst *len_array, Inst *index, Inst *inst, bool check_upper,
                                           bool check_lower)
{
    ASSERT(loop != nullptr && len_array != nullptr && index != nullptr && inst != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    if (bounds_checks_.find(loop) == bounds_checks_.end()) {
        auto it1 = bounds_checks_.emplace(loop, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it1.second);
        auto it2 = it1.first->second.emplace(len_array, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it2.second);
        InitItemForNewIndex(&it2.first->second, index, inst, check_upper, check_lower);
    } else if (bounds_checks_.at(loop).find(len_array) == bounds_checks_.at(loop).end()) {
        auto it1 = bounds_checks_.at(loop).emplace(len_array, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it1.second);
        InitItemForNewIndex(&it1.first->second, index, inst, check_upper, check_lower);
    } else if (auto &len_a_bc = bounds_checks_.at(loop).at(len_array); len_a_bc.find(index) == len_a_bc.end()) {
        auto parent_index = index;
        int64_t val {};
        if (IsInstIncOrDec(index)) {
            parent_index = index->GetInput(0).GetInst();
            val = GetInc(index);
        } else if (index->IsConst()) {
            parent_index = nullptr;
            val = static_cast<int64_t>(index->CastToConstant()->GetIntValue());
        }
        if (parent_index == index || len_a_bc.find(parent_index) == len_a_bc.end()) {
            InitItemForNewIndex(&len_a_bc, index, inst, check_upper, check_lower);
        } else {
            auto &item = len_a_bc.at(parent_index);
            std::get<0>(item).push_back(inst);
            if (val > std::get<1>(item) && check_upper) {
                std::get<1>(item) = val;
            } else if (val < std::get<2U>(item) && check_lower) {
                std::get<2U>(item) = val;
            }
        }
    } else {
        auto &item = bounds_checks_.at(loop).at(len_array).at(index);
        std::get<0>(item).push_back(inst);
        if (std::get<1>(item) < 0 && check_upper) {
            std::get<1>(item) = 0;
        }
        if (std::get<2U>(item) > 0 && check_lower) {
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

template <Opcode OPC, bool CHECK_FULL_DOM, typename CheckInputs>
void ChecksElimination::TryRemoveDominatedChecks(Inst *inst, CheckInputs check_inputs)
{
    for (auto &user : inst->GetInput(0).GetInst()->GetUsers()) {
        auto user_inst = user.GetInst();
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (user_inst->GetOpcode() == OPC && user_inst != inst && user_inst->GetType() == inst->GetType() &&
            check_inputs(user_inst) &&
            (CHECK_FULL_DOM ? inst->IsDominate(user_inst) : inst->InSameBlockOrDominate(user_inst))) {
            ASSERT(inst->IsDominate(user_inst));
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                // NOLINTNEXTLINE(readability-magic-numbers)
                << GetOpcodeString(OPC) << " with id = " << inst->GetId() << " dominate on " << GetOpcodeString(OPC)
                << " with id = " << user_inst->GetId();
            ReplaceUsersAndRemoveCheck(user_inst, inst);
        }
    }
}

// Remove consecutive checks: NullCheck -> NullCheck -> NullCheck
template <Opcode OPC>
void ChecksElimination::TryRemoveConsecutiveChecks(Inst *inst)
{
    auto end = inst->GetUsers().end();
    for (auto user = inst->GetUsers().begin(); user != end;) {
        auto user_inst = (*user).GetInst();
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (user_inst->GetOpcode() == OPC) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Remove consecutive " << GetOpcodeString(OPC);
            ReplaceUsersAndRemoveCheck(user_inst, inst);
            // Start iteration from beginning, because the new successors may be added.
            user = inst->GetUsers().begin();
            end = inst->GetUsers().end();
        } else {
            ++user;
        }
    }
}

template <Opcode OPC>
bool ChecksElimination::TryRemoveCheckByBounds(Inst *inst, Inst *input)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(OPC == Opcode::ZeroCheck || OPC == Opcode::NegativeCheck || OPC == Opcode::NotPositiveCheck ||
                  OPC == Opcode::NullCheck);
    ASSERT(inst->GetOpcode() == OPC);
    if (input->GetType() == DataType::UINT64) {
        return false;
    }

    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(block, input);
    bool result = false;
    // NOLINTNEXTLINE(readability-magic-numbers, readability-braces-around-statements, bugprone-branch-clone)
    if constexpr (OPC == Opcode::ZeroCheck) {
        result = range.IsLess(BoundsRange(0)) || range.IsMore(BoundsRange(0));
    } else if constexpr (OPC == Opcode::NullCheck) {  // NOLINT
        result = range.IsMore(BoundsRange(0));
    } else if constexpr (OPC == Opcode::NegativeCheck) {  // NOLINT
        result = range.IsNotNegative();
    } else if constexpr (OPC == Opcode::NotPositiveCheck) {  // NOLINT
        result = range.IsPositive();
    }
    if (result) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << GetOpcodeString(OPC) << " have correct bounds";
        ReplaceUsersAndRemoveCheck(inst, input);
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers, readability-braces-around-statements)
        if constexpr (OPC == Opcode::ZeroCheck || OPC == Opcode::NullCheck) {
            result = range.IsEqual(BoundsRange(0));
        } else if constexpr (OPC == Opcode::NegativeCheck) {  // NOLINT
            result = range.IsNegative();
        } else if constexpr (OPC == Opcode::NotPositiveCheck) {  // NOLINT
            result = range.IsNotPositive();
        }
        if (result) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                // NOLINTNEXTLINE(readability-magic-numbers)
                << GetOpcodeString(OPC) << " must throw, saved for replace by unconditional deoptimize";
            PushNewCheckMustThrow(inst);
        }
    }
    return result;
}

template <Opcode OPC, bool CHECK_FULL_DOM>
bool ChecksElimination::TryRemoveCheck(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(OPC == Opcode::ZeroCheck || OPC == Opcode::NegativeCheck || OPC == Opcode::NotPositiveCheck ||
                  OPC == Opcode::NullCheck);
    ASSERT(inst->GetOpcode() == OPC);

    // NOLINTNEXTLINE(readability-magic-numbers)
    TryRemoveDominatedChecks<OPC, CHECK_FULL_DOM>(inst);
    // NOLINTNEXTLINE(readability-magic-numbers)
    TryRemoveConsecutiveChecks<OPC>(inst);

    auto input = inst->GetInput(0).GetInst();
    // NOLINTNEXTLINE(readability-magic-numbers)
    return TryRemoveCheckByBounds<OPC>(inst, input);
}

template <Opcode OPC>
void ChecksElimination::TryOptimizeOverflowCheck(Inst *inst)
{
    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(block, inst);
    bool can_overflow = true;
    if constexpr (OPC == Opcode::NegOverflowAndZeroCheck) {
        can_overflow = range.CanOverflowNeg(DataType::INT32);
    } else {
        can_overflow = range.CanOverflow(DataType::INT32);
    }
    if (!can_overflow) {
        block->RemoveOverflowCheck(inst);
        SetApplied();
        return;
    }
    bool const_inputs = true;
    for (size_t i = 0; i < inst->GetInputsCount() - 1; ++i) {
        const_inputs &= inst->GetInput(i).GetInst()->IsConst();
    }
    if (const_inputs) {
        // replace by deopt
        PushNewCheckMustThrow(inst);
        return;
    }
    PushNewCheckForMoveOutOfLoop(inst);
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
        auto next = block->GetDominator();
        // The case when the dominant block is the head of a inner loop
        if (next != nullptr && next->GetLoop()->GetOuterLoop() == block->GetLoop()) {
            return nullptr;
        }
        block = next;
    }
    return nullptr;
}

Inst *ChecksElimination::FindOptimalSaveStateForHoist(Inst *inst, Inst **optimal_insert_after)
{
    ASSERT(inst->RequireState());
    auto block = inst->GetBasicBlock();
    if (block == nullptr) {
        return nullptr;
    }
    auto loop = block->GetLoop();
    *optimal_insert_after = nullptr;

    while (!loop->IsRoot() && !loop->GetHeader()->IsOsrEntry() && !loop->IsIrreducible()) {
        for (auto back_edge : loop->GetBackEdges()) {
            if (!block->IsDominate(back_edge)) {
                // avoid taking checks out of slowpath
                return *optimal_insert_after;
            }
        }
        // Find save state
        Inst *ss = FindSaveState(loop);
        if (ss == nullptr) {
            return *optimal_insert_after;
        }
        auto insert_after = ss;

        // Check that inputs are dominate on ss
        bool inputs_are_dominate = true;
        for (size_t i = 0; i < inst->GetInputsCount() - 1; ++i) {
            auto input = inst->GetInput(i).GetInst();
            if (!input->IsDominate(insert_after)) {
                if (insert_after->GetBasicBlock() == input->GetBasicBlock()) {
                    insert_after = input;
                } else {
                    inputs_are_dominate = false;
                    break;
                }
            }
        }

        if (!inputs_are_dominate) {
            return *optimal_insert_after;
        }
        *optimal_insert_after = insert_after;
        if (insert_after != ss) {
            // some inputs are dominate on insert_after but not dominate on ss, stop here
            // the only case when return value is not equal to *optimal_insert_after
            return ss;
        }
        block = loop->GetHeader();  // block will be used to check for hot path
        loop = loop->GetOuterLoop();
    }
    return *optimal_insert_after;
}

void ChecksElimination::InsertInstAfter(Inst *inst, Inst *after, BasicBlock *block)
{
    if (after->IsPhi()) {
        block->PrependInst(inst);
    } else {
        block->InsertAfter(inst, after);
    }
}

void ChecksElimination::InsertBoundsCheckDeoptimization(ConditionCode cc, Inst *left, int64_t val, Inst *right,
                                                        Inst *ss, Inst *insert_after, Opcode new_left_opcode)
{
    auto block = insert_after->GetBasicBlock();
    Inst *new_left = nullptr;
    if (val == 0) {
        new_left = left;
    } else if (left == nullptr) {
        ASSERT(new_left_opcode == Opcode::Add);
        new_left = GetGraph()->FindOrCreateConstant(val);
    } else {
        auto cnst = GetGraph()->FindOrCreateConstant(val);
        new_left = GetGraph()->CreateInst(new_left_opcode);
        new_left->SetType(DataType::INT32);
        new_left->SetInput(0, left);
        new_left->SetInput(1, cnst);
        if (new_left->RequireState()) {
            new_left->SetSaveState(ss);
        }
        InsertInstAfter(new_left, insert_after, block);
        insert_after = new_left;
    }
    auto deopt_comp = GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, new_left, right, DataType::INT32, cc);
    auto deopt = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, ss->GetPc(), deopt_comp, ss,
                                                    DeoptimizeType::BOUNDS_CHECK);
    InsertInstAfter(deopt_comp, insert_after, block);
    block->InsertAfter(deopt, deopt_comp);
}

Inst *ChecksElimination::InsertDeoptimization(ConditionCode cc, Inst *left, Inst *right, Inst *ss, Inst *insert_after,
                                              DeoptimizeType deopt_type)
{
    auto deopt_comp = GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, left, right, left->GetType(), cc);
    auto deopt = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, ss->GetPc(), deopt_comp, ss, deopt_type);
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
        bool is_head_loop_exit = loop_info_value.if_imm->GetBasicBlock() == loop->GetHeader();
        bool has_pre_header_compare = CountableLoopParser::HasPreHeaderCompare(loop, loop_info_value);
        ASSERT(loop_info_value.index->GetOpcode() == Opcode::Phi);
        if (loop_info_value.is_inc) {
            return std::make_tuple(loop_info_value, ss, loop_info_value.init, loop_info_value.test,
                                   loop_info_value.normalized_cc == CC_LE ? CC_LE : CC_LT, is_head_loop_exit,
                                   has_pre_header_compare);
        }
        return std::make_tuple(loop_info_value, ss, loop_info_value.test, loop_info_value.init, CC_LE,
                               is_head_loop_exit, has_pre_header_compare);
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Not countable loop isn't supported";
    return std::nullopt;
}

Inst *ChecksElimination::InsertNewLenArray(Inst *len_array, Inst *ss)
{
    if (len_array->IsDominate(ss)) {
        return len_array;
    }
    if (len_array->GetOpcode() == Opcode::LenArray) {
        auto null_check = len_array->GetInput(0).GetInst();
        auto ref = len_array->GetDataFlowInput(null_check);
        if (ref->IsDominate(ss)) {
            // Build nullcheck + lenarray before loop
            auto nullcheck = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, ss->GetPc(), ref, ss);
            nullcheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
            auto new_len_array = len_array->Clone(GetGraph());
            new_len_array->SetInput(0, nullcheck);
            auto block = ss->GetBasicBlock();
            block->InsertAfter(new_len_array, ss);
            block->InsertAfter(nullcheck, ss);
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Builded new NullCheck(id=" << nullcheck->GetId()
                                             << ") and LenArray(id=" << new_len_array->GetId() << ") before loop";
            ChecksElimination::VisitNullCheck<true>(this, nullcheck);
            return new_len_array;
        }
    }
    return nullptr;
}

void ChecksElimination::InsertDeoptimizationForIndexOverflow(CountableLoopInfo *countable_loop_info,
                                                             BoundsRange index_upper_range, Inst *ss)
{
    auto loop_cc = countable_loop_info->normalized_cc;
    if (loop_cc == CC_LT || loop_cc == CC_LE) {
        auto loop_upper = countable_loop_info->test;
        auto step = countable_loop_info->const_step;
        auto index_type = countable_loop_info->index->GetType();
        ASSERT(index_type == DataType::INT32);
        auto max_upper = BoundsRange::GetMax(index_type) - step + (loop_cc == CC_LT ? 1 : 0);
        auto bri = loop_upper->GetBasicBlock()->GetGraph()->GetBoundsRangeInfo();
        auto loop_upper_range = bri->FindBoundsRange(countable_loop_info->index->GetBasicBlock(), loop_upper);
        // Upper bound of loop index assuming (index + max_add < len_array)
        index_upper_range = index_upper_range.Add(BoundsRange(step)).FitInType(index_type);
        if (!BoundsRange(max_upper).IsMoreOrEqual(loop_upper_range) && index_upper_range.IsMaxRange(index_type)) {
            // loop index can overflow
            Inst *insert_after = loop_upper->IsDominate(ss) ? ss : loop_upper;
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for loop index overflow";
            // Create deoptimize if loop index can become negative
            InsertBoundsCheckDeoptimization(ConditionCode::CC_LT, nullptr, max_upper, loop_upper, ss, insert_after);
        }
    }
}

bool ChecksElimination::NeedUpperDeoptimization(BasicBlock *header, Inst *len_array, BoundsRange len_array_range,
                                                Inst *upper, BoundsRange upper_range, int64_t max_add, ConditionCode cc,
                                                bool *insert_new_len_array)
{
    if (max_add == std::numeric_limits<int64_t>::min()) {
        return false;
    }
    if (upper_range.Add(BoundsRange(max_add)).IsLess(len_array_range)) {
        return false;
    }
    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto new_upper = upper;
    auto new_max_add = max_add;
    int64_t upper_add = 0;
    if (IsInstIncOrDec(upper)) {
        upper_add = GetInc(upper);
        new_max_add += upper_add;
        new_upper = upper->GetInput(0).GetInst();
    }
    if (len_array == new_upper) {
        if (new_max_add < 0 || (new_max_add == 0 && cc == CC_LT)) {
            return false;
        }
    }
    auto use_upper_len = upper_add >= 0 || !upper_range.IsMaxRange(upper->GetType());
    if (use_upper_len && new_max_add <= 0) {
        auto new_upper_range = bri->FindBoundsRange(header, new_upper);
        if (new_upper_range.GetLenArray() == len_array) {
            return false;
        }
    }
    *insert_new_len_array = new_upper != len_array;
    return true;
}

bool ChecksElimination::TryInsertDeoptimizationForLargeStep(ConditionCode cc, Inst *result_len_array, Inst *lower,
                                                            Inst *upper, int64_t max_add, Inst *insert_deopt_after,
                                                            Inst *ss, uint64_t const_step)
{
    auto block = insert_deopt_after->GetBasicBlock();
    if (!lower->IsDominate(insert_deopt_after)) {
        if (lower->GetBasicBlock() == block) {
            insert_deopt_after = lower;
        } else {
            return false;
        }
    }
    auto sub_value = lower;
    if (cc == CC_LT) {
        sub_value = GetGraph()->CreateInstAdd(DataType::INT32, INVALID_PC, lower, GetGraph()->FindOrCreateConstant(1));
        InsertInstAfter(sub_value, insert_deopt_after, block);
        insert_deopt_after = sub_value;
    }
    auto sub = GetGraph()->CreateInstSub(DataType::INT32, INVALID_PC, upper, sub_value);
    InsertInstAfter(sub, insert_deopt_after, block);
    auto mod =
        GetGraph()->CreateInstMod(DataType::INT32, INVALID_PC, sub, GetGraph()->FindOrCreateConstant(const_step));
    block->InsertAfter(mod, sub);
    if (result_len_array == upper) {
        auto max_add_const = GetGraph()->FindOrCreateConstant(max_add);
        // (upper - lower [- 1]) % step </<= max_add
        InsertBoundsCheckDeoptimization(cc, mod, 0, max_add_const, ss, mod, Opcode::NOP);
    } else {
        // result_len_array - max_add </<= upper - (upper - lower [- 1]) % step
        auto max_index_value = GetGraph()->CreateInstSub(DataType::INT32, INVALID_PC, upper, mod);
        block->InsertAfter(max_index_value, mod);
        auto opcode = max_add > 0 ? Opcode::Sub : Opcode::SubOverflowCheck;
        InsertBoundsCheckDeoptimization(cc, result_len_array, max_add, max_index_value, ss, max_index_value, opcode);
    }
    return true;
}

bool ChecksElimination::TryInsertDeoptimization(LoopInfo loop_info, Inst *len_array, int64_t max_add, int64_t min_add,
                                                bool has_check_in_header)
{
    auto [countable_loop_info, ss, lower, upper, cc, is_head_loop_exit, has_pre_header_compare] = loop_info;
    ASSERT(cc == CC_LT || cc == CC_LE);
    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto header = countable_loop_info.index->GetBasicBlock();
    auto upper_range = bri->FindBoundsRange(header, upper);
    auto lower_range = bri->FindBoundsRange(header, lower);
    auto len_array_range = bri->FindBoundsRange(header, len_array);
    auto has_check_before_exit = has_check_in_header || !is_head_loop_exit;
    if (!has_pre_header_compare && !lower_range.IsLess(upper_range) && has_check_before_exit) {
        // if lower > upper, removing BoundsCheck may be wrong for the first iteration
        return false;
    }
    uint64_t lower_inc = (countable_loop_info.normalized_cc == CC_GT ? 1 : 0);
    bool need_lower_deopt = (min_add != std::numeric_limits<int64_t>::max()) &&
                            !lower_range.Add(BoundsRange(min_add)).Add(BoundsRange(lower_inc)).IsNotNegative();
    bool insert_lower_deopt = lower->IsDominate(ss);
    if (need_lower_deopt && !insert_lower_deopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for lower value";
        return false;
    }

    bool insert_new_len_array;
    if (NeedUpperDeoptimization(header, len_array, len_array_range, upper, upper_range, max_add, cc,
                                &insert_new_len_array)) {
        auto result_len_array = insert_new_len_array ? InsertNewLenArray(len_array, ss) : len_array;
        if (result_len_array == nullptr) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for upper value";
            return false;
        }
        auto insert_deopt_after = len_array != result_len_array ? result_len_array : ss;
        if (!upper->IsDominate(insert_deopt_after)) {
            insert_deopt_after = upper;
        }
        ASSERT(insert_deopt_after->GetBasicBlock()->IsDominate(header));
        if (insert_deopt_after->GetBasicBlock() == header) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for upper value";
            return false;
        }
        auto const_step = countable_loop_info.const_step;
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Try to build deoptimize for upper value";
        if (const_step == 1 ||
            (countable_loop_info.normalized_cc == CC_GT || countable_loop_info.normalized_cc == CC_GE)) {
            auto opcode = max_add > 0 ? Opcode::Sub : Opcode::SubOverflowCheck;
            // Create deoptimize if result_len_array - max_add <=(<) upper
            // result_len_array is >= 0, so if max_add > 0, overflow is not possible
            // that's why we do not add max_add to upper instead
            InsertBoundsCheckDeoptimization(cc, result_len_array, max_add, upper, ss, insert_deopt_after, opcode);
        } else if (lower_range.IsConst() && lower_range.GetLeft() == 0 && countable_loop_info.normalized_cc == CC_LT &&
                   result_len_array == upper && max_add == static_cast<int64_t>(const_step) - 1) {
            // for (int i = 0; i < len; i += x) process(a[i], ..., a[i + x - 1])
            // deoptimize if len % x != 0
            auto zero_const = GetGraph()->FindOrCreateConstant(0);
            InsertBoundsCheckDeoptimization(ConditionCode::CC_NE, result_len_array, const_step, zero_const, ss,
                                            insert_deopt_after, Opcode::Mod);
        } else if (!TryInsertDeoptimizationForLargeStep(cc, result_len_array, lower, upper, max_add, insert_deopt_after,
                                                        ss, const_step)) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for upper value with step > 1";
            return false;
        }
    }
    InsertDeoptimizationForIndexOverflow(&countable_loop_info, len_array_range.Sub(BoundsRange(max_add)), ss);
    if (need_lower_deopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for lower value";
        // Create deoptimize if lower < 0 (or -1 for loop with CC_GT)
        auto lower_const = GetGraph()->FindOrCreateConstant(-lower_inc);
        InsertBoundsCheckDeoptimization(ConditionCode::CC_LT, lower, min_add, lower_const, ss, ss);
    }
    return true;
}

void ChecksElimination::HoistLoopInvariantBoundsChecks(Inst *len_array, GroupedBoundsChecks *index_boundschecks,
                                                       Loop *loop)
{
    auto len_arr_loop = len_array->GetBasicBlock()->GetLoop();
    // len_array isn't loop invariant
    if (len_arr_loop == loop) {
        return;
    }
    for (auto &[index, bound_checks_info] : *index_boundschecks) {
        // Check that index is loop invariant, if index is nullptr it means that it was a constant
        if (index != nullptr && index->GetBasicBlock()->GetLoop() == loop) {
            continue;
        }
        for (auto bounds_check : std::get<0>(bound_checks_info)) {
            PushNewCheckForMoveOutOfLoop(bounds_check);
        }
    }
}

void ChecksElimination::ProcessingGroupBoundsCheck(GroupedBoundsChecks *index_boundschecks, LoopInfo loop_info,
                                                   Inst *len_array)
{
    auto phi_index = std::get<0>(loop_info).index;
    if (index_boundschecks->find(phi_index) == index_boundschecks->end()) {
        HoistLoopInvariantBoundsChecks(len_array, index_boundschecks, phi_index->GetBasicBlock()->GetLoop());
        return;
    }
    auto &[insts_to_delete, max_add, min_add] = index_boundschecks->at(phi_index);
    ASSERT(!insts_to_delete.empty());
    bool has_check_in_header = false;
    for (const auto &inst : insts_to_delete) {
        if (inst->GetBasicBlock() == phi_index->GetBasicBlock()) {
            has_check_in_header = true;
        }
    }
    if (TryInsertDeoptimization(loop_info, len_array, max_add, min_add, has_check_in_header)) {
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

void ChecksElimination::MoveCheckOutOfLoop()
{
    for (auto inst : checks_for_move_out_of_loop_) {
        Inst *insert_after = nullptr;
        auto ss = FindOptimalSaveStateForHoist(inst, &insert_after);
        if (ss == nullptr) {
            continue;
        }
        ASSERT(insert_after != nullptr);
        ASSERT(ss->GetBasicBlock() == insert_after->GetBasicBlock());
        auto block = inst->GetBasicBlock();
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Move check " << GetOpcodeString(inst->GetOpcode())
                                         << " with id = " << inst->GetId() << " from bb " << block->GetId() << " to bb "
                                         << ss->GetBasicBlock()->GetId();
        block->EraseInst(inst);
        ss->GetBasicBlock()->InsertAfter(inst, insert_after);
        inst->SetSaveState(ss);
        inst->SetPc(ss->GetPc());
        inst->SetFlag(inst_flags::CAN_DEOPTIMIZE);
        SetApplied();
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
                if (max != std::numeric_limits<int64_t>::min()) {
                    // Create deoptimize if max_index >= len_array
                    InsertBoundsCheckDeoptimization(ConditionCode::CC_GE, index, max, len_array, save_state,
                                                    insert_after);
                }
                if (index != nullptr && min != std::numeric_limits<int64_t>::max()) {
                    // Create deoptimize if min_index < 0
                    auto zero_const = GetGraph()->FindOrCreateConstant(0);
                    InsertBoundsCheckDeoptimization(ConditionCode::CC_LT, index, min, zero_const, save_state,
                                                    insert_after);
                } else {
                    // No lower check needed based on BoundsAnalysis
                    // if index is null, group of bounds checks consists of constants
                    ASSERT(min >= 0);
                }
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
            SetLoopDeleted();
        }
    }
}

}  // namespace panda::compiler
