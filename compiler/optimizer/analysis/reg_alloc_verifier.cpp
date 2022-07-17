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

#include "optimizer/analysis/reg_alloc_verifier.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/live_registers.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/code_generator/callconv.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/optimizations/regalloc/reg_type.h"

namespace panda::compiler {
namespace {
std::string ToString(LocationState::State state)
{
    switch (state) {
        case LocationState::State::UNKNOWN:
            return "UNKNOWN";
        case LocationState::State::KNOWN:
            return "KNOWN";
        case LocationState::State::CONFLICT:
            return "CONFLICT";
        default:
            UNREACHABLE();
    }
}

const LocationState &GetPhiLocationState(const BlockState &state, const ArenaVector<LocationState> &immediates,
                                         const LifeIntervals *interval, bool is_high)
{
    auto location = interval->GetLocation();
    if (location.IsStack()) {
        ASSERT(!is_high);
        return state.GetStack(location.GetValue());
    }
    if (location.IsConstant()) {
        ASSERT(location.GetValue() < immediates.size());
        return immediates[location.GetValue()];
    }
    if (location.IsFpRegister()) {
        ASSERT(IsFloatType(interval->GetType()));
        return state.GetVReg(location.GetValue() + (is_high ? 1U : 0U));
    }
    ASSERT(location.IsRegister());
    return state.GetReg(location.GetValue() + (is_high ? 1U : 0U));
}

LocationState &GetPhiLocationState(BlockState *state, const LifeIntervals *interval, bool is_high)
{
    auto location = interval->GetLocation();
    if (location.IsStack()) {
        ASSERT(!is_high);
        return state->GetStack(location.GetValue());
    }
    if (location.IsFpRegister()) {
        ASSERT(IsFloatType(interval->GetType()));
        return state->GetVReg(location.GetValue() + (is_high ? 1U : 0U));
    }
    ASSERT(location.IsRegister());
    return state->GetReg(location.GetValue() + (is_high ? 1U : 0U));
}

bool MergeImpl(const ArenaVector<LocationState> &from, ArenaVector<LocationState> *to)
{
    bool updated = false;
    for (size_t offset = 0; offset < from.size(); ++offset) {
        if (to->at(offset).ShouldSkip()) {
            to->at(offset).SetSkip(false);
            continue;
        }
        updated |= to->at(offset).Merge(from[offset]);
    }
    return updated;
}

class BlockStates {
public:
    BlockStates(size_t regs, size_t vregs, size_t stack_slots, size_t stack_params, ArenaAllocator *allocator)
        : start_state_(regs, vregs, stack_slots, stack_params, allocator),
          end_state_(regs, vregs, stack_slots, stack_params, allocator)
    {
    }
    ~BlockStates() = default;
    NO_COPY_SEMANTIC(BlockStates);
    NO_MOVE_SEMANTIC(BlockStates);

    BlockState &GetStart()
    {
        return start_state_;
    }

    BlockState &GetEnd()
    {
        return end_state_;
    }

    BlockState &ResetEndToStart()
    {
        end_state_.Copy(&start_state_);
        return end_state_;
    }

    bool IsUpdated() const
    {
        return updated_;
    }

    void SetUpdated(bool upd)
    {
        updated_ = upd;
    }

    bool IsVisited() const
    {
        return visited_;
    }

    void SetVisited(bool visited)
    {
        visited_ = visited;
    }

private:
    BlockState start_state_;
    BlockState end_state_;
    bool updated_ {false};
    bool visited_ {false};
};

void InitStates(ArenaUnorderedMap<uint32_t, BlockStates> *blocks, const Graph *graph)
{
    auto used_regs = graph->GetUsedRegs<DataType::INT64>()->size();
    auto used_vregs = graph->GetUsedRegs<DataType::FLOAT64>()->size();
    for (auto &bb : graph->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }
        [[maybe_unused]] auto res = blocks->try_emplace(bb->GetId(), used_regs, used_vregs, MAX_NUM_STACK_SLOTS,
                                                        MAX_NUM_STACK_SLOTS, graph->GetLocalAllocator());
        ASSERT(res.second);
    }
}

void CheckAllBlocksVisited([[maybe_unused]] const ArenaUnorderedMap<uint32_t, BlockStates> &blocks)
{
#ifndef NDEBUG
    for (auto &block_state : blocks) {
        ASSERT(block_state.second.IsVisited());
    }
#endif
}

bool IsRegisterPair(Location location, DataType::Type type, Graph *graph)
{
    if (!location.IsRegister()) {
        return false;
    }
    return Is64Bits(type, graph->GetArch()) && !Is64BitsArch(graph->GetArch());
}

bool IsRegisterPair(const LifeIntervals *li)
{
    if (!li->HasReg()) {
        return false;
    }
    return IsRegisterPair(Location::MakeRegister(li->GetReg()), li->GetType(),
                          li->GetInst()->GetBasicBlock()->GetGraph());
}
}  // namespace

