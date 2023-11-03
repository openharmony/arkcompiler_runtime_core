/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef COMPILER_ANALYSIS_ESCAPE_H
#define COMPILER_ANALYSIS_ESCAPE_H

#include <algorithm>
#include <variant>

#include "compiler_options.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/runtime_interface.h"
#include "mem/arena_allocator.h"
#include "utils/bit_vector.h"
#include "compiler_logger.h"
#include "utils/logger.h"

namespace panda::compiler {
using FieldId = uint64_t;
using InstId = uint32_t;
using BlockId = uint32_t;
using StateId = uint32_t;
using FieldPtr = RuntimeInterface::FieldPtr;

class VirtualState;
class BasicBlockState;
class PhiState;
class ScalarReplacement;
class ZeroInst;

using StateOwner = std::variant<Inst *, PhiState *, const ZeroInst *>;
using MaterializationSite = std::variant<Inst *, BasicBlock *>;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class EscapeAnalysis : public Optimization, public GraphVisitor {
public:
    static constexpr StateId MATERIALIZED_ID = 0;

    explicit EscapeAnalysis(Graph *graph)
        : Optimization(graph),
          block_states_(graph->GetLocalAllocator()->Adapter()),
          aliases_(graph->GetLocalAllocator()->Adapter()),
          materialization_info_(graph->GetLocalAllocator()->Adapter()),
          phis_(graph->GetLocalAllocator()->Adapter()),
          save_state_info_(graph->GetLocalAllocator()->Adapter()),
          virtualizable_allocations_(graph->GetLocalAllocator()->Adapter()),
          merge_processor_(this),
          live_ins_(graph)
    {
    }

    NO_MOVE_SEMANTIC(EscapeAnalysis);
    NO_COPY_SEMANTIC(EscapeAnalysis);
    ~EscapeAnalysis() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "EscapeAnalysis";
    }

