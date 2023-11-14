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
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/memory_coalescing.h"

namespace panda::compiler {
/**
 * Basic analysis for variables used in loops. It works as follows:
 * 1) Identify variables that are derived from another variables and their difference (AddI, SubI supported).
 * 2) Based on previous step reveal loop variables and their iteration increment if possible.
 */
class VariableAnalysis {
public:
    struct BaseVariable {
        int64_t initial;
        int64_t step;
    };
    struct DerivedVariable {
        Inst *base;
        int64_t diff;
    };

    explicit VariableAnalysis(Graph *graph)
        : base_(graph->GetLocalAllocator()->Adapter()), derived_(graph->GetLocalAllocator()->Adapter())
    {
        for (auto block : graph->GetBlocksRPO()) {
            for (auto inst : block->AllInsts()) {
                if (GetCommonType(inst->GetType()) == DataType::INT64) {
                    AddUsers(inst);
                }
            }
        }
        for (auto loop : graph->GetRootLoop()->GetInnerLoops()) {
            if (loop->IsIrreducible()) {
                continue;
            }
            auto header = loop->GetHeader();
            for (auto phi : header->PhiInsts()) {
                constexpr auto INPUTS_COUNT = 2;
                if (phi->GetInputsCount() != INPUTS_COUNT || GetCommonType(phi->GetType()) != DataType::INT64) {
                    continue;
                }
                auto var = phi->CastToPhi();
                Inst *initial = var->GetPhiInput(var->GetPhiInputBb(0));
                Inst *update = var->GetPhiInput(var->GetPhiInputBb(1));
                if (var->GetPhiInputBb(0) != loop->GetPreHeader()) {
                    std::swap(initial, update);
                }

                if (!initial->IsConst()) {
                    continue;
                }

                if (derived_.find(update) != derived_.end()) {
                    auto init_val = static_cast<int64_t>(initial->CastToConstant()->GetIntValue());
                    base_[var] = {init_val, derived_[update].diff};
                }
            }
        }

        COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Evolution variables:";
        for (auto entry : base_) {
            COMPILER_LOG(DEBUG, MEMORY_COALESCING)
                << "v" << entry.first->GetId() << " = {" << entry.second.initial << ", " << entry.second.step << "}";
        }
        COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Loop variables:";
        for (auto entry : derived_) {
            COMPILER_LOG(DEBUG, MEMORY_COALESCING)
                << "v" << entry.first->GetId() << " = v" << entry.second.base->GetId() << " + " << entry.second.diff;
        }
    }

    DEFAULT_MOVE_SEMANTIC(VariableAnalysis);
    DEFAULT_COPY_SEMANTIC(VariableAnalysis);
    ~VariableAnalysis() = default;

    bool IsAnalyzed(Inst *inst) const
    {
        return derived_.find(inst) != derived_.end();
    }

    Inst *GetBase(Inst *inst) const
    {
        return derived_.at(inst).base;
    }

    int64_t GetInitial(Inst *inst) const
    {
        auto var = derived_.at(inst);
        return base_.at(var.base).initial + var.diff;
    }

    int64_t GetDiff(Inst *inst) const
    {
        return derived_.at(inst).diff;
    }

    int64_t GetStep(Inst *inst) const
    {
        return base_.at(derived_.at(inst).base).step;
    }

    bool IsEvoluted(Inst *inst) const
    {
        return derived_.at(inst).base->IsPhi();
    }