bool LocationState::Merge(const LocationState &other)
{
    if (state_ == LocationState::State::CONFLICT) {
        return false;
    }
    if (state_ == LocationState::State::UNKNOWN) {
        state_ = other.state_;
        id_ = other.id_;
        return other.state_ != LocationState::State::UNKNOWN;
    }
    if (other.state_ == LocationState::State::CONFLICT) {
        state_ = LocationState::State::CONFLICT;
        return true;
    }
    if (other.state_ == LocationState::State::KNOWN && state_ == LocationState::State::KNOWN && other.id_ != id_) {
        state_ = LocationState::State::CONFLICT;
        return true;
    }
    return false;
}

BlockState::BlockState(size_t regs, size_t vregs, size_t stack_slots, size_t stack_params, ArenaAllocator *alloc)
    : regs_(regs, alloc->Adapter()),
      vregs_(vregs, alloc->Adapter()),
      stack_(stack_slots, alloc->Adapter()),
      stack_param_(stack_params, alloc->Adapter()),
      stack_arg_(0, alloc->Adapter())

{
}

bool BlockState::Merge(const BlockState &state, const PhiInstSafeIter &phis, BasicBlock *pred,
                       const ArenaVector<LocationState> &immediates, const LivenessAnalyzer &la)
{
    bool updated = false;
    /* Phi instructions are resolved into moves at the end of predecessor blocks,
     * but corresponding locations contain ids of instructions merged by a phi.
     * Phi's location at the beginning of current block is only updated when
     * the same location at the end of predecessor holds result of an instruction
     * that is a phi's input. Otherwise phi's location state will be either remains unknown,
     * or it will be merged to conflicting state.
     */
    constexpr std::array<bool, 2U> IS_HIGH = {false, true};
    for (auto phi : phis) {
        auto phi_interval = la.GetInstLifeIntervals(phi);
        auto input = phi->CastToPhi()->GetPhiDataflowInput(pred);
        for (size_t is_high_idx = 0; is_high_idx < (IsRegisterPair(phi_interval) ? 2U : 1U); is_high_idx++) {
            auto &input_state = GetPhiLocationState(state, immediates, phi_interval, IS_HIGH[is_high_idx]);
            auto &phi_state = GetPhiLocationState(this, phi_interval, IS_HIGH[is_high_idx]);

            if (input_state.GetState() != LocationState::State::KNOWN) {
                continue;
            }
            if (input_state.IsValid(input)) {
                updated = phi_state.GetState() != LocationState::State::KNOWN || phi_state.GetId() != phi->GetId();
                phi_state.SetId(phi->GetId());
                // don't merge it
                phi_state.SetSkip(true);
            }
        }
    }
    updated |= MergeImpl(state.regs_, &regs_);
    updated |= MergeImpl(state.vregs_, &vregs_);
    updated |= MergeImpl(state.stack_, &stack_);
    updated |= MergeImpl(state.stack_param_, &stack_param_);
    // note that stack_arg_ is not merged as it is only used during call handing
    return updated;
}

void BlockState::Copy(BlockState *state)
{
    std::copy(state->regs_.begin(), state->regs_.end(), regs_.begin());
    std::copy(state->vregs_.begin(), state->vregs_.end(), vregs_.begin());
    std::copy(state->stack_.begin(), state->stack_.end(), stack_.begin());
    std::copy(state->stack_param_.begin(), state->stack_param_.end(), stack_param_.begin());
    // note that stack_arg_ is not merged as it is only used during call handing
}

RegAllocVerifier::RegAllocVerifier(Graph *graph, bool save_live_regs_on_call)
    : Analysis(graph),
      immediates_(graph->GetLocalAllocator()->Adapter()),
      saved_regs_(GetGraph()->GetLocalAllocator()->Adapter()),
      saved_vregs_(GetGraph()->GetLocalAllocator()->Adapter()),
      save_live_regs_(save_live_regs_on_call)
{
}

