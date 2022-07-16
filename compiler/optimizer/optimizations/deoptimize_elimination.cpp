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
#include "optimizer/analysis/dominators_tree.h"
#include "deoptimize_elimination.h"

namespace panda::compiler {

bool DeoptimizeElimination::RunImpl()
{
    uint64_t insts_number = VisitGraphAndCount();

    ReplaceDeoptimizeIfByUnconditionalDeoptimize();

    if (!HaveCalls() && insts_number <= options.GetCompilerSafepointEliminationLimit()) {
        RemoveSafePoints();
    }

    return IsApplied();
}

void DeoptimizeElimination::ReplaceDeoptimizeIfByUnconditionalDeoptimize()
{
    for (auto &inst : deoptimize_must_throw_) {
        auto block = inst->GetBasicBlock();
        if (block != nullptr) {
            block->ReplaceInstByDeoptimize(inst);
            SetApplied();
        }
    }
}

void DeoptimizeElimination::RemoveSafePoints()
{
    auto block = GetGraph()->GetStartBlock();
    ASSERT(block != nullptr && block->IsStartBlock());
    for (auto sp : block->Insts()) {
        if (sp->GetOpcode() == Opcode::SafePoint) {
            sp->ClearFlag(inst_flags::NO_DCE);
            SetApplied();
            COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "SafePoint " << sp->GetId() << " is deleted from start block";
            block->GetGraph()->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(sp->GetOpcode()),
                                                                           sp->GetId(), sp->GetPc());
        }
    }
}

bool DeoptimizeElimination::RequireRegMap(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst->RequireRegMap()) {
            return true;
        }
        if (user_inst->GetOpcode() == Opcode::CallStatic || user_inst->GetOpcode() == Opcode::CallVirtual) {
            // Inlined method can contain Deoptimize or DeoptimizeIf
            if (static_cast<CallInst *>(user_inst)->IsInlined()) {
                return true;
            }
        }
    }
    return false;
}

void DeoptimizeElimination::VisitDefault(Inst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    for (auto &user : inst->GetUsers()) {
        auto user_inst = user.GetInst();
        if (!user_inst->IsSaveState()) {
            return;
        }
        if (user_inst->GetOpcode() == Opcode::SafePoint) {
            continue;
        }
        if (RequireRegMap(user_inst)) {
            return;
        }
    }

    inst->RemoveUsers<true>();

    SetApplied();
    COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "All users the instructions " << inst->GetId() << " are SaveStates";
    inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(inst->GetOpcode()),
                                                                                   inst->GetId(), inst->GetPc());
}

void DeoptimizeElimination::VisitSaveState(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<DeoptimizeElimination *>(v);
    if (visitor->TryToRemoveRedundantSaveState(inst)) {
        return;
    }

    if (visitor->RequireRegMap(inst)) {
        return;
    }

    auto ss = inst->CastToSaveState();
    if (ss->RemoveNumericInputs()) {
        visitor->SetApplied();
        COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "SaveState " << ss->GetId() << " numeric inputs were deleted";
        ss->GetBasicBlock()->GetGraph()->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(ss->GetOpcode()),
                                                                                     ss->GetId(), ss->GetPc());
#ifndef NDEBUG
        ss->SetInputsWereDeleted();
#endif
    }
}

void DeoptimizeElimination::VisitSaveStateDeoptimize(GraphVisitor *v, Inst *inst)
{
    static_cast<DeoptimizeElimination *>(v)->TryToRemoveRedundantSaveState(inst);
}

void DeoptimizeElimination::VisitDeoptimizeIf(GraphVisitor *v, Inst *inst)
{
    auto input = inst->GetInput(0).GetInst();
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto visitor = static_cast<DeoptimizeElimination *>(v);
    if (input->IsConst()) {
        if (input->CastToConstant()->GetIntValue() == 0) {
            visitor->RemoveDeoptimizeIf(inst);
        } else {
            visitor->PushNewDeoptimizeIf(inst);
        }
    } else if (input->GetOpcode() == Opcode::IsMustDeoptimize) {
        if (visitor->CanRemoveGuard(input)) {
            visitor->RemoveGuard(input);
        }
    } else {
        for (auto &user : input->GetUsers()) {
            auto user_inst = user.GetInst();
            if (user_inst != inst && user_inst->GetOpcode() == Opcode::DeoptimizeIf &&
                !(graph->IsOsrMode() && block->GetLoop() != user_inst->GetBasicBlock()->GetLoop()) &&
                inst->InSameBlockOrDominate(user_inst)) {
                ASSERT(inst->IsDominate(user_inst));
                visitor->RemoveDeoptimizeIf(user_inst);
            }
        }
    }
}

