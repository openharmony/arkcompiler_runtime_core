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

#include <algorithm>
#include "optimizer/ir/basicblock.h"
#include "optimizer/optimizations/escape.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/inst.h"
#include "compiler/compiler_logger.h"

namespace ark::compiler {
namespace {
constexpr ZeroInst *ZERO_INST {nullptr};
}  // namespace

class VirtualState {
public:
    VirtualState(Inst *inst, StateId id, ArenaAllocator *alloc)
        : inst_(inst), id_(id), fields_(alloc->Adapter()), aliases_(alloc->Adapter())
    {
        AddAlias(inst);
    }

    ~VirtualState() = default;
    NO_COPY_SEMANTIC(VirtualState);
    NO_MOVE_SEMANTIC(VirtualState);

    void SetField(FieldPtr field, StateOwner inst)
    {
        fields_[field] = inst;
    }

    StateOwner GetFieldOrDefault(FieldPtr field, StateOwner defaultValue) const
    {
        auto it = fields_.find(field);
        if (it == fields_.end()) {
            return defaultValue;
        }
        return it->second;
    }

    const ArenaMap<FieldPtr, StateOwner> &GetFields() const
    {
        return fields_;
    }

    StateId GetId() const
    {
        return id_;
    }

    VirtualState *Copy(ArenaAllocator *alloc) const
    {
        auto copy = alloc->New<VirtualState>(inst_, id_, alloc);
        if (copy == nullptr) {
            UNREACHABLE();
        }
        for (auto [ptr, id] : fields_) {
            copy->SetField(ptr, id);
        }
        copy->aliases_ = aliases_;
        return copy;
    }

    Inst *GetInst() const
    {
        return inst_;
    }

    void AddAlias(Inst *inst)
    {
        auto it = std::find(aliases_.begin(), aliases_.end(), inst);
        if (it != aliases_.end()) {
            return;
        }
        aliases_.push_back(inst);
    }

    bool Equals(const VirtualState *other) const
    {
        if (other == nullptr) {
            return false;
        }
        if (fields_ == other->fields_) {
            ASSERT(aliases_ == other->aliases_);
            return true;
        }
        return false;
    }

    const ArenaVector<Inst *> &GetAliases() const
    {
        return aliases_;
    }

private:
    Inst *inst_;
    StateId id_;
    ArenaMap<FieldPtr, StateOwner> fields_;
    ArenaVector<Inst *> aliases_;
};

class PhiState {
public:
    PhiState(ArenaAllocator *alloc, DataType::Type type) : inputs_(alloc->Adapter()), type_(type) {}

    ~PhiState() = default;
    NO_COPY_SEMANTIC(PhiState);
    NO_MOVE_SEMANTIC(PhiState);

    void AddInput(StateOwner input)
    {
        inputs_.push_back(input);
    }

    void SetInput(size_t idx, StateOwner input)
    {
        ASSERT(idx < inputs_.size());
        inputs_[idx] = input;
    }

    const ArenaVector<StateOwner> &GetInputs() const
    {
        return inputs_;
    }

    bool IsReference() const
    {
        return DataType::IsReference(type_);
    }

    DataType::Type GetType() const
    {
        return type_;
    }

private:
    ArenaVector<StateOwner> inputs_;
    DataType::Type type_;
};

class BasicBlockState {
public:
    explicit BasicBlockState(ArenaAllocator *alloc) : states_(alloc->Adapter()), stateValues_(alloc->Adapter()) {}

    ~BasicBlockState() = default;
    NO_COPY_SEMANTIC(BasicBlockState);
    NO_MOVE_SEMANTIC(BasicBlockState);

    StateId GetStateId(StateOwner inst) const
    {
        if (!std::holds_alternative<Inst *>(inst)) {
            return EscapeAnalysis::MATERIALIZED_ID;
        }
        auto it = states_.find(std::get<Inst *>(inst));
        return it == states_.end() ? EscapeAnalysis::MATERIALIZED_ID : it->second;
    }

    void SetStateId(const StateOwner &inst, StateId state)
    {
        ASSERT(std::holds_alternative<Inst *>(inst));
        auto instInst = std::get<Inst *>(inst);
        if (state != EscapeAnalysis::MATERIALIZED_ID) {
            auto vstate = stateValues_[state];
            vstate->AddAlias(instInst);
            states_[instInst] = state;
        } else {
            states_.erase(instInst);
        }
    }

    void SetState(const StateOwner &inst, VirtualState *vstate)
    {
        ASSERT(std::holds_alternative<Inst *>(inst));
        auto instInst = std::get<Inst *>(inst);
        states_[instInst] = vstate->GetId();
        if (stateValues_.size() <= vstate->GetId()) {
            stateValues_.resize(vstate->GetId() + 1);
        }
        vstate->AddAlias(instInst);
        stateValues_[vstate->GetId()] = vstate;
    }

    bool HasState(const StateOwner &inst) const
    {
        if (std::holds_alternative<Inst *>(inst)) {
            return states_.find(std::get<Inst *>(inst)) != states_.end();
        }
        return false;
    }

    bool HasStateWithId(StateId inst) const
    {
        return stateValues_.size() > inst && stateValues_[inst] != nullptr;
    }

    VirtualState *GetStateById(StateId state) const
    {
        if (state == EscapeAnalysis::MATERIALIZED_ID) {
            return nullptr;
        }
        ASSERT(state < stateValues_.size());
        auto value = stateValues_[state];
        ASSERT(value != nullptr);
        return value;
    }

    VirtualState *GetState(const StateOwner &inst) const
    {
        return GetStateById(GetStateId(inst));
    }

    const ArenaMap<Inst *, StateId> &GetStates() const
    {
        return states_;
    }

    void Materialize(const StateOwner &inst)
    {
        if (!std::holds_alternative<Inst *>(inst)) {
            return;
        }
        auto instInst = std::get<Inst *>(inst);
        auto it = states_.find(instInst);
        if (it == states_.end()) {
            return;
        }
        auto stateId = it->second;
        if (auto vstate = stateValues_[stateId]) {
            for (auto &alias : vstate->GetAliases()) {
                states_.erase(alias);
            }
            states_.erase(vstate->GetInst());
        } else {
            states_.erase(instInst);
        }
        stateValues_[stateId] = nullptr;
    }

    void Clear()
    {
        states_.clear();
        stateValues_.clear();
    }

    BasicBlockState *Copy(ArenaAllocator *alloc)
    {
        auto copy = alloc->New<BasicBlockState>(alloc);
        if (copy == nullptr) {
            UNREACHABLE();
        }
        for (auto &[k, v] : states_) {
            copy->states_[k] = v;
        }
        copy->stateValues_.resize(stateValues_.size());
        for (StateId id = 0; id < stateValues_.size(); ++id) {
            if (auto v = stateValues_[id]) {
                copy->stateValues_[id] = v->Copy(alloc);
            }
        }
        return copy;
    }

