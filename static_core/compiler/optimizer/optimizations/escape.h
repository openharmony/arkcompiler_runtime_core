/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {
using FieldId = uint64_t;
using InstId = uint32_t;
using BlockId = uint32_t;
using StateId = uint32_t;
using FieldPtr = RuntimeInterface::FieldPtr;
using ClassPtr = RuntimeInterface::ClassPtr;

class VirtualState;
class BasicBlockState;
class PhiState;
class ScalarReplacement;
class ZeroInst;
struct Index;

using StateOwner = std::variant<Inst *, PhiState *, const ZeroInst *>;
using MaterializationSite = std::variant<Inst *, BasicBlock *>;
using Field = std::variant<FieldPtr, Index>;

struct Index {
    ClassPtr klass;  // NOLINT(misc-non-private-member-variables-in-classes)
    uint64_t index;  // NOLINT(misc-non-private-member-variables-in-classes)

    bool operator==(const Index &other) const
    {
        return klass == other.klass && index == other.index;
    }

    bool operator<(const Index &other) const
    {
        return klass < other.klass || (klass == other.klass && index < other.index);
    }
};

struct FieldComporator {
    bool operator()(const Field &field1, const Field &field2) const
    {
        return GetUniqVal(field1) < GetUniqVal(field2);
    }

private:
    uint64_t GetUniqVal(const Field &field) const
    {
        if (std::holds_alternative<FieldPtr>(field)) {
            return reinterpret_cast<uint64_t>(std::get<FieldPtr>(field));
        }
        auto index = std::get<Index>(field);
        // NOLINTNEXTLINE(readability-magic-numbers)
        return (reinterpret_cast<uint64_t>(index.klass) << 32U) | index.index;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class EscapeAnalysis : public Optimization, public GraphVisitor {
public:
    static constexpr StateId MATERIALIZED_ID = 0;

    explicit EscapeAnalysis(Graph *graph)
        : Optimization(graph),
          blockStates_(graph->GetLocalAllocator()->Adapter()),
          aliases_(graph->GetLocalAllocator()->Adapter()),
          materializationInfo_(graph->GetLocalAllocator()->Adapter()),
          phis_(graph->GetLocalAllocator()->Adapter()),
          saveStateInfo_(graph->GetLocalAllocator()->Adapter()),
          virtualizableAllocations_(graph->GetLocalAllocator()->Adapter()),
          mergeProcessor_(this),
          liveIns_(graph)
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
        return g_options.IsCompilerScalarReplacement();
    }

    static bool IsAllocInst(Inst *inst)
    {
        return inst->GetOpcode() == Opcode::NewObject || inst->GetOpcode() == Opcode::NewArray;
    }
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_VISIT(InstName)                                   \
    static void Visit##InstName(GraphVisitor *v, Inst *inst)     \
    {                                                            \
        static_cast<EscapeAnalysis *>(v)->Visit##InstName(inst); \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_VISIT_WITH_CALLBACK(InstName, Callback)       \
    static void Visit##InstName(GraphVisitor *v, Inst *inst) \
    {                                                        \
        static_cast<EscapeAnalysis *>(v)->Callback(inst);    \
    }

    DEFINE_VISIT(NewObject);
    DEFINE_VISIT(NewArray);
    DEFINE_VISIT(LoadObject);
    DEFINE_VISIT(LoadArray);
    DEFINE_VISIT(StoreObject);
    DEFINE_VISIT(StoreArray);
    DEFINE_VISIT(NullCheck);
    DEFINE_VISIT(SaveState);
    DEFINE_VISIT(SafePoint);
    // NOTE(schernykh): support it
    // DEFINE_VISIT(SaveStateDeoptimize)
    DEFINE_VISIT(GetInstanceClass);

    DEFINE_VISIT_WITH_CALLBACK(Deoptimize, MaterializeDeoptSaveState);
    DEFINE_VISIT_WITH_CALLBACK(DeoptimizeIf, MaterializeDeoptSaveState);
    DEFINE_VISIT_WITH_CALLBACK(DeoptimizeCompare, MaterializeDeoptSaveState);
    DEFINE_VISIT_WITH_CALLBACK(DeoptimizeCompareImm, MaterializeDeoptSaveState);
    DEFINE_VISIT_WITH_CALLBACK(LoadAndInitClass, VisitSaveStateUser);
    DEFINE_VISIT_WITH_CALLBACK(LoadClass, VisitSaveStateUser);

#undef DEFINE_VISIT
#undef DEFINE_VISIT_WITH_CALLBACK

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
    void VisitAllocation(Inst *inst);

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        UNREACHABLE();
    }

    // only for testing
    void RelaxClassRestrictions()
    {
        relaxClassRestrictions_ = true;
    }

#include "optimizer/ir/visitor.inc"
private:
    static constexpr InstId ZERO_INST_ID = std::numeric_limits<InstId>::max() - 1U;
    static constexpr size_t MAX_NESTNESS = 5;

    class MergeProcessor {
    public:
        explicit MergeProcessor(EscapeAnalysis *parent)
            : parent_(parent),
              fieldsMergeBuffer_(parent->GetLocalAllocator()->Adapter()),
              statesMergeBuffer_(parent->GetLocalAllocator()->Adapter()),
              allFields_(parent->GetLocalAllocator()->Adapter()),
              pendingInsts_(parent->GetLocalAllocator()->Adapter())
        {
        }
        ~MergeProcessor() = default;
        NO_COPY_SEMANTIC(MergeProcessor);
        NO_MOVE_SEMANTIC(MergeProcessor);

        // Compute initial state of a block by merging states of its predecessors.
        void MergeStates(BasicBlock *block);

    private:
        EscapeAnalysis *parent_;
        ArenaVector<StateOwner> fieldsMergeBuffer_;
        ArenaVector<StateOwner> statesMergeBuffer_;
        ArenaVector<Field> allFields_;
        ArenaVector<StateOwner> pendingInsts_;

        bool MergeFields(BasicBlock *block, BasicBlockState *blockState, StateId stateToMerge, VirtualState *vstate,
                         ArenaVector<StateOwner> &mergeQueue);
        bool MergeState(StateOwner inst, BasicBlock *block, BasicBlockState *newState);
        void CheckStatesAndInsertIntoBuffer(StateOwner inst, BasicBlock *block);
        void MaterializeObjectsAtTheBeginningOfBlock(BasicBlock *block);
        void CollectInstructionsToMerge(BasicBlock *block);
        void ProcessPhis(BasicBlock *block, BasicBlockState *newState);
    };

    class LiveInAnalysis {
    public:
        explicit LiveInAnalysis(Graph *graph)
            : graph_(graph),
              liveIn_(graph_->GetLocalAllocator()->Adapter()),
              allInsts_(graph_->GetLocalAllocator()->Adapter())
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
            auto &liveIns = liveIn_[block->GetId()];
            auto span = liveIns.GetContainerDataSpan();
            for (size_t maskIdx = 0; maskIdx < span.size(); ++maskIdx) {
                auto mask = span[maskIdx];
                size_t maskOffset = 0;
                while (mask != 0) {
                    auto offset = static_cast<size_t>(Ctz(mask));
                    mask = (mask >> offset) >> 1U;
                    maskOffset += offset;
                    size_t liveIdx = maskIdx * sizeof(mask) * BITS_PER_BYTE + maskOffset;
                    ++maskOffset;  // consume current bit
                    ASSERT(liveIdx < allInsts_.size());
                    ASSERT(allInsts_[liveIdx] != nullptr);
                    cb(allInsts_[liveIdx]);
                }
            }
        }

    private:
        Graph *graph_;
        // Live ins evaluation requires union of successor's live ins,
        // bit vector's union works faster than set's union.
        ArenaVector<ArenaBitVector> liveIn_;
        ArenaVector<Inst *> allInsts_;

        void AddInst(Inst *inst)
        {
            if (inst->GetId() >= allInsts_.size()) {
                allInsts_.resize(inst->GetId() + 1);
            }
            allInsts_[inst->GetId()] = inst;
        }

        void ProcessBlock(BasicBlock *block);
    };

    Marker visited_ {UNDEF_MARKER};
    ArenaVector<BasicBlockState *> blockStates_;
    ArenaMap<Inst *, StateOwner> aliases_;
    // list of instructions with corresponding virtual state values bound
    // to a save state or an allocation site
    ArenaUnorderedMap<MaterializationSite, ArenaMap<Inst *, VirtualState *>> materializationInfo_;
    ArenaVector<ArenaMap<Field, PhiState *, FieldComporator>> phis_;
    ArenaUnorderedMap<Inst *, ArenaBitVector> saveStateInfo_;
    ArenaSet<Inst *> virtualizableAllocations_;
    MergeProcessor mergeProcessor_;
    LiveInAnalysis liveIns_;
    // 0 is materialized state
    StateId stateId_ = MATERIALIZED_ID + 1;
    bool virtualizationChanged_ {false};
    bool relaxClassRestrictions_ {false};

    void VisitCmpRef(Inst *inst, ConditionCode cc);
    void VisitSaveStateUser(Inst *inst);
    void VisitCall(CallInst *inst);
    void VisitNewObject(Inst *inst);
    void VisitLoadObject(Inst *inst);
    void VisitStoreObject(Inst *inst);
    void VisitNewArray(Inst *inst);
    void VisitStoreArray(Inst *inst);
    void VisitLoadArray(Inst *inst);
    void VisitNullCheck(Inst *inst);
    void VisitSaveState(Inst *inst);
    void VisitSaveStateDeoptimize(Inst *inst);
    void VisitSafePoint(Inst *inst);
    void VisitGetInstanceClass(Inst *inst);
    void VisitBlockInternal(BasicBlock *block);
    void HandleMaterializationSite(Inst *inst);

    void MaterializeDeoptSaveState(Inst *inst);

    void Initialize();
    void PropagateLoadObjectsUpwards();
    void CreateTemporaryLoads(Inst *phi);
    bool FindVirtualizableAllocations();
    bool AllPredecessorsVisited(const BasicBlock *block);
#ifndef NDEBUG
    Marker removableLoads_ {UNDEF_MARKER};
    bool EnsureLoadsRemoval();
#endif

    bool IsRefField(FieldPtr field) const
    {
        return IsReference(GetGraph()->GetRuntime()->GetFieldType(field));
    }

    BasicBlockState *GetState(const BasicBlock *block) const
    {
        return blockStates_[block->GetId()];
    }

    std::pair<PhiState *, bool> CreatePhi(BasicBlock *targetBlock, BasicBlockState *blockState, Field field,
                                          ArenaVector<StateOwner> &inputs, VirtualState *vstate);
    VirtualState *CreateVState(Inst *inst);
    VirtualState *CreateVState(Inst *inst, StateId id);

    void MaterializeInBlock(StateOwner inst, BasicBlock *block);
    void Materialize(StateOwner inst, BasicBlock *block);
    void Materialize(StateOwner inst, Inst *before);
    void Materialize(Inst *inst);
    void RegisterMaterialization(MaterializationSite site, Inst *inst);
    void RegisterVirtualObjectFieldsForMaterialization(Inst *ss);
    bool RegisterFieldsMaterialization(Inst *site, VirtualState *state, BasicBlockState *blockState,
                                       const ArenaMap<Inst *, VirtualState *> &states);

    bool ProcessBlock(BasicBlock *block, size_t depth = 0);
    bool ProcessLoop(BasicBlock *header, size_t depth = 0);
    void FillVirtualInputs(Inst *inst);
    void AddVirtualizableAllocation(Inst *inst)
    {
        COMPILER_LOG(DEBUG, PEA) << "Candidate for virtualization: " << *inst;
        virtualizableAllocations_.insert(inst);
    }
    void RemoveVirtualizableAllocation(Inst *inst)
    {
        virtualizableAllocations_.erase(inst);
    }

    ArenaAllocator *GetLocalAllocator()
    {
        return GetGraph()->GetLocalAllocator();
    }

    void DumpVStates();
    void DumpMStates();
    void DumpAliases();
    void Dump();
};

class ScalarReplacement {
public:
    ScalarReplacement(Graph *graph, ArenaMap<Inst *, StateOwner> &aliases,
                      ArenaVector<ArenaMap<Field, PhiState *, FieldComporator>> &phis,
                      ArenaUnorderedMap<MaterializationSite, ArenaMap<Inst *, VirtualState *>> &materializationSites,
                      ArenaUnorderedMap<Inst *, ArenaBitVector> &saveStateLiveness)
        : graph_(graph),
          aliases_(aliases),
          phis_(phis),
          materializationSites_(materializationSites),
          saveStateLiveness_(saveStateLiveness),
          allocatedPhis_(graph_->GetLocalAllocator()->Adapter()),
          materializedObjects_(graph_->GetLocalAllocator()->Adapter()),
          removalQueue_(graph_->GetLocalAllocator()->Adapter())
    {
    }

    ~ScalarReplacement() = default;
    NO_COPY_SEMANTIC(ScalarReplacement);
    NO_MOVE_SEMANTIC(ScalarReplacement);

    void Apply(ArenaSet<Inst *> &candidates);

private:
    Graph *graph_;
    ArenaMap<Inst *, StateOwner> &aliases_;
    ArenaVector<ArenaMap<Field, PhiState *, FieldComporator>> &phis_;
    ArenaUnorderedMap<MaterializationSite, ArenaMap<Inst *, VirtualState *>> &materializationSites_;
    ArenaUnorderedMap<Inst *, ArenaBitVector> &saveStateLiveness_;

    ArenaMap<PhiState *, PhiInst *> allocatedPhis_;
    ArenaMap<Inst *, ArenaVector<Inst *>> materializedObjects_;

    ArenaVector<Inst *> removalQueue_;
    Marker removeInstMarker_ {UNDEF_MARKER};

    void ProcessRemovalQueue();
    bool IsEnqueuedForRemoval(Inst *inst) const;
    void EnqueueForRemoval(Inst *inst);
    void UpdateAllocationUsers();
    void UpdateSaveStates();
    Inst *ResolveAllocation(Inst *inst, BasicBlock *block);
    void ResolvePhiInputs();
    void ReplaceAliases();
    Inst *ResolveAlias(const StateOwner &alias, const Inst *inst);
    Inst *CreateNewObject(Inst *originalInst, Inst *saveState);
    Inst *CreateNewArray(Inst *originalInst, Inst *saveState);
    void InitializeObject(Inst *alloc, Inst *instBefore, VirtualState *state);
    void MaterializeObjects();
    void Materialize(Inst *originalInst, Inst *ssAlloc, Inst *ssInit, VirtualState *state);
    void MaterializeAtNewSaveState(Inst *site, ArenaMap<Inst *, VirtualState *> &state);
    void MaterializeInEmptyBlock(BasicBlock *block, ArenaMap<Inst *, VirtualState *> &state);
    void MaterializeAtExistingSaveState(SaveStateInst *saveState, ArenaMap<Inst *, VirtualState *> &state);
    void CreatePhis();
    SaveStateInst *CopySaveState(Inst *inst, VirtualState *except);
    void PatchSaveStates();
    void PatchSaveStatesInBlock(BasicBlock *block, ArenaVector<ArenaSet<Inst *>> &liveness);
    void PatchSaveStatesInLoop(Loop *loop, ArenaSet<Inst *> &loopLiveIns, ArenaVector<ArenaSet<Inst *>> &liveness);
    void FillLiveInsts(BasicBlock *block, ArenaSet<Inst *> &liveIns, ArenaVector<ArenaSet<Inst *>> &liveness);
    void PatchSaveState(SaveStateInst *saveState, ArenaSet<Inst *> &liveInstructions);
    void AddLiveInputs(Inst *inst, ArenaSet<Inst *> &liveIns);
    CallInst *FindCallerInst(BasicBlock *target, Inst *start = nullptr);
    void FixPhiInputTypes();
    bool HasUsageOutsideBlock(Inst *inst, BasicBlock *initialBlock);
};
}  // namespace ark::compiler

#endif  // COMPILER_ANALYSIS_ESCAPE_H