void RegAllocVerifier::InitImmediates()
{
    immediates_.clear();
    for (size_t imm_slot = 0; imm_slot < GetGraph()->GetSpilledConstantsCount(); ++imm_slot) {
        auto con = GetGraph()->GetSpilledConstant(imm_slot);
        immediates_.emplace_back(LocationState::State::KNOWN, con->GetId());
    }
}

LocationState &RegAllocVerifier::GetLocationState(Location location)
{
    auto loc = location.GetKind();
    auto offset = location.GetValue();

    if (loc == LocationType::STACK) {
        return current_state_->GetStack(offset);
    }
    if (loc == LocationType::REGISTER) {
        return current_state_->GetReg(offset);
    }
    if (loc == LocationType::FP_REGISTER) {
        return current_state_->GetVReg(offset);
    }
    if (loc == LocationType::STACK_ARGUMENT) {
        return current_state_->GetStackArg(offset);
    }
    if (loc == LocationType::STACK_PARAMETER) {
        return current_state_->GetStackParam(offset);
    }
    ASSERT(loc == LocationType::IMMEDIATE);
    ASSERT(offset < immediates_.size());
    return immediates_[offset];
}

// Apply callback to a location corresponding to loc, offset and type.
// If location is represented by registers pair then apply the callback to both parts.
// Return true if all invocations of the callback returned true.
template <typename T>
bool RegAllocVerifier::ForEachLocation(Location location, DataType::Type type, T callback)
{
    bool success = callback(GetLocationState(location));
    if (IsRegisterPair(location, type, GetGraph())) {
        auto pired_reg = Location::MakeRegister(location.GetValue() + 1);
        success &= callback(GetLocationState(pired_reg));
    }
    return success;
}

void RegAllocVerifier::UpdateLocation(Location location, DataType::Type type, uint32_t inst_id)
{
    ForEachLocation(location, type, [inst_id](auto &state) {
        state.SetId(inst_id);
        return true;
    });
}

void RegAllocVerifier::HandleDest(Inst *inst)
{
    if (inst->NoDest()) {
        return;
    }
    auto type = inst->GetType();
    if (GetGraph()->GetZeroReg() != INVALID_REG && inst->GetDstReg() == GetGraph()->GetZeroReg()) {
        UpdateLocation(Location::MakeRegister(inst->GetDstReg()), type, LocationState::ZERO_INST);
        return;
    }

    if (inst->GetDstCount() == 1) {
        UpdateLocation(Location::MakeRegister(inst->GetDstReg(), type), type, inst->GetId());
        return;
    }

    size_t handled_users = 0;
    for (auto &user : inst->GetUsers()) {
        if (IsPseudoUserOfMultiOutput(user.GetInst())) {
            auto inst_id = user.GetInst()->GetId();
            auto idx = user.GetInst()->CastToLoadPairPart()->GetSrcRegIndex();
            UpdateLocation(Location::MakeRegister(inst->GetDstReg(idx), type), type, inst_id);
            ++handled_users;
        }
    }
    ASSERT(handled_users == inst->GetDstCount());
}

BasicBlock *PropagateBlockState(BasicBlock *current_block, BlockState *current_state,
                                const ArenaVector<LocationState> &immediates,
                                ArenaUnorderedMap<uint32_t, BlockStates> *blocks)
{
    BasicBlock *next_block {nullptr};
    auto &la = current_block->GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto succ : current_block->GetSuccsBlocks()) {
        auto phis = succ->PhiInstsSafe();
        auto &succ_state = blocks->at(succ->GetId());
        bool merge_updated = succ_state.GetStart().Merge(*current_state, phis, current_block, immediates, la);
        // if merged did not update the state, but successor is not visited yet then force its visit
        merge_updated |= !succ_state.IsVisited();
        succ_state.SetUpdated(succ_state.IsUpdated() || merge_updated);
        if (merge_updated) {
            next_block = succ;
        }
    }
    return next_block;
}

