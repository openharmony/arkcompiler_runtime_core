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

#include "reg_alloc_resolver.h"
#include "reg_type.h"
#include "compiler/optimizer/code_generator/codegen.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace panda::compiler {
/*
 * For each instruction set destination register if it is assigned,
 * Pop inputs from stack and push result on stack if stack slot is assigned.
 */
void RegAllocResolver::Resolve()
{
    // We use RPO order because we need to calculate Caller's roots mask before its inlined callees
    // (see PropagateCallerMasks).
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInstsSafe()) {
            if (inst->IsSaveState()) {
                ResolveSaveState(inst);
                continue;
            }
            ResolveInputs(inst);
            ResolveOutput(inst);
            if (GetGraph()->IsInstThrowable(inst)) {
                AddCatchPhiMoves(inst);
            }
        }
    }
}

void RegAllocResolver::AddCatchPhiMoves(Inst *inst)
{
    auto spill_fill_inst = GetGraph()->CreateInstSpillFill();
    spill_fill_inst->SetSpillFillType(SpillFillType::INPUT_FILL);
    auto handlers = GetGraph()->GetThrowableInstHandlers(inst);

    for (auto catch_handler : handlers) {
        for (auto catch_inst : catch_handler->AllInsts()) {
            if (!catch_inst->IsCatchPhi() || catch_inst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            auto catch_phi = catch_inst->CastToCatchPhi();
            const auto &throwable_insts = catch_phi->GetThrowableInsts();
            auto it = std::find(throwable_insts->begin(), throwable_insts->end(), inst);
            if (it == throwable_insts->end()) {
                continue;
            }
            int index = std::distance(throwable_insts->begin(), it);
            auto catch_input = catch_phi->GetDataFlowInput(index);
            auto input_interval = liveness_->GetInstLifeIntervals(catch_input);
            ASSERT(input_interval->GetSibling() == nullptr);
            auto catch_phi_interval = liveness_->GetInstLifeIntervals(catch_phi);
            if (input_interval->GetLocation() != catch_phi_interval->GetLocation()) {
                ConnectIntervals(spill_fill_inst, input_interval, catch_phi_interval);
            }
        }
    }
    if (!spill_fill_inst->GetSpillFills().empty()) {
        inst->InsertBefore(spill_fill_inst);
    }
}

void RegAllocResolver::ResolveInputs(Inst *inst)
{
    if (inst->IsPhi() || inst->IsCatchPhi() || IsPseudoUserOfMultiOutput(inst)) {
        return;
    }

    // Life-position before instruction to analyze intervals, that were splited directly before it
    auto pre_ins_ln = liveness_->GetInstLifeIntervals(inst)->GetBegin() - 1U;

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        auto location = inst->GetLocation(i);
        auto input_interval = liveness_->GetInstLifeIntervals(inst->GetDataFlowInput(i));

        if (CanReadFromAccumulator(inst, i) || input_interval->NoDest() || location.IsInvalid()) {
            continue;
        }

        // Interval with fixed register can be splited before `inst`: we don't need any extra moves in that case,
        // since fixed register can't be overwrite
        auto sibling = input_interval->FindSiblingAt(pre_ins_ln);
        ASSERT(sibling != nullptr);
        if (location.IsFixedRegister() && sibling->GetLocation() == location) {
            continue;
        }

        // Otherwise use sibling covering `inst`
        if (sibling->GetEnd() == pre_ins_ln) {
            sibling = sibling->GetSibling();
        }

        // Input's location required any register: specify the allocated one
        if (location.IsUnallocatedRegister()) {
            ASSERT(sibling->HasReg());
            inst->SetLocation(i, sibling->GetLocation());
            continue;
        }

        // Finally, if input's location is not equal to the required one, add spill-fill
        if (sibling->GetLocation() != location) {
            AddMoveToFixedLocation(inst, sibling->GetLocation(), i);
        }
    }
}

void RegAllocResolver::AddMoveToFixedLocation(Inst *inst, Location input_location, size_t input_num)
{
    // Create or get existing SpillFillInst
    SpillFillInst *sf_inst {};
    if (inst->GetPrev() != nullptr && inst->GetPrev()->IsSpillFill()) {
        sf_inst = inst->GetPrev()->CastToSpillFill();
    } else {
        sf_inst = GetGraph()->CreateInstSpillFill();
        sf_inst->SetSpillFillType(SpillFillType::INPUT_FILL);
        inst->InsertBefore(sf_inst);
    }

    // Add move from input to fixed location
    auto type = ConvertRegType(GetGraph(), inst->GetInputType(input_num));
    auto fixed_location = inst->GetLocation(input_num);
    if (fixed_location.IsFixedRegister()) {
        GetGraph()->SetRegUsage(fixed_location.GetValue(), type);
    }
    sf_inst->AddSpillFill(input_location, fixed_location, type);
}