    bool HasKnownEvolution(Inst *inst) const
    {
        Inst *base = derived_.at(inst).base;
        return base->IsPhi() && base_.find(base) != base_.end();
    }

private:
    /// Add derived variables if we can deduce the change from INST
    void AddUsers(Inst *inst)
    {
        auto acc = 0;
        auto base = inst;
        if (derived_.find(inst) != derived_.end()) {
            acc += derived_[inst].diff;
            base = derived_[inst].base;
        } else {
            derived_[inst] = {inst, 0};
        }
        for (auto &user : inst->GetUsers()) {
            auto uinst = user.GetInst();
            ASSERT(uinst->IsPhi() || derived_.find(uinst) == derived_.end());
            switch (uinst->GetOpcode()) {
                case Opcode::AddI: {
                    auto val = static_cast<int64_t>(uinst->CastToAddI()->GetImm());
                    derived_[uinst] = {base, acc + val};
                    break;
                }
                case Opcode::SubI: {
                    auto val = static_cast<int64_t>(uinst->CastToSubI()->GetImm());
                    derived_[uinst] = {base, acc - val};
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    ArenaUnorderedMap<Inst *, struct BaseVariable> base_;
    ArenaUnorderedMap<Inst *, struct DerivedVariable> derived_;
};

/**
 * The visitor collects pairs of memory instructions that can be coalesced.
 * It operates in scope of basic block. During observation of instructions we
 * collect memory instructions in one common queue of candidates that can be merged.
 *
 * Candidate is marked as invalid in the following conditions:
 * - it has been paired already
 * - it is a store and SaveState has been met
 * - a BARRIER or CAN_TROW instruction has been met
 *
 * To pair valid array accesses:
 * - check that accesses happen on the consecutive indices of the same array
 * - find the lowest position the dominator access can be sunk
 * - find the highest position the dominatee access can be hoisted
 * - if highest position dominates lowest position the coalescing is possible
 */
class PairCreatorVisitor : public GraphVisitor {
public:
    explicit PairCreatorVisitor(Graph *graph, const AliasAnalysis &aliases, const VariableAnalysis &vars, Marker mrk,
                                bool aligned)
        : aligned_only_(aligned),
          mrk_invalid_(mrk),
          graph_(graph),
          aliases_(aliases),
          vars_(vars),
          pairs_(graph->GetLocalAllocator()->Adapter()),
          candidates_(graph->GetLocalAllocator()->Adapter())
    {
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return graph_->GetBlocksRPO();
    }

    NO_MOVE_SEMANTIC(PairCreatorVisitor);
    NO_COPY_SEMANTIC(PairCreatorVisitor);
    ~PairCreatorVisitor() override = default;

    static void VisitLoadArray(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccess(inst);
    }

    static void VisitStoreArray(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccess(inst);
    }

    static void VisitLoadArrayI(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccessI(inst);
    }

    static void VisitStoreArrayI(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccessI(inst);
    }

    static bool IsNotAcceptableForStore(Inst *inst)
    {
        if (inst->GetOpcode() == Opcode::SaveState) {
            for (auto &user : inst->GetUsers()) {
                auto *ui = user.GetInst();
                if (ui->CanThrow() || ui->CanDeoptimize()) {
                    return true;
                }
            }
        }
        return inst->GetOpcode() == Opcode::SaveStateDeoptimize;
    }

    void VisitDefault(Inst *inst) override
    {
        if (inst->IsMemory()) {
            candidates_.push_back(inst);
            return;
        }
        if (inst->IsSaveState()) {
            // 1. Load & Store can be moved through SafePoint
            if (inst->GetOpcode() == Opcode::SafePoint) {
                return;
            }
            // 2. Load & Store can't be moved through SaveStateOsr
            if (inst->GetOpcode() == Opcode::SaveStateOsr) {
                candidates_.clear();
                return;
            }
            // 3. Load can be moved through SaveState and SaveStateDeoptimize
            // 4. Store can't be moved through SaveStateDeoptimize and SaveState with Users that are IsCheck or
            //    CanDeoptimize. It is checked in IsNotAcceptableForStore
            if (IsNotAcceptableForStore(inst)) {
                InvalidateStores();
                return;
            }
        }
        if (inst->IsBarrier()) {
            candidates_.clear();
            return;
        }
        if (inst->CanThrow()) {
            InvalidateStores();
            return;
        }
    }

    void Reset()
    {
        candidates_.clear();
    }

    ArenaUnorderedMap<Inst *, MemoryCoalescing::CoalescedPair> &GetPairs()
    {
        return pairs_;
    }

#include "optimizer/ir/visitor.inc"
private:
    void InvalidateStores()
    {
        for (auto cand : candidates_) {
            if (cand->IsStore()) {
                cand->SetMarker(mrk_invalid_);
            }
        }
    }

    static bool IsPairInst(Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::LoadArrayPair:
            case Opcode::LoadArrayPairI:
            case Opcode::StoreArrayPair:
            case Opcode::StoreArrayPairI:
                return true;
            default:
                return false;
        }
    }

    /**
     * Return the highest instructions that INST can be inserted after (in scope of basic block).
     * Consider aliased memory accesses and volatile operations. CHECK_CFG enables the check of INST inputs
     * as well.
     */
    Inst *FindUpperInsertAfter(Inst *inst, Inst *bound, bool check_cfg)
    {
        ASSERT(bound != nullptr);
        auto upper_after = bound;
        // We do not move higher than bound
        auto lower_input = upper_after;
        if (check_cfg) {
            // Update upper bound according to def-use chains
            for (auto &input_item : inst->GetInputs()) {
                auto input = input_item.GetInst();
                if (input->GetBasicBlock() == inst->GetBasicBlock() && lower_input->IsPrecedingInSameBlock(input)) {
                    ASSERT(input->IsPrecedingInSameBlock(inst));
                    lower_input = input;
                }
            }
            upper_after = lower_input;
        }

        auto bound_it = std::find(candidates_.rbegin(), candidates_.rend(), bound);
        ASSERT(bound_it != candidates_.rend());
        for (auto it = candidates_.rbegin(); it != bound_it; it++) {
            auto cand = *it;
            if (check_cfg && cand->IsPrecedingInSameBlock(lower_input)) {
                return lower_input;
            }
            // Can't hoist load over aliased store and store over aliased memory instructions
            if (inst->IsStore() || cand->IsStore()) {
                auto check_inst = cand;
                if (IsPairInst(cand)) {
                    // We have already checked the second inst. We now want to check the first one
                    // for alias.
                    auto pair = pairs_[cand];
                    check_inst = pair.first->IsPrecedingInSameBlock(pair.second) ? pair.first : pair.second;
                }
                if (aliases_.CheckInstAlias(inst, check_inst) != NO_ALIAS) {
                    return cand;
                }
            }
            // Can't hoist over volatile load
            if (cand->IsLoad() && IsVolatileMemInst(cand)) {
                return cand;
            }
        }
        return upper_after;
    }

    /**
     * Return the lowest instructions that INST can be inserted after (in scope of basic block).
     * Consider aliased memory accesses and volatile operations. CHECK_CFG enables the check of INST users
     * as well.
     */
    Inst *FindLowerInsertAfter(Inst *inst, Inst *bound, bool check_cfg = true)
    {
        ASSERT(bound != nullptr);
        auto lower_after = bound->GetPrev();
        // We do not move lower than bound
        auto upper_user = lower_after;
        ASSERT(upper_user != nullptr);
        if (check_cfg) {
            // Update lower bound according to def-use chains
            for (auto &user_item : inst->GetUsers()) {
                auto user = user_item.GetInst();
                if (!user->IsPhi() && user->GetBasicBlock() == inst->GetBasicBlock() &&
                    user->IsPrecedingInSameBlock(upper_user)) {
                    ASSERT(inst->IsPrecedingInSameBlock(user));
                    upper_user = user->GetPrev();
                    ASSERT(upper_user != nullptr);
                }
            }
            lower_after = upper_user;
        }

        auto inst_it = std::find(candidates_.begin(), candidates_.end(), inst);
        ASSERT(inst_it != candidates_.end());
        for (auto it = inst_it + 1; it != candidates_.end(); it++) {
            auto cand = *it;
            if (check_cfg && upper_user->IsPrecedingInSameBlock(cand)) {
                return upper_user;
            }
            // Can't lower load over aliased store and store over aliased memory instructions
            if (inst->IsStore() || cand->IsStore()) {
                auto check_inst = cand;
                if (IsPairInst(cand)) {
                    // We have already checked the first inst. We now want to check the second one
                    // for alias.
                    auto pair = pairs_[cand];
                    check_inst = pair.first->IsPrecedingInSameBlock(pair.second) ? pair.second : pair.first;
                }
                if (aliases_.CheckInstAlias(inst, check_inst) != NO_ALIAS) {
                    ASSERT(cand->GetPrev() != nullptr);
                    return cand->GetPrev();
                }
            }
            // Can't lower over volatile store
            if (cand->IsStore() && IsVolatileMemInst(cand)) {
                ASSERT(cand->GetPrev() != nullptr);
                return cand->GetPrev();
            }
        }
        return lower_after;
    }

    /// Add a pair if a difference between indices equals to one. The first in pair is with lower index.
    bool TryAddCoalescedPair(Inst *inst, int64_t inst_idx, Inst *cand, int64_t cand_idx)
    {
        Inst *first = nullptr;
        Inst *second = nullptr;
        Inst *insert_after = nullptr;
        if (inst_idx == cand_idx - 1) {
            first = inst;
            second = cand;
        } else if (cand_idx == inst_idx - 1) {
            first = cand;
            second = inst;
        } else {
            return false;
        }

        ASSERT(inst->IsMemory() && cand->IsMemory());
        ASSERT(inst->GetOpcode() == cand->GetOpcode());
        ASSERT(inst != cand && cand->IsPrecedingInSameBlock(inst));
        Inst *cand_lower_after = nullptr;
        Inst *inst_upper_after = nullptr;
        if (first->IsLoad()) {
            // Consider dominance of load users
            bool check_cfg = true;
            cand_lower_after = FindLowerInsertAfter(cand, inst, check_cfg);
            // Do not need index if v0[v1] preceeds v0[v1 + 1] because v1 + 1 is not used in paired load.
            check_cfg = second->IsPrecedingInSameBlock(first);
            inst_upper_after = FindUpperInsertAfter(inst, cand, check_cfg);
        } else if (first->IsStore()) {
            // Store instructions do not have users. Don't check them
            bool check_cfg = false;
            cand_lower_after = FindLowerInsertAfter(cand, inst, check_cfg);
            // Should check that stored value is ready
            check_cfg = true;
            inst_upper_after = FindUpperInsertAfter(inst, cand, check_cfg);
        } else {
            UNREACHABLE();
        }

        // No intersection in reordering ranges
        if (!inst_upper_after->IsPrecedingInSameBlock(cand_lower_after)) {
            return false;
        }
        if (cand->IsPrecedingInSameBlock(inst_upper_after)) {
            insert_after = inst_upper_after;
        } else {
            insert_after = cand;
        }

        first->SetMarker(mrk_invalid_);
        second->SetMarker(mrk_invalid_);
        InsertPair(first, second, insert_after);
        return true;
    }

    void HandleArrayAccessI(Inst *inst)
    {
        Inst *obj = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
        uint64_t idx = GetInstImm(inst);
        if (!MemoryCoalescing::AcceptedType(inst->GetType())) {
            candidates_.push_back(inst);
            return;
        }
        /* Last candidates more likely to be coalesced */
        for (auto iter = candidates_.rbegin(); iter != candidates_.rend(); iter++) {
            auto cand = *iter;
            /* Skip not interesting candidates */
            if (cand->IsMarked(mrk_invalid_) || cand->GetOpcode() != inst->GetOpcode()) {
                continue;
            }

            Inst *cand_obj = cand->GetDataFlowInput(cand->GetInput(0).GetInst());
            /* Array objects must alias each other */
            if (aliases_.CheckRefAlias(obj, cand_obj) != MUST_ALIAS) {
                continue;
            }
            /* The difference between indices should be equal to one */
            uint64_t cand_idx = GetInstImm(cand);
            /* To keep alignment the lowest index should be even */
            if (aligned_only_ && ((idx < cand_idx && (idx & 1U) != 0) || (cand_idx < idx && (cand_idx & 1U) != 0))) {
                continue;
            }
            if (TryAddCoalescedPair(inst, idx, cand, cand_idx)) {
                break;
            }
        }

        candidates_.push_back(inst);
    }

    void HandleArrayAccess(Inst *inst)
    {
        Inst *obj = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
        Inst *idx = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
        if (!vars_.IsAnalyzed(idx) || !MemoryCoalescing::AcceptedType(inst->GetType())) {
            candidates_.push_back(inst);
            return;
        }
        /* Last candidates more likely to be coalesced */
        for (auto iter = candidates_.rbegin(); iter != candidates_.rend(); iter++) {
            auto cand = *iter;
            /* Skip not interesting candidates */
            if (cand->IsMarked(mrk_invalid_) || cand->GetOpcode() != inst->GetOpcode()) {
                continue;
            }

            Inst *cand_obj = cand->GetDataFlowInput(cand->GetInput(0).GetInst());
            auto cand_idx = cand->GetDataFlowInput(cand->GetInput(1).GetInst());
            /* We need to have info about candidate's index and array objects must alias each other */
            if (!vars_.IsAnalyzed(cand_idx) || aliases_.CheckRefAlias(obj, cand_obj) != MUST_ALIAS) {
                continue;
            }
            if (vars_.HasKnownEvolution(idx) && vars_.HasKnownEvolution(cand_idx)) {
                /* Accesses inside loop */
                auto idx_step = vars_.GetStep(idx);
                auto idx_initial = vars_.GetInitial(idx);
                auto cand_step = vars_.GetStep(cand_idx);
                auto cand_initial = vars_.GetInitial(cand_idx);
                /* Indices should be incremented at the same value and their
                   increment should be even to hold alignment */
                if (idx_step != cand_step) {
                    continue;
                }
                /* To keep alignment we need to have even step and even lowest initial */
                constexpr auto IMM_2 = 2;
                if (aligned_only_ && idx_step % IMM_2 != 0 &&
                    ((idx_initial < cand_initial && idx_initial % IMM_2 != 0) ||
                     (cand_initial < idx_initial && cand_initial % IMM_2 != 0))) {
                    continue;
                }
                if (TryAddCoalescedPair(inst, idx_initial, cand, cand_initial)) {
                    break;
                }
            } else if (!aligned_only_ && !vars_.HasKnownEvolution(idx) && !vars_.HasKnownEvolution(cand_idx)) {
                /* Accesses outside loop */
                if (vars_.GetBase(idx) != vars_.GetBase(cand_idx)) {
                    continue;
                }
                if (TryAddCoalescedPair(inst, vars_.GetDiff(idx), cand, vars_.GetDiff(cand_idx))) {
                    break;
                }
            }
        }

        candidates_.push_back(inst);
    }

    void InsertPair(Inst *first, Inst *second, Inst *insert_after)
    {
        COMPILER_LOG(DEBUG, MEMORY_COALESCING)
            << "Access that may be coalesced: v" << first->GetId() << " v" << second->GetId();

        ASSERT(first->GetType() == second->GetType());
        Inst *paired = nullptr;
        switch (first->GetOpcode()) {
            case Opcode::LoadArray:
                paired = ReplaceLoadArray(first, second, insert_after);
                break;
            case Opcode::LoadArrayI:
                paired = ReplaceLoadArrayI(first, second, insert_after);
                break;
            case Opcode::StoreArray:
                paired = ReplaceStoreArray(first, second, insert_after);
                break;
            case Opcode::StoreArrayI:
                paired = ReplaceStoreArrayI(first, second, insert_after);
                break;
            default:
                UNREACHABLE();
        }

        COMPILER_LOG(DEBUG, MEMORY_COALESCING)
            << "Coalescing of {v" << first->GetId() << " v" << second->GetId() << "} is successful";
        graph_->GetEventWriter().EventMemoryCoalescing(first->GetId(), first->GetPc(), second->GetId(), second->GetPc(),
                                                       paired->GetId(), paired->IsStore() ? "Store" : "Load");

        pairs_[paired] = {first, second};
        paired->SetMarker(mrk_invalid_);
        candidates_.insert(std::find_if(candidates_.rbegin(), candidates_.rend(),
                                        [paired](auto x) { return x->IsPrecedingInSameBlock(paired); })
                               .base(),
                           paired);
    }

    Inst *ReplaceLoadArray(Inst *first, Inst *second, Inst *insert_after)
    {
        ASSERT(first->GetOpcode() == Opcode::LoadArray);
        ASSERT(second->GetOpcode() == Opcode::LoadArray);

        auto pload = graph_->CreateInstLoadArrayPair(first->GetType(), INVALID_PC, first->GetInput(0).GetInst(),
                                                     first->GetInput(1).GetInst());
        pload->CastToLoadArrayPair()->SetNeedBarrier(first->CastToLoadArray()->GetNeedBarrier() ||
                                                     second->CastToLoadArray()->GetNeedBarrier());
        insert_after->InsertAfter(pload);
        if (first->CanThrow() || second->CanThrow()) {
            pload->SetFlag(compiler::inst_flags::CAN_THROW);
        }

        return pload;
    }

    Inst *ReplaceLoadArrayI(Inst *first, Inst *second, Inst *insert_after)
    {
        ASSERT(first->GetOpcode() == Opcode::LoadArrayI);
        ASSERT(second->GetOpcode() == Opcode::LoadArrayI);

        auto pload = graph_->CreateInstLoadArrayPairI(first->GetType(), INVALID_PC, first->GetInput(0).GetInst(),
                                                      first->CastToLoadArrayI()->GetImm());
        pload->CastToLoadArrayPairI()->SetNeedBarrier(first->CastToLoadArrayI()->GetNeedBarrier() ||
                                                      second->CastToLoadArrayI()->GetNeedBarrier());
        insert_after->InsertAfter(pload);
        if (first->CanThrow() || second->CanThrow()) {
            pload->SetFlag(compiler::inst_flags::CAN_THROW);
        }

        return pload;
    }

    Inst *ReplaceStoreArray(Inst *first, Inst *second, Inst *insert_after)
    {
        ASSERT(first->GetOpcode() == Opcode::StoreArray);
        ASSERT(second->GetOpcode() == Opcode::StoreArray);

        auto pstore = graph_->CreateInstStoreArrayPair(
            first->GetType(), INVALID_PC, first->GetInput(0).GetInst(), first->CastToStoreArray()->GetIndex(),
            first->CastToStoreArray()->GetStoredValue(), second->CastToStoreArray()->GetStoredValue());
        pstore->CastToStoreArrayPair()->SetNeedBarrier(first->CastToStoreArray()->GetNeedBarrier() ||
                                                       second->CastToStoreArray()->GetNeedBarrier());
        insert_after->InsertAfter(pstore);
        if (first->CanThrow() || second->CanThrow()) {
            pstore->SetFlag(compiler::inst_flags::CAN_THROW);
        }

        return pstore;
    }

    Inst *ReplaceStoreArrayI(Inst *first, Inst *second, Inst *insert_after)
    {
        ASSERT(first->GetOpcode() == Opcode::StoreArrayI);
        ASSERT(second->GetOpcode() == Opcode::StoreArrayI);

        auto pstore = graph_->CreateInstStoreArrayPairI(
            first->GetType(), INVALID_PC, first->GetInput(0).GetInst(), first->CastToStoreArrayI()->GetStoredValue(),
            second->CastToStoreArrayI()->GetStoredValue(), first->CastToStoreArrayI()->GetImm());
        pstore->CastToStoreArrayPairI()->SetNeedBarrier(first->CastToStoreArrayI()->GetNeedBarrier() ||
                                                        second->CastToStoreArrayI()->GetNeedBarrier());
        insert_after->InsertAfter(pstore);
        if (first->CanThrow() || second->CanThrow()) {
            pstore->SetFlag(compiler::inst_flags::CAN_THROW);
        }

        return pstore;
    }

    uint64_t GetInstImm(Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::LoadArrayI:
                return inst->CastToLoadArrayI()->GetImm();
            case Opcode::StoreArrayI:
                return inst->CastToStoreArrayI()->GetImm();
            default:
                UNREACHABLE();
        }
    }

private:
    bool aligned_only_;
    Marker mrk_invalid_;
    Graph *graph_ {nullptr};
    const AliasAnalysis &aliases_;
    const VariableAnalysis &vars_;
    ArenaUnorderedMap<Inst *, MemoryCoalescing::CoalescedPair> pairs_;
    InstVector candidates_;
};

static void ReplaceLoadByPair(Inst *load, Inst *paired_load, int32_t dst_idx)
{
    auto graph = paired_load->GetBasicBlock()->GetGraph();
    auto pair_getter = graph->CreateInstLoadPairPart(load->GetType(), INVALID_PC, paired_load, dst_idx);
    load->ReplaceUsers(pair_getter);
    paired_load->InsertAfter(pair_getter);
}

/**
 * This optimization coalesces two loads (stores) that read (write) values from (to) the consecutive memory into
 * a single operation.
 *
 * 1) If we have two memory instruction that can be coalesced then we are trying to find a position for
 *    coalesced operation. If it is possible, the memory operations are coalesced and skipped otherwise.
 * 2) The instruction of Aarch64 requires memory address alignment. For arrays
 *    it means we can coalesce only accesses that starts from even index.
 * 3) The implemented coalescing for arrays supposes there is no volatile array element accesses.
 */
bool MemoryCoalescing::RunImpl()
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        COMPILER_LOG(INFO, MEMORY_COALESCING) << "Skipping Memory Coalescing for unsupported architecture";
        return false;
    }

    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<AliasAnalysis>();

