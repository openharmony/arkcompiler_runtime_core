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
#include "compiler/optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/lse.h"

namespace panda::compiler {

static std::string LogInst(const Inst *inst)
{
    return "v" + std::to_string(inst->GetId()) + " (" + GetOpcodeString(inst->GetOpcode()) + ")";
}

static Lse::EquivClass GetEquivClass(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::LoadArrayI:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
        case Opcode::LoadArrayPair:
        case Opcode::LoadArrayPairI:
        case Opcode::StoreArrayPair:
        case Opcode::StoreArrayPairI:
        case Opcode::LoadConstArray:
        case Opcode::FillConstArray:
            return Lse::EquivClass::EQ_ARRAY;
        case Opcode::LoadStatic:
        case Opcode::StoreStatic:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::LoadResolvedObjectFieldStatic:
        case Opcode::StoreResolvedObjectFieldStatic:
            return Lse::EquivClass::EQ_STATIC;
        case Opcode::LoadString:
        case Opcode::LoadType:
        case Opcode::UnresolvedLoadType:
            return Lse::EquivClass::EQ_POOL;
        default:
            return Lse::EquivClass::EQ_OBJECT;
    }
}

class LseVisitor {
public:
    explicit LseVisitor(Graph *graph, Lse::HeapEqClasses *heaps)
        : aa_(graph->GetAnalysis<AliasAnalysis>()),
          heaps_(*heaps),
          eliminations_(graph->GetLocalAllocator()->Adapter()),
          shadowed_stores_(graph->GetLocalAllocator()->Adapter()),
          disabled_objects_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(LseVisitor);
    NO_COPY_SEMANTIC(LseVisitor);
    ~LseVisitor() = default;

    void MarkForElimination(Inst *inst, Inst *reason, const Lse::HeapValue *hvalue)
    {
        if (Lse::CanEliminateInstruction(inst)) {
            auto heap = *hvalue;
            if (eliminations_.find(heap.val) != eliminations_.end()) {
                heap = eliminations_[heap.val];
            }
            ASSERT(eliminations_.find(heap.val) == eliminations_.end());
            eliminations_[inst] = heap;
            COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is eliminated because of " << LogInst(reason);
        }
    }

    void VisitStore(Inst *inst, Inst *val)
    {
        if (val == nullptr) {
            /* Instruction has no stored value (e.g. FillConstArray) */
            return;
        }
        auto base_object = inst->GetDataFlowInput(0);
        for (auto disabled_object : disabled_objects_) {
            alias_calls_++;
            if (aa_.CheckRefAlias(base_object, disabled_object) == MUST_ALIAS) {
                return;
            }
        }
        auto hvalue = GetHeapValue(inst);
        /* Value can be eliminated already */
        auto alive = val;
        auto eliminated = eliminations_.find(val);
        if (eliminated != eliminations_.end()) {
            alive = eliminated->second.val;
        }
        /* If store was assigned to VAL then we can eliminate the second assignment */
        if (hvalue != nullptr && hvalue->val == alive) {
            MarkForElimination(inst, hvalue->origin, hvalue);
            return;
        }
        if (hvalue != nullptr && hvalue->origin->IsStore() && !hvalue->read) {
            if (hvalue->origin->GetBasicBlock() == inst->GetBasicBlock()) {
                const Lse::HeapValue heap = {inst, alive, false, false};
                MarkForElimination(hvalue->origin, inst, &heap);
            } else {
                shadowed_stores_[hvalue->origin].push_back(inst);
            }
        }
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " updated heap with v" << alive->GetId();

        /* Stores added to eliminations_ above aren't checked versus phis -> no double instruction elimination */
        UpdatePhis(inst);

        auto &block_heap = heaps_.at(GetEquivClass(inst)).first.at(inst->GetBasicBlock());
        uint32_t encounters = 0;
        /* Erase all aliased values, because they may be overwritten */
        for (auto heap_iter = block_heap.begin(), heap_last = block_heap.end(); heap_iter != heap_last;) {
            auto hinst = heap_iter->first;
            ASSERT(GetEquivClass(inst) == GetEquivClass(hinst));
            alias_calls_++;
            if (aa_.CheckInstAlias(inst, hinst) == NO_ALIAS) {
                // Keep track if it's the same object but with different offset
                heap_iter++;
                alias_calls_++;
                if (aa_.CheckRefAlias(base_object, hinst->GetDataFlowInput(0)) == MUST_ALIAS) {
                    encounters++;
                }
            } else {
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "\tDrop from heap { " << LogInst(hinst) << ", v" << heap_iter->second.val->GetId() << "}";
                heap_iter = block_heap.erase(heap_iter);
            }
        }

        // If we reached limit for this object, remove all its MUST_ALIASed instructions from heap
        // and disable this object for this BB
        if (encounters > Lse::LS_ACCESS_LIMIT) {
            for (auto heap_iter = block_heap.begin(), heap_last = block_heap.end(); heap_iter != heap_last;) {
                auto hinst = heap_iter->first;
                alias_calls_++;
                if (HasBaseObject(hinst) && aa_.CheckRefAlias(base_object, hinst->GetDataFlowInput(0)) == MUST_ALIAS) {
                    COMPILER_LOG(DEBUG, LSE_OPT)
                        << "\tDrop from heap { " << LogInst(hinst) << ", v" << heap_iter->second.val->GetId() << "}";
                    heap_iter = block_heap.erase(heap_iter);
                } else {
                    heap_iter++;
                }
            }
            disabled_objects_.push_back(base_object);
            return;
        }

        /* Set value of the inst to VAL */
        block_heap[inst] = {inst, alive, false, false};
    }