Inst *GetFirstUserOrInst(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::ReturnInlined) {
            return user.GetInst();
        }
    }
    return inst;
}

// For implicit null check we need to find the first null check's user to
// correctly capture SaveState's input locations, because implicit null checks are fired
// when its input is accessed by its users (for example, when LoadArray instruction is loading
// value from null array reference). Some life intervals may change its location (due to spilling)
// between NullCheck and its users, so locations captured at implicit null check could be incorrect.
// While implicit NullCheck may have multiple users we can use only a user dominating all other users,
// because null check either will be fired at it, or won't be fired at all.
Inst *GetExplicitUser(Inst *inst)
{
    if (!inst->IsNullCheck() || !inst->CastToNullCheck()->IsImplicit() || inst->GetUsers().Empty()) {
        return inst;
    }
    if (inst->HasSingleUser()) {
        return inst->GetUsers().Front().GetInst();
    }

    Inst *user_inst {nullptr};
    for (auto &user : inst->GetUsers()) {
        auto curr_inst = user.GetInst();
        if (!IsSuitableForImplicitNullCheck(curr_inst)) {
            continue;
        }
        if (curr_inst->GetInput(0).GetInst() != inst) {
            continue;
        }
        if (!curr_inst->CanThrow()) {
            continue;
        }
        user_inst = curr_inst;
        break;
    }
#ifndef NDEBUG
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->IsPhi()) {
            continue;
        }
        ASSERT(user_inst != nullptr && user_inst->IsDominate(user.GetInst()));
    }
#endif
    return user_inst;
}

void RegAllocResolver::PropagateCallerMasks(SaveStateInst *save_state)
{
    save_state->CreateRootsStackMask(GetGraph()->GetAllocator());
    auto user = GetExplicitUser(GetFirstUserOrInst(save_state));
    // Get location of save state inputs at the save state user (note that at this point
    // all inputs will have the same location at all users (excluding ReturnInlined that should be skipped)).
    FillSaveStateRootsMask(save_state, user, save_state);
    for (auto caller_inst = save_state->GetCallerInst(); caller_inst != nullptr;
         caller_inst = caller_inst->GetSaveState()->GetCallerInst()) {
        auto caller_ss = caller_inst->GetSaveState();
        FillSaveStateRootsMask(caller_ss, user, save_state);
    }
}

void RegAllocResolver::FillSaveStateRootsMask(SaveStateInst *save_state, Inst *user, SaveStateInst *target_ss)
{
    auto dst_ln = liveness_->GetInstLifeIntervals(user)->GetBegin();

    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        auto input_inst = save_state->GetDataFlowInput(i);
        if (input_inst->IsConst() || !IsTypeCollectable(input_inst->GetType())) {
            continue;
        }
        auto input_interval = liveness_->GetInstLifeIntervals(input_inst);
        auto sibling = input_interval->FindSiblingAt(dst_ln);
        ASSERT(sibling != nullptr);
        if (!sibling->SplitCover(dst_ln)) {
            continue;
        }
        AddLocationToRoots(sibling->GetLocation(), target_ss, GetGraph());
#ifndef NDEBUG
        for (auto &test_user : target_ss->GetUsers()) {
            if (test_user.GetInst()->GetOpcode() == Opcode::ReturnInlined ||
                test_user.GetInst()->GetId() == user->GetId()) {
                continue;
            }
            auto explicit_test_user = GetExplicitUser(test_user.GetInst());
            auto udst_ln = liveness_->GetInstLifeIntervals(explicit_test_user)->GetBegin();
            ASSERT(sibling->GetLocation() == input_interval->FindSiblingAt(udst_ln)->GetLocation());
        }
#endif
    }
}