bool DeoptimizeElimination::TryToRemoveRedundantSaveState(Inst *inst)
{
    if (inst->GetUsers().Empty()) {
        auto block = inst->GetBasicBlock();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        inst->RemoveInputs();
        SetApplied();
        COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "SaveState " << inst->GetId() << " without users is deleted";
        block->GetGraph()->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(inst->GetOpcode()),
                                                                       inst->GetId(), inst->GetPc());
        return true;
    }
    return false;
}

bool DeoptimizeElimination::CanRemoveGuard(Inst *guard)
{
    auto guard_block = guard->GetBasicBlock();
    auto it = InstSafeIterator<IterationType::INST, IterationDirection::BACKWARD>(*guard_block, guard);
    for (++it; it != guard_block->InstsSafeReverse().end(); ++it) {
        auto inst = *it;
        if (inst->IsRuntimeCall()) {
            return false;
        }
        if (inst->GetOpcode() == Opcode::IsMustDeoptimize) {
            return true;
        }
    }
    auto mrk = guard_block->GetGraph()->NewMarker();
    auto remove_mrk = guard_block->GetGraph()->NewMarker();

    /*
     * Run search recursively from current block to start block.
     * We can remove guard, if guard is met in all ways and there should be no call instructions between current
     * guard and found guards.
     */
    bool can_remove = true;
    for (auto succ_block : guard_block->GetPredsBlocks()) {
        can_remove &= CanRemoveGuardRec(succ_block, guard, mrk, remove_mrk);
        if (!can_remove) {
            break;
        }
    }
    guard_block->GetGraph()->EraseMarker(mrk);
    guard_block->GetGraph()->EraseMarker(remove_mrk);
    return can_remove;
}

bool DeoptimizeElimination::CanRemoveGuardRec(BasicBlock *block, Inst *guard, const Marker &mrk,
                                              const Marker &remove_mrk)
{
    if (block->IsStartBlock()) {
        return false;
    }
    auto block_type = GetBlockType(block);
    if (block->SetMarker(mrk)) {
        return block->IsMarked(remove_mrk);
    }
    if (block_type == BlockType::INVALID) {
        for (auto inst : block->InstsSafeReverse()) {
            if (inst->IsRuntimeCall()) {
                PushNewBlockType(block, BlockType::RUNTIME_CALL);
                return false;
            }
            if (inst->GetOpcode() == Opcode::IsMustDeoptimize) {
                [[maybe_unused]] auto result = block->SetMarker(remove_mrk);
                ASSERT(!result);
                PushNewBlockType(block, BlockType::GUARD);
                return true;
            }
        }
        PushNewBlockType(block, BlockType::NOTHING);
    } else if (block_type != BlockType::NOTHING) {
        if (block_type == BlockType::GUARD) {
            [[maybe_unused]] auto result = block->SetMarker(remove_mrk);
            ASSERT(!result);
            return true;
        }
        return false;
    }
    for (const auto &succ_block : block->GetPredsBlocks()) {
        if (!CanRemoveGuardRec(succ_block, guard, mrk, remove_mrk)) {
            return false;
        }
    }
    [[maybe_unused]] auto result = block->SetMarker(remove_mrk);
    ASSERT(!result);
    return true;
}

void DeoptimizeElimination::RemoveGuard(Inst *guard)
{
    ASSERT(guard->GetOpcode() == Opcode::IsMustDeoptimize);
    ASSERT(guard->HasSingleUser());

    auto deopt = guard->GetNext();
    ASSERT(deopt->GetOpcode() == Opcode::DeoptimizeIf);
    auto block = guard->GetBasicBlock();
    auto graph = block->GetGraph();
    guard->RemoveInputs();
    block->ReplaceInst(guard, graph->CreateInstNOP());

    COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "Dublicated Guard " << guard->GetId() << " is deleted";
    graph->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(guard->GetOpcode()), guard->GetId(),
                                                       guard->GetPc());
    RemoveDeoptimizeIf(deopt);
}

void DeoptimizeElimination::RemoveDeoptimizeIf(Inst *inst)
{
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto savestate = inst->GetInput(1).GetInst();

    inst->RemoveInputs();
    block->ReplaceInst(inst, graph->CreateInstNOP());

    COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "Dublicated or redundant DeoptimizeIf " << inst->GetId() << " is deleted";
    graph->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(inst->GetOpcode()), inst->GetId(),
                                                       inst->GetPc());

    if (savestate->GetUsers().Empty()) {
        savestate->GetBasicBlock()->ReplaceInst(savestate, graph->CreateInstNOP());
        savestate->RemoveInputs();

        COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "SaveState " << savestate->GetId() << " without users is deleted";
        graph->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(savestate->GetOpcode()), savestate->GetId(),
                                                           savestate->GetPc());
    }
    SetApplied();
}

void DeoptimizeElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}
}  // namespace panda::compiler
