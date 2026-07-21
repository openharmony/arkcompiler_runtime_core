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

#include "string_flat_check_elimination.h"

#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/optimizations/string_flat_check.h"

namespace ark::compiler {

bool StringFlatCheckElimination::RunImpl()
{
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : block->AllInsts()) {
            if (inst->Is(Opcode::StringFlatCheck)) {
                VisitStringFlatCheck(inst);
            }
        }
    }
    return IsApplied();
}

bool StringFlatCheckElimination::IsEnable() const
{
    // The pass makes no sense if no StringFlatCheck instructions have been inserted by the StringFlatCheck pass.
    return StringFlatCheck::IsEnable(GetGraph()->GetRuntime());
}

void StringFlatCheckElimination::VisitStringFlatCheck(Inst *inst)
{
    ASSERT(inst->RequireState() && inst->GetInputsCount() == 2U);

    // We must not eliminate StringFlatCheck instructions when users require flat strings. Unfortunately, we cannot
    // memoize whether a phi or an intrinsic requires a flat string because an intrinsic can have different requirements
    // for strings used by different inputs. There is no prior known correspondence between the input number in a phi
    // and the input number in the intrinsic that uses this phi, so every use edge must be tested.
    auto visitedHolder = MarkerHolder(GetGraph());
    visited_ = visitedHolder.GetMarker();
    if (UsersRequireFlatString(inst)) {
        return;
    }

    inst->ReplaceUsers(inst->GetInput(0).GetInst());
    auto *block = inst->GetBasicBlock();
    block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
    inst->RemoveInputs();

    SetApplied();
    COMPILER_LOG(DEBUG, CLEANUP) << "Not needed anymore StringFlatCheck removed " << inst->GetId();
    GetGraph()->GetEventWriter().EventCleanup(inst->GetId(), inst->GetPc());
}

bool StringFlatCheckElimination::UsersRequireFlatString(Inst *inst) const
{
    return std::any_of(inst->GetUsers().begin(), inst->GetUsers().end(),
                       [this](User &user) { return UserRequiresFlatString(user); });
}

bool StringFlatCheckElimination::UserRequiresFlatString(User &user) const
{
    auto *inst = user.GetInst();
    // Requiredness is per (intrinsic, operand index): must be tested for every use edge, so it must NOT be gated by
    // the visited marker.
    if (inst->Is(Opcode::Intrinsic)) {
        return StringFlatCheck::IntrinsicRequiresFlatCheck(inst->CastToIntrinsic(), user.GetIndex());
    }

    // A check (NullCheck, RefTypeCheck, ...) is transparent with respect to the reference it guards: its result is the
    // same string object as its input, so any flat-string requirement imposed by the check's users applies to our value
    // too.
    if (inst->IsCheck()) {
        return UsersRequireFlatString(inst);
    }

    // Only phis are marked: they are the sole source of recursion, and a phi's requiredness is the same regardless of
    // which incoming edge we arrived on. Re-reaching a marked phi returns false - for a diamond it was already fully
    // explored (proven no flat-required user); for a loop back-edge it is still being explored by the outer call, so
    // skipping it just breaks the cycle without losing any requiredness that outer call won't already find.
    if (inst->Is(Opcode::Phi)) {
        if (inst->SetMarker(visited_)) {
            return false;
        }
        return UsersRequireFlatString(inst);
    }

    return false;
}
}  // namespace ark::compiler