namespace {
SaveStateInst *CopySaveState(Graph *graph, SaveStateInst *inst)
{
    auto copy = static_cast<SaveStateInst *>(inst->Clone(graph));
    ASSERT(copy->GetCallerInst() == inst->GetCallerInst());
    for (size_t input_idx = 0; input_idx < inst->GetInputsCount(); input_idx++) {
        copy->AppendInput(inst->GetInput(input_idx));
        copy->SetVirtualRegister(input_idx, inst->GetVirtualRegister(input_idx));
    }
    copy->SetLinearNumber(inst->GetLinearNumber());
    return copy;
}

bool HasSameLocation(LifeIntervals *interval, LifeNumber pos1, LifeNumber pos2)
{
    auto sibling1 = interval->FindSiblingAt(pos1);
    auto sibling2 = interval->FindSiblingAt(pos2);
    ASSERT(sibling1 != nullptr);
    ASSERT(sibling2 != nullptr);
    return sibling1->SplitCover(pos1) && sibling1->SplitCover(pos2) &&
           sibling1->GetLocation() == sibling2->GetLocation();
}

bool SaveStateCopyRequired(Inst *inst, User *curr_user, User *prev_user, const LivenessAnalyzer *la)
{
    ASSERT(inst->IsSaveState());
    auto curr_user_ln = la->GetInstLifeIntervals(GetExplicitUser(curr_user->GetInst()))->GetBegin();
    auto prev_user_ln = la->GetInstLifeIntervals(GetExplicitUser(prev_user->GetInst()))->GetBegin();
    bool need_copy = false;
    // If current save state is part of inlined method then we have to check location for all
    // parent save states.
    for (auto ss = static_cast<SaveStateInst *>(inst); ss != nullptr && !need_copy;) {
        for (size_t input_idx = 0; input_idx < ss->GetInputsCount() && !need_copy; input_idx++) {
            auto input_interval = la->GetInstLifeIntervals(ss->GetDataFlowInput(input_idx));
            need_copy = !HasSameLocation(input_interval, curr_user_ln, prev_user_ln);
        }
        auto caller = ss->GetCallerInst();
        if (caller == nullptr) {
            ss = nullptr;
        } else {
            ss = caller->GetSaveState();
        }
    }
    return need_copy;
}
}  // namespace

void RegAllocResolver::ResolveSaveState(Inst *inst)
{
    if (GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }
    ASSERT(inst->IsSaveState());

    bool handled_all_users = inst->HasSingleUser() || !inst->HasUsers();
    while (!handled_all_users) {
        size_t copy_users = 0;
        auto user_it = inst->GetUsers().begin();
        User *prev_user = &*user_it;
        ++user_it;
        bool need_copy = false;

        // Find first user having different location for some of the save state inputs and use SaveState's
        // copy for all preceding users.
        for (; user_it != inst->GetUsers().end() && !need_copy; ++user_it, copy_users++) {
            auto &curr_user = *user_it;
            // ReturnInline's SaveState is required only for SaveState's inputs life range propagation,
            // so it does not actually matter which interval will be actually used.
            if (prev_user->GetInst()->GetOpcode() == Opcode::ReturnInlined) {
                prev_user = &*user_it;
                continue;
            }
            if (curr_user.GetInst()->GetOpcode() == Opcode::ReturnInlined) {
                continue;
            }
            need_copy = SaveStateCopyRequired(inst, &curr_user, prev_user, liveness_);
            prev_user = &*user_it;
        }
        if (need_copy) {
            auto copy = CopySaveState(GetGraph(), static_cast<SaveStateInst *>(inst));
            // Replace original SaveState with the copy for first N users (N = `copy_users` ).
            while (copy_users > 0) {
                auto user_inst = inst->GetUsers().Front().GetInst();
                user_inst->ReplaceInput(inst, copy);
                copy_users--;
            }
            inst->GetBasicBlock()->InsertAfter(copy, inst);
            PropagateCallerMasks(copy);
            handled_all_users = inst->HasSingleUser();
        } else {
            handled_all_users = !(user_it != inst->GetUsers().end());
        }
    }
    // At this point inst either has single user or all its inputs have the same location at all users.
    PropagateCallerMasks(static_cast<SaveStateInst *>(inst));
}

/*
 * Pop output on stack from reserved register
 */