    void VisitLoad(Inst *inst)
    {
        if (HasBaseObject(inst)) {
            auto input = inst->GetDataFlowInput(0);
            for (auto disabled_object : disabled_objects_) {
                alias_calls_++;
                if (aa_.CheckRefAlias(input, disabled_object) == MUST_ALIAS) {
                    return;
                }
            }
        }
        /* If we have a heap value for this load instruction then we can eliminate it */
        auto hvalue = GetHeapValue(inst);
        if (hvalue != nullptr) {
            MarkForElimination(inst, hvalue->origin, hvalue);
            return;
        }
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " updated heap with v" << inst->GetId();

        /* Loads added to eliminations_ above are not checked versus phis -> no double instruction elimination */
        UpdatePhis(inst);

        /* Otherwise set the value of instruction to itself and update MUST_ALIASes */
        heaps_.at(GetEquivClass(inst)).first.at(inst->GetBasicBlock())[inst] = {inst, inst, true, false};
    }

    bool HasBaseObject(Inst *inst)
    {
        ASSERT(inst->IsMemory());
        if (inst->GetInputsCount() == 0 || inst->GetInput(0).GetInst()->IsSaveState() ||
            inst->GetDataFlowInput(0)->GetType() == DataType::POINTER) {
            return false;
        }
        ASSERT(inst->GetDataFlowInput(0)->IsReferenceOrAny());
        return true;
    }

