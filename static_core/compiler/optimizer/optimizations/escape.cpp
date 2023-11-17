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

#include <algorithm>
#include "optimizer/ir/basicblock.h"
#include "optimizer/optimizations/escape.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/inst.h"
#include "compiler/compiler_logger.h"

namespace panda::compiler {
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

    StateOwner GetFieldOrDefault(FieldPtr field, StateOwner default_value) const
    {
        auto it = fields_.find(field);
        if (it == fields_.end()) {
            return default_value;
        }
        return it->second;
    }

    const ArenaUnorderedMap<FieldPtr, StateOwner> &GetFields() const
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
    ArenaUnorderedMap<FieldPtr, StateOwner> fields_;
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
    explicit BasicBlockState(ArenaAllocator *alloc) : states_(alloc->Adapter()), state_values_(alloc->Adapter()) {}

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
        auto inst_inst = std::get<Inst *>(inst);
        if (state != EscapeAnalysis::MATERIALIZED_ID) {
            auto vstate = state_values_[state];
            vstate->AddAlias(inst_inst);
            states_[inst_inst] = state;
        } else {
            states_.erase(inst_inst);
        }
    }

    void SetState(const StateOwner &inst, VirtualState *vstate)
    {
        ASSERT(std::holds_alternative<Inst *>(inst));
        auto inst_inst = std::get<Inst *>(inst);
        states_[inst_inst] = vstate->GetId();
        if (state_values_.size() <= vstate->GetId()) {
            state_values_.resize(vstate->GetId() + 1);
        }
        vstate->AddAlias(inst_inst);
        state_values_[vstate->GetId()] = vstate;
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
        return state_values_.size() > inst && state_values_[inst] != nullptr;
    }

    VirtualState *GetStateById(StateId state) const
    {
        if (state == EscapeAnalysis::MATERIALIZED_ID) {
            return nullptr;
        }
        ASSERT(state < state_values_.size());
        auto value = state_values_[state];
        ASSERT(value != nullptr);
        return value;
    }

    VirtualState *GetState(const StateOwner &inst) const
    {
        return GetStateById(GetStateId(inst));
    }

    const ArenaUnorderedMap<Inst *, StateId> &GetStates() const
    {
        return states_;
    }

    void Materialize(const StateOwner &inst)
    {
        if (!std::holds_alternative<Inst *>(inst)) {
            return;
        }
        auto inst_inst = std::get<Inst *>(inst);
        auto it = states_.find(inst_inst);
        if (it == states_.end()) {
            return;
        }
        auto state_id = it->second;
        if (auto vstate = state_values_[state_id]) {
            for (auto &alias : vstate->GetAliases()) {
                states_.erase(alias);
            }
            states_.erase(vstate->GetInst());
        } else {
            states_.erase(inst_inst);
        }
        state_values_[state_id] = nullptr;
    }

    void Clear()
    {
        states_.clear();
        state_values_.clear();
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
        copy->state_values_.resize(state_values_.size());
        for (StateId id = 0; id < state_values_.size(); ++id) {
            if (auto v = state_values_[id]) {
                copy->state_values_[id] = v->Copy(alloc);
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
        for (id = 0; id < std::min<size_t>(state_values_.size(), other->state_values_.size()); ++id) {
            auto v = state_values_[id];
            auto other_state = other->state_values_[id];
            if ((v == nullptr) != (other_state == nullptr)) {
                return false;
            }
            if (v == nullptr) {
                continue;
            }
            if (!v->Equals(other_state)) {
                return false;
            }
        }
        for (; id < state_values_.size(); ++id) {
            if (state_values_[id] != nullptr) {
                return false;
            }
        }
        for (; id < other->state_values_.size(); ++id) {
            if (other->state_values_[id] != nullptr) {
                return false;
            }
        }

        return true;
    }

private:
    ArenaUnorderedMap<Inst *, StateId> states_;
    ArenaVector<VirtualState *> state_values_;
};

void EscapeAnalysis::LiveInAnalysis::ProcessBlock(BasicBlock *block)
{
    auto &live_ins = live_in_[block->GetId()];

    for (auto succ : block->GetSuccsBlocks()) {
        live_ins |= live_in_[succ->GetId()];
        for (auto phi_inst : succ->PhiInsts()) {
            auto phi_input = phi_inst->CastToPhi()->GetPhiInput(block);
            if (phi_input->GetBasicBlock() != block && DataType::IsReference(phi_input->GetType())) {
                live_ins.SetBit(phi_input->GetId());
                AddInst(phi_input);
            }
        }
    }

    for (auto inst : block->InstsReverse()) {
        if (DataType::IsReference(inst->GetType())) {
            live_ins.ClearBit(inst->GetId());
        }
        for (auto &input : inst->GetInputs()) {
            auto input_inst = input.GetInst();
            if (!DataType::IsReference(input_inst->GetType())) {
                continue;
            }
            live_ins.SetBit(input_inst->GetId());
            AddInst(input_inst);
            auto df_input = inst->GetDataFlowInput(input_inst);
            if (df_input != input_inst) {
                live_ins.SetBit(df_input->GetId());
                AddInst(df_input);
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
    for (auto loop_block : graph_->GetVectorBlocks()) {
        if (loop_block == nullptr || loop_block == block) {
            continue;
        }
        if (loop_block->GetLoop() == loop || loop_block->GetLoop()->IsInside(loop)) {
            auto &loop_block_live_ins = live_in_[loop_block->GetId()];
            loop_block_live_ins |= live_ins;
        }
    }
}

bool EscapeAnalysis::LiveInAnalysis::Run()
{
    bool has_allocs = false;
    for (auto block : graph_->GetBlocksRPO()) {
        if (has_allocs) {
            break;
        }
        for (auto inst : block->Insts()) {
            has_allocs = has_allocs || inst->GetOpcode() == Opcode::NewObject;
            if (has_allocs) {
                break;
            }
        }
    }
    if (!has_allocs) {
        return false;
    }

    live_in_.clear();
    for (size_t idx = 0; idx < graph_->GetVectorBlocks().size(); ++idx) {
        live_in_.emplace_back(graph_->GetLocalAllocator());
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

    auto new_state = parent_->GetLocalAllocator()->New<BasicBlockState>(parent_->GetLocalAllocator());
    if (new_state == nullptr) {
        UNREACHABLE();
    }

    // Materialization of some fields may require further materalialization of other objects.
    // Repeat merging process until no new materialization will happen.
    bool state_changed = true;
    while (state_changed) {
        state_changed = false;
        new_state->Clear();
        ProcessPhis(block, new_state);

        states_merge_buffer_.clear();
        CollectInstructionsToMerge(block);

        while (!states_merge_buffer_.empty()) {
            pending_insts_.clear();
            for (auto &inst : states_merge_buffer_) {
                state_changed = state_changed || MergeState(inst, block, new_state);
            }
            states_merge_buffer_.clear();
            states_merge_buffer_.swap(pending_insts_);
        }
        parent_->block_states_[block->GetId()] = new_state;
    }
    MaterializeObjectsAtTheBeginningOfBlock(block);
}

void EscapeAnalysis::MergeProcessor::ProcessPhis(BasicBlock *block, BasicBlockState *new_state)
{
    for (auto phi : block->PhiInsts()) {
        if (!DataType::IsReference(phi->GetType())) {
            continue;
        }
        for (size_t input_idx = 0; input_idx < phi->GetInputsCount(); ++input_idx) {
            auto bb = phi->CastToPhi()->GetPhiInputBb(input_idx);
            parent_->Materialize(phi->GetInput(input_idx).GetInst(), bb);
        }
        new_state->Materialize(phi);
    }
}

void EscapeAnalysis::MergeProcessor::CheckStatesAndInsertIntoBuffer(StateOwner inst, BasicBlock *block)
{
    for (auto pred_block : block->GetPredsBlocks()) {
        auto pred_state = parent_->GetState(pred_block);
        if (!pred_state->HasState(inst)) {
            return;
        }
    }
    states_merge_buffer_.push_back(inst);
}

void EscapeAnalysis::MergeProcessor::CollectInstructionsToMerge(BasicBlock *block)
{
    parent_->live_ins_.VisitAlive(block, [&buffer = states_merge_buffer_](auto inst) { buffer.push_back(inst); });
}

bool EscapeAnalysis::MergeProcessor::MergeState(StateOwner inst, BasicBlock *block, BasicBlockState *new_state)
{
    auto &preds_blocks = block->GetPredsBlocks();
    auto common_id = parent_->GetState(preds_blocks.front())->GetStateId(inst);
    // Materialization is required if predecessors have different
    // states for the same instruction or all predecessors have materialized state for it.
    bool materialize = common_id == EscapeAnalysis::MATERIALIZED_ID;
    bool new_materialization = false;
    for (auto it = std::next(preds_blocks.begin()), end = preds_blocks.end(); it != end; ++it) {
        auto pred_block = *it;
        auto pred_id = parent_->GetState(pred_block)->GetStateId(inst);
        if (pred_id == EscapeAnalysis::MATERIALIZED_ID || pred_id != common_id) {
            materialize = true;
            break;
        }
    }

    if (materialize) {
        new_materialization =
            parent_->GetState(block->GetDominator())->GetStateId(inst) != EscapeAnalysis::MATERIALIZED_ID;
        parent_->Materialize(inst, block->GetDominator());
        new_state->Materialize(inst);
        return new_materialization;
    }

    if (new_state->HasStateWithId(common_id)) {
        // we already handled that vstate so we should skip further processing
        new_state->SetStateId(inst, common_id);
        return false;
    }

    auto vstate = parent_->CreateVState(
        parent_->GetState(block->GetPredsBlocks().front())->GetStateById(common_id)->GetInst(), common_id);
    new_materialization = MergeFields(block, new_state, common_id, vstate, pending_insts_);
    new_state->SetState(inst, vstate);
    // if inst is an alias then we also need to process original inst
    pending_insts_.push_back(vstate->GetInst());
    return new_materialization;
}

bool EscapeAnalysis::MergeProcessor::MergeFields(BasicBlock *block, BasicBlockState *block_state,
                                                 StateId state_to_merge, VirtualState *vstate,
                                                 ArenaVector<StateOwner> &merge_queue)
{
    all_fields_.clear();
    for (auto pred : block->GetPredsBlocks()) {
        for (auto &field : parent_->GetState(pred)->GetStateById(state_to_merge)->GetFields()) {
            all_fields_.push_back(field.first);
        }
    }
    std::sort(all_fields_.begin(), all_fields_.end());
    auto end = std::unique(all_fields_.begin(), all_fields_.end());

    for (auto it = all_fields_.begin(); it != end; ++it) {
        auto field = *it;
        fields_merge_buffer_.clear();
        // Use ZERO_INST as a placeholder for a field that was not set.
        // When it'll come to scalar replacement this placeholder will be
        // replaced with actual zero or nullptr const.
        StateOwner common_id = parent_->GetState(block->GetPredsBlocks().front())
                                   ->GetStateById(state_to_merge)
                                   ->GetFieldOrDefault(field, ZERO_INST);
        bool need_merge = false;
        for (auto pred_block : block->GetPredsBlocks()) {
            auto pred_state = parent_->GetState(pred_block)->GetStateById(state_to_merge);
            auto pred_field_value = pred_state->GetFieldOrDefault(field, ZERO_INST);
            if (common_id != pred_field_value) {
                need_merge = true;
            }
            fields_merge_buffer_.push_back(pred_field_value);
        }
        if (!need_merge) {
            vstate->SetField(field, common_id);
            // enqueue inst id for state copying
            if (!block_state->HasState(common_id)) {
                merge_queue.push_back(common_id);
            }
            continue;
        }
        auto [phi, new_materialization] = parent_->CreatePhi(block, block_state, field, fields_merge_buffer_);
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
    auto m_it = parent_->materialization_info_.find(block);
    auto block_state = parent_->GetState(block);
    if (m_it == parent_->materialization_info_.end()) {
        return;
    }
    auto &mat_states = m_it->second;
    for (auto states_it = mat_states.begin(); states_it != mat_states.end();) {
        auto inst = states_it->first;
        // If the instruction has virtual state then materialize it and save
        // copy of a virtual state (it will be used to initialize fields after allocation).
        // Otherwise remove the instruction from list of instruction requiring materialization
        // at this block.
        if (block_state->GetStateId(inst) != EscapeAnalysis::MATERIALIZED_ID) {
            states_it->second = block_state->GetState(inst)->Copy(parent_->GetLocalAllocator());
            block_state->Materialize(inst);
            ++states_it;
        } else {
            states_it = mat_states.erase(states_it);
        }
    }
}

void EscapeAnalysis::Initialize()
{
    auto &blocks = GetGraph()->GetVectorBlocks();
    block_states_.resize(blocks.size());
    phis_.reserve(blocks.size());
    for (auto block : blocks) {
        if (block != nullptr) {
            if (auto new_block = GetLocalAllocator()->New<BasicBlockState>(GetLocalAllocator())) {
                block_states_[block->GetId()] = new_block;
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

    if (!live_ins_.Run()) {
        return false;
    }

    if (!FindVirtualizableAllocations()) {
        return false;
    }

    if (virtualizable_allocations_.empty()) {
        COMPILER_LOG(DEBUG, PEA) << "No allocations to virtualize";
        return false;
    }

    ScalarReplacement sr {GetGraph(), aliases_, phis_, materialization_info_, save_state_info_};
    sr.Apply(virtualizable_allocations_);
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
    virtualization_changed_ = true;
    while (virtualization_changed_) {
        state_id_ = 1U;
        virtualization_changed_ = false;
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
        auto block_state = GetState(ss->GetBasicBlock());
        // We're trying to materialize an object at the save state predecessing current instruction,
        // as a result the object might be alredy materialized in this basic block. If it actually is
        // then register its materialization at current save state to ensure that on the next iteration
        // it will be materialized here. Otherwise try to find a state owner and materialize it instead
        // of an alias.
        if (auto state = block_state->GetState(input.GetInst())) {
            RegisterMaterialization(ss, state->GetInst());
            block_state->Materialize(input.GetInst());
        } else {
            RegisterMaterialization(ss, input.GetInst());
        }
    }
    Materialize(inst);
}

std::pair<PhiState *, bool> EscapeAnalysis::CreatePhi(BasicBlock *target_block, BasicBlockState *block_state,
                                                      FieldPtr field, ArenaVector<StateOwner> &inputs)
{
    // try to reuse existing phi
    auto it = phis_.at(target_block->GetId()).find(field);
    if (it != phis_.at(target_block->GetId()).end()) {
        auto phi = it->second;
        for (size_t idx = 0; idx < inputs.size(); ++idx) {
            ASSERT(GetState(target_block->GetPredsBlocks()[idx])->GetStateId(inputs[idx]) == MATERIALIZED_ID);
            phi->SetInput(idx, inputs[idx]);
        }
        block_state->Materialize(phi);
        return std::make_pair(phi, false);
    }
    auto field_type = GetGraph()->GetRuntime()->GetFieldType(field);
    auto phi_state = GetLocalAllocator()->New<PhiState>(GetLocalAllocator(), field_type);
    if (phi_state == nullptr) {
        UNREACHABLE();
    }
    auto &preds = target_block->GetPredsBlocks();
    ASSERT(inputs.size() == preds.size());
    bool new_materialization = false;
    for (size_t idx = 0; idx < inputs.size(); ++idx) {
        phi_state->AddInput(inputs[idx]);
        new_materialization = new_materialization || GetState(preds[idx])->GetStateId(inputs[idx]) != MATERIALIZED_ID;
        Materialize(inputs[idx], preds[idx]);
    }
    if (phi_state->IsReference()) {
        // phi is always materialized
        block_state->Materialize(phi_state);
    }
    phis_.at(target_block->GetId())[field] = phi_state;
    return std::make_pair(phi_state, new_materialization);
}

void EscapeAnalysis::MaterializeInBlock(StateOwner inst, BasicBlock *block)
{
    auto block_state = GetState(block);
    inst = block_state->GetState(inst)->GetInst();
    if (block_state->GetStateId(inst) == EscapeAnalysis::MATERIALIZED_ID) {
        return;
    }
    RegisterMaterialization(block, std::get<Inst *>(inst));
    auto inst_state = block_state->GetState(inst);
    block_state->Materialize(inst);
    for (auto &t : inst_state->GetFields()) {
        auto &field_inst = t.second;
        if (block_state->GetStateId(field_inst) == MATERIALIZED_ID) {
            continue;
        }
        MaterializeInBlock(field_inst, block);
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
    auto block_state = GetState(before->GetBasicBlock());
    if (block_state->GetStateId(inst) == EscapeAnalysis::MATERIALIZED_ID) {
        return;
    }
    if (!std::holds_alternative<Inst *>(inst)) {
        return;
    }

    COMPILER_LOG(DEBUG, PEA) << "Materialized " << *std::get<Inst *>(inst) << " before " << *before;
    auto inst_state = block_state->GetState(inst);
    // If an inst is an alias (like NullCheck inst) then get original NewObject inst from the state;
    inst = inst_state->GetInst();
    auto target_inst = std::get<Inst *>(inst);
    // Mark object non-virtualizable if it should be materialized in the same block it was allocated.
    Inst *res = before;
    bool can_materialize_fields = true;
    if (target_inst->GetBasicBlock() == before->GetBasicBlock()) {
        res = target_inst;
        can_materialize_fields = false;
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
    auto prev_state = block_state->GetState(inst);
    block_state->Materialize(inst);
    RegisterMaterialization(res, target_inst);
    for (auto &t : prev_state->GetFields()) {
        auto &field_inst = t.second;
        if (block_state->GetStateId(field_inst) == MATERIALIZED_ID) {
            continue;
        }
        if (can_materialize_fields) {
            Materialize(field_inst, res);
        } else {
            block_state->Materialize(field_inst);
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
    auto &mat_info = materialization_info_.try_emplace(site, alloc->Adapter()).first->second;
    if (mat_info.find(inst) != mat_info.end()) {
        return;
    }
    virtualization_changed_ = true;
    mat_info[inst] = nullptr;
}

// Register all states' virtual fields as "should be materialized" at given site.
bool EscapeAnalysis::RegisterFieldsMaterialization(Inst *site, VirtualState *state, BasicBlockState *block_state,
                                                   const ArenaUnorderedMap<Inst *, VirtualState *> &states)
{
    bool materialized_fields = false;
    for (auto &fields : state->GetFields()) {
        auto field_state_id = block_state->GetStateId(fields.second);
        // Skip already materialized objects and objects already registered for materialization at given site.
        if (field_state_id == MATERIALIZED_ID || (states.count(std::get<Inst *>(fields.second)) != 0)) {
            continue;
        }
        materialized_fields = true;
        ASSERT(std::holds_alternative<Inst *>(fields.second));
        RegisterMaterialization(site, std::get<Inst *>(fields.second));
    }
    return materialized_fields;
}

void EscapeAnalysis::RegisterVirtualObjectFieldsForMaterialization(Inst *ss)
{
    auto states = materialization_info_.find(ss);
    ASSERT(states != materialization_info_.end());

    auto block_state = GetState(ss->GetBasicBlock());
    bool materialized_fields = true;
    while (materialized_fields) {
        materialized_fields = false;
        for (auto &t : states->second) {
            auto state = block_state->GetState(t.first);
            // Object has materialized state
            if (state == nullptr) {
                continue;
            }
            materialized_fields =
                materialized_fields || RegisterFieldsMaterialization(ss, state, block_state, states->second);
        }
    }
}

VirtualState *EscapeAnalysis::CreateVState(Inst *inst)
{
    auto vstate = GetLocalAllocator()->New<VirtualState>(inst, state_id_++, GetLocalAllocator());
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
    auto block_state = GetState(inst->GetBasicBlock());
    auto lhs = inst->GetInput(0).GetInst();
    auto rhs = inst->GetInput(1).GetInst();
    auto lhs_state_id = block_state->GetStateId(lhs);
    auto rhs_state_id = block_state->GetStateId(rhs);
    if (rhs_state_id == MATERIALIZED_ID && lhs_state_id == MATERIALIZED_ID) {
        aliases_.erase(inst);
        return;
    }
    auto same_state = lhs_state_id == rhs_state_id;
    bool cmp_result = same_state == (cc == ConditionCode::CC_EQ);
    aliases_[inst] = GetGraph()->FindOrCreateConstant(cmp_result);
}

void EscapeAnalysis::VisitNewObject(Inst *inst)
{
    VisitSaveStateUser(inst);

    // There are several reasons to materialize an object right at the allocation site:
    // (1) the object is an input for some instruction inside a catch block
    // (2) we already marked the object as one requiring materialization
    bool materialize = inst->GetFlag(inst_flags::Flags::CATCH_INPUT) ||
                       materialization_info_.find(inst) != materialization_info_.end();

    if (!relax_class_restrictions_) {
        auto klass_input = inst->GetInput(0).GetInst();
        if (!materialize) {
            auto opc = klass_input->GetOpcode();
            // (3) object's class originated from some instruction we can't handle
            materialize = opc != Opcode::LoadImmediate && opc != Opcode::LoadAndInitClass && opc != Opcode::LoadClass;
        }
        if (!materialize) {
            RuntimeInterface::ClassPtr klass_ptr;
            if (klass_input->GetOpcode() == Opcode::LoadImmediate) {
                klass_ptr = klass_input->CastToLoadImmediate()->GetObject();
            } else {
                klass_ptr = static_cast<ClassInst *>(klass_input)->GetClass();
            }
            // (4) object's class is not instantiable (for example, it's an interface or an abstract class)
            // (5) we can't apply scalar replacement for this particular class (for example, a class declares a function
            //     to invoke before GC (like LUA's finalizable objects))
            materialize = !GetGraph()->GetRuntime()->IsInstantiable(klass_ptr) ||
                          !GetGraph()->GetRuntime()->CanScalarReplaceObject(klass_ptr);
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
    auto block_state = GetState(inst->GetBasicBlock());

    if (inst->GetFlag(inst_flags::Flags::CATCH_INPUT)) {
        Materialize(inst->GetDataFlowInput(0), inst);
        VisitSaveStateUser(inst);
        return;
    }

    VisitSaveStateUser(inst);

    auto aliased_inst = inst->GetDataFlowInput(0);
    auto aliased_state_id = block_state->GetStateId(aliased_inst);
    block_state->SetStateId(inst, aliased_state_id);
    if (aliased_state_id != MATERIALIZED_ID) {
        aliases_[inst] = block_state->GetState(aliased_inst)->GetInst();
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
    auto it = materialization_info_.find(inst);
    if (it == materialization_info_.end()) {
        return;
    }
    auto block_state = GetState(inst->GetBasicBlock());
    // Ensure that for every object registered for materialization at the save state
    // all fields will be also registered for materialization here.
    RegisterVirtualObjectFieldsForMaterialization(inst);

    ArenaUnorderedMap<Inst *, VirtualState *> &insts_map = it->second;
    ArenaVector<Inst *> pending_insts {GetGraph()->GetLocalAllocator()->Adapter()};
    // If an alias was registered for materialization then try to find original instruction
    // and register it too.
    for (auto &t : insts_map) {
        // Allocation marked as non-virtualizable
        if (t.first == inst) {
            continue;
        }
        if (auto vstate = block_state->GetState(t.first)) {
            if (vstate->GetInst() != t.first && insts_map.count(vstate->GetInst()) == 0) {
                pending_insts.push_back(vstate->GetInst());
            }
        }
    }
    for (auto new_inst : pending_insts) {
        // insert only a key, value will be populated later
        insts_map[new_inst] = nullptr;
    }

    for (auto &t : insts_map) {
        // skip aliases
        if (t.first->GetOpcode() != Opcode::NewObject || t.first == inst) {
            continue;
        }
        auto candidate_inst = t.first;
        if (auto vstate = block_state->GetState(candidate_inst)) {
            // make a snapshot of virtual object's fields to use it during
            // scalar replacement to correctly initialize materialized object
            t.second = vstate->Copy(GetLocalAllocator());
            block_state->Materialize(candidate_inst);
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
    auto block_state = GetState(inst->GetBasicBlock());
    auto input = inst->GetInput(0).GetInst();
    if (auto vstate = block_state->GetState(input)) {
        auto new_obj = vstate->GetInst();
        auto load_class = new_obj->GetInput(0).GetInst();
        if (!load_class->IsClassInst() && load_class->GetOpcode() != Opcode::GetInstanceClass) {
            return;
        }
        aliases_[inst] = load_class;
    }
}

void EscapeAnalysis::FillVirtualInputs(Inst *inst)
{
    auto block_state = GetState(inst->GetBasicBlock());
    auto &states = save_state_info_.try_emplace(inst, GetLocalAllocator()).first->second;
    states.clear();
    // We can iterate over set bits only if it is not the first time we call this method
    // for the inst.
    for (size_t input_idx = 0; input_idx < inst->GetInputsCount(); ++input_idx) {
        auto input_inst = inst->GetDataFlowInput(input_idx);
        if (!DataType::IsReference(input_inst->GetType())) {
            continue;
        }
        if (block_state->GetStateId(input_inst) != MATERIALIZED_ID) {
            states.SetBit(input_idx);
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
    auto field_inst_id = vstate->GetFieldOrDefault(field, ZERO_INST);
    if (DataType::IsReference(inst->GetType())) {
        GetState(inst->GetBasicBlock())->SetStateId(inst, GetState(inst->GetBasicBlock())->GetStateId(field_inst_id));
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

    auto block_state = GetState(inst->GetBasicBlock());

    // If an instruction is materialized at save state user
    // then it should be materialized before the save state too.
    auto virtual_inputs = save_state_info_.at(ss);
    for (auto input_idx : virtual_inputs.GetSetBitsIndices()) {
        auto input_inst = ss->GetInput(input_idx).GetInst();
        if (block_state->GetStateId(input_inst) == MATERIALIZED_ID) {
            Materialize(input_inst, ss);
        }
    }
}

bool EscapeAnalysis::ProcessBlock(BasicBlock *block, size_t depth)
{
    if (block->IsMarked(visited_)) {
        return true;
    }
    if (AllPredecessorsVisited(block)) {
        merge_processor_.MergeStates(block);
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
    auto start_it = find(rpo.begin(), rpo.end(), header);
    // There should be only one visited predecessor.
    // Use its state as initial loop header's state (instead of executing MergeStates).
    auto header_pred = *std::find_if(header->GetPredsBlocks().begin(), header->GetPredsBlocks().end(),
                                     [marker = visited_](auto b) { return b->IsMarked(marker); });
    auto header_state = GetState(header_pred)->Copy(GetGraph()->GetLocalAllocator());
    // Set materialized states for all phis
    for (auto phi : header->PhiInsts()) {
        if (DataType::IsReference(phi->GetType())) {
            header_state->Materialize(phi);
        }
    }
    block_states_[header->GetId()] = header_state;
    // Copy header's initial state to compare it after loop processing
    header_state = header_state->Copy(GetGraph()->GetLocalAllocator());
    VisitBlockInternal(header);

    bool state_changed = true;
    while (state_changed) {
        header->SetMarker(visited_);
        // Reset visited marker for the loop and all its nested loops
        for (auto it = std::next(start_it), end = rpo.end(); it != end; ++it) {
            if ((*it)->GetLoop() == header->GetLoop() || (*it)->GetLoop()->IsInside(header->GetLoop())) {
                (*it)->ResetMarker(visited_);
            }
        }

        // Process loop and its nested loops only
        for (auto it = std::next(start_it), end = rpo.end(); it != end; ++it) {
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
        merge_processor_.MergeStates(header);
        // Check if merged state differs from previous loop header's state
        state_changed = !header_state->Equals(GetState(header));
        if (state_changed) {
            header_state = GetState(header)->Copy(GetLocalAllocator());
        }
        VisitBlockInternal(header);
    }
    return true;
}

SaveStateInst *ScalarReplacement::CopySaveState(Inst *inst, [[maybe_unused]] Inst *except)
{
    auto ss = inst->CastToSaveState();
    auto copy = static_cast<SaveStateInst *>(ss->Clone(graph_));
    auto &virt_insts = save_state_liveness_.try_emplace(copy, save_state_liveness_.at(inst)).first->second;
    for (size_t input_idx = 0, copy_idx = 0; input_idx < ss->GetInputsCount(); input_idx++) {
        copy->AppendInput(inst->GetInput(input_idx));
        copy->SetVirtualRegister(copy_idx++, ss->GetVirtualRegister(input_idx));

        if (inst->GetDataFlowInput(input_idx) == except || inst->GetInput(input_idx).GetInst() == except) {
            virt_insts.SetBit(input_idx);
            continue;
        }

        auto it = aliases_.find(inst->GetInput(input_idx).GetInst());
        if (it != aliases_.end() && std::holds_alternative<Inst *>(it->second) &&
            std::get<Inst *>(it->second) == except) {
            virt_insts.SetBit(input_idx);
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
            auto phi_inst = graph_->CreateInstPhi(state->GetType(), bb->GetGuestPc());
            allocated_phis_[state] = phi_inst;
            bb->AppendPhi(phi_inst);
        }
    }
}

void ScalarReplacement::MaterializeObjects()
{
    for (auto &[site, state] : materialization_sites_) {
        if (std::holds_alternative<BasicBlock *>(site)) {
            MaterializeInEmptyBlock(std::get<BasicBlock *>(site), state);
            continue;
        }
        auto site_inst = std::get<Inst *>(site);
        if (site_inst->GetOpcode() == Opcode::SaveState) {
            MaterializeAtExistingSaveState(site_inst->CastToSaveState(), state);
        } else {
            MaterializeAtNewSaveState(site_inst, state);
        }
    }
}

void ScalarReplacement::MaterializeAtExistingSaveState(SaveStateInst *save_state,
                                                       ArenaUnorderedMap<Inst *, VirtualState *> &state)
{
    auto previous_save_state = save_state;
    for (auto t : state) {
        if (t.second == nullptr || t.first->GetOpcode() != Opcode::NewObject) {
            continue;
        }
        auto orig_alloc = t.first;
        auto curr_ss = CopySaveState(previous_save_state, orig_alloc);
        previous_save_state = curr_ss;

        auto new_alloc = CreateNewObject(orig_alloc, curr_ss);
        auto &allocs =
            materialized_objects_.try_emplace(orig_alloc, graph_->GetLocalAllocator()->Adapter()).first->second;
        allocs.push_back(new_alloc);

        InitializeObject(new_alloc, save_state, t.second);
    }
}

void ScalarReplacement::MaterializeAtNewSaveState(Inst *site, ArenaUnorderedMap<Inst *, VirtualState *> &state)
{
    auto block = site->GetBasicBlock();
    auto ss_insertion_point = site;
    for (auto t : state) {
        if (t.second == nullptr || t.first->GetOpcode() != Opcode::NewObject || t.first == site) {
            continue;
        }
        auto orig_alloc = t.first;
        auto curr_ss = graph_->CreateInstSaveState();
        if (auto caller_inst = FindCallerInst(block, site)) {
            curr_ss->SetMethod(caller_inst->GetCallMethod());
            curr_ss->SetCallerInst(caller_inst);
            curr_ss->SetInliningDepth(caller_inst->GetSaveState()->GetInliningDepth() + 1);
        } else {
            curr_ss->SetMethod(graph_->GetMethod());
        }
        block->InsertBefore(curr_ss, ss_insertion_point);
        ss_insertion_point = curr_ss;

        auto new_alloc = CreateNewObject(orig_alloc, curr_ss);
        auto &allocs =
            materialized_objects_.try_emplace(orig_alloc, graph_->GetLocalAllocator()->Adapter()).first->second;
        allocs.push_back(new_alloc);

        InitializeObject(new_alloc, site, t.second);
    }
}

void ScalarReplacement::MaterializeInEmptyBlock(BasicBlock *block, ArenaUnorderedMap<Inst *, VirtualState *> &state)
{
    ASSERT(block->IsEmpty());
    for (auto t : state) {
        if (t.second == nullptr || t.first->GetOpcode() != Opcode::NewObject) {
            continue;
        }
        auto orig_alloc = t.first;
        auto curr_ss = graph_->CreateInstSaveState();
        if (auto caller_inst = FindCallerInst(block)) {
            curr_ss->SetMethod(caller_inst->GetCallMethod());
            curr_ss->SetCallerInst(caller_inst);
            curr_ss->SetInliningDepth(caller_inst->GetSaveState()->GetInliningDepth() + 1);
        } else {
            curr_ss->SetMethod(graph_->GetMethod());
        }
        block->PrependInst(curr_ss);

        auto new_alloc = CreateNewObject(orig_alloc, curr_ss);
        auto &allocs =
            materialized_objects_.try_emplace(orig_alloc, graph_->GetLocalAllocator()->Adapter()).first->second;
        allocs.push_back(new_alloc);

        InitializeObject(new_alloc, nullptr, t.second);
    }
}

Inst *ScalarReplacement::CreateNewObject(Inst *original_inst, Inst *save_state)
{
    auto new_alloc = graph_->CreateInstNewObject(
        original_inst->GetType(), original_inst->GetPc(), original_inst->GetInput(0).GetInst(), save_state,
        original_inst->CastToNewObject()->GetTypeId(), original_inst->CastToNewObject()->GetMethod());
    save_state->GetBasicBlock()->InsertAfter(new_alloc, save_state);
    COMPILER_LOG(DEBUG, PEA) << "Materialized " << original_inst->GetId() << " at SavePoint " << save_state->GetId()
                             << " as " << *new_alloc;
    return new_alloc;
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
            auto call_inst = static_cast<CallInst *>(inst);
            auto is_inlined = call_inst->IsInlined();
            if (is_inlined && depth == 0) {
                return call_inst;
            }
            if (is_inlined) {
                depth--;
            }
        }
        block = block->GetDominator();
        start = nullptr;
    }
    return nullptr;
}

void ScalarReplacement::InitializeObject(Inst *alloc, Inst *inst_before, VirtualState *state)
{
    for (auto &[field, field_source] : state->GetFields()) {
        Inst *field_source_inst {nullptr};
        if (std::holds_alternative<Inst *>(field_source)) {
            field_source_inst = std::get<Inst *>(field_source);
        } else if (std::holds_alternative<PhiState *>(field_source)) {
            auto phis_state = std::get<PhiState *>(field_source);
            field_source_inst = allocated_phis_[phis_state];
            ASSERT(field_source_inst != nullptr);
        } else {
            // Field initialized with zero constant / nullptr, it's a default value,
            // so there is no need to insert explicit store instruction.
            continue;
        }

        auto field_type = graph_->GetRuntime()->GetFieldType(field);
        auto store = graph_->CreateInstStoreObject(field_type, alloc->GetPc(), alloc, field_source_inst,
                                                   graph_->GetRuntime()->GetFieldId(field), graph_->GetMethod(), field,
                                                   graph_->GetRuntime()->IsFieldVolatile(field),
                                                   DataType::IsReference(field_type));
        if (inst_before != nullptr) {
            inst_before->GetBasicBlock()->InsertBefore(store, inst_before);
        } else {
            alloc->GetBasicBlock()->AppendInst(store);
        }
    }
}

Inst *ScalarReplacement::ResolveAlias(const StateOwner &alias, const Inst *inst)
{
    if (std::holds_alternative<PhiState *>(alias)) {
        auto phi_inst = allocated_phis_[std::get<PhiState *>(alias)];
        ASSERT(phi_inst != nullptr);
        return phi_inst;
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
    ArenaVector<Inst *> replace_inputs {graph_->GetLocalAllocator()->Adapter()};
    for (auto &[inst, alias] : aliases_) {
        Inst *replacement = ResolveAlias(alias, inst);
        if (replacement != inst && replacement != nullptr) {
            for (auto &user : inst->GetUsers()) {
                replace_inputs.push_back(user.GetInst());
            }
        }

        bool replaced = false;
        auto inst_type_size = DataType::GetTypeSize(inst->GetType(), graph_->GetArch());
        if (replacement != nullptr && inst->GetOpcode() == Opcode::LoadObject &&
            inst_type_size < DataType::GetTypeSize(replacement->GetType(), graph_->GetArch())) {
            // In case of loads/stores explicit casts could be eliminated before scalar replacement.
            // To use correct values after load's replacement with a value stored into a field we
            // need to insert some casts back.
            replaced = true;
            for (auto user : replace_inputs) {
                auto cast = graph_->CreateInstCast(inst->GetType(), inst->GetPc(), replacement, replacement->GetType());
                inst->InsertBefore(cast);
                replacement = cast;
                user->ReplaceInput(inst, replacement);
            }
        } else if (replacement != nullptr && replacement->IsClassInst()) {
            auto class_imm = graph_->CreateInstLoadImmediate(DataType::REFERENCE, replacement->GetPc(),
                                                             static_cast<ClassInst *>(replacement)->GetClass());
            replacement->InsertAfter(class_imm);
            replacement = class_imm;
        }

        if (replacement != nullptr) {
            COMPILER_LOG(DEBUG, PEA) << "Replacing " << *inst << " with " << *replacement;
        }
        if (!replaced) {
            for (auto user : replace_inputs) {
                user->ReplaceInput(inst, replacement);
            }
        }
        replace_inputs.clear();
        EnqueueForRemoval(inst);
    }
}

void ScalarReplacement::ResolvePhiInputs()
{
    for (auto [state, inst] : allocated_phis_) {
        auto preds = inst->GetBasicBlock()->GetPredsBlocks();
        auto inputs = state->GetInputs();
        ASSERT(preds.size() == inputs.size());
        for (size_t idx = 0; idx < inputs.size(); idx++) {
            auto input_inst = ResolveAlias(inputs[idx], inst);
            ASSERT(input_inst != nullptr);
            if (input_inst->GetOpcode() == Opcode::NewObject) {
                input_inst = ResolveAllocation(input_inst, preds[idx]);
            }
            inst->AppendInput(input_inst);
        }
    }
}

Inst *ScalarReplacement::ResolveAllocation(Inst *inst, BasicBlock *block)
{
    ASSERT(inst != nullptr);
    auto it = materialized_objects_.find(inst);
    if (it == materialized_objects_.end()) {
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
    for (auto &[site, virtual_objects] : save_state_liveness_) {
        bool is_call = site->IsCall();
        ASSERT(!is_call || static_cast<CallInst *>(site)->IsInlined());
        // Remove virtual inputs (i.e. objects that are not alive)
        for (ssize_t input_idx = site->GetInputsCount() - 1; input_idx >= 0; --input_idx) {
            if (!virtual_objects.GetBit(input_idx)) {
                continue;
            }
            if (is_call) {
                site->SetInput(input_idx, graph_->GetOrCreateNullPtr());
                continue;
            }
            site->RemoveInput(input_idx);
        }
    }
}

void ScalarReplacement::UpdateAllocationUsers()
{
    ArenaVector<std::tuple<Inst *, size_t, Inst *>> queue {graph_->GetLocalAllocator()->Adapter()};
    for (auto &[old_alloc, new_allocs] : materialized_objects_) {
        // At these point:
        // - all aliases should be already processed and corresponding users will be enqueued for removal
        // - all save states and inlined calls will be already processed
        // - inputs for newly allocated phis will be processed as well
        for (auto &user : old_alloc->GetUsers()) {
            auto user_inst = user.GetInst();
            if (IsEnqueuedForRemoval(user_inst)) {
                continue;
            }
            // There might be multiple materializations of the same object so we need to find
            // the one dominating current user.
            auto user_block = user_inst->IsPhi() ? user_inst->CastToPhi()->GetPhiInputBb(user.GetIndex())
                                                 : user_inst->GetBasicBlock();
            auto replacement_it = std::find_if(new_allocs.begin(), new_allocs.end(), [user_block](auto new_alloc) {
                return new_alloc->GetBasicBlock()->IsDominate(user_block);
            });
            ASSERT(replacement_it != new_allocs.end());
            queue.emplace_back(user_inst, user.GetIndex(), *replacement_it);
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
    inst->SetMarker(remove_inst_marker_);
    removal_queue_.push_back(inst);
}

bool ScalarReplacement::IsEnqueuedForRemoval(Inst *inst) const
{
    return inst->IsMarked(remove_inst_marker_);
}

static bool InputSizeGtSize(Inst *inst)
{
    uint32_t size = 0;
    auto arch = inst->GetBasicBlock()->GetGraph()->GetArch();

    for (auto input : inst->GetInputs()) {
        auto input_size = DataType::GetTypeSize(input.GetInst()->GetType(), arch);
        size = size < input_size ? input_size : size;
    }
    return size > DataType::GetTypeSize(inst->GetType(), arch);
}

/* Phi instruction can have inputs that are wider than
 * the phi type, we have to insert casts, so that, the
 * phi takes the types matching it's own type */
void ScalarReplacement::FixPhiInputTypes()
{
    auto arch = graph_->GetArch();
    for (auto alphi : allocated_phis_) {
        auto phi = alphi.second;
        if (LIKELY(!InputSizeGtSize(phi) || !phi->HasUsers())) {
            continue;
        }
        auto phi_size = DataType::GetTypeSize(phi->GetType(), arch);
        for (auto &i : phi->GetInputs()) {
            auto input = i.GetInst();
            /* constants are taken care of by constprop and existing
             * phis have to be properly casted already */
            if (LIKELY(phi_size == DataType::GetTypeSize(input->GetType(), arch) ||
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
    for (auto inst : removal_queue_) {
        COMPILER_LOG(DEBUG, PEA) << "Removing inst " << inst->GetId();
        inst->GetBasicBlock()->RemoveInst(inst);
    }
    FixPhiInputTypes();
}

void ScalarReplacement::Apply(ArenaUnorderedSet<Inst *> &candidates)
{
    remove_inst_marker_ = graph_->NewMarker();
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
    graph_->EraseMarker(remove_inst_marker_);
}

void ScalarReplacement::PatchSaveStates()
{
    ArenaVector<ArenaUnorderedSet<Inst *>> liveness {graph_->GetLocalAllocator()->Adapter()};
    for (size_t idx = 0; idx < graph_->GetVectorBlocks().size(); ++idx) {
        liveness.emplace_back(graph_->GetLocalAllocator()->Adapter());
    }

    auto &rpo = graph_->GetBlocksRPO();
    for (auto it = rpo.rbegin(), end = rpo.rend(); it != end; ++it) {
        auto block = *it;
        PatchSaveStatesInBlock(block, liveness);
    }
}

void ScalarReplacement::FillLiveInsts(BasicBlock *block, ArenaUnorderedSet<Inst *> &live_ins,
                                      ArenaVector<ArenaUnorderedSet<Inst *>> &liveness)
{
    for (auto succ : block->GetSuccsBlocks()) {
        live_ins.insert(liveness[succ->GetId()].begin(), liveness[succ->GetId()].end());
        for (auto phi_inst : succ->PhiInsts()) {
            auto phi_input = phi_inst->CastToPhi()->GetPhiInput(block);
            if (phi_input->GetBasicBlock() != succ && DataType::IsReference(phi_input->GetType())) {
                live_ins.insert(phi_input);
            }
        }
    }
}

// Compute live ref-valued insturctions at each save state and insert any missing live instruction into a save state.
void ScalarReplacement::PatchSaveStatesInBlock(BasicBlock *block, ArenaVector<ArenaUnorderedSet<Inst *>> &liveness)
{
    auto &live_ins = liveness[block->GetId()];
    FillLiveInsts(block, live_ins, liveness);

    auto loop = block->GetLoop();
    bool loop_is_header = !loop->IsRoot() && loop->GetHeader() == block;
    if (loop_is_header) {
        for (auto inst : block->InstsReverse()) {
            if (IsEnqueuedForRemoval(inst)) {
                continue;
            }
            AddLiveInputs(inst, live_ins);
        }
    }
    for (auto inst : block->InstsReverse()) {
        if (inst->IsSaveState()) {
            PatchSaveState(static_cast<SaveStateInst *>(inst), live_ins);
        }
        if (DataType::IsReference(inst->GetType())) {
            live_ins.erase(inst);
        }
        if (IsEnqueuedForRemoval(inst)) {
            continue;
        }
        if (!loop_is_header) {
            AddLiveInputs(inst, live_ins);
        }
    }

    if (!loop->IsRoot() && loop->GetHeader() == block) {
        for (auto phi_inst : block->PhiInsts()) {
            bool propagate_through_loop = false;
            for (auto &user : phi_inst->GetUsers()) {
                propagate_through_loop =
                    propagate_through_loop ||
                    (user.GetInst()->IsPhi() && user.GetInst()->GetBasicBlock() == phi_inst->GetBasicBlock());
            }
            if (!propagate_through_loop) {
                live_ins.erase(phi_inst);
            }
        }
        PatchSaveStatesInLoop(loop, live_ins, liveness);
    }
    for (auto phi_inst : block->PhiInsts()) {
        live_ins.erase(phi_inst);
    }
}

void ScalarReplacement::AddLiveInputs(Inst *inst, ArenaUnorderedSet<Inst *> &live_ins)
{
    for (auto &input : inst->GetInputs()) {
        auto input_inst = inst->GetDataFlowInput(input.GetInst());
        if (!DataType::IsReference(input_inst->GetType()) || input_inst->IsStore() || !input_inst->IsMovableObject()) {
            continue;
        }
        live_ins.insert(input_inst);
    }
}

void ScalarReplacement::PatchSaveStatesInLoop(Loop *loop, ArenaUnorderedSet<Inst *> &loop_live_ins,
                                              ArenaVector<ArenaUnorderedSet<Inst *>> &liveness)
{
    for (auto loop_block : graph_->GetVectorBlocks()) {
        if (loop_block == nullptr) {
            continue;
        }
        if (loop_block->GetLoop() != loop && !loop_block->GetLoop()->IsInside(loop)) {
            continue;
        }
        if (loop_block != loop->GetHeader()) {
            auto &loop_block_live_ins = liveness[loop_block->GetId()];
            loop_block_live_ins.insert(loop_live_ins.begin(), loop_live_ins.end());
        }
        for (auto inst : loop_block->Insts()) {
            if (inst->IsSaveState()) {
                PatchSaveState(static_cast<SaveStateInst *>(inst), loop_live_ins);
            }
        }
    }
}

void ScalarReplacement::PatchSaveState(SaveStateInst *save_state, ArenaUnorderedSet<Inst *> &live_instructions)
{
    for (auto inst : live_instructions) {
        inst = Inst::GetDataFlowInput(inst);
        if (!inst->IsMovableObject()) {
            continue;
        }
        auto inputs = save_state->GetInputs();
        auto it = std::find_if(inputs.begin(), inputs.end(), [inst](auto &in) { return in.GetInst() == inst; });
        if (it != inputs.end()) {
            continue;
        }
        save_state->AppendBridge(inst);
    }
}
}  // namespace panda::compiler