void RegAllocResolver::ResolveOutput(Inst *inst)
{
    // Don't process LiveOut, since it is instruction with pseudo destination
    if (inst->GetOpcode() == Opcode::LiveOut) {
        return;
    }
    // Multi-output instructions' dst registers will be filled after procecssing theirs pseudo users
    if (inst->GetLinearNumber() == INVALID_LINEAR_NUM || inst->GetDstCount() > 1) {
        return;
    }

    if (CanStoreToAccumulator(inst)) {
        return;
    }

    auto inst_interval = liveness_->GetInstLifeIntervals(inst);
    if (inst_interval->NoDest()) {
        inst->SetDstReg(INVALID_REG);
        return;
    }

    if (inst->GetOpcode() == Opcode::Parameter) {
        inst->CastToParameter()->GetLocationData().SetDst(inst_interval->GetLocation());
    }
    // Process multi-output inst
    size_t dst_mum = inst->GetSrcRegIndex();
    if (IsPseudoUserOfMultiOutput(inst)) {
        inst = inst->GetInput(0).GetInst();
    }
    // Wrtie dst
    auto reg_type = inst_interval->GetType();
    if (inst_interval->HasReg()) {
        auto reg = inst_interval->GetReg();
        inst->SetDstReg(dst_mum, reg);
        GetGraph()->SetRegUsage(reg, reg_type);
    } else {
        ASSERT(inst->IsConst() || inst->IsPhi() || inst->IsParameter());
    }
}

bool RegAllocResolver::ResolveCatchPhis()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (!block->IsCatchBegin()) {
            continue;
        }
        for (auto inst : block->AllInstsSafe()) {
            if (!inst->IsCatchPhi()) {
                break;
            }
            if (inst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            // This is the case when all throwable instructions were removed from the try-block,
            // so that catch-handler is unreachable
            if (inst->GetInputs().Empty()) {
                return false;
            }
            auto new_catch_phi = SqueezeCatchPhiInputs(inst->CastToCatchPhi());
            if (new_catch_phi != nullptr) {
                inst->ReplaceUsers(new_catch_phi);
                block->RemoveInst(inst);
            }
        }
    }
    return true;
}

/**
 * Try to remove catch phi's inputs:
 * If the input's corresponding throwable instruction dominates other throwable inst, we can remove other equal catch
 * phi's input
 *
 * CatchPhi(v1, v1, v1, v2, v2, v2) -> CatchPhi(v1, v2)
 *
 * Return nullptr if inputs count was not reduced.
 */
Inst *RegAllocResolver::SqueezeCatchPhiInputs(CatchPhiInst *catch_phi)
{
    bool inputs_are_identical = true;
    auto first_input = catch_phi->GetInput(0).GetInst();
    for (size_t i = 1; i < catch_phi->GetInputsCount(); ++i) {
        if (catch_phi->GetInput(i).GetInst() != first_input) {
            inputs_are_identical = false;
            break;
        }
    }
    if (inputs_are_identical) {
        return first_input;
    }

    // Create a new one and fill it with the necessary inputs
    auto new_catch_phi = GetGraph()->CreateInstCatchPhi(catch_phi->GetType(), catch_phi->GetPc());
    ASSERT(catch_phi->GetBasicBlock()->GetFirstInst()->IsCatchPhi());
    catch_phi->GetBasicBlock()->PrependInst(new_catch_phi);
    for (size_t i = 0; i < catch_phi->GetInputsCount(); i++) {
        auto input_inst = catch_phi->GetInput(i).GetInst();
        auto current_throwable_inst = catch_phi->GetThrowableInst(i);
        ASSERT(GetGraph()->IsInstThrowable(current_throwable_inst));
        bool skip = false;
        for (size_t j = 0; j < new_catch_phi->GetInputsCount(); j++) {
            auto saved_inst = new_catch_phi->GetInput(j).GetInst();
            if (saved_inst != input_inst) {
                continue;
            }
            auto saved_throwable_inst = new_catch_phi->GetThrowableInst(j);
            if (saved_throwable_inst->IsDominate(current_throwable_inst)) {
                skip = true;
            }
            if (current_throwable_inst->IsDominate(saved_throwable_inst)) {
                new_catch_phi->ReplaceThrowableInst(saved_throwable_inst, current_throwable_inst);
                skip = true;
            }
            if (skip) {
                break;
            }
        }
        if (!skip) {
            new_catch_phi->AppendInput(input_inst);
            new_catch_phi->AppendThrowableInst(current_throwable_inst);
        }
    }
    if (new_catch_phi->GetInputsCount() == catch_phi->GetInputsCount()) {
        new_catch_phi->GetBasicBlock()->RemoveInst(new_catch_phi);
        return nullptr;
    }
    return new_catch_phi;
}

}  // namespace panda::compiler