    void VisitIntrinsic(Inst *inst, InstVector *invs)
    {
        switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
#include "intrinsics_lse_heap_inv_args.inl"
            default:
                return;
        }
        for (auto inv : *invs) {
            for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
                auto &block_heap = heaps_.at(eq_class).first.at(inv->GetBasicBlock());
                for (auto heap_iter = block_heap.begin(), heap_last = block_heap.end(); heap_iter != heap_last;) {
                    auto hinst = heap_iter->first;
                    alias_calls_++;

                    if (!HasBaseObject(hinst) || aa_.CheckRefAlias(inv, hinst->GetDataFlowInput(0)) == NO_ALIAS) {
                        heap_iter++;
                    } else {
                        COMPILER_LOG(DEBUG, LSE_OPT) << "\tDrop from heap { " << LogInst(hinst) << ", v"
                                                     << heap_iter->second.val->GetId() << "}";
                        heap_iter = block_heap.erase(heap_iter);
                    }
                }
            }
        }
        invs->clear();
    }

    /// Completely resets the accumulated state: heap and phi candidates.
    void InvalidateHeap(BasicBlock *block)
    {
        for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
            heaps_.at(eq_class).first.at(block).clear();
            auto loop = block->GetLoop();
            while (!loop->IsRoot()) {
                heaps_.at(eq_class).second.at(loop).clear();
                loop = loop->GetOuterLoop();
            }
        }
        disabled_objects_.clear();
    }

    /// Clears heap of local only values as we don't want them to affect analysis further
    void ClearLocalValuesFromHeap(BasicBlock *block)
    {
        for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
            auto &block_heap = heaps_.at(eq_class).first.at(block);

            auto heap_it = block_heap.begin();
            while (heap_it != block_heap.end()) {
                if (heap_it->second.local) {
                    heap_it = block_heap.erase(heap_it);
                } else {
                    heap_it++;
                }
            }
        }
    }

    /// Release objects and reset Alias Analysis count
    void ResetLimits()
    {
        disabled_objects_.clear();
        alias_calls_ = 0;
    }

    uint32_t GetAliasAnalysisCallCount()
    {
        return alias_calls_;
    }

    /// Marks all values currently on heap as potentially read
    void SetHeapAsRead(BasicBlock *block)
    {
        for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
            auto &bheap = heaps_.at(eq_class).first.at(block);
            for (auto it = bheap.begin(); it != bheap.end();) {
                it->second.read = true;
                it++;
            }
        }
    }

    ArenaUnorderedMap<Inst *, struct Lse::HeapValue> &GetEliminations()
    {
        return eliminations_;
    }

    bool ProcessBackedges(PhiInst *phi, Loop *loop, Inst *cand, InstVector *insts)
    {
        // Now find which values are alive for each backedge. Due to MUST_ALIAS requirement,
        // there should only be one
        auto &heap = heaps_.at(GetEquivClass(cand)).first;
        for (auto bb : loop->GetHeader()->GetPredsBlocks()) {
            if (bb == loop->GetPreHeader()) {
                phi->AppendInput(cand->IsStore() ? InstStoredValue(cand) : cand);
                continue;
            }
            auto &block_heap = heap.at(bb);
            Inst *alive = nullptr;
            for (auto &entry : block_heap) {
                if (std::find(insts->begin(), insts->end(), entry.first) != insts->end()) {
                    alive = entry.first;
                    break;
                }
            }
            if (alive == nullptr) {
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "Skipping phi candidate " << LogInst(cand) << ": no alive insts found for backedge block";
                return false;
            }
            // There are several cases when a MUST_ALIAS load is alive at backedge while stores
            // exist in the loop. The one we're interested here is inner loops.
            if (alive->IsLoad() && !loop->GetInnerLoops().empty()) {
                auto it = std::find(insts->rbegin(), insts->rend(), alive);
                // We've checked that no stores exist in inner loops earlier, which allows us to just check
                // the first store before the load
                while (it != insts->rend()) {
                    auto s_inst = *it;
                    if (s_inst->IsStore()) {
                        break;
                    }
                    it++;
                }
                if (it != insts->rend() && (*it)->IsDominate(alive)) {
                    auto val = heap.at((*it)->GetBasicBlock())[(*it)].val;
                    // Use the store instead of load
                    phi->AppendInput(val);
                    // And eliminate load
                    struct Lse::HeapValue hvalue = {*it, val, false, false};
                    MarkForElimination(alive, *it, &hvalue);
                    continue;
                }
            }
            phi->AppendInput(block_heap[alive].val);
        }
        return true;
    }

    void LoopDoElimination(Inst *cand, Loop *loop, PhiInst *phi, InstVector *insts)
    {
        auto replacement = phi != nullptr ? phi : cand;
        struct Lse::HeapValue hvalue = {
            replacement, replacement->IsStore() ? InstStoredValue(replacement) : replacement, false, false};
        for (auto inst : *insts) {
            // No need to check MUST_ALIAS again, but we need to check for double elim
            if (eliminations_.find(inst) != eliminations_.end()) {
                continue;
            }

            // Don't replace loads that are also phi inputs
            if (phi != nullptr &&
                std::find_if(phi->GetInputs().begin(), phi->GetInputs().end(),
                             [&inst](const Input &x) { return x.GetInst() == inst; }) != phi->GetInputs().end()) {
                continue;
            }
            if (inst->IsLoad()) {
                MarkForElimination(inst, replacement, &hvalue);
            }
        }
        // And fix savestates for loads
        if (phi != nullptr) {
            for (size_t i = 0; i < phi->GetInputsCount(); i++) {
                auto bb = loop->GetHeader()->GetPredecessor(i);
                auto phi_input = phi->GetInput(i).GetInst();
                if (bb == loop->GetPreHeader() && cand->IsLoad() && cand->IsMovableObject()) {
                    ssb_.SearchAndCreateMissingObjInSaveState(bb->GetGraph(), cand, phi);
                } else if (phi_input->IsMovableObject()) {
                    ssb_.SearchAndCreateMissingObjInSaveState(bb->GetGraph(), phi_input, bb->GetLastInst(), nullptr,
                                                              bb);
                }
            }
        } else if (hvalue.val->IsMovableObject()) {
            ASSERT(!loop->IsIrreducible());
            auto head = loop->GetHeader();
            // Add cand value to all SaveStates in this loop and inner loops
            ssb_.SearchAndCreateMissingObjInSaveState(head->GetGraph(), hvalue.val, head->GetLastInst(), nullptr, head);
        }
    }

    /// Add eliminations inside loops if there is no overwrites on backedges.
    void FinalizeLoops(Graph *graph)
    {
        InstVector insts_must_alias(graph->GetLocalAllocator()->Adapter());
        for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
            for (auto &[loop, phis] : heaps_.at(eq_class).second) {
                COMPILER_LOG(DEBUG, LSE_OPT) << "Finalizing loop #" << loop->GetId();
                for (auto &[cand, insts] : phis) {
                    if (insts.empty()) {
                        COMPILER_LOG(DEBUG, LSE_OPT) << "Skipping phi candidate " << LogInst(cand) << " (no users)";
                        continue;
                    }

                    insts_must_alias.clear();

                    COMPILER_LOG(DEBUG, LSE_OPT) << "Processing phi candidate: " << LogInst(cand);

                    bool has_stores = false;
                    bool has_loads = false;

                    bool valid = true;

                    for (auto inst : insts) {
                        // Skip eliminated instructions
                        if (eliminations_.find(inst) != eliminations_.end()) {
                            continue;
                        }
                        auto alias_result = aa_.CheckInstAlias(cand, inst);
                        if (alias_result == MAY_ALIAS) {
                            // Ignore MAY_ALIAS loads, they won't interfere with our analysis
                            if (inst->IsLoad()) {
                                continue;
                            }
                            // If we have a MAY_ALIAS store we can't be sure about our values
                            // in phi creation
                            ASSERT(inst->IsStore());
                            COMPILER_LOG(DEBUG, LSE_OPT)
                                << "Skipping phi candidate " << LogInst(cand) << ": MAY_ALIAS by " << LogInst(inst);
                            valid = false;
                            break;
                        }
                        ASSERT(alias_result == MUST_ALIAS);

                        if (inst->IsStore() && inst->GetBasicBlock()->GetLoop() != loop) {
                            // We can handle if loads are in inner loop, but if a store is in inner loop
                            // then we can't replace anything
                            COMPILER_LOG(DEBUG, LSE_OPT) << "Skipping phi candidate " << LogInst(cand) << ": "
                                                         << LogInst(inst) << " is in inner loop";
                            valid = false;
                            break;
                        }

                        insts_must_alias.push_back(inst);
                        if (inst->IsStore()) {
                            has_stores = true;
                        } else if (inst->IsLoad()) {
                            has_loads = true;
                        }
                    }
                    // Other than validity, it's also possible that all instructions are already eliminated
                    if (!valid || insts_must_alias.empty()) {
                        continue;
                    }

                    if (has_stores) {
                        if (!has_loads) {
                            // Nothing to replace
                            COMPILER_LOG(DEBUG, LSE_OPT)
                                << "Skipping phi candidate " << LogInst(cand) << ": no loads to convert to phi";
                            continue;
                        }

                        auto phi = cand->GetBasicBlock()->GetGraph()->CreateInstPhi(cand->GetType(), cand->GetPc());
                        loop->GetHeader()->AppendPhi(phi);

                        if (!ProcessBackedges(phi, loop, cand, &insts_must_alias)) {
                            loop->GetHeader()->RemoveInst(phi);
                            continue;
                        }

                        LoopDoElimination(cand, loop, phi, &insts_must_alias);
                    } else {
                        ASSERT(has_loads);
                        // Without stores, we can replace all MUST_ALIAS loads with instruction itself
                        LoopDoElimination(cand, loop, nullptr, &insts_must_alias);
                    }
                }
            }

            COMPILER_LOG(DEBUG, LSE_OPT) << "Fixing elimination list after backedge substitutions";
            for (auto &entry : eliminations_) {
                auto hvalue = entry.second;
                if (eliminations_.find(hvalue.val) == eliminations_.end()) {
                    continue;
                }

                [[maybe_unused]] auto initial = hvalue.val;
                while (eliminations_.find(hvalue.val) != eliminations_.end()) {
                    auto elim_value = eliminations_[hvalue.val];
                    COMPILER_LOG(DEBUG, LSE_OPT) << "\t" << LogInst(hvalue.val)
                                                 << " is eliminated. Trying to replace by " << LogInst(elim_value.val);
                    hvalue = elim_value;
                    ASSERT_PRINT(initial != hvalue.val, "A cyclic elimination has been detected");
                }
                entry.second = hvalue;
            }
        }
    }

    bool ExistsPathWithoutShadowingStores(BasicBlock *start, BasicBlock *block, Marker marker)
    {
        if (block->IsEndBlock()) {
            // Found a path without shadowing stores
            return true;
        }
        if (block->IsMarked(marker)) {
            // Found a path with shadowing store
            return false;
        }
        ASSERT(start->GetLoop() == block->GetLoop());
        for (auto succ : block->GetSuccsBlocks()) {
            if (block->GetLoop() != succ->GetLoop()) {
                // Edge to a different loop. We currently don't carry heap values through loops and don't
                // handle irreducible loops, so we can't be sure there are shadows on this path.
                return true;
            }
            if (succ == block->GetLoop()->GetHeader()) {
                // If next block is a loop header for this block's loop it means that instruction shadows itself
                return true;
            }
            if (ExistsPathWithoutShadowingStores(start, succ, marker)) {
                return true;
            }
        }
        return false;
    }

    void FinalizeShadowedStores()
    {
        // We want to see if there are no paths from the store that evade any of its shadow stores
        for (auto &entry : shadowed_stores_) {
            auto inst = entry.first;
            auto marker = inst->GetBasicBlock()->GetGraph()->NewMarker();
            auto &shadows = shadowed_stores_.at(inst);
            for (auto shadow : shadows) {
                shadow->GetBasicBlock()->SetMarker(marker);
            }
            if (!ExistsPathWithoutShadowingStores(inst->GetBasicBlock(), inst->GetBasicBlock(), marker)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is fully shadowed by aliased stores";
                auto alive = InstStoredValue(entry.second[0]);
                auto eliminated = eliminations_.find(alive);
                if (eliminated != eliminations_.end()) {
                    alive = eliminated->second.val;
                }
                const Lse::HeapValue heap = {entry.second[0], alive, false, false};
                MarkForElimination(inst, entry.second[0], &heap);
            }
            inst->GetBasicBlock()->GetGraph()->EraseMarker(marker);
        }
    }