    bool Equals(const BasicBlockState *other) const
    {
        if (states_ != other->states_) {
            return false;
        }

        StateId id;
        for (id = 0; id < std::min<size_t>(stateValues_.size(), other->stateValues_.size()); ++id) {
            auto v = stateValues_[id];
            auto otherState = other->stateValues_[id];
            if ((v == nullptr) != (otherState == nullptr)) {
                return false;
            }
            if (v == nullptr) {
                continue;
            }
            if (!v->Equals(otherState)) {
                return false;
            }
        }
        for (; id < stateValues_.size(); ++id) {
            if (stateValues_[id] != nullptr) {
                return false;
            }
        }
        for (; id < other->stateValues_.size(); ++id) {
            if (other->stateValues_[id] != nullptr) {
                return false;
            }
        }

        return true;
    }

private:
    ArenaMap<Inst *, StateId> states_;
    ArenaVector<VirtualState *> stateValues_;
};

void EscapeAnalysis::LiveInAnalysis::ProcessBlock(BasicBlock *block)
{
    auto &liveIns = liveIn_[block->GetId()];

    for (auto succ : block->GetSuccsBlocks()) {
        liveIns |= liveIn_[succ->GetId()];
        for (auto phiInst : succ->PhiInsts()) {
            auto phiInput = phiInst->CastToPhi()->GetPhiInput(block);
            if (phiInput->GetBasicBlock() != block && DataType::IsReference(phiInput->GetType())) {
                liveIns.SetBit(phiInput->GetId());
                AddInst(phiInput);
            }
        }
    }

    for (auto inst : block->InstsReverse()) {
        if (DataType::IsReference(inst->GetType())) {
            liveIns.ClearBit(inst->GetId());
        }
        for (auto &input : inst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (!DataType::IsReference(inputInst->GetType())) {
                continue;
            }
            liveIns.SetBit(inputInst->GetId());
            AddInst(inputInst);
            auto dfInput = inst->GetDataFlowInput(inputInst);
            if (dfInput != inputInst) {
                liveIns.SetBit(dfInput->GetId());
                AddInst(dfInput);
            }
        }
    }

    // Loop header's live-ins are alive throughout the whole loop.
    // If current block is a header then propagate its live-ins
    // to all current loop's blocks as well as to all inner loops.
    auto loop = block->GetLoop();
    if (loop->IsRoot() || loop->GetHeader() != block) {
        return;
    }
    for (auto loopBlock : graph_->GetVectorBlocks()) {
        if (loopBlock == nullptr || loopBlock == block) {
            continue;
        }
        if (loopBlock->GetLoop() == loop || loopBlock->GetLoop()->IsInside(loop)) {
            auto &loopBlockLiveIns = liveIn_[loopBlock->GetId()];
            loopBlockLiveIns |= liveIns;
        }
    }
}

bool EscapeAnalysis::LiveInAnalysis::Run()
{
    bool hasAllocs = false;
    for (auto block : graph_->GetBlocksRPO()) {
        if (hasAllocs) {
            break;
        }
        for (auto inst : block->Insts()) {
            hasAllocs = hasAllocs || inst->GetOpcode() == Opcode::NewObject;
            if (hasAllocs) {
                break;
            }
        }
    }
    if (!hasAllocs) {
        return false;
    }

    liveIn_.clear();
    for (size_t idx = 0; idx < graph_->GetVectorBlocks().size(); ++idx) {
        liveIn_.emplace_back(graph_->GetLocalAllocator());
    }

    auto &rpo = graph_->GetBlocksRPO();
    for (auto it = rpo.rbegin(), end = rpo.rend(); it != end; ++it) {
        ProcessBlock(*it);
    }
    return true;
}

// Create state corresponding to the beginning of the basic block by merging
// states of its predecessors.
void EscapeAnalysis::MergeProcessor::MergeStates(BasicBlock *block)
{
    if (block->GetPredsBlocks().empty()) {
        return;
    }

    auto newState = parent_->GetLocalAllocator()->New<BasicBlockState>(parent_->GetLocalAllocator());
    if (newState == nullptr) {
        UNREACHABLE();
    }

    // Materialization of some fields may require further materalialization of other objects.
    // Repeat merging process until no new materialization will happen.
    bool stateChanged = true;
    while (stateChanged) {
        stateChanged = false;
        newState->Clear();
        ProcessPhis(block, newState);

        statesMergeBuffer_.clear();
        CollectInstructionsToMerge(block);

        while (!statesMergeBuffer_.empty()) {
            pendingInsts_.clear();
            for (auto &inst : statesMergeBuffer_) {
                stateChanged = stateChanged || MergeState(inst, block, newState);
            }
            statesMergeBuffer_.clear();
            statesMergeBuffer_.swap(pendingInsts_);
        }
        parent_->blockStates_[block->GetId()] = newState;
    }
    MaterializeObjectsAtTheBeginningOfBlock(block);
}

void EscapeAnalysis::MergeProcessor::ProcessPhis(BasicBlock *block, BasicBlockState *newState)
{
    for (auto phi : block->PhiInsts()) {
        if (!DataType::IsReference(phi->GetType())) {
            continue;
        }
        for (size_t inputIdx = 0; inputIdx < phi->GetInputsCount(); ++inputIdx) {
            auto bb = phi->CastToPhi()->GetPhiInputBb(inputIdx);
            parent_->Materialize(phi->GetInput(inputIdx).GetInst(), bb);
        }
        newState->Materialize(phi);
    }
}

void EscapeAnalysis::MergeProcessor::CheckStatesAndInsertIntoBuffer(StateOwner inst, BasicBlock *block)
{
    for (auto predBlock : block->GetPredsBlocks()) {
        auto predState = parent_->GetState(predBlock);
        if (!predState->HasState(inst)) {
            return;
        }
    }
    statesMergeBuffer_.push_back(inst);
}

void EscapeAnalysis::MergeProcessor::CollectInstructionsToMerge(BasicBlock *block)
{
    parent_->liveIns_.VisitAlive(block, [&buffer = statesMergeBuffer_](auto inst) { buffer.push_back(inst); });
}

bool EscapeAnalysis::MergeProcessor::MergeState(StateOwner inst, BasicBlock *block, BasicBlockState *newState)
{
    auto &predsBlocks = block->GetPredsBlocks();
    auto commonId = parent_->GetState(predsBlocks.front())->GetStateId(inst);
    // Materialization is required if predecessors have different
    // states for the same instruction or all predecessors have materialized state for it.
    bool materialize = commonId == EscapeAnalysis::MATERIALIZED_ID;
    bool newMaterialization = false;
    for (auto it = std::next(predsBlocks.begin()), end = predsBlocks.end(); it != end; ++it) {
        auto predBlock = *it;
        auto predId = parent_->GetState(predBlock)->GetStateId(inst);
        if (predId == EscapeAnalysis::MATERIALIZED_ID || predId != commonId) {
            materialize = true;
            break;
        }
    }

    if (materialize) {
        newMaterialization =
            parent_->GetState(block->GetDominator())->GetStateId(inst) != EscapeAnalysis::MATERIALIZED_ID;
        parent_->Materialize(inst, block->GetDominator());
        newState->Materialize(inst);
        return newMaterialization;
    }

    if (newState->HasStateWithId(commonId)) {
        // we already handled that vstate so we should skip further processing
        newState->SetStateId(inst, commonId);
        return false;
    }

    auto vstate = parent_->CreateVState(
        parent_->GetState(block->GetPredsBlocks().front())->GetStateById(commonId)->GetInst(), commonId);
    newMaterialization = MergeFields(block, newState, commonId, vstate, pendingInsts_);
    newState->SetState(inst, vstate);
    // if inst is an alias then we also need to process original inst
    pendingInsts_.push_back(vstate->GetInst());
    return newMaterialization;
}

bool EscapeAnalysis::MergeProcessor::MergeFields(BasicBlock *block, BasicBlockState *blockState, StateId stateToMerge,
                                                 VirtualState *vstate, ArenaVector<StateOwner> &mergeQueue)
{
    allFields_.clear();
    for (auto pred : block->GetPredsBlocks()) {
        for (auto &field : parent_->GetState(pred)->GetStateById(stateToMerge)->GetFields()) {
            allFields_.push_back(field.first);
        }
    }
    std::sort(allFields_.begin(), allFields_.end());
    auto end = std::unique(allFields_.begin(), allFields_.end());

    for (auto it = allFields_.begin(); it != end; ++it) {
        auto field = *it;
        fieldsMergeBuffer_.clear();
        // Use ZERO_INST as a placeholder for a field that was not set.
        // When it'll come to scalar replacement this placeholder will be
        // replaced with actual zero or nullptr const.
        StateOwner commonId = parent_->GetState(block->GetPredsBlocks().front())
                                  ->GetStateById(stateToMerge)
                                  ->GetFieldOrDefault(field, ZERO_INST);
        bool needMerge = false;
        for (auto predBlock : block->GetPredsBlocks()) {
            auto predState = parent_->GetState(predBlock)->GetStateById(stateToMerge);
            auto predFieldValue = predState->GetFieldOrDefault(field, ZERO_INST);
            if (commonId != predFieldValue) {
                needMerge = true;
            }
            fieldsMergeBuffer_.push_back(predFieldValue);
        }
        if (!needMerge) {
            vstate->SetField(field, commonId);
            // enqueue inst id for state copying
            if (!blockState->HasState(commonId)) {
                mergeQueue.push_back(commonId);
            }
            continue;
        }
        auto [phi, new_materialization] = parent_->CreatePhi(block, blockState, field, fieldsMergeBuffer_);
        vstate->SetField(field, phi);
        if (new_materialization) {
            return true;
        }
    }
    return false;
}

void EscapeAnalysis::MergeProcessor::MaterializeObjectsAtTheBeginningOfBlock(BasicBlock *block)
{
    // Find all objects that should be materialized at the beginning of given block.
    auto mIt = parent_->materializationInfo_.find(block);
    auto blockState = parent_->GetState(block);
    if (mIt == parent_->materializationInfo_.end()) {
        return;
    }
    auto &matStates = mIt->second;
    for (auto statesIt = matStates.begin(); statesIt != matStates.end();) {
        auto inst = statesIt->first;
        // If the instruction has virtual state then materialize it and save
        // copy of a virtual state (it will be used to initialize fields after allocation).
        // Otherwise remove the instruction from list of instruction requiring materialization
        // at this block.
        if (blockState->GetStateId(inst) != EscapeAnalysis::MATERIALIZED_ID) {
            statesIt->second = blockState->GetState(inst)->Copy(parent_->GetLocalAllocator());
            blockState->Materialize(inst);
            ++statesIt;
        } else {
            statesIt = matStates.erase(statesIt);
        }
    }
}

void EscapeAnalysis::Initialize()
{
    auto &blocks = GetGraph()->GetVectorBlocks();
    blockStates_.resize(blocks.size());
    phis_.reserve(blocks.size());
    for (auto block : blocks) {
        if (block != nullptr) {
            if (auto newBlock = GetLocalAllocator()->New<BasicBlockState>(GetLocalAllocator())) {
                blockStates_[block->GetId()] = newBlock;
            } else {
                UNREACHABLE();
            }
        }
        phis_.emplace_back(GetLocalAllocator()->Adapter());
    }

    if (!GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        GetGraph()->RunPass<LoopAnalyzer>();
    }
    if (!GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        GetGraph()->RunPass<DominatorsTree>();
    }
}

bool EscapeAnalysis::RunImpl()
{
    if (!GetGraph()->HasEndBlock()) {
        return false;
    }
    Initialize();

    if (!liveIns_.Run()) {
        return false;
    }

    if (!FindVirtualizableAllocations()) {
        return false;
    }

    if (virtualizableAllocations_.empty()) {
        COMPILER_LOG(DEBUG, PEA) << "No allocations to virtualize";
        return false;
    }

    ScalarReplacement sr {GetGraph(), aliases_, phis_, materializationInfo_, saveStateInfo_};
    sr.Apply(virtualizableAllocations_);
    return true;
}

bool EscapeAnalysis::AllPredecessorsVisited(const BasicBlock *block)
{
    for (auto pred : block->GetPredsBlocks()) {
        if (!pred->IsMarked(visited_)) {
            return false;
        }
    }
    return true;
}

bool EscapeAnalysis::HasRefFields(const VirtualState *state) const
{
    if (state == nullptr) {
        return false;
    }
    for (auto &kv : state->GetFields()) {
        if (IsRefField(kv.first)) {
            return true;
        }
    }
    return false;
}

bool EscapeAnalysis::FindVirtualizableAllocations()
{
    virtualizationChanged_ = true;
    while (virtualizationChanged_) {
        stateId_ = 1U;
        virtualizationChanged_ = false;
        MarkerHolder mh {GetGraph()};
        visited_ = mh.GetMarker();

        for (auto block : GetGraph()->GetBlocksRPO()) {
            if (!ProcessBlock(block)) {
                return false;
            }
        }
    }
    return true;
}

void EscapeAnalysis::MaterializeDeoptSaveState(Inst *inst)
{
    auto ss = inst->GetSaveState();
    if (ss == nullptr) {
        return;
    }
    // If deoptimization is happening inside inlined call then
    // all objects captured in inlined call's save state should be materialized
    // to correctly restore values of virtual registers in interpreted frame during deoptimization.
    if (auto caller = ss->GetCallerInst()) {
        MaterializeDeoptSaveState(caller);
    }
    for (auto input : ss->GetInputs()) {
        auto blockState = GetState(ss->GetBasicBlock());
        // We're trying to materialize an object at the save state predecessing current instruction,
        // as a result the object might be alredy materialized in this basic block. If it actually is
        // then register its materialization at current save state to ensure that on the next iteration
        // it will be materialized here. Otherwise try to find a state owner and materialize it instead
        // of an alias.
        if (auto state = blockState->GetState(input.GetInst())) {
            RegisterMaterialization(ss, state->GetInst());
            blockState->Materialize(input.GetInst());
        } else {
            RegisterMaterialization(ss, input.GetInst());
        }
    }
    Materialize(inst);
}

std::pair<PhiState *, bool> EscapeAnalysis::CreatePhi(BasicBlock *targetBlock, BasicBlockState *blockState,
                                                      FieldPtr field, ArenaVector<StateOwner> &inputs)
{
    // try to reuse existing phi
    auto it = phis_.at(targetBlock->GetId()).find(field);
    if (it != phis_.at(targetBlock->GetId()).end()) {
        auto phi = it->second;
        for (size_t idx = 0; idx < inputs.size(); ++idx) {
            ASSERT(GetState(targetBlock->GetPredsBlocks()[idx])->GetStateId(inputs[idx]) == MATERIALIZED_ID);
            phi->SetInput(idx, inputs[idx]);
        }
        blockState->Materialize(phi);
        return std::make_pair(phi, false);
    }
    auto fieldType = GetGraph()->GetRuntime()->GetFieldType(field);
    auto phiState = GetLocalAllocator()->New<PhiState>(GetLocalAllocator(), fieldType);
    if (phiState == nullptr) {
        UNREACHABLE();
    }
    auto &preds = targetBlock->GetPredsBlocks();
    ASSERT(inputs.size() == preds.size());
    bool newMaterialization = false;
    for (size_t idx = 0; idx < inputs.size(); ++idx) {
        phiState->AddInput(inputs[idx]);
        newMaterialization = newMaterialization || GetState(preds[idx])->GetStateId(inputs[idx]) != MATERIALIZED_ID;
        Materialize(inputs[idx], preds[idx]);
    }
    if (phiState->IsReference()) {
        // phi is always materialized
        blockState->Materialize(phiState);
    }
    phis_.at(targetBlock->GetId())[field] = phiState;
    return std::make_pair(phiState, newMaterialization);
}

void EscapeAnalysis::MaterializeInBlock(StateOwner inst, BasicBlock *block)
{
    auto blockState = GetState(block);
    inst = blockState->GetState(inst)->GetInst();
    if (blockState->GetStateId(inst) == EscapeAnalysis::MATERIALIZED_ID) {
        return;
    }
    RegisterMaterialization(block, std::get<Inst *>(inst));
    auto instState = blockState->GetState(inst);
    blockState->Materialize(inst);
    for (auto &t : instState->GetFields()) {
        auto &fieldInst = t.second;
        if (blockState->GetStateId(fieldInst) == MATERIALIZED_ID) {
            continue;
        }
        MaterializeInBlock(fieldInst, block);
    }
}

void EscapeAnalysis::Materialize(StateOwner inst, BasicBlock *block)
{
    if (GetState(block)->GetStateId(inst) == EscapeAnalysis::MATERIALIZED_ID) {
        return;
    }
    if (block->IsEmpty()) {
        MaterializeInBlock(inst, block);
    } else {
        Materialize(inst, block->GetLastInst());
    }
}

void EscapeAnalysis::Materialize(StateOwner inst, Inst *before)
{
    ASSERT(before != nullptr);
    auto blockState = GetState(before->GetBasicBlock());
    if (blockState->GetStateId(inst) == EscapeAnalysis::MATERIALIZED_ID) {
        return;
    }
    if (!std::holds_alternative<Inst *>(inst)) {
        return;
    }

    COMPILER_LOG(DEBUG, PEA) << "Materialized " << *std::get<Inst *>(inst) << " before " << *before;
    auto instState = blockState->GetState(inst);
    // If an inst is an alias (like NullCheck inst) then get original NewObject inst from the state;
    inst = instState->GetInst();
    auto targetInst = std::get<Inst *>(inst);
    // Mark object non-virtualizable if it should be materialized in the same block it was allocated.
    Inst *res = before;
    bool canMaterializeFields = true;
    if (targetInst->GetBasicBlock() == before->GetBasicBlock()) {
        res = targetInst;
        canMaterializeFields = false;
    } else if (auto ss = before->GetSaveState()) {
        // In some cases (for example, when before is ReturnInlined) SaveState may be
        // placed in some other basic block and materialized object may not be defined at it yet.
        if (ss->GetBasicBlock() == before->GetBasicBlock()) {
            res = ss;
        }
    } else if (before->IsControlFlow() && before->GetPrev() != nullptr) {
        // Typical case is when block ends with If or IfImm and previous
        // instruction is Compare. To avoid materialization in between (which may prevent other optimizations)
        // use previous instruction as materialization point.
        auto prev = before->GetPrev();
        if ((before->GetOpcode() == Opcode::If || before->GetOpcode() == Opcode::IfImm) &&
            prev->GetOpcode() == Opcode::Compare) {
            res = before->GetPrev();
        }
    }
    auto prevState = blockState->GetState(inst);
    blockState->Materialize(inst);
    RegisterMaterialization(res, targetInst);
    for (auto &t : prevState->GetFields()) {
        auto &fieldInst = t.second;
        if (blockState->GetStateId(fieldInst) == MATERIALIZED_ID) {
            continue;
        }
        if (canMaterializeFields) {
            Materialize(fieldInst, res);
        } else {
            blockState->Materialize(fieldInst);
        }
    }
}

void EscapeAnalysis::Materialize(Inst *inst)
{
    for (auto &input : inst->GetInputs()) {
        if (DataType::IsReference(input.GetInst()->GetType())) {
            Materialize(input.GetInst(), inst);
        }
    }
    if (inst->NoDest() && !inst->HasPseudoDestination()) {
        return;
    }
    if (DataType::IsReference(inst->GetType())) {
        GetState(inst->GetBasicBlock())->SetStateId(inst, EscapeAnalysis::MATERIALIZED_ID);
    }
}

void EscapeAnalysis::RegisterMaterialization(MaterializationSite site, Inst *inst)
{
    auto alloc = GetLocalAllocator();
    auto &matInfo = materializationInfo_.try_emplace(site, alloc->Adapter()).first->second;
    if (matInfo.find(inst) != matInfo.end()) {
        return;
    }
    virtualizationChanged_ = true;
    matInfo[inst] = nullptr;
}

// Register all states' virtual fields as "should be materialized" at given site.
bool EscapeAnalysis::RegisterFieldsMaterialization(Inst *site, VirtualState *state, BasicBlockState *blockState,
                                                   const ArenaMap<Inst *, VirtualState *> &states)
{
    bool materializedFields = false;
    for (auto &fields : state->GetFields()) {
        auto fieldStateId = blockState->GetStateId(fields.second);
        // Skip already materialized objects and objects already registered for materialization at given site.
        if (fieldStateId == MATERIALIZED_ID || (states.count(std::get<Inst *>(fields.second)) != 0)) {
            continue;
        }
        materializedFields = true;
        ASSERT(std::holds_alternative<Inst *>(fields.second));
        RegisterMaterialization(site, std::get<Inst *>(fields.second));
    }
    return materializedFields;
}

void EscapeAnalysis::RegisterVirtualObjectFieldsForMaterialization(Inst *ss)
{
    auto states = materializationInfo_.find(ss);
    ASSERT(states != materializationInfo_.end());

    auto blockState = GetState(ss->GetBasicBlock());
    bool materializedFields = true;
    while (materializedFields) {
        materializedFields = false;
        for (auto &t : states->second) {
            auto state = blockState->GetState(t.first);
            // Object has materialized state
            if (state == nullptr) {
                continue;
            }
            materializedFields =
                materializedFields || RegisterFieldsMaterialization(ss, state, blockState, states->second);
        }
    }
}

VirtualState *EscapeAnalysis::CreateVState(Inst *inst)
{
    auto vstate = GetLocalAllocator()->New<VirtualState>(inst, stateId_++, GetLocalAllocator());
    if (vstate == nullptr) {
        UNREACHABLE();
    }
    return vstate;
}

VirtualState *EscapeAnalysis::CreateVState(Inst *inst, StateId id)
{
    auto vstate = GetLocalAllocator()->New<VirtualState>(inst, id, GetLocalAllocator());
    if (vstate == nullptr) {
        UNREACHABLE();
    }
    return vstate;
}

void EscapeAnalysis::VisitCmpRef(Inst *inst, ConditionCode cc)
{
    if (!DataType::IsReference(inst->GetInputType(0U))) {
        return;
    }
    auto blockState = GetState(inst->GetBasicBlock());
    auto lhs = inst->GetInput(0).GetInst();
    auto rhs = inst->GetInput(1).GetInst();
    auto lhsStateId = blockState->GetStateId(lhs);
    auto rhsStateId = blockState->GetStateId(rhs);
    if (rhsStateId == MATERIALIZED_ID && lhsStateId == MATERIALIZED_ID) {
        aliases_.erase(inst);
        return;
    }
    auto sameState = lhsStateId == rhsStateId;
    bool cmpResult = sameState == (cc == ConditionCode::CC_EQ);
    aliases_[inst] = GetGraph()->FindOrCreateConstant(cmpResult);
}

void EscapeAnalysis::VisitNewObject(Inst *inst)
{
    VisitSaveStateUser(inst);

    // There are several reasons to materialize an object right at the allocation site:
    // (1) the object is an input for some instruction inside a catch block
    // (2) we already marked the object as one requiring materialization
    bool materialize =
        inst->GetFlag(inst_flags::Flags::CATCH_INPUT) || materializationInfo_.find(inst) != materializationInfo_.end();

    if (!relaxClassRestrictions_) {
        auto klassInput = inst->GetInput(0).GetInst();
        if (!materialize) {
            auto opc = klassInput->GetOpcode();
            // (3) object's class originated from some instruction we can't handle
            materialize = opc != Opcode::LoadImmediate && opc != Opcode::LoadAndInitClass && opc != Opcode::LoadClass;
        }
        if (!materialize) {
            RuntimeInterface::ClassPtr klassPtr;
            if (klassInput->GetOpcode() == Opcode::LoadImmediate) {
                klassPtr = klassInput->CastToLoadImmediate()->GetObject();
            } else {
                klassPtr = static_cast<ClassInst *>(klassInput)->GetClass();
            }
            // (4) object's class is not instantiable (for example, it's an interface or an abstract class)
            // (5) we can't apply scalar replacement for this particular class (for example, a class declares a function
            //     to invoke before GC (like LUA's finalizable objects))
            materialize = !GetGraph()->GetRuntime()->IsInstantiable(klassPtr) ||
                          !GetGraph()->GetRuntime()->CanScalarReplaceObject(klassPtr);
        }
    }

    if (materialize) {
        GetState(inst->GetBasicBlock())->Materialize(inst);
        RemoveVirtualizableAllocation(inst);
        return;
    }
    auto vstate = CreateVState(inst);
    GetState(inst->GetBasicBlock())->SetState(inst, vstate);
    AddVirtualizableAllocation(inst);
}

void EscapeAnalysis::VisitNullCheck(Inst *inst)
{
    auto blockState = GetState(inst->GetBasicBlock());

    if (inst->GetFlag(inst_flags::Flags::CATCH_INPUT)) {
        Materialize(inst->GetDataFlowInput(0), inst);
        VisitSaveStateUser(inst);
        return;
    }

    VisitSaveStateUser(inst);

    auto aliasedInst = inst->GetDataFlowInput(0);
    auto aliasedStateId = blockState->GetStateId(aliasedInst);
    blockState->SetStateId(inst, aliasedStateId);
    if (aliasedStateId != MATERIALIZED_ID) {
        aliases_[inst] = blockState->GetState(aliasedInst)->GetInst();
    } else {
        aliases_.erase(inst);
    }
}

void EscapeAnalysis::VisitBlockInternal(BasicBlock *block)
{
    for (auto inst : block->AllInsts()) {
        HandleMaterializationSite(inst);
        TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
    }
}

void EscapeAnalysis::HandleMaterializationSite(Inst *inst)
{
    auto it = materializationInfo_.find(inst);
    if (it == materializationInfo_.end()) {
        return;
    }
    auto blockState = GetState(inst->GetBasicBlock());
    // Ensure that for every object registered for materialization at the save state
    // all fields will be also registered for materialization here.
    RegisterVirtualObjectFieldsForMaterialization(inst);

    ArenaMap<Inst *, VirtualState *> &instsMap = it->second;
    ArenaVector<Inst *> pendingInsts {GetGraph()->GetLocalAllocator()->Adapter()};
    // If an alias was registered for materialization then try to find original instruction
    // and register it too.
    for (auto &t : instsMap) {
        // Allocation marked as non-virtualizable
        if (t.first == inst) {
            continue;
        }
        if (auto vstate = blockState->GetState(t.first)) {
            if (vstate->GetInst() != t.first && instsMap.count(vstate->GetInst()) == 0) {
                pendingInsts.push_back(vstate->GetInst());
            }
        }
    }
    for (auto newInst : pendingInsts) {
        // insert only a key, value will be populated later
        instsMap[newInst] = nullptr;
    }

    for (auto &t : instsMap) {
        // skip aliases
        if (t.first->GetOpcode() != Opcode::NewObject || t.first == inst) {
            continue;
        }
        auto candidateInst = t.first;
        if (auto vstate = blockState->GetState(candidateInst)) {
            // make a snapshot of virtual object's fields to use it during
            // scalar replacement to correctly initialize materialized object
            t.second = vstate->Copy(GetLocalAllocator());
            blockState->Materialize(candidateInst);
        } else {
            // instruction is already materialized, clear it's state snapshot
            t.second = nullptr;
        }
    }
}

void EscapeAnalysis::VisitSaveState(Inst *inst)
{
    FillVirtualInputs(inst);
}

void EscapeAnalysis::VisitSafePoint(Inst *inst)
{
    FillVirtualInputs(inst);
}

void EscapeAnalysis::VisitGetInstanceClass(Inst *inst)
{
    auto blockState = GetState(inst->GetBasicBlock());
    auto input = inst->GetInput(0).GetInst();
    if (auto vstate = blockState->GetState(input)) {
        auto newObj = vstate->GetInst();
        auto loadClass = newObj->GetInput(0).GetInst();
        if (!loadClass->IsClassInst() && loadClass->GetOpcode() != Opcode::GetInstanceClass) {
            return;
        }
        aliases_[inst] = loadClass;
    }
}

void EscapeAnalysis::FillVirtualInputs(Inst *inst)
{
    auto blockState = GetState(inst->GetBasicBlock());
    auto &states = saveStateInfo_.try_emplace(inst, GetLocalAllocator()).first->second;
    states.clear();
    // We can iterate over set bits only if it is not the first time we call this method
    // for the inst.
    for (size_t inputIdx = 0; inputIdx < inst->GetInputsCount(); ++inputIdx) {
        auto inputInst = inst->GetDataFlowInput(inputIdx);
        if (!DataType::IsReference(inputInst->GetType())) {
            continue;
        }
        if (blockState->GetStateId(inputInst) != MATERIALIZED_ID) {
            states.SetBit(inputIdx);
        }
    }
}

void EscapeAnalysis::VisitCall(CallInst *inst)
{
    if (inst->IsInlined()) {
        FillVirtualInputs(inst);
    } else {
        Materialize(inst);
    }
    VisitSaveStateUser(inst);
}

void EscapeAnalysis::VisitLoadObject(Inst *inst)
{
    auto vstate = GetState(inst->GetBasicBlock())->GetState(inst->GetDataFlowInput(0));
    auto field = inst->CastToLoadObject()->GetObjField();

    if (vstate != nullptr) {
        aliases_[inst] = vstate->GetFieldOrDefault(field, ZERO_INST);
    } else {
        aliases_.erase(inst);
    }

    if (!DataType::IsReference(inst->GetType())) {
        return;
    }
    if (vstate == nullptr) {
        GetState(inst->GetBasicBlock())->SetStateId(inst, MATERIALIZED_ID);
        return;
    }
    auto fieldInstId = vstate->GetFieldOrDefault(field, ZERO_INST);
    if (DataType::IsReference(inst->GetType())) {
        GetState(inst->GetBasicBlock())->SetStateId(inst, GetState(inst->GetBasicBlock())->GetStateId(fieldInstId));
    }
}

void EscapeAnalysis::VisitStoreObject(Inst *inst)
{
    auto vstate = GetState(inst->GetBasicBlock())->GetState(inst->GetDataFlowInput(0));
    auto field = inst->CastToStoreObject()->GetObjField();

    if (vstate != nullptr) {
        vstate->SetField(field, inst->GetDataFlowInput(1U));
        // mark inst for removal
        aliases_[inst] = inst;
    } else {
        if (DataType::IsReference(inst->GetType())) {
            Materialize(inst->GetDataFlowInput(1U), inst);
        }
        aliases_.erase(inst);
    }
}

void EscapeAnalysis::VisitSaveStateUser(Inst *inst)
{
    auto ss = inst->GetSaveState();
    if (ss == nullptr || ss->GetOpcode() != Opcode::SaveState) {
        return;
    }

    auto blockState = GetState(inst->GetBasicBlock());

    // If an instruction is materialized at save state user
    // then it should be materialized before the save state too.
    auto virtualInputs = saveStateInfo_.at(ss);
    for (auto inputIdx : virtualInputs.GetSetBitsIndices()) {
        auto inputInst = ss->GetInput(inputIdx).GetInst();
        if (blockState->GetStateId(inputInst) == MATERIALIZED_ID) {
            Materialize(inputInst, ss);
        }
    }
}

bool EscapeAnalysis::ProcessBlock(BasicBlock *block, size_t depth)
{
    if (block->IsMarked(visited_)) {
        return true;
    }
    if (AllPredecessorsVisited(block)) {
        mergeProcessor_.MergeStates(block);
        VisitBlockInternal(block);
        block->SetMarker(visited_);
    } else if (!block->GetLoop()->IsRoot()) {
        ASSERT(block->GetLoop()->GetHeader() == block);
        if (!ProcessLoop(block, depth + 1)) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

bool EscapeAnalysis::ProcessLoop(BasicBlock *header, size_t depth)
{
    if (depth >= MAX_NESTNESS) {
        return false;
    }
    auto &rpo = GetGraph()->GetBlocksRPO();
    auto startIt = find(rpo.begin(), rpo.end(), header);
    // There should be only one visited predecessor.
    // Use its state as initial loop header's state (instead of executing MergeStates).
    auto headerPred = *std::find_if(header->GetPredsBlocks().begin(), header->GetPredsBlocks().end(),
                                    [marker = visited_](auto b) { return b->IsMarked(marker); });
    auto headerState = GetState(headerPred)->Copy(GetGraph()->GetLocalAllocator());
    // Set materialized states for all phis
    for (auto phi : header->PhiInsts()) {
        if (DataType::IsReference(phi->GetType())) {
            headerState->Materialize(phi);
        }
    }
    blockStates_[header->GetId()] = headerState;
    // Copy header's initial state to compare it after loop processing
    headerState = headerState->Copy(GetGraph()->GetLocalAllocator());
    VisitBlockInternal(header);

    bool stateChanged = true;
    while (stateChanged) {
        header->SetMarker(visited_);
        // Reset visited marker for the loop and all its nested loops
        for (auto it = std::next(startIt), end = rpo.end(); it != end; ++it) {
            if ((*it)->GetLoop() == header->GetLoop() || (*it)->GetLoop()->IsInside(header->GetLoop())) {
                (*it)->ResetMarker(visited_);
            }
        }

        // Process loop and its nested loops only
        for (auto it = std::next(startIt), end = rpo.end(); it != end; ++it) {
            auto block = *it;
            if (block->GetLoop() != header->GetLoop() && !(*it)->GetLoop()->IsInside(header->GetLoop())) {
                continue;
            }
            if (!ProcessBlock(block, depth)) {
                return false;
            }
        }
        // Now all loop header's predecessors should be visited and we can merge its states
        ASSERT(AllPredecessorsVisited(header));
        mergeProcessor_.MergeStates(header);
        // Check if merged state differs from previous loop header's state
        stateChanged = !headerState->Equals(GetState(header));
        if (stateChanged) {
            headerState = GetState(header)->Copy(GetLocalAllocator());
        }
        VisitBlockInternal(header);
    }
    return true;
}

SaveStateInst *ScalarReplacement::CopySaveState(Inst *inst, VirtualState *except)
{
    auto ss = inst->CastToSaveState();
    auto copy = static_cast<SaveStateInst *>(ss->Clone(graph_));
    auto &virtInsts = saveStateLiveness_.try_emplace(copy, saveStateLiveness_.at(inst)).first->second;
    const auto &aliases = except->GetAliases();
    for (size_t inputIdx = 0, copyIdx = 0; inputIdx < ss->GetInputsCount(); inputIdx++) {
        copy->AppendInput(inst->GetInput(inputIdx));
        copy->SetVirtualRegister(copyIdx++, ss->GetVirtualRegister(inputIdx));

        if (std::find(aliases.begin(), aliases.end(), inst->GetDataFlowInput(inputIdx)) != aliases.end()) {
            // Mark SaveState to fix later.
            virtInsts.SetBit(inputIdx);
        }
    }

    ss->GetBasicBlock()->InsertBefore(copy, ss);
    return copy;
}

void ScalarReplacement::CreatePhis()
{
    for (auto bb : graph_->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }
        for (auto &t : phis_.at(bb->GetId())) {
            auto state = t.second;
            auto phiInst = graph_->CreateInstPhi(state->GetType(), bb->GetGuestPc());
            allocatedPhis_[state] = phiInst;
            bb->AppendPhi(phiInst);
        }
    }
}

void ScalarReplacement::MaterializeObjects()
{
    for (auto &[site, state] : materializationSites_) {
        if (std::holds_alternative<BasicBlock *>(site)) {
            MaterializeInEmptyBlock(std::get<BasicBlock *>(site), state);
            continue;
        }
        auto siteInst = std::get<Inst *>(site);
        if (siteInst->GetOpcode() == Opcode::SaveState) {
            MaterializeAtExistingSaveState(siteInst->CastToSaveState(), state);
        } else {
            MaterializeAtNewSaveState(siteInst, state);
        }
    }
}

void ScalarReplacement::MaterializeAtExistingSaveState(SaveStateInst *saveState,
                                                       ArenaMap<Inst *, VirtualState *> &state)
{
    auto previousSaveState = saveState;
    for (auto t : state) {
        if (t.second == nullptr || t.first->GetOpcode() != Opcode::NewObject) {
            continue;
        }
        auto origAlloc = t.first;
        ASSERT(origAlloc == t.second->GetInst());
        auto currSs = CopySaveState(previousSaveState, t.second);
        previousSaveState = currSs;

        auto newAlloc = CreateNewObject(origAlloc, currSs);
        auto &allocs =
            materializedObjects_.try_emplace(origAlloc, graph_->GetLocalAllocator()->Adapter()).first->second;
        allocs.push_back(newAlloc);

        InitializeObject(newAlloc, saveState, t.second);
    }
}

void ScalarReplacement::MaterializeAtNewSaveState(Inst *site, ArenaMap<Inst *, VirtualState *> &state)
{
    auto block = site->GetBasicBlock();
    auto ssInsertionPoint = site;
    for (auto t : state) {
        if (t.second == nullptr || t.first->GetOpcode() != Opcode::NewObject || t.first == site) {
            continue;
        }
        auto origAlloc = t.first;
        auto currSs = graph_->CreateInstSaveState();
        if (auto callerInst = FindCallerInst(block, site)) {
            currSs->SetMethod(callerInst->GetCallMethod());
            currSs->SetCallerInst(callerInst);
            currSs->SetInliningDepth(callerInst->GetSaveState()->GetInliningDepth() + 1);
        } else {
            currSs->SetMethod(graph_->GetMethod());
        }
        block->InsertBefore(currSs, ssInsertionPoint);
        ssInsertionPoint = currSs;

        auto newAlloc = CreateNewObject(origAlloc, currSs);
        auto &allocs =
            materializedObjects_.try_emplace(origAlloc, graph_->GetLocalAllocator()->Adapter()).first->second;
        allocs.push_back(newAlloc);

        InitializeObject(newAlloc, site, t.second);
    }
}

void ScalarReplacement::MaterializeInEmptyBlock(BasicBlock *block, ArenaMap<Inst *, VirtualState *> &state)
{
    ASSERT(block->IsEmpty());
    for (auto t : state) {
        if (t.second == nullptr || t.first->GetOpcode() != Opcode::NewObject) {
            continue;
        }
        auto origAlloc = t.first;
        auto currSs = graph_->CreateInstSaveState();
        if (auto callerInst = FindCallerInst(block)) {
            currSs->SetMethod(callerInst->GetCallMethod());
            currSs->SetCallerInst(callerInst);
            currSs->SetInliningDepth(callerInst->GetSaveState()->GetInliningDepth() + 1);
        } else {
            currSs->SetMethod(graph_->GetMethod());
        }
        block->PrependInst(currSs);

        auto newAlloc = CreateNewObject(origAlloc, currSs);
        auto &allocs =
            materializedObjects_.try_emplace(origAlloc, graph_->GetLocalAllocator()->Adapter()).first->second;
        allocs.push_back(newAlloc);

        InitializeObject(newAlloc, nullptr, t.second);
    }
}

Inst *ScalarReplacement::CreateNewObject(Inst *originalInst, Inst *saveState)
{
    auto newAlloc = graph_->CreateInstNewObject(
        originalInst->GetType(), originalInst->GetPc(), originalInst->GetInput(0).GetInst(), saveState,
        originalInst->CastToNewObject()->GetTypeId(), originalInst->CastToNewObject()->GetMethod());
    saveState->GetBasicBlock()->InsertAfter(newAlloc, saveState);
    COMPILER_LOG(DEBUG, PEA) << "Materialized " << originalInst->GetId() << " at SavePoint " << saveState->GetId()
                             << " as " << *newAlloc;
    return newAlloc;
}

CallInst *ScalarReplacement::FindCallerInst(BasicBlock *target, Inst *start)
{
    auto block = start == nullptr ? target->GetDominator() : target;
    size_t depth = 0;
    while (block != nullptr) {
        auto iter = InstBackwardIterator<IterationType::INST>(*block, start);
        for (auto inst : iter) {
            if (inst == start) {
                continue;
            }
            if (inst->GetOpcode() == Opcode::ReturnInlined) {
                depth++;
            }
            if (!inst->IsCall()) {
                continue;
            }
            auto callInst = static_cast<CallInst *>(inst);
            auto isInlined = callInst->IsInlined();
            if (isInlined && depth == 0) {
                return callInst;
            }
            if (isInlined) {
                depth--;
            }
        }
        block = block->GetDominator();
        start = nullptr;
    }
    return nullptr;
}

void ScalarReplacement::InitializeObject(Inst *alloc, Inst *instBefore, VirtualState *state)
{
    for (auto &[field, field_source] : state->GetFields()) {
        Inst *fieldSourceInst {nullptr};
        if (std::holds_alternative<Inst *>(field_source)) {
            fieldSourceInst = std::get<Inst *>(field_source);
        } else if (std::holds_alternative<PhiState *>(field_source)) {
            auto phisState = std::get<PhiState *>(field_source);
            fieldSourceInst = allocatedPhis_[phisState];
            ASSERT(fieldSourceInst != nullptr);
        } else {
            // Field initialized with zero constant / nullptr, it's a default value,
            // so there is no need to insert explicit store instruction.
            continue;
        }

        auto fieldType = graph_->GetRuntime()->GetFieldType(field);
        auto store = graph_->CreateInstStoreObject(
            fieldType, alloc->GetPc(), alloc, fieldSourceInst, graph_->GetRuntime()->GetFieldId(field),
            graph_->GetMethod(), field, graph_->GetRuntime()->IsFieldVolatile(field), DataType::IsReference(fieldType));
        if (instBefore != nullptr) {
            instBefore->GetBasicBlock()->InsertBefore(store, instBefore);
        } else {
            alloc->GetBasicBlock()->AppendInst(store);
        }
    }
}

Inst *ScalarReplacement::ResolveAlias(const StateOwner &alias, const Inst *inst)
{
    if (std::holds_alternative<PhiState *>(alias)) {
        auto phiInst = allocatedPhis_[std::get<PhiState *>(alias)];
        ASSERT(phiInst != nullptr);
        return phiInst;
    }
    if (std::holds_alternative<Inst *>(alias)) {
        auto target = std::get<Inst *>(alias);
        // try to unwind aliasing chain
        auto it = aliases_.find(target);
        if (it == aliases_.end()) {
            return target;
        }
        if (std::holds_alternative<Inst *>(it->second) && std::get<Inst *>(it->second) == target) {
            return target;
        }
        return ResolveAlias(it->second, inst);
    }
    // It's neither PhiState, nor Inst, so it should be ZeroInst.
    if (DataType::IsReference(inst->GetType())) {
        return graph_->GetOrCreateNullPtr();
    }
    if (inst->GetType() == DataType::FLOAT32) {
        return graph_->FindOrCreateConstant(0.0F);
    }
    if (inst->GetType() == DataType::FLOAT64) {
        return graph_->FindOrCreateConstant(0.0);
    }
    return graph_->FindOrCreateConstant(0U);
}

void ScalarReplacement::ReplaceAliases()
{
    ArenaVector<Inst *> replaceInputs {graph_->GetLocalAllocator()->Adapter()};
    for (auto &[inst, alias] : aliases_) {
        Inst *replacement = ResolveAlias(alias, inst);
        if (replacement != inst && replacement != nullptr) {
            for (auto &user : inst->GetUsers()) {
                replaceInputs.push_back(user.GetInst());
            }
        }

        bool replaced = false;
        auto instTypeSize = DataType::GetTypeSize(inst->GetType(), graph_->GetArch());
        if (replacement != nullptr && inst->GetOpcode() == Opcode::LoadObject &&
            instTypeSize < DataType::GetTypeSize(replacement->GetType(), graph_->GetArch())) {
            // In case of loads/stores explicit casts could be eliminated before scalar replacement.
            // To use correct values after load's replacement with a value stored into a field we
            // need to insert some casts back.
            replaced = true;
            for (auto user : replaceInputs) {
                auto cast = graph_->CreateInstCast(inst->GetType(), inst->GetPc(), replacement, replacement->GetType());
                inst->InsertBefore(cast);
                replacement = cast;
                user->ReplaceInput(inst, replacement);
            }
        } else if (replacement != nullptr && replacement->IsClassInst()) {
            auto classImm = graph_->CreateInstLoadImmediate(DataType::REFERENCE, replacement->GetPc(),
                                                            static_cast<ClassInst *>(replacement)->GetClass());
            replacement->InsertAfter(classImm);
            replacement = classImm;
        }

        if (replacement != nullptr) {
            COMPILER_LOG(DEBUG, PEA) << "Replacing " << *inst << " with " << *replacement;
        }
        if (!replaced) {
            for (auto user : replaceInputs) {
                user->ReplaceInput(inst, replacement);
            }
        }
        replaceInputs.clear();
        EnqueueForRemoval(inst);
    }
}

void ScalarReplacement::ResolvePhiInputs()
{
    for (auto [state, inst] : allocatedPhis_) {
        auto preds = inst->GetBasicBlock()->GetPredsBlocks();
        auto inputs = state->GetInputs();
        ASSERT(preds.size() == inputs.size());
        for (size_t idx = 0; idx < inputs.size(); idx++) {
            auto inputInst = ResolveAlias(inputs[idx], inst);
            ASSERT(inputInst != nullptr);
            if (inputInst->GetOpcode() == Opcode::NewObject) {
                inputInst = ResolveAllocation(inputInst, preds[idx]);
            }
            inst->AppendInput(inputInst);
        }
    }
}

Inst *ScalarReplacement::ResolveAllocation(Inst *inst, BasicBlock *block)
{
    ASSERT(inst != nullptr);
    auto it = materializedObjects_.find(inst);
    if (it == materializedObjects_.end()) {
        return inst;
    }
    auto &allocs = it->second;
    for (auto alloc : allocs) {
        if (alloc->GetBasicBlock()->IsDominate(block)) {
            return alloc;
        }
    }
    UNREACHABLE();
    return inst;
}

void ScalarReplacement::UpdateSaveStates()
{
    ArenaVector<Inst *> queue {graph_->GetLocalAllocator()->Adapter()};
    for (auto &[site, virtual_objects] : saveStateLiveness_) {
        bool isCall = site->IsCall();
        ASSERT(!isCall || static_cast<CallInst *>(site)->IsInlined());
        // Remove virtual inputs (i.e. objects that are not alive)
        for (ssize_t inputIdx = static_cast<ssize_t>(site->GetInputsCount()) - 1; inputIdx >= 0; --inputIdx) {
            if (!virtual_objects.GetBit(inputIdx)) {
                continue;
            }
            if (isCall) {
                site->SetInput(inputIdx, graph_->GetOrCreateNullPtr());
                continue;
            }
            site->RemoveInput(inputIdx);
        }
    }
}

void ScalarReplacement::UpdateAllocationUsers()
{
    ArenaVector<std::tuple<Inst *, size_t, Inst *>> queue {graph_->GetLocalAllocator()->Adapter()};
    for (auto &[old_alloc, new_allocs] : materializedObjects_) {
        // At these point:
        // - all aliases should be already processed and corresponding users will be enqueued for removal
        // - all save states and inlined calls will be already processed
        // - inputs for newly allocated phis will be processed as well
        for (auto &user : old_alloc->GetUsers()) {
            auto userInst = user.GetInst();
            if (IsEnqueuedForRemoval(userInst)) {
                continue;
            }
            // There might be multiple materializations of the same object so we need to find
            // the one dominating current user.
            auto userBlock =
                userInst->IsPhi() ? userInst->CastToPhi()->GetPhiInputBb(user.GetIndex()) : userInst->GetBasicBlock();
            auto replacementIt = std::find_if(new_allocs.begin(), new_allocs.end(), [userBlock](auto newAlloc) {
                return newAlloc->GetBasicBlock()->IsDominate(userBlock);
            });
            ASSERT(replacementIt != new_allocs.end());
            queue.emplace_back(userInst, user.GetIndex(), *replacementIt);
        }
        for (auto &[user, idx, replacement] : queue) {
            user->SetInput(idx, replacement);
        }
        queue.clear();

        EnqueueForRemoval(old_alloc);
    }
}

void ScalarReplacement::EnqueueForRemoval(Inst *inst)
{
    if (IsEnqueuedForRemoval(inst)) {
        return;
    }
    inst->SetMarker(removeInstMarker_);
    removalQueue_.push_back(inst);
}

bool ScalarReplacement::IsEnqueuedForRemoval(Inst *inst) const
{
    return inst->IsMarked(removeInstMarker_);
}

static bool InputSizeGtSize(Inst *inst)
{
    uint32_t size = 0;
    auto arch = inst->GetBasicBlock()->GetGraph()->GetArch();

    for (auto input : inst->GetInputs()) {
        auto inputSize = DataType::GetTypeSize(input.GetInst()->GetType(), arch);
        size = size < inputSize ? inputSize : size;
    }
    return size > DataType::GetTypeSize(inst->GetType(), arch);
}

/* Phi instruction can have inputs that are wider than
 * the phi type, we have to insert casts, so that, the
 * phi takes the types matching it's own type */
void ScalarReplacement::FixPhiInputTypes()
{
    auto arch = graph_->GetArch();
    for (auto alphi : allocatedPhis_) {
        auto phi = alphi.second;
        if (LIKELY(!InputSizeGtSize(phi) || !phi->HasUsers())) {
            continue;
        }
        auto phiSize = DataType::GetTypeSize(phi->GetType(), arch);
        for (auto &i : phi->GetInputs()) {
            auto input = i.GetInst();
            /* constants are taken care of by constprop and existing
             * phis have to be properly casted already */
            if (LIKELY(phiSize == DataType::GetTypeSize(input->GetType(), arch) ||
                       input->GetOpcode() == Opcode::Constant || input->GetOpcode() == Opcode::Phi)) {
                continue;
            }
            /* replace the wider-than-phi-type input with the cast */
            auto cast = graph_->CreateInstCast(phi->GetType(), input->GetPc(), input, input->GetType());
            phi->ReplaceInput(input, cast);
            input->InsertAfter(cast);
        }
    }
}

void ScalarReplacement::ProcessRemovalQueue()
{
    for (auto inst : removalQueue_) {
        COMPILER_LOG(DEBUG, PEA) << "Removing inst " << inst->GetId();
        inst->GetBasicBlock()->RemoveInst(inst);
    }
    FixPhiInputTypes();
}

void ScalarReplacement::Apply(ArenaSet<Inst *> &candidates)
{
    removeInstMarker_ = graph_->NewMarker();
    CreatePhis();
    MaterializeObjects();
    ReplaceAliases();
    ResolvePhiInputs();
    UpdateSaveStates();
    UpdateAllocationUsers();
    PatchSaveStates();
    for (auto candidate : candidates) {
        EnqueueForRemoval(candidate);
    }
    ProcessRemovalQueue();
    graph_->EraseMarker(removeInstMarker_);
}

void ScalarReplacement::PatchSaveStates()
{
    ArenaVector<ArenaSet<Inst *>> liveness {graph_->GetLocalAllocator()->Adapter()};
    for (size_t idx = 0; idx < graph_->GetVectorBlocks().size(); ++idx) {
        liveness.emplace_back(graph_->GetLocalAllocator()->Adapter());
    }

    auto &rpo = graph_->GetBlocksRPO();
    for (auto it = rpo.rbegin(), end = rpo.rend(); it != end; ++it) {
        auto block = *it;
        PatchSaveStatesInBlock(block, liveness);
    }
}

void ScalarReplacement::FillLiveInsts(BasicBlock *block, ArenaSet<Inst *> &liveIns,
                                      ArenaVector<ArenaSet<Inst *>> &liveness)
{
    for (auto succ : block->GetSuccsBlocks()) {
        liveIns.insert(liveness[succ->GetId()].begin(), liveness[succ->GetId()].end());
        for (auto phiInst : succ->PhiInsts()) {
            auto phiInput = phiInst->CastToPhi()->GetPhiInput(block);
            if (phiInput->GetBasicBlock() != succ && DataType::IsReference(phiInput->GetType())) {
                liveIns.insert(phiInput);
            }
        }
    }
}

// Compute live ref-valued insturctions at each save state and insert any missing live instruction into a save state.
void ScalarReplacement::PatchSaveStatesInBlock(BasicBlock *block, ArenaVector<ArenaSet<Inst *>> &liveness)
{
    auto &liveIns = liveness[block->GetId()];
    FillLiveInsts(block, liveIns, liveness);

    auto loop = block->GetLoop();
    bool loopIsHeader = !loop->IsRoot() && loop->GetHeader() == block;
    if (loopIsHeader) {
        for (auto inst : block->InstsReverse()) {
            if (IsEnqueuedForRemoval(inst)) {
                continue;
            }
            AddLiveInputs(inst, liveIns);
        }
    }
    for (auto inst : block->InstsReverse()) {
        if (inst->IsSaveState()) {
            PatchSaveState(static_cast<SaveStateInst *>(inst), liveIns);
        }
        if (DataType::IsReference(inst->GetType())) {
            liveIns.erase(inst);
        }
        if (IsEnqueuedForRemoval(inst)) {
            continue;
        }
        if (!loopIsHeader) {
            AddLiveInputs(inst, liveIns);
        }
    }

    if (!loop->IsRoot() && loop->GetHeader() == block) {
        for (auto phiInst : block->PhiInsts()) {
            bool propagateThroughLoop = false;
            for (auto &user : phiInst->GetUsers()) {
                propagateThroughLoop =
                    propagateThroughLoop ||
                    (user.GetInst()->IsPhi() && user.GetInst()->GetBasicBlock() == phiInst->GetBasicBlock());
            }
            if (!propagateThroughLoop) {
                liveIns.erase(phiInst);
            }
        }
        PatchSaveStatesInLoop(loop, liveIns, liveness);
    }
    for (auto phiInst : block->PhiInsts()) {
        liveIns.erase(phiInst);
    }
}

void ScalarReplacement::AddLiveInputs(Inst *inst, ArenaSet<Inst *> &liveIns)
{
    for (auto &input : inst->GetInputs()) {
        auto inputInst = inst->GetDataFlowInput(input.GetInst());
        if (!DataType::IsReference(inputInst->GetType()) || inputInst->IsStore() || !inputInst->IsMovableObject()) {
            continue;
        }
        liveIns.insert(inputInst);
    }
}

void ScalarReplacement::PatchSaveStatesInLoop(Loop *loop, ArenaSet<Inst *> &loopLiveIns,
                                              ArenaVector<ArenaSet<Inst *>> &liveness)
{
    for (auto loopBlock : graph_->GetVectorBlocks()) {
        if (loopBlock == nullptr) {
            continue;
        }
        if (loopBlock->GetLoop() != loop && !loopBlock->GetLoop()->IsInside(loop)) {
            continue;
        }
        if (loopBlock != loop->GetHeader()) {
            auto &loopBlockLiveIns = liveness[loopBlock->GetId()];
            loopBlockLiveIns.insert(loopLiveIns.begin(), loopLiveIns.end());
        }
        for (auto inst : loopBlock->Insts()) {
            if (inst->IsSaveState()) {
                PatchSaveState(static_cast<SaveStateInst *>(inst), loopLiveIns);
            }
        }
    }
}

void ScalarReplacement::PatchSaveState(SaveStateInst *saveState, ArenaSet<Inst *> &liveInstructions)
{
    for (auto inst : liveInstructions) {
        inst = Inst::GetDataFlowInput(inst);
        if (!inst->IsMovableObject()) {
            continue;
        }
        auto inputs = saveState->GetInputs();
        auto it = std::find_if(inputs.begin(), inputs.end(), [inst](auto &in) { return in.GetInst() == inst; });
        if (it != inputs.end()) {
            continue;
        }
        saveState->AppendBridge(inst);
    }
}
}  // namespace ark::compiler