    bool IsEnable() const override
    {
        return OPTIONS.IsCompilerScalarReplacement();
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DefineVisit(InstName)                                    \
    static void Visit##InstName(GraphVisitor *v, Inst *inst)     \
    {                                                            \
        static_cast<EscapeAnalysis *>(v)->Visit##InstName(inst); \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DefineVisitWithCallback(InstName, Callback)          \
    static void Visit##InstName(GraphVisitor *v, Inst *inst) \
    {                                                        \
        static_cast<EscapeAnalysis *>(v)->Callback(inst);    \
    }

    DefineVisit(NewObject);
    DefineVisit(LoadObject);
    DefineVisit(StoreObject);
    DefineVisit(NullCheck);
    DefineVisit(SaveState);
    DefineVisit(SafePoint);
    DefineVisit(GetInstanceClass);

    DefineVisitWithCallback(Deoptimize, MaterializeDeoptSaveState);
    DefineVisitWithCallback(DeoptimizeIf, MaterializeDeoptSaveState);
    DefineVisitWithCallback(DeoptimizeCompare, MaterializeDeoptSaveState);
    DefineVisitWithCallback(DeoptimizeCompareImm, MaterializeDeoptSaveState);
    DefineVisitWithCallback(LoadAndInitClass, VisitSaveStateUser);
    DefineVisitWithCallback(LoadClass, VisitSaveStateUser);

#undef DefineVisit
#undef DefineVisitWithCallback

    static void VisitCallStatic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
    {
        static_cast<EscapeAnalysis *>(v)->VisitCall(inst->CastToCallStatic());
    }
    static void VisitCallVirtual([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
    {
        static_cast<EscapeAnalysis *>(v)->VisitCall(inst->CastToCallVirtual());
    }
    static void VisitCompare([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
    {
        static_cast<EscapeAnalysis *>(v)->VisitCmpRef(inst, inst->CastToCompare()->GetCc());
    }
    static void VisitIf([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
    {
        static_cast<EscapeAnalysis *>(v)->VisitCmpRef(inst, inst->CastToIf()->GetCc());
    }
    static void VisitPhi([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
    {
        // Phi's handled during block's state merge
    }
    static void VisitReturnInlined([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
    {
        // do nothing
    }
    // by default all ref-input are materialized and materialized state is recorded for ref-output
    void VisitDefault([[maybe_unused]] Inst *inst) override
    {
        VisitSaveStateUser(inst);
        Materialize(inst);
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        UNREACHABLE();
    }

    // only for testing
    void RelaxClassRestrictions()
    {
        relax_class_restrictions_ = true;
    }

#include "optimizer/ir/visitor.inc"
private:
    static constexpr InstId ZERO_INST_ID = std::numeric_limits<InstId>::max() - 1U;
    static constexpr size_t MAX_NESTNESS = 5;

    class MergeProcessor {
    public:
        explicit MergeProcessor(EscapeAnalysis *parent)
            : parent_(parent),
              fields_merge_buffer_(parent->GetLocalAllocator()->Adapter()),
              states_merge_buffer_(parent->GetLocalAllocator()->Adapter()),
              all_fields_(parent->GetLocalAllocator()->Adapter()),
              pending_insts_(parent->GetLocalAllocator()->Adapter())
        {
        }
        ~MergeProcessor() = default;
        NO_COPY_SEMANTIC(MergeProcessor);
        NO_MOVE_SEMANTIC(MergeProcessor);

        // Compute initial state of a block by merging states of its predecessors.
        void MergeStates(BasicBlock *block);

    private:
        EscapeAnalysis *parent_;
        ArenaVector<StateOwner> fields_merge_buffer_;
        ArenaVector<StateOwner> states_merge_buffer_;
        ArenaVector<FieldPtr> all_fields_;
        ArenaVector<StateOwner> pending_insts_;

        bool MergeFields(BasicBlock *block, BasicBlockState *block_state, StateId state_to_merge, VirtualState *vstate,
                         ArenaVector<StateOwner> &merge_queue);
        bool MergeState(StateOwner inst, BasicBlock *block, BasicBlockState *new_state);
        void CheckStatesAndInsertIntoBuffer(StateOwner inst, BasicBlock *block);
        void MaterializeObjectsAtTheBeginningOfBlock(BasicBlock *block);
        void CollectInstructionsToMerge(BasicBlock *block);
        void ProcessPhis(BasicBlock *block, BasicBlockState *new_state);
    };

    class LiveInAnalysis {
    public:
        explicit LiveInAnalysis(Graph *graph)
            : graph_(graph),
              live_in_(graph_->GetLocalAllocator()->Adapter()),
              all_insts_(graph_->GetLocalAllocator()->Adapter())
        {
        }
        ~LiveInAnalysis() = default;
        NO_COPY_SEMANTIC(LiveInAnalysis);
        NO_MOVE_SEMANTIC(LiveInAnalysis);

        // Compute set of ref-typed instructions live at the beginning of each
        // basic blocks. Return false if graph don't contain NewObject instructions.
        bool Run();

        template <typename Callback>
        void VisitAlive(const BasicBlock *block, Callback &&cb)
        {
            auto &live_ins = live_in_[block->GetId()];
            auto span = live_ins.GetContainerDataSpan();
            for (size_t mask_idx = 0; mask_idx < span.size(); ++mask_idx) {
                auto mask = span[mask_idx];
                size_t mask_offset = 0;
                while (mask != 0) {
                    size_t offset = Ctz(mask);
                    mask = (mask >> offset) >> 1U;
                    mask_offset += offset;
                    size_t live_idx = mask_idx * sizeof(mask) * BITS_PER_BYTE + mask_offset;
                    ++mask_offset;  // consume current bit
                    ASSERT(live_idx < all_insts_.size());
                    ASSERT(all_insts_[live_idx] != nullptr);
                    cb(all_insts_[live_idx]);
                }
            }
        }

    private:
        Graph *graph_;
        // Live ins evaluation requires union of successor's live ins,
        // bit vector's union works faster than unordered set's union.
        ArenaVector<ArenaBitVector> live_in_;
        ArenaVector<Inst *> all_insts_;

        void AddInst(Inst *inst)
        {
            if (inst->GetId() >= all_insts_.size()) {
                all_insts_.resize(inst->GetId() + 1);
            }
            all_insts_[inst->GetId()] = inst;
        }

        void ProcessBlock(BasicBlock *block);
    };

    Marker visited_ {UNDEF_MARKER};
    ArenaVector<BasicBlockState *> block_states_;
    ArenaUnorderedMap<Inst *, StateOwner> aliases_;
    // list of instructions with corresponding virtual state values bound
    // to a save state or an allocation site
    ArenaUnorderedMap<MaterializationSite, ArenaUnorderedMap<Inst *, VirtualState *>> materialization_info_;
    ArenaVector<ArenaUnorderedMap<FieldPtr, PhiState *>> phis_;
    ArenaUnorderedMap<Inst *, ArenaBitVector> save_state_info_;
    ArenaUnorderedSet<Inst *> virtualizable_allocations_;
    MergeProcessor merge_processor_;
    LiveInAnalysis live_ins_;
    // 0 is materialized state
    StateId state_id_ = MATERIALIZED_ID + 1;
    bool virtualization_changed_ {false};
    bool relax_class_restrictions_ {false};

    void VisitCmpRef(Inst *inst, ConditionCode cc);
    void VisitSaveStateUser(Inst *inst);
    void VisitCall(CallInst *inst);
    void VisitNewObject(Inst *inst);
    void VisitLoadObject(Inst *inst);
    void VisitStoreObject(Inst *inst);
    void VisitNullCheck(Inst *inst);
    void VisitSaveState(Inst *inst);
    void VisitSafePoint(Inst *inst);
    void VisitGetInstanceClass(Inst *inst);
    void VisitBlockInternal(BasicBlock *block);
    void HandleMaterializationSite(Inst *inst);

    void MaterializeDeoptSaveState(Inst *inst);

    void Initialize();
    bool FindVirtualizableAllocations();
    bool AllPredecessorsVisited(const BasicBlock *block);

    bool IsRefField(FieldPtr field) const
    {
        return IsReference(GetGraph()->GetRuntime()->GetFieldType(field));
    }

    bool HasRefFields(const VirtualState *state) const;

    BasicBlockState *GetState(const BasicBlock *block) const
    {
        return block_states_[block->GetId()];
    }

    std::pair<PhiState *, bool> CreatePhi(BasicBlock *target_block, BasicBlockState *block_state, FieldPtr field,
                                          ArenaVector<StateOwner> &inputs);
    VirtualState *CreateVState(Inst *inst);
    VirtualState *CreateVState(Inst *inst, StateId id);

    void MaterializeInBlock(StateOwner inst, BasicBlock *block);
    void Materialize(StateOwner inst, BasicBlock *block);
    void Materialize(StateOwner inst, Inst *before);
    void Materialize(Inst *inst);
    void RegisterMaterialization(MaterializationSite site, Inst *inst);
    void RegisterVirtualObjectFieldsForMaterialization(Inst *ss);
    bool RegisterFieldsMaterialization(Inst *site, VirtualState *state, BasicBlockState *block_state,
                                       const ArenaUnorderedMap<Inst *, VirtualState *> &states);

    bool ProcessBlock(BasicBlock *block, size_t depth = 0);
    bool ProcessLoop(BasicBlock *header, size_t depth = 0);
    void FillVirtualInputs(Inst *inst);
    void AddVirtualizableAllocation(Inst *inst)
    {
        COMPILER_LOG(DEBUG, PEA) << "Candidate for virtualization: " << *inst;
        virtualizable_allocations_.insert(inst);
    }
    void RemoveVirtualizableAllocation(Inst *inst)
    {
        virtualizable_allocations_.erase(inst);
    }

    ArenaAllocator *GetLocalAllocator()
    {
        return GetGraph()->GetLocalAllocator();
    }
};

class ScalarReplacement {
public:
    ScalarReplacement(
        Graph *graph, ArenaUnorderedMap<Inst *, StateOwner> &aliases,
        ArenaVector<ArenaUnorderedMap<FieldPtr, PhiState *>> &phis,
        ArenaUnorderedMap<MaterializationSite, ArenaUnorderedMap<Inst *, VirtualState *>> &materialization_sites,
        ArenaUnorderedMap<Inst *, ArenaBitVector> &save_state_liveness)
        : graph_(graph),
          aliases_(aliases),
          phis_(phis),
          materialization_sites_(materialization_sites),
          save_state_liveness_(save_state_liveness),
          allocated_phis_(graph_->GetLocalAllocator()->Adapter()),
          materialized_objects_(graph_->GetLocalAllocator()->Adapter()),
          removal_queue_(graph_->GetLocalAllocator()->Adapter())
    {
    }

    ~ScalarReplacement() = default;
    NO_COPY_SEMANTIC(ScalarReplacement);
    NO_MOVE_SEMANTIC(ScalarReplacement);

    void Apply(ArenaUnorderedSet<Inst *> &candidates);

private:
    Graph *graph_;
    ArenaUnorderedMap<Inst *, StateOwner> &aliases_;
    ArenaVector<ArenaUnorderedMap<FieldPtr, PhiState *>> &phis_;
    ArenaUnorderedMap<MaterializationSite, ArenaUnorderedMap<Inst *, VirtualState *>> &materialization_sites_;
    ArenaUnorderedMap<Inst *, ArenaBitVector> &save_state_liveness_;

    ArenaUnorderedMap<PhiState *, PhiInst *> allocated_phis_;
    ArenaUnorderedMap<Inst *, ArenaVector<Inst *>> materialized_objects_;

    ArenaVector<Inst *> removal_queue_;
    Marker remove_inst_marker_ {UNDEF_MARKER};

    void ProcessRemovalQueue();
    bool IsEnqueuedForRemoval(Inst *inst) const;
    void EnqueueForRemoval(Inst *inst);
    void UpdateAllocationUsers();
    void UpdateSaveStates();
    Inst *ResolveAllocation(Inst *inst, BasicBlock *block);
    void ResolvePhiInputs();
    void ReplaceAliases();
    Inst *ResolveAlias(const StateOwner &alias, const Inst *inst);
    Inst *CreateNewObject(Inst *original_inst, Inst *save_state);
    void InitializeObject(Inst *alloc, Inst *inst_before, VirtualState *state);
    void MaterializeObjects();
    void MaterializeAtNewSaveState(Inst *site, ArenaUnorderedMap<Inst *, VirtualState *> &state);
    void MaterializeInEmptyBlock(BasicBlock *block, ArenaUnorderedMap<Inst *, VirtualState *> &state);
    void MaterializeAtExistingSaveState(SaveStateInst *save_state, ArenaUnorderedMap<Inst *, VirtualState *> &state);
    void CreatePhis();
    SaveStateInst *CopySaveState(Inst *inst, Inst *except);
    void PatchSaveStates();
    void PatchSaveStatesInBlock(BasicBlock *block, ArenaVector<ArenaUnorderedSet<Inst *>> &liveness);
    void PatchSaveStatesInLoop(Loop *loop, ArenaUnorderedSet<Inst *> &loop_live_ins,
                               ArenaVector<ArenaUnorderedSet<Inst *>> &liveness);
    void PatchSaveState(SaveStateInst *save_state, ArenaUnorderedSet<Inst *> &live_instructions);
    void AddLiveInputs(Inst *inst, ArenaUnorderedSet<Inst *> &live_ins);
    CallInst *FindCallerInst(BasicBlock *target, Inst *start = nullptr);
    void FixPhiInputTypes();
};
}  // namespace panda::compiler

#endif  // COMPILER_ANALYSIS_ESCAPE_H