private:
    /// Return a MUST_ALIASed heap entry, nullptr if not present.
    const Lse::HeapValue *GetHeapValue(Inst *inst)
    {
        auto &block_heap = heaps_.at(GetEquivClass(inst)).first.at(inst->GetBasicBlock());
        for (auto &entry : block_heap) {
            alias_calls_++;
            if (aa_.CheckInstAlias(inst, entry.first) == MUST_ALIAS) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    /// Update phi candidates with aliased accesses
    void UpdatePhis(Inst *inst)
    {
        Loop *loop = inst->GetBasicBlock()->GetLoop();

        while (!loop->IsRoot()) {
            auto &phis = heaps_.at(GetEquivClass(inst)).second.at(loop);
            for (auto &[mem, values] : phis) {
                alias_calls_++;
                if (aa_.CheckInstAlias(inst, mem) != NO_ALIAS) {
                    values.push_back(inst);
                }
            }
            loop = loop->GetOuterLoop();
        }
    }

private:
    AliasAnalysis &aa_;
    Lse::HeapEqClasses &heaps_;
    /* Map of instructions to be deleted with values to replace them with */
    ArenaUnorderedMap<Inst *, struct Lse::HeapValue> eliminations_;
    ArenaUnorderedMap<Inst *, std::vector<Inst *>> shadowed_stores_;
    SaveStateBridgesBuilder ssb_;
    InstVector disabled_objects_;
    uint32_t alias_calls_ {0};
};

/// Returns true if the instruction invalidates the whole heap
static bool IsHeapInvalidatingInst(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadStatic:
            return inst->CastToLoadStatic()->GetVolatile();
        case Opcode::LoadObject:
            return inst->CastToLoadObject()->GetVolatile();
        case Opcode::InitObject:
        case Opcode::InitClass:
        case Opcode::LoadAndInitClass:
        case Opcode::UnresolvedLoadAndInitClass:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::ResolveObjectField:
        case Opcode::ResolveObjectFieldStatic:
            return true;  //  runtime calls invalidate the heap
        case Opcode::CallVirtual:
            return !inst->CastToCallVirtual()->IsInlined();
        case Opcode::CallResolvedVirtual:
            return !inst->CastToCallResolvedVirtual()->IsInlined();
        case Opcode::CallStatic:
            return !inst->CastToCallStatic()->IsInlined();
        case Opcode::CallDynamic:
            return !inst->CastToCallDynamic()->IsInlined();
        case Opcode::CallResolvedStatic:
            return !inst->CastToCallResolvedStatic()->IsInlined();
        case Opcode::Monitor:
            return inst->CastToMonitor()->IsEntry();
        default:
            return inst->GetFlag(compiler::inst_flags::HEAP_INV);
    }
}

