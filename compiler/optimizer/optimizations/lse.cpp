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

static std::string LogInst(Inst *inst)
{
    return "v" + std::to_string(inst->GetId()) + " (" + GetOpcodeString(inst->GetOpcode()) + ")";
}

class LseVisitor {
public:
    explicit LseVisitor(Graph *graph, Lse::Heap *heap, Lse::PhiCands *phis)
        : aa_(graph->GetAnalysis<AliasAnalysis>()),
          heap_(*heap),
          phis_(*phis),
          eliminations_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(LseVisitor);
    NO_COPY_SEMANTIC(LseVisitor);
    ~LseVisitor() = default;

    void VisitStore(Inst *inst, Inst *val)
    {
        auto hvalue = GetHeapValue(inst);
        /* Value can be eliminated already */
        auto alive = val;
        auto eliminated = eliminations_.find(val);
        if (eliminated != eliminations_.end()) {
            alive = eliminated->second.val;
        }
        /* If store was assigned to VAL then we can eliminate the second assignment */
        if (hvalue != nullptr && hvalue->val == alive) {
            if (Lse::CanEliminateInstruction(inst)) {
                auto heap = *hvalue;
                if (eliminations_.find(heap.val) != eliminations_.end()) {
                    heap = eliminations_[heap.val];
                }
                ASSERT(eliminations_.find(heap.val) == eliminations_.end());
                eliminations_[inst] = heap;
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is eliminated because of " << LogInst(heap.origin);
            }
            return;
        }
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " updated heap with v" << alive->GetId();

        /* Stores added to eliminations_ above aren't checked versus phis -> no double instruction elimination */
        UpdatePhis(inst);

        auto &block_heap = heap_.at(inst->GetBasicBlock());
        /* Erase all aliased values, because they may be overwritten */
        for (auto heap_iter = block_heap.begin(), heap_last = block_heap.end(); heap_iter != heap_last;) {
            auto hinst = heap_iter->first;
            if (aa_.CheckInstAlias(inst, hinst) == NO_ALIAS) {
                heap_iter++;
            } else {
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "\tDrop from heap { " << LogInst(hinst) << ", v" << heap_iter->second.val->GetId() << "}";
                heap_iter = block_heap.erase(heap_iter);
            }
        }

        /* Set value of the inst to VAL */
        block_heap[inst] = {inst, alive};
    }

    void VisitLoad(Inst *inst)
    {
        /* If we have a heap value for this load instruction then we can eliminate it */
        auto hvalue = GetHeapValue(inst);
        if (hvalue != nullptr) {
            if (Lse::CanEliminateInstruction(inst)) {
                auto heap = *hvalue;
                if (eliminations_.find(heap.val) != eliminations_.end()) {
                    heap = eliminations_[heap.val];
                }
                ASSERT(eliminations_.find(heap.val) == eliminations_.end());
                eliminations_[inst] = heap;
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is eliminated because of " << LogInst(heap.origin);
            }
            return;
        }
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " updated heap with v" << inst->GetId();

        /* Loads added to eliminations_ above are not checked versus phis -> no double instruction elimination */
        UpdatePhis(inst);

        /* Otherwise set the value of instruction to itself and update MUST_ALIASes */
        heap_.at(inst->GetBasicBlock())[inst] = {inst, inst};
    }

    /**
     * Completely resets the accumulated state: heap and phi candidates.
     */
    void InvalidateHeap(BasicBlock *block)
    {
        heap_.at(block).clear();
        auto loop = block->GetLoop();
        while (!loop->IsRoot()) {
            phis_.at(loop).clear();
            loop = loop->GetOuterLoop();
        }
    }

    /**
     * Reset accumulated references that may be relocated during GC.
     * If SaveState is provided then references mentioned in SaveState are not reset.
     */
    void InvalidateRefs(BasicBlock *block, SaveStateInst *ss)
    {
        auto &bheap = heap_.at(block);
        for (auto it = bheap.begin(); it != bheap.end();) {
            auto hvalue = it->second;
            if (IsRelocatableValue(hvalue.val, ss)) {
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "\tDrop from heap { " << LogInst(it->first) << ", v" << hvalue.val->GetId() << "}";
                it = bheap.erase(it);
            } else {
                it++;
            }
        }

        auto loop = block->GetLoop();
        while (!loop->IsRoot()) {
            auto &cands = phis_.at(loop);
            for (auto it = cands.begin(); it != cands.end();) {
                auto cand = it->first;
                auto val = cand->IsStore() ? InstStoredValue(cand) : cand;
                if (IsRelocatableValue(val, ss)) {
                    COMPILER_LOG(DEBUG, LSE_OPT)
                        << "\tDrop a phi-cand { " << LogInst(cand) << ", v" << val->GetId() << "}";
                    it = cands.erase(it);
                } else {
                    it++;
                }
            }
            loop = loop->GetOuterLoop();
        }
    }