bool RegAllocVerifier::RunImpl()
{
    ArenaUnorderedMap<uint32_t, BlockStates> blocks(GetGraph()->GetLocalAllocator()->Adapter());
    InitStates(&blocks, GetGraph());
    InitImmediates();
    implicit_null_check_handled_marker_ = GetGraph()->NewMarker();

    success_ = true;
    current_block_ = GetGraph()->GetStartBlock();
    while (success_) {
        ASSERT(current_block_ != nullptr);

        blocks.at(current_block_->GetId()).SetUpdated(false);
        blocks.at(current_block_->GetId()).SetVisited(true);
        // use state at the end of the block as current state and reset it to the state
        // at the beginning of the block before processing.
        current_state_ = &blocks.at(current_block_->GetId()).ResetEndToStart();

        ProcessCurrentBlock();
        if (!success_) {
            break;
        }

        current_block_ = PropagateBlockState(current_block_, current_state_, immediates_, &blocks);
        if (current_block_ != nullptr) {
            continue;
        }
        // pick a block pending processing
        auto it = std::find_if(blocks.begin(), blocks.end(), [](auto &t) { return t.second.IsUpdated(); });
        if (it == blocks.end()) {
            break;
        }
        auto bbs = GetGraph()->GetVectorBlocks();
        auto bb_id = it->first;
        auto block_it =
            std::find_if(bbs.begin(), bbs.end(), [bb_id](auto bb) { return bb != nullptr && bb->GetId() == bb_id; });
        ASSERT(block_it != bbs.end());
        current_block_ = *block_it;
    }

    GetGraph()->EraseMarker(implicit_null_check_handled_marker_);
    if (success_) {
        CheckAllBlocksVisited(blocks);
    }
    return success_;
}

void RegAllocVerifier::ProcessCurrentBlock()
{
    for (auto inst : current_block_->InstsSafe()) {
        if (!success_) {
            return;
        }
        if (IsSaveRestoreRegisters(inst)) {
            HandleSaveRestoreRegisters(inst);
            continue;
        }
        if (inst->GetOpcode() == Opcode::ReturnInlined) {
            // ReturnInlined is pseudo instruction having SaveState input that
            // only prolongs SaveState input's lifetime. There are no guarantees that
            // SaveState's inputs will be stored in locations specified in SaveState
            // at ReturnInlined.
            continue;
        }

        TryHandleImplicitNullCheck(inst);
        // pseudo user of multi output will be handled by HandleDest
        if (inst->IsSaveState() || IsPseudoUserOfMultiOutput(inst)) {
            continue;
        }

        if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
            ASSERT(inst->GetDstReg() == INVALID_REG);
            continue;
        }

        if (inst->IsParameter()) {
            HandleParameter(inst->CastToParameter());
        } else if (inst->IsSpillFill()) {
            HandleSpillFill(inst->CastToSpillFill());
        } else if (inst->IsConst()) {
            HandleConst(inst->CastToConstant());
        } else {
            HandleInst(inst);
        }
    }
}

// Set inst's id to corresponding location
void RegAllocVerifier::HandleParameter(ParameterInst *inst)
{
    auto sf = inst->GetLocationData();
    UpdateLocation(sf.GetDst(), inst->GetType(), inst->GetId());
}

bool SameState(LocationState &left, LocationState &right)
{
    return left.GetId() == right.GetId() && left.GetState() == right.GetState();
}

// Copy inst's id from source location to destination location,
// source location should contain known value.
void RegAllocVerifier::HandleSpillFill(SpillFillInst *inst)
{
    for (auto sf : inst->GetSpillFills()) {
        LocationState *state = nullptr;
        success_ &= ForEachLocation(
            sf.GetSrc(), sf.GetType(), [&state, &sf, inst, arch = GetGraph()->GetArch()](LocationState &st) {
                if (state == nullptr) {
                    state = &st;
                } else if (!SameState(*state, st)) {
                    COMPILER_LOG(ERROR, REGALLOC) << "SpillFill is accessing " << sf.GetSrc().ToString(arch)
                                                  << " with different state for high and low parts";
                    return false;
                }
                if (st.GetState() == LocationState::State::KNOWN) {
                    return true;
                }
                COMPILER_LOG(ERROR, REGALLOC)
                    << "SpillFill is copying value from location with state " << ToString(st.GetState()) << ", "
                    << inst->GetId() << " read from " << sf.GetSrc().ToString(arch);
                return false;
            });
        ASSERT(state != nullptr);
        UpdateLocation(sf.GetDst(), sf.GetType(), state->GetId());
    }
}