/// Returns true if the instruction reads from heap and we're not sure about its effects
static bool IsHeapReadingInst(Inst *inst)
{
    if (inst->CanThrow()) {
        return true;
    }
    if (inst->IsIntrinsic()) {
        for (auto input : inst->GetInputs()) {
            if (input.GetInst()->GetType() == DataType::REFERENCE) {
                return true;
            }
        }
    }
    if (inst->IsStore() && IsVolatileMemInst(inst)) {
        // Heap writes before the volatile store should be visible from another thread after a corresponding volatile
        // load
        return true;
    }
    if (inst->GetOpcode() == Opcode::Monitor) {
        return inst->CastToMonitor()->IsExit();
    }
    return false;
}

bool Lse::CanEliminateInstruction(Inst *inst)
{
    if (inst->IsBarrier()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: a barrier";
        return false;
    }
    auto loop = inst->GetBasicBlock()->GetLoop();
    if (loop->IsIrreducible()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: an irreducible loop";
        return false;
    }
    if (loop->IsOsrLoop()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: an OSR loop";
        return false;
    }
    if (loop->IsTryCatchLoop()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: an try-catch loop";
        return false;
    }
    return true;
}

void Lse::InitializeHeap(BasicBlock *block, HeapEqClasses *heaps)
{
    for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
        auto &heap = heaps->at(eq_class).first;
        auto &phi_cands = heaps->at(eq_class).second;
        [[maybe_unused]] auto it = heap.emplace(block, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it.second);
        if (block->IsLoopHeader()) {
            [[maybe_unused]] auto phit =
                phi_cands.emplace(block->GetLoop(), GetGraph()->GetLocalAllocator()->Adapter());
            ASSERT(phit.second);
        }
    }
}

/**
 * While entering in the loop we put all heap values obtained from loads as phi candidates.
 * Further phi candidates would replace MUST_ALIAS accesses in the loop if no aliased stores were met.
 */
void Lse::MergeHeapValuesForLoop(BasicBlock *block, HeapEqClasses *heaps)
{
    ASSERT(block->IsLoopHeader());
    auto loop = block->GetLoop();

    // Do not eliminate anything in irreducible or osr loops
    if (loop->IsIrreducible() || loop->IsOsrLoop() || loop->IsTryCatchLoop()) {
        return;
    }

    auto preheader = loop->GetPreHeader();

    for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
        auto &preheader_heap = heaps->at(eq_class).first.at(preheader);

        auto &block_phis = heaps->at(eq_class).second.at(loop);
        for (auto mem : preheader_heap) {
            block_phis.try_emplace(mem.second.origin, GetGraph()->GetLocalAllocator()->Adapter());
            COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(mem.first) << " is a phi cand for BB #" << block->GetId();
        }
    }
}