    ArenaUnorderedMap<Inst *, struct Lse::HeapValue> &GetEliminations()
    {
        return eliminations_;
    }

    /**
     * Add eliminations inside loops if there is no overwrites on backedges.
     */
    void FinalizeLoops()
    {
        for (auto &[loop, phis] : phis_) {
            COMPILER_LOG(DEBUG, LSE_OPT) << "Finalizing loop #" << loop->GetId();
            for (auto &[cand, insts] : phis) {
                if (insts.empty()) {
                    COMPILER_LOG(DEBUG, LSE_OPT) << "Skipping phi candidate " << LogInst(cand) << " (no users)";
                    continue;
                }

                bool valid = true;
                for (auto inst : insts) {
                    // One MAY_ALIASed or MUST_ALIASed store is enough to reject the candidate
                    if (inst->IsStore()) {
                        COMPILER_LOG(DEBUG, LSE_OPT)
                            << "Skipping phi candidate " << LogInst(cand) << " because of store " << LogInst(inst);
                        valid = false;
                        break;
                    }
                }
                if (!valid) {
                    continue;
                }

                COMPILER_LOG(DEBUG, LSE_OPT) << "Processing phi candidate: " << LogInst(cand);
                for (auto inst : insts) {
                    if (eliminations_.find(inst) != eliminations_.end() ||
                        aa_.CheckInstAlias(cand, inst) != MUST_ALIAS) {
                        continue;
                    }

                    struct Lse::HeapValue hvalue = {cand, cand->IsStore() ? InstStoredValue(cand) : cand};
                    eliminations_[inst] = hvalue;
                    COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is replaced by " << LogInst(hvalue.val);
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
                COMPILER_LOG(DEBUG, LSE_OPT) << "\t" << LogInst(hvalue.val) << " is eliminated. Trying to replace by "
                                             << LogInst(elim_value.val);
                hvalue = elim_value;
                ASSERT_PRINT(initial != hvalue.val, "A cyclic elimination has been detected");
            }
            entry.second = hvalue;
        }
    }

private:
    /**
     * Return a MUST_ALIASed heap entry, nullptr if not present.
     */
    const Lse::HeapValue *GetHeapValue(Inst *inst) const
    {
        auto &block_heap = heap_.at(inst->GetBasicBlock());
        for (auto &entry : block_heap) {
            if (aa_.CheckInstAlias(inst, entry.first) == MUST_ALIAS) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    /**
     * Update phi candidates with aliased accesses
     */
    void UpdatePhis(Inst *inst)
    {
        Loop *loop = inst->GetBasicBlock()->GetLoop();

        while (!loop->IsRoot()) {
            auto &phis = phis_.at(loop);
            for (auto &[mem, values] : phis) {
                if (aa_.CheckInstAlias(inst, mem) != NO_ALIAS) {
                    values.push_back(inst);
                }
            }
            loop = loop->GetOuterLoop();
        }
    }

    /**
     * Return true if a value can be relocated after save state
     */
    bool IsRelocatableValue(Inst *val, SaveStateInst *ss)
    {
        // Skip primitive values on the heap
        if (val->GetType() != DataType::REFERENCE) {
            return false;
        }
        // If it is saved we can keep it
        for (size_t i = 0; ss != nullptr && i < ss->GetInputsCount(); ++i) {
            auto saved = ss->GetDataFlowInput(i);
            // input can be eliminated
            if (eliminations_.find(saved) != eliminations_.end()) {
                saved = eliminations_[saved].val;
            }
            if (saved == val) {
                return false;
            }
        }

        return true;
    }

private:
    AliasAnalysis &aa_;
    Lse::Heap &heap_;
    /* Mem accesses that could be replaced with Phi */
    Lse::PhiCands &phis_;
    /* Map of instructions to be deleted with values to replace them with */
    ArenaUnorderedMap<Inst *, struct Lse::HeapValue> eliminations_;
};

/**
 * Returns true if the instruction invalidates the whole heap
 */
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
        case Opcode::UnresolvedCallVirtual:
        case Opcode::UnresolvedCallStatic:
        case Opcode::UnresolvedLoadStatic:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::UnresolvedLoadObject:
            return true;
        case Opcode::CallVirtual:
            return !inst->CastToCallVirtual()->IsInlined();
        case Opcode::CallStatic:
            return !inst->CastToCallStatic()->IsInlined();
        default:
            return inst->GetFlag(compiler::inst_flags::HEAP_INV);
    }
}

/**
 * Returns true if after instruction execution loaded references can be changed
 */
static bool IsGCInst(Inst *inst)
{
    return inst->GetOpcode() == Opcode::SafePoint || inst->IsRuntimeCall();
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

/**
 * While entering in the loop we put all heap values obtained from loads as phi candidates.
 * Further phi candidates would replace MUST_ALIAS accesses in the loop if no aliased stores were met.
 */
void Lse::MergeHeapValuesForLoop(BasicBlock *block, Heap *heap, PhiCands *phis)
{
    ASSERT(block->IsLoopHeader());
    [[maybe_unused]] auto it = heap->emplace(block, GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(it.second);
    auto loop = block->GetLoop();
    auto phit = phis->emplace(loop, GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(phit.second);

    // Do not eliminate anything in irreducible or osr loops
    if (loop->IsIrreducible() || loop->IsOsrLoop() || loop->IsTryCatchLoop()) {
        return;
    }

    auto preheader = loop->GetPreHeader();
    auto &preheader_heap = heap->at(preheader);

    auto &block_phis = phit.first->second;

    for (auto mem : preheader_heap) {
        block_phis.try_emplace(mem.second.origin, GetGraph()->GetLocalAllocator()->Adapter());
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(mem.first) << " is a phi cand for BB #" << block->GetId();
    }
}

/**
 * Merge heap values for passed block from its direct predecessors.
 */
void Lse::MergeHeapValuesForBlock(BasicBlock *block, Heap *heap)
{
    auto it = heap->emplace(block, GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(it.second);

    auto &block_heap = it.first->second;
    /* Copy a heap of one of predecessors */
    auto preds = block->GetPredsBlocks();
    auto pred_it = preds.begin();
    if (pred_it != preds.end()) {
        block_heap.insert(heap->at(*pred_it).begin(), heap->at(*pred_it).end());
        pred_it++;
    }

    /* Erase from the heap anything that disappeared or was changed in other predecessors */
    while (pred_it != preds.end()) {
        auto pred_heap = heap->at(*pred_it);
        auto heap_it = block_heap.begin();
        while (heap_it != block_heap.end()) {
            if (pred_heap.find(heap_it->first) == pred_heap.end() ||
                pred_heap[heap_it->first].val != heap_it->second.val) {
                heap_it = block_heap.erase(heap_it);
            } else {
                heap_it++;
            }
        }
        pred_it++;
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
    ASSERT(inst->IsMemory() && origin->IsMemory());
    if (inst->IsLoad()) {
        if (origin->IsLoad()) {
            return "LL";
        }
        if (origin->IsStore()) {
            return "LS";
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
        auto cast = GetGraph()->CreateInstCast(inst->GetType(), inst->GetPc());
        cast->SetOperandsType(value->GetType());
        cast->SetInput(0, value);
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

bool Lse::HasMonitor()
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetMonitorBlock()) {
            return true;
        }
    }
    return false;
}

bool Lse::RunImpl()
{
    if (HasMonitor()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "Load-Store Elimination skipped: monitors";
        return false;
    }

    Heap heap(GetGraph()->GetLocalAllocator()->Adapter());
    PhiCands phis(GetGraph()->GetLocalAllocator()->Adapter());

    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<AliasAnalysis>();

    LseVisitor visitor(GetGraph(), &heap, &phis);
    for (auto block : GetGraph()->GetBlocksRPO()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "Processing BB " << block->GetId();
        if (block->IsLoopHeader()) {
            MergeHeapValuesForLoop(block, &heap, &phis);
        } else {
            MergeHeapValuesForBlock(block, &heap);
        }

        for (auto inst : block->Insts()) {
            if (IsHeapInvalidatingInst(inst)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " invalidates heap";
                visitor.InvalidateHeap(block);
            } else if (IsGCInst(inst)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " relocates refs";
                SaveStateInst *ss = inst->GetSaveState();
                if (inst->GetOpcode() == Opcode::SafePoint) {
                    ss = inst->CastToSafePoint();
                }
                visitor.InvalidateRefs(block, ss);
            } else if (inst->IsLoad()) {
                visitor.VisitLoad(inst);
            } else if (inst->IsStore()) {
                visitor.VisitStore(inst, InstStoredValue(inst));
            }
        }
    }
    visitor.FinalizeLoops();

    auto &eliminated = visitor.GetEliminations();
    for (auto elim : eliminated) {
        Inst *inst = elim.first;
        Inst *origin = elim.second.origin;
        Inst *value = elim.second.val;

        ASSERT_DO(eliminated.find(value) == eliminated.end(),
                  (std::cerr << "Instruction:\n", inst->Dump(&std::cerr),
                   std::cerr << "is replaced by eliminated value:\n", value->Dump(&std::cerr)));

        GetGraph()->GetEventWriter().EventLse(inst->GetId(), inst->GetPc(), origin->GetId(), origin->GetPc(),
                                              GetEliminationCode(inst, origin));
        DeleteInstruction(inst, value);
    }

    COMPILER_LOG(DEBUG, LSE_OPT) << "Load-Store Elimination complete";
    return applied_;
}
}  // namespace panda::compiler