// Set instn's id to corresponding location.
void RegAllocVerifier::HandleConst(ConstantInst *inst)
{
    if (inst->GetDstReg() != INVALID_REG) {
        HandleDest(inst);
        return;
    }

    // if const inst does not have valid register then it was spilled to imm table.
    auto imm_slot = GetGraph()->FindSpilledConstantSlot(inst->CastToConstant());
    // zero const is a special case - we're using zero reg to encode it.
    if (imm_slot == INVALID_IMM_TABLE_SLOT && IsZeroConstantOrNullPtr(inst)) {
        return;
    }
    ASSERT(imm_slot != INVALID_IMM_TABLE_SLOT);
}

// Verify instn's inputs and set instn's id to destination location.
void RegAllocVerifier::HandleInst(Inst *inst)
{
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        auto input = inst->GetDataFlowInput(i);
        if (input->NoDest() && !IsPseudoUserOfMultiOutput(input)) {
            ASSERT(!inst->GetLocation(i).IsFixedRegister());
            continue;
        }

        auto input_type = inst->GetInputType(i);
        if (input_type == DataType::NO_TYPE) {
            ASSERT(inst->GetOpcode() == Opcode::IndirectJump || inst->GetOpcode() == Opcode::CallIndirect);
            input_type = Is64BitsArch(GetGraph()->GetArch()) ? DataType::INT64 : DataType::INT32;
        } else {
            input_type = ConvertRegType(GetGraph(), input_type);
        }

        success_ &= ForEachLocation(inst->GetLocation(i), input_type, [input, inst, i](LocationState &location) {
            if (location.GetState() != LocationState::State::KNOWN) {
                COMPILER_LOG(ERROR, REGALLOC) << "Input #" << i << " is loaded from location with state "
                                              << ToString(location.GetState()) << std::endl
                                              << "Affected inst: " << *inst << std::endl
                                              << "Input inst: " << *input;
                return false;
            }
            if (location.IsValid(input)) {
                return true;
            }
            COMPILER_LOG(ERROR, REGALLOC) << "Input #" << i << " is loaded from location holding instruction "
                                          << location.GetId() << " instead of " << input->GetId() << std::endl
                                          << "Affected inst: " << *inst;
            return false;
        });
    }
    HandleDest(inst);
}

bool RegAllocVerifier::IsSaveRestoreRegisters(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto intrinsic_id = inst->CastToIntrinsic()->GetIntrinsicId();
    return intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_SAVE_REGISTERS_EP ||
           intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_RESTORE_REGISTERS_EP;
}

void RegAllocVerifier::HandleSaveRestoreRegisters(Inst *inst)
{
    ASSERT(inst->IsIntrinsic());
    auto used_regs = GetGraph()->GetUsedRegs<DataType::INT64>()->size();
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_SAVE_REGISTERS_EP:
            ASSERT(saved_regs_.empty());
            for (size_t reg = 0; reg < used_regs; reg++) {
                saved_regs_.push_back(current_state_->GetReg(reg));
            }
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_RESTORE_REGISTERS_EP:
            ASSERT(!saved_regs_.empty());
            for (size_t reg = 0; reg < used_regs; reg++) {
                if (saved_regs_[reg].GetState() == LocationState::State::UNKNOWN) {
                    continue;
                }
                current_state_->GetReg(reg) = saved_regs_[reg];
            }
            saved_regs_.clear();
            break;
        default:
            UNREACHABLE();
    }

    HandleDest(inst);
}

// Implicit null checks are not encoded as machine instructions, but instead
// added to method's metadata that allow to treat some memory-access related
// signals as null pointer exceptions.
// SaveState instruction bound to implicit null check captures locations
// of its inputs at first null check's user (See RegAllocResolver::GetExplicitUser).
// Check if current instruction use implicit null check that was not yet handled
// and handle its save state.
void RegAllocVerifier::TryHandleImplicitNullCheck(Inst *inst)
{
    NullCheckInst *nc = nullptr;
    for (auto &input : inst->GetInputs()) {
        if (input.GetInst()->IsNullCheck() && input.GetInst()->CastToNullCheck()->IsImplicit() &&
            !input.GetInst()->IsMarked(implicit_null_check_handled_marker_)) {
            nc = input.GetInst()->CastToNullCheck();
            break;
        }
    }
    if (nc == nullptr) {
        return;
    }
    nc->SetMarker(implicit_null_check_handled_marker_);
}

}  // namespace panda::compiler