/// Merge heap values for passed block from its direct predecessors.
int Lse::MergeHeapValuesForBlock(BasicBlock *block, HeapEqClasses *heaps, Marker phi_fixup_mrk)
{
    int alias_calls = 0;
    for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
        auto &heap = heaps->at(eq_class).first;
        auto &block_heap = heap.at(block);
        /* Copy a heap of one of predecessors */
        auto preds = block->GetPredsBlocks();
        auto pred_it = preds.begin();
        if (pred_it != preds.end()) {
            block_heap.insert(heap.at(*pred_it).begin(), heap.at(*pred_it).end());
            pred_it++;
        }

        /* Erase from the heap anything that disappeared or was changed in other predecessors */
        while (pred_it != preds.end()) {
            auto pred_heap = heap.at(*pred_it);
            auto heap_it = block_heap.begin();
            while (heap_it != block_heap.end()) {
                auto &heap_value = heap_it->second;
                auto pred_inst_it = pred_heap.begin();
                while (pred_inst_it != pred_heap.end()) {
                    if (pred_inst_it->first == heap_it->first) {
                        break;
                    }
                    alias_calls++;
                    if (GetGraph()->CheckInstAlias(heap_it->first, pred_inst_it->first) == MUST_ALIAS) {
                        break;
                    }
                    pred_inst_it++;
                }
                if (pred_inst_it == pred_heap.end()) {
                    // If this is a phi we're creating during merge, delete it
                    if (heap_value.val->IsPhi() && heap_value.local) {
                        block->RemoveInst(heap_value.val);
                    }
                    heap_it = block_heap.erase(heap_it);
                    continue;
                }
                if (pred_inst_it->second.val != heap_value.val) {
                    // Try to create a phi instead
                    // We limit possible phis to cases where value originated in the same predecessor
                    // in order to not increase register usage too much
                    if (block->GetLoop()->IsIrreducible() || block->IsCatch() ||
                        pred_inst_it->second.origin->GetBasicBlock() != *pred_it) {
                        // If this is a phi we're creating during merge, delete it
                        if (heap_value.val->IsPhi() && heap_value.local) {
                            block->RemoveInst(heap_value.val);
                        }
                        heap_it = block_heap.erase(heap_it);
                        continue;
                    }
                    if (heap_value.val->IsPhi() && heap_value.local) {
                        heap_value.val->AppendInput(pred_inst_it->second.val);
                    } else {
                        // Values can only originate in a single block. If this predecessor is not
                        // the second predecessor, that means that this value did not originate in other
                        // predecessors, thus we don't create a phi
                        if (heap_value.origin->GetBasicBlock() != *(preds.begin()) ||
                            std::distance(preds.begin(), pred_it) > 1) {
                            heap_it = block_heap.erase(heap_it);
                            continue;
                        }
                        auto phi =
                            block->GetGraph()->CreateInstPhi(heap_value.origin->GetType(), heap_value.origin->GetPc());
                        block->AppendPhi(phi);
                        phi->AppendInput(heap_value.val);
                        phi->AppendInput(pred_inst_it->second.val);
                        phi->SetMarker(phi_fixup_mrk);
                        heap_value.val = phi;
                        heap_value.local = true;
                    }
                } else {
                    heap_it->second.read |= pred_inst_it->second.read;
                }
                heap_it++;
            }
            pred_it++;
        }
    }
    return alias_calls;
}

/**
 * When creating phis while merging predecessor heaps, we don't know yet if
 * we're creating a useful phi, and can't fix SaveStates because of that.
 * Do that here.
 */
void Lse::FixupPhisInBlock(BasicBlock *block, Marker phi_fixup_mrk)
{
    for (auto phi_inst : block->PhiInstsSafe()) {
        auto phi = phi_inst->CastToPhi();
        if (!phi->IsMarked(phi_fixup_mrk)) {
            continue;
        }
        if (!phi->HasUsers()) {
            block->RemoveInst(phi);
        } else if (!GetGraph()->IsBytecodeOptimizer() && phi->IsReferenceOrAny()) {
            for (auto i = 0U; i < phi->GetInputsCount(); i++) {
                auto input = phi->GetInput(i);
                if (input.GetInst()->IsMovableObject()) {
                    auto bb = phi->GetPhiInputBb(i);
                    ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), input.GetInst(), bb->GetLastInst(), nullptr,
                                                              bb);
                }
            }
        }
    }
}

/**
 * Returns the elimination code in two letter format.
 *
 * The first letter describes a [L]oad or [S]tore that was eliminated.
 * The second letter describes the dominant [L]oad or [S]tore that is the
 * reason why instruction was eliminated.
 */
const char *Lse::GetEliminationCode(Inst *inst, Inst *origin)
{
    ASSERT(inst->IsMemory() && (origin->IsMemory() || origin->IsPhi()));
    if (inst->IsLoad()) {
        if (origin->IsLoad()) {
            return "LL";
        }
        if (origin->IsStore()) {
            return "LS";
        }
        if (origin->IsPhi()) {
            return "LP";
        }
    }
    if (inst->IsStore()) {
        if (origin->IsLoad()) {
            return "SL";
        }
        if (origin->IsStore()) {
            return "SS";
        }
    }
    UNREACHABLE();
}

/**
 * In the codegen of bytecode optimizer, we don't have corresponding pandasm
 * for the IR `Cast` of with some pairs of input types and output types. So
 * in the bytecode optimizer mode, we need to avoid generating such `Cast` IR.
 * The following function gives the list of legal pairs of types.
 * This function should not be used in compiler mode.
 */