    VariableAnalysis variables(GetGraph());
    auto &aliases = GetGraph()->GetValidAnalysis<AliasAnalysis>();
    Marker mrk = GetGraph()->NewMarker();
    PairCreatorVisitor collector(GetGraph(), aliases, variables, mrk, aligned_only_);
    for (auto block : GetGraph()->GetBlocksRPO()) {
        collector.VisitBlock(block);
        collector.Reset();
    }
    GetGraph()->EraseMarker(mrk);

    for (auto pair : collector.GetPairs()) {
        auto bb = pair.first->GetBasicBlock();
        if (pair.first->IsLoad()) {
            ReplaceLoadByPair(pair.second.second, pair.first, 1);
            ReplaceLoadByPair(pair.second.first, pair.first, 0);
        }
        bb->RemoveInst(pair.second.first);
        bb->RemoveInst(pair.second.second);
    };

    if (!collector.GetPairs().empty()) {
        SaveStateBridgesBuilder ssb;
        for (auto bb : GetGraph()->GetBlocksRPO()) {
            if (!bb->IsEmpty() && !bb->IsStartBlock()) {
                ssb.FixSaveStatesInBB(bb);
            }
        }
    }
    COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Memory Coalescing complete";
    return !collector.GetPairs().empty();
}
}  // namespace panda::compiler