static bool IsTypeLegalForCast(DataType::Type output, DataType::Type input)
{
    ASSERT(output != input);
    switch (input) {
        case DataType::INT32:
        case DataType::INT64:
        case DataType::FLOAT64:
            switch (output) {
                case DataType::FLOAT64:
                case DataType::INT64:
                case DataType::UINT32:
                case DataType::INT32:
                case DataType::INT16:
                case DataType::UINT16:
                case DataType::INT8:
                case DataType::UINT8:
                case DataType::ANY:
                    return true;
                default:
                    return false;
            }
        case DataType::REFERENCE:
            return output == DataType::ANY;
        default:
            return false;
    }
}

/**
 * Replace inputs of INST with VALUE and delete this INST.  If deletion led to
 * appearance of instruction that has no users delete this instruction too.
 */
void Lse::DeleteInstruction(Inst *inst, Inst *value)
{
    // Have to cast a value to the type of eliminated inst. Actually required only for loads.
    if (inst->GetType() != value->GetType() && inst->HasUsers()) {
        ASSERT(inst->GetType() != DataType::REFERENCE && value->GetType() != DataType::REFERENCE);
        // We will do nothing in bytecode optimizer mode when the types are not legal for cast.
        if (GetGraph()->IsBytecodeOptimizer() && !IsTypeLegalForCast(inst->GetType(), value->GetType())) {
            COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was not eliminated: requires an inappropriate cast";
            return;
        }
        auto cast = GetGraph()->CreateInstCast(inst->GetType(), inst->GetPc(), value, value->GetType());
        inst->InsertAfter(cast);
        value = cast;
    }
    inst->ReplaceUsers(value);

    ArenaQueue<Inst *> queue(GetGraph()->GetLocalAllocator()->Adapter());
    queue.push(inst);
    while (!queue.empty()) {
        Inst *front_inst = queue.front();
        BasicBlock *block = front_inst->GetBasicBlock();
        queue.pop();

        // Have been already deleted or could not be deleted
        if (block == nullptr || front_inst->HasUsers()) {
            continue;
        }

        for (auto &input : front_inst->GetInputs()) {
            /* Delete only instructions that has no data flow impact */
            if (input.GetInst()->HasPseudoDestination()) {
                queue.push(input.GetInst());
            }
        }
        block->RemoveInst(front_inst);
        applied_ = true;
    }
}

void Lse::DeleteInstructions(const ArenaUnorderedMap<Inst *, struct HeapValue> &eliminated)
{
    for (auto elim : eliminated) {
        Inst *inst = elim.first;
        Inst *origin = elim.second.origin;
        Inst *value = elim.second.val;

        ASSERT_DO(eliminated.find(value) == eliminated.end(),
                  (std::cerr << "Instruction:\n", inst->Dump(&std::cerr),
                   std::cerr << "is replaced by eliminated value:\n", value->Dump(&std::cerr)));

        while (origin->GetBasicBlock() == nullptr) {
            auto elim_it = eliminated.find(origin);
            ASSERT(elim_it != eliminated.end());
            origin = elim_it->second.origin;
        }

        GetGraph()->GetEventWriter().EventLse(inst->GetId(), inst->GetPc(), origin->GetId(), origin->GetPc(),
                                              GetEliminationCode(inst, origin));
        // Try to update savestates
        if (!GetGraph()->IsBytecodeOptimizer() && value->IsMovableObject()) {
            if (!value->IsPhi() && origin->IsMovableObject() && origin->IsLoad() && origin->IsDominate(inst)) {
                // this branch is not required, but can be faster if origin is closer to inst than value
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), origin, inst);
            } else {
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), value, inst);
            }
        }
        DeleteInstruction(inst, value);
    }
}

void Lse::ApplyHoistToCandidate(Loop *loop, Inst *alive)
{
    ASSERT(alive->IsLoad());
    COMPILER_LOG(DEBUG, LSE_OPT) << " v" << alive->GetId();
    if (alive->GetBasicBlock()->GetLoop() != loop) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because inst is part of a more inner loop";
        return;
    }
    if (GetGraph()->IsInstThrowable(alive)) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because inst is throwable";
        return;
    }
    for (const auto &input : alive->GetInputs()) {
        if (!input.GetInst()->GetBasicBlock()->IsDominate(loop->GetPreHeader())) {
            COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because of def-use chain of inputs: " << LogInst(input.GetInst());
            return;
        }
    }
    const auto &rpo = GetGraph()->GetBlocksRPO();
    auto block_iter = std::find(rpo.rbegin(), rpo.rend(), alive->GetBasicBlock());
    ASSERT(block_iter != rpo.rend());
    auto inst = alive->GetPrev();
    while (*block_iter != loop->GetPreHeader()) {
        while (inst != nullptr) {
            if (IsHeapInvalidatingInst(inst) || (inst->IsMemory() && GetEquivClass(inst) == GetEquivClass(alive) &&
                                                 GetGraph()->CheckInstAlias(inst, alive) != NO_ALIAS)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because of invalidating inst:" << LogInst(inst);
                return;
            }
            inst = inst->GetPrev();
        }
        block_iter++;
        inst = (*block_iter)->GetLastInst();
    }
    alive->GetBasicBlock()->EraseInst(alive, true);
    auto last_inst = loop->GetPreHeader()->GetLastInst();
    if (last_inst != nullptr && last_inst->IsControlFlow()) {
        loop->GetPreHeader()->InsertBefore(alive, last_inst);
    } else {
        loop->GetPreHeader()->AppendInst(alive);
    }
    if (!GetGraph()->IsBytecodeOptimizer() && alive->IsMovableObject()) {
        ASSERT(!loop->IsIrreducible());
        // loop backedges will be walked inside SSB
        ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), alive, loop->GetHeader()->GetLastInst(), nullptr,
                                                  loop->GetHeader());
    }
    applied_ = true;
}

void Lse::TryToHoistLoadFromLoop(Loop *loop, HeapEqClasses *heaps,
                                 const ArenaUnorderedMap<Inst *, struct HeapValue> *eliminated)
{
    for (auto inner_loop : loop->GetInnerLoops()) {
        TryToHoistLoadFromLoop(inner_loop, heaps, eliminated);
    }

    if (loop->IsIrreducible() || loop->IsOsrLoop()) {
        return;
    }

    auto &back_bbs = loop->GetBackEdges();
    be_alive_.clear();

    // Initiate alive set
    auto back_bb = back_bbs.begin();
    ASSERT(back_bb != back_bbs.end());
    for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
        for (const auto &entry : heaps->at(eq_class).first.at(*back_bb)) {
            // Do not touch Stores and eliminated ones
            if (entry.first->IsLoad() && eliminated->find(entry.first) == eliminated->end()) {
                be_alive_.insert(entry.first);
            }
        }
    }

    // Throw values not alive on other backedges
    while (++back_bb != back_bbs.end()) {
        auto alive = be_alive_.begin();
        while (alive != be_alive_.end()) {
            auto &heap = heaps->at(GetEquivClass(*alive)).first;
            if (heap.at(*back_bb).find(*alive) == heap.at(*back_bb).end()) {
                alive = be_alive_.erase(alive);
            } else {
                alive++;
            }
        }
    }
    COMPILER_LOG(DEBUG, LSE_OPT) << "Loop #" << loop->GetId() << " has the following motion candidates:";
    for (auto alive : be_alive_) {
        ApplyHoistToCandidate(loop, alive);
    }
}

bool Lse::RunImpl()
{
    if (GetGraph()->IsBytecodeOptimizer() && GetGraph()->IsDynamicMethod()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "Load-Store Elimination skipped: es bytecode optimizer";
        return false;
    }

    HeapEqClasses heaps(GetGraph()->GetLocalAllocator()->Adapter());
    for (int eq_class = Lse::EquivClass::EQ_ARRAY; eq_class != Lse::EquivClass::EQ_LAST; eq_class++) {
        std::pair<Heap, PhiCands> heap_phi(GetGraph()->GetLocalAllocator()->Adapter(),
                                           GetGraph()->GetLocalAllocator()->Adapter());
        heaps.emplace(eq_class, heap_phi);
    }
    InstVector invs(GetGraph()->GetLocalAllocator()->Adapter());

    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<AliasAnalysis>();

    LseVisitor visitor(GetGraph(), &heaps);
    auto marker_holder = MarkerHolder(GetGraph());
    auto phi_fixup_mrk = marker_holder.GetMarker();
    int alias_calls = 0;

    for (auto block : GetGraph()->GetBlocksRPO()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "Processing BB " << block->GetId();
        InitializeHeap(block, &heaps);

        if (block->IsLoopHeader()) {
            MergeHeapValuesForLoop(block, &heaps);
        } else {
            alias_calls += MergeHeapValuesForBlock(block, &heaps, phi_fixup_mrk);
        }

        for (auto inst : block->Insts()) {
            if (IsHeapReadingInst(inst)) {
                visitor.SetHeapAsRead(block);
            }
            if (IsHeapInvalidatingInst(inst)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " invalidates heap";
                visitor.InvalidateHeap(block);
            } else if (inst->IsLoad()) {
                visitor.VisitLoad(inst);
            } else if (inst->IsStore()) {
                visitor.VisitStore(inst, InstStoredValue(inst));
            }
            if (inst->IsIntrinsic()) {
                visitor.VisitIntrinsic(inst, &invs);
            }
            // If we call Alias Analysis too much, we assume that this block has too many
            // instructions and we should bail in favor of performance.
            if (visitor.GetAliasAnalysisCallCount() + alias_calls > Lse::AA_CALLS_LIMIT) {
                COMPILER_LOG(DEBUG, LSE_OPT) << "Exiting BB " << block->GetId() << ": too many Alias Analysis calls";
                visitor.InvalidateHeap(block);
                break;
            }
        }
        visitor.ClearLocalValuesFromHeap(block);
        visitor.ResetLimits();
    }
    visitor.FinalizeShadowedStores();
    visitor.FinalizeLoops(GetGraph());

    auto &eliminated = visitor.GetEliminations();
    GetGraph()->RunPass<DominatorsTree>();
    if (hoist_loads_) {
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            TryToHoistLoadFromLoop(loop, &heaps, &eliminated);
        }
    }

    DeleteInstructions(visitor.GetEliminations());

    for (auto block : GetGraph()->GetBlocksRPO()) {
        FixupPhisInBlock(block, phi_fixup_mrk);
    }

    COMPILER_LOG(DEBUG, LSE_OPT) << "Load-Store Elimination complete";
    return applied_;
}
}  // namespace panda::compiler
