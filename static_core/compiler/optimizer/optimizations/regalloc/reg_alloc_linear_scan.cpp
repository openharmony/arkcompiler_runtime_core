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

#include "reg_alloc_linear_scan.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"

namespace panda::compiler {

static constexpr auto MAX_LIFE_NUMBER = std::numeric_limits<LifeNumber>::max();

/// Add interval in sorted order
static void AddInterval(LifeIntervals *interval, InstructionsIntervals *dest)
{
    auto cmp = [](LifeIntervals *lhs, LifeIntervals *rhs) { return lhs->GetBegin() >= rhs->GetBegin(); };

    if (dest->empty()) {
        dest->push_back(interval);
        return;
    }

    auto iter = dest->end();
    --iter;
    while (true) {
        if (cmp(interval, *iter)) {
            dest->insert(++iter, interval);
            break;
        }
        if (iter == dest->begin()) {
            dest->push_front(interval);
            break;
        }
        --iter;
    }
}

/// Use graph's registers masks and `MAX_NUM_STACK_SLOTS` stack slots
RegAllocLinearScan::RegAllocLinearScan(Graph *graph)
    : RegAllocBase(graph),
      working_intervals_(graph->GetLocalAllocator()),
      regs_use_positions_(graph->GetLocalAllocator()->Adapter()),
      general_intervals_(graph->GetLocalAllocator()),
      vector_intervals_(graph->GetLocalAllocator()),
      reg_map_(graph->GetLocalAllocator()),
      remat_constants_(!graph->IsBytecodeOptimizer() && OPTIONS.IsCompilerRematConst())
{
}

/// Use dynamic general registers mask (without vector regs) and zero stack slots
RegAllocLinearScan::RegAllocLinearScan(Graph *graph, [[maybe_unused]] EmptyRegMask mask)
    : RegAllocBase(graph, VIRTUAL_FRAME_SIZE),
      working_intervals_(graph->GetLocalAllocator()),
      regs_use_positions_(graph->GetLocalAllocator()->Adapter()),
      general_intervals_(graph->GetLocalAllocator()),
      vector_intervals_(graph->GetLocalAllocator()),
      reg_map_(graph->GetLocalAllocator()),
      remat_constants_(!graph->IsBytecodeOptimizer() && OPTIONS.IsCompilerRematConst())
{
}

/// Allocate registers
bool RegAllocLinearScan::Allocate()
{
    AssignLocations<false>();
    AssignLocations<true>();
    return success_;
}

void RegAllocLinearScan::InitIntervals()
{
    GetIntervals<false>().fixed.resize(MAX_NUM_REGS);
    GetIntervals<true>().fixed.resize(MAX_NUM_VREGS);
    ReserveTempRegisters();
}

void RegAllocLinearScan::PrepareInterval(LifeIntervals *interval)
{
    bool is_fp = DataType::IsFloatType(interval->GetType());
    auto &intervals = is_fp ? GetIntervals<true>() : GetIntervals<false>();

    if (interval->IsPhysical()) {
        ASSERT(intervals.fixed.size() > interval->GetReg());
        ASSERT(intervals.fixed[interval->GetReg()] == nullptr);
        intervals.fixed[interval->GetReg()] = interval;
        return;
    }

    if (!interval->HasInst()) {
        AddInterval(interval, &intervals.regular);
        return;
    }

    if (interval->NoDest() || interval->GetInst()->GetDstCount() > 1U || interval->GetReg() == ACC_REG_ID) {
        return;
    }

    if (interval->IsPreassigned() && interval->GetReg() == GetGraph()->GetZeroReg()) {
        ASSERT(interval->GetReg() != INVALID_REG);
        return;
    }

    AddInterval(interval, &intervals.regular);
}

template <bool IS_FP>
void RegAllocLinearScan::AssignLocations()
{
    auto &regular_intervals = GetIntervals<IS_FP>().regular;
    if (regular_intervals.empty()) {
        return;
    }

    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (IS_FP) {
        GetGraph()->SetHasFloatRegs();
    }

    working_intervals_.Clear();
    auto arch = GetGraph()->GetArch();
    size_t priority = arch != Arch::NONE ? GetFirstCalleeReg(arch, IS_FP) : 0;
    reg_map_.SetMask(GetLocationMask<IS_FP>(), priority);
    regs_use_positions_.resize(reg_map_.GetAvailableRegsCount());

    AddFixedIntervalsToWorkingIntervals<IS_FP>();
    PreprocessPreassignedIntervals<IS_FP>();
    while (!regular_intervals.empty() && success_) {
        ExpireIntervals<IS_FP>(regular_intervals.front()->GetBegin());
        WalkIntervals<IS_FP>();
    }
    RemapRegistersIntervals();
}

template <bool IS_FP>
void RegAllocLinearScan::PreprocessPreassignedIntervals()
{
    for (auto &interval : GetIntervals<IS_FP>().regular) {
        if (!interval->IsPreassigned() || interval->IsSplitSibling() || interval->GetReg() == ACC_REG_ID) {
            continue;
        }
        interval->SetPreassignedReg(reg_map_.CodegenToRegallocReg(interval->GetReg()));
        COMPILER_LOG(DEBUG, REGALLOC) << "Preassigned interval " << interval->template ToString<true>();
    }
}

/// Free registers from expired intervals
template <bool IS_FP>
void RegAllocLinearScan::ExpireIntervals(LifeNumber current_position)
{
    IterateIntervalsWithErasion(working_intervals_.active, [this, current_position](const auto &interval) {
        if (!interval->HasReg() || interval->GetEnd() <= current_position) {
            working_intervals_.handled.push_back(interval);
            return true;
        }
        if (!interval->SplitCover(current_position)) {
            AddInterval(interval, &working_intervals_.inactive);
            return true;
        }
        return false;
    });
    IterateIntervalsWithErasion(working_intervals_.inactive, [this, current_position](const auto &interval) {
        if (!interval->HasReg() || interval->GetEnd() <= current_position) {
            working_intervals_.handled.push_back(interval);
            return true;
        }
        if (interval->SplitCover(current_position)) {
            AddInterval(interval, &working_intervals_.active);
            return true;
        }
        return false;
    });
    IterateIntervalsWithErasion(working_intervals_.stack, [this, current_position](const auto &interval) {
        if (interval->GetEnd() <= current_position) {
            GetStackMask().Reset(interval->GetLocation().GetValue());
            return true;
        }
        return false;
    });
}

/// Working intervals processing
template <bool IS_FP>
void RegAllocLinearScan::WalkIntervals()
{
    auto current_interval = GetIntervals<IS_FP>().regular.front();
    GetIntervals<IS_FP>().regular.pop_front();
    COMPILER_LOG(DEBUG, REGALLOC) << "----------------";
    COMPILER_LOG(DEBUG, REGALLOC) << "Process interval " << current_interval->template ToString<true>();

    // Parameter that was passed in the stack slot: split its interval before first use
    if (current_interval->GetLocation().IsStackParameter()) {
        ASSERT(current_interval->GetInst()->IsParameter());
        COMPILER_LOG(DEBUG, REGALLOC) << "Interval was defined in the stack parameter slot";
        auto next_use = current_interval->GetNextUsage(current_interval->GetBegin() + 1U);
        SplitBeforeUse<IS_FP>(current_interval, next_use);
        return;
    }

    if (!current_interval->HasReg()) {
        if (TryToAssignRegister<IS_FP>(current_interval)) {
            COMPILER_LOG(DEBUG, REGALLOC)
                << current_interval->GetLocation().ToString(GetGraph()->GetArch()) << " was assigned to the interval "
                << current_interval->template ToString<true>();
        } else {
            COMPILER_LOG(ERROR, REGALLOC) << "There are no available registers";
            success_ = false;
            return;
        }
    } else {
        ASSERT(current_interval->IsPreassigned());
        COMPILER_LOG(DEBUG, REGALLOC) << "Interval has preassigned "
                                      << current_interval->GetLocation().ToString(GetGraph()->GetArch());
        if (!IsIntervalRegFree(current_interval, current_interval->GetReg())) {
            SplitAndSpill<IS_FP>(&working_intervals_.active, current_interval);
            SplitAndSpill<IS_FP>(&working_intervals_.inactive, current_interval);
        }
    }
    HandleFixedIntervalIntersection<IS_FP>(current_interval);
    AddInterval(current_interval, &working_intervals_.active);
}

template <bool IS_FP>
bool RegAllocLinearScan::TryToAssignRegister(LifeIntervals *current_interval)
{
    auto reg = GetSuitableRegister(current_interval);
    if (reg != INVALID_REG) {
        current_interval->SetReg(reg);
        return true;
    }

    // Try to assign blocked register
    auto [blocked_reg, next_blocked_use] = GetBlockedRegister(current_interval);
    auto next_use = current_interval->GetNextUsage(current_interval->GetBegin());

    // Spill current interval if its first use later than use of blocked register
    if (blocked_reg != INVALID_REG && next_blocked_use < next_use && !IsNonSpillableConstInterval(current_interval)) {
        SplitBeforeUse<IS_FP>(current_interval, next_use);
        AssignStackSlot(current_interval);
        return true;
    }

    // Blocked register that will be used in the next position mustn't be reassigned
    if (blocked_reg == INVALID_REG || next_blocked_use < current_interval->GetBegin() + LIFE_NUMBER_GAP) {
        return false;
    }

    current_interval->SetReg(blocked_reg);
    SplitAndSpill<IS_FP>(&working_intervals_.active, current_interval);
    SplitAndSpill<IS_FP>(&working_intervals_.inactive, current_interval);
    return true;
}

template <bool IS_FP>
void RegAllocLinearScan::SplitAndSpill(const InstructionsIntervals *intervals, const LifeIntervals *current_interval)
{
    for (auto interval : *intervals) {
        if (interval->GetReg() != current_interval->GetReg() ||
            interval->GetFirstIntersectionWith(current_interval) == INVALID_LIFE_NUMBER) {
            continue;
        }
        COMPILER_LOG(DEBUG, REGALLOC) << "Found active interval: " << interval->template ToString<true>();
        SplitActiveInterval<IS_FP>(interval, current_interval->GetBegin());
    }
}

/**
 * Split interval with assigned 'reg' into 3 parts:
 * [interval|split|split_next]
 *
 * 'interval' - holds assigned register;
 * 'split' - stack slot is assigned to it;
 * 'split_next' - added to the queue for future assignment;
 */
template <bool IS_FP>
void RegAllocLinearScan::SplitActiveInterval(LifeIntervals *interval, LifeNumber split_pos)
{
    BeforeConstantIntervalSpill(interval, split_pos);
    auto prev_use_pos = interval->GetPrevUsage(split_pos);
    auto next_use_pos = interval->GetNextUsage(split_pos + 1U);
    COMPILER_LOG(DEBUG, REGALLOC) << "Prev use position: " << std::to_string(prev_use_pos)
                                  << ", Next use position: " << std::to_string(next_use_pos);
    auto split = interval;
    if (prev_use_pos == INVALID_LIFE_NUMBER) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Spill the whole interval " << interval->template ToString<true>();
        interval->ClearLocation();
    } else {
        auto split_position = (split_pos % 2U == 1) ? split_pos : split_pos - 1;
        COMPILER_LOG(DEBUG, REGALLOC) << "Split interval " << interval->template ToString<true>() << " at position "
                                      << static_cast<int>(split_position);

        split = interval->SplitAt(split_position, GetGraph()->GetAllocator());
    }
    SplitBeforeUse<IS_FP>(split, next_use_pos);
    AssignStackSlot(split);
}

template <bool IS_FP>
void RegAllocLinearScan::AddToQueue(LifeIntervals *interval)
{
    COMPILER_LOG(DEBUG, REGALLOC) << "Add to the queue: " << interval->template ToString<true>();
    AddInterval(interval, &GetIntervals<IS_FP>().regular);
}

Register RegAllocLinearScan::GetSuitableRegister(const LifeIntervals *current_interval)
{
    if (!current_interval->HasInst()) {
        return GetFreeRegister(current_interval);
    }
    // First of all, try to assign register using hint
    auto &use_table = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetUseTable();
    auto hint_reg = use_table.GetNextUseOnFixedLocation(current_interval->GetInst(), current_interval->GetBegin());
    if (hint_reg != INVALID_REG) {
        auto reg = reg_map_.CodegenToRegallocReg(hint_reg);
        if (reg_map_.IsRegAvailable(reg, GetGraph()->GetArch()) && IsIntervalRegFree(current_interval, reg)) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Hint-register is available";
            return reg;
        }
    }
    // If hint doesn't exist or hint-register not available, try to assign any free register
    return GetFreeRegister(current_interval);
}

Register RegAllocLinearScan::GetFreeRegister(const LifeIntervals *current_interval)
{
    std::fill(regs_use_positions_.begin(), regs_use_positions_.end(), MAX_LIFE_NUMBER);

    auto set_fixed_usage = [this, &current_interval](const auto &interval, LifeNumber intersection) {
        // If intersection is equal to the current_interval's begin
        // than it means that current_interval is a call and fixed interval's range was
        // created for it.
        // Do not disable fixed register for a call.
        if (intersection == current_interval->GetBegin()) {
            LiveRange range;
            [[maybe_unused]] bool succ = interval->FindRangeCoveringPosition(intersection, &range);
            ASSERT(succ);
            if (range.GetBegin() == intersection) {
                return;
            }
        }
        ASSERT(regs_use_positions_.size() > interval->GetReg());
        regs_use_positions_[interval->GetReg()] = intersection;
    };
    auto set_inactive_usage = [this](const auto &interval, LifeNumber intersection) {
        ASSERT(regs_use_positions_.size() > interval->GetReg());
        auto &reg_use = regs_use_positions_[interval->GetReg()];
        reg_use = std::min<size_t>(intersection, reg_use);
    };

    EnumerateIntersectedIntervals(working_intervals_.fixed, current_interval, set_fixed_usage);
    EnumerateIntersectedIntervals(working_intervals_.inactive, current_interval, set_inactive_usage);
    EnumerateIntervals(working_intervals_.active, [this](const auto &interval) {
        ASSERT(regs_use_positions_.size() > interval->GetReg());
        regs_use_positions_[interval->GetReg()] = 0;
    });

    BlockOverlappedRegisters(current_interval);

    // Select register with max position
    auto it = std::max_element(regs_use_positions_.cbegin(), regs_use_positions_.cend());
    // Register is free if it's available for the whole interval
    return (*it >= current_interval->GetEnd()) ? std::distance(regs_use_positions_.cbegin(), it) : INVALID_REG;
}

// Return blocked register and its next use position
std::pair<Register, LifeNumber> RegAllocLinearScan::GetBlockedRegister(const LifeIntervals *current_interval)
{
    // Using blocked registers is impossible in the `BytecodeOptimizer` mode
    if (GetGraph()->IsBytecodeOptimizer()) {
        return {INVALID_REG, INVALID_LIFE_NUMBER};
    }

    std::fill(regs_use_positions_.begin(), regs_use_positions_.end(), MAX_LIFE_NUMBER);

    auto set_fixed_usage = [this, current_interval](const auto &interval, LifeNumber intersection) {
        // If intersection is equal to the current_interval's begin
        // than it means that current_interval is a call and fixed interval's range was
        // created for it.
        // Do not disable fixed register for a call.
        if (intersection == current_interval->GetBegin()) {
            LiveRange range;
            [[maybe_unused]] bool succ = interval->FindRangeCoveringPosition(intersection, &range);
            ASSERT(succ);
            if (range.GetBegin() == intersection) {
                return;
            }
        }
        ASSERT(regs_use_positions_.size() > interval->GetReg());
        regs_use_positions_[interval->GetReg()] = intersection;
    };
    auto set_inactive_usage = [this](const auto &interval, LifeNumber intersection) {
        ASSERT(regs_use_positions_.size() > interval->GetReg());
        auto &reg_use = regs_use_positions_[interval->GetReg()];
        reg_use = std::min<size_t>(interval->GetNextUsage(intersection), reg_use);
    };

    EnumerateIntersectedIntervals(working_intervals_.fixed, current_interval, set_fixed_usage);
    EnumerateIntersectedIntervals(working_intervals_.inactive, current_interval, set_inactive_usage);
    EnumerateIntervals(working_intervals_.active, [this, &current_interval](const auto &interval) {
        ASSERT(regs_use_positions_.size() > interval->GetReg());
        auto &reg_use = regs_use_positions_[interval->GetReg()];
        reg_use = std::min<size_t>(interval->GetNextUsage(current_interval->GetBegin()), reg_use);
    });

    BlockOverlappedRegisters(current_interval);

    // Select register with max position
    auto it = std::max_element(regs_use_positions_.cbegin(), regs_use_positions_.cend());
    auto reg = std::distance(regs_use_positions_.cbegin(), it);
    COMPILER_LOG(DEBUG, REGALLOC) << "Selected blocked r" << static_cast<int>(reg) << " with use next use position "
                                  << *it;
    return {reg, regs_use_positions_[reg]};
}

bool RegAllocLinearScan::IsIntervalRegFree(const LifeIntervals *current_interval, Register reg) const
{
    for (auto interval : working_intervals_.fixed) {
        if (interval == nullptr || interval->GetReg() != reg) {
            continue;
        }
        if (interval->GetFirstIntersectionWith(current_interval) < current_interval->GetBegin() + LIFE_NUMBER_GAP) {
            return false;
        }
    }

    for (auto interval : working_intervals_.inactive) {
        if (interval->GetReg() == reg && interval->GetFirstIntersectionWith(current_interval) != INVALID_LIFE_NUMBER) {
            return false;
        }
    }
    for (auto interval : working_intervals_.active) {
        if (interval->GetReg() == reg) {
            return false;
        }
    }
    return true;
}

void RegAllocLinearScan::AssignStackSlot(LifeIntervals *interval)
{
    ASSERT(!interval->GetLocation().IsStack());
    ASSERT(interval->HasInst());
    if (remat_constants_ && interval->GetInst()->IsConst()) {
        auto imm_slot = GetGraph()->AddSpilledConstant(interval->GetInst()->CastToConstant());
        if (imm_slot != INVALID_IMM_TABLE_SLOT) {
            interval->SetLocation(Location::MakeConstant(imm_slot));
            COMPILER_LOG(DEBUG, REGALLOC) << interval->GetLocation().ToString(GetGraph()->GetArch())
                                          << " was assigned to the interval " << interval->template ToString<true>();
            return;
        }
    }

    auto slot = GetNextStackSlot(interval);
    if (slot != INVALID_STACK_SLOT) {
        interval->SetLocation(Location::MakeStackSlot(slot));
        COMPILER_LOG(DEBUG, REGALLOC) << interval->GetLocation().ToString(GetGraph()->GetArch())
                                      << " was assigned to the interval " << interval->template ToString<true>();
        working_intervals_.stack.push_back(interval);
    } else {
        COMPILER_LOG(ERROR, REGALLOC) << "There are no available stack slots";
        success_ = false;
    }
}

void RegAllocLinearScan::RemapRegallocReg(LifeIntervals *interval)
{
    if (interval->HasReg()) {
        auto reg = interval->GetReg();
        interval->SetReg(reg_map_.RegallocToCodegenReg(reg));
    }
}

void RegAllocLinearScan::RemapRegistersIntervals()
{
    for (auto interval : working_intervals_.handled) {
        RemapRegallocReg(interval);
    }
    for (auto interval : working_intervals_.active) {
        RemapRegallocReg(interval);
    }
    for (auto interval : working_intervals_.inactive) {
        RemapRegallocReg(interval);
    }
    for (auto interval : working_intervals_.fixed) {
        if (interval != nullptr) {
            RemapRegallocReg(interval);
        }
    }
}

template <bool IS_FP>
void RegAllocLinearScan::AddFixedIntervalsToWorkingIntervals()
{
    working_intervals_.fixed.resize(GetLocationMask<IS_FP>().GetSize());
    // remap registers for fixed intervals and add it to working intervals
    for (auto fixed_interval : GetIntervals<IS_FP>().fixed) {
        if (fixed_interval == nullptr) {
            continue;
        }
        auto reg = reg_map_.CodegenToRegallocReg(fixed_interval->GetReg());
        fixed_interval->SetReg(reg);
        working_intervals_.fixed[reg] = fixed_interval;
        COMPILER_LOG(DEBUG, REGALLOC) << "Fixed interval for r" << static_cast<int>(fixed_interval->GetReg()) << ": "
                                      << fixed_interval->template ToString<true>();
    }
}

template <bool IS_FP>
void RegAllocLinearScan::HandleFixedIntervalIntersection(LifeIntervals *current_interval)
{
    if (!current_interval->HasReg()) {
        return;
    }
    auto reg = current_interval->GetReg();
    if (reg >= working_intervals_.fixed.size() || working_intervals_.fixed[reg] == nullptr) {
        return;
    }
    auto fixed_interval = working_intervals_.fixed[reg];
    auto intersection = current_interval->GetFirstIntersectionWith(fixed_interval);
    if (intersection == current_interval->GetBegin()) {
        // Current interval can intersect fixed interval at the beginning of its live range
        // only if it's a call and fixed interval's range was created for it.
        // Try to find first intersection excluding the range blocking registers during a call.
        intersection = current_interval->GetFirstIntersectionWith(fixed_interval, intersection + 1U);
    }
    if (intersection == INVALID_LIFE_NUMBER) {
        return;
    }
    COMPILER_LOG(DEBUG, REGALLOC) << "Intersection with fixed interval at: " << std::to_string(intersection);

    auto &use_table = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetUseTable();
    if (use_table.HasUseOnFixedLocation(current_interval->GetInst(), intersection)) {
        // Instruction is used at intersection position: split before that use
        SplitBeforeUse<IS_FP>(current_interval, intersection);
        return;
    }

    BeforeConstantIntervalSpill(current_interval, intersection);
    auto last_use_before = current_interval->GetLastUsageBefore(intersection);
    if (last_use_before != INVALID_LIFE_NUMBER) {
        // Split after the last use before intersection
        SplitBeforeUse<IS_FP>(current_interval, last_use_before + LIFE_NUMBER_GAP);
        return;
    }

    // There is no use before intersection, split after intersection add splitted-interval to the queue
    auto next_use = current_interval->GetNextUsage(intersection);
    current_interval->ClearLocation();
    SplitBeforeUse<IS_FP>(current_interval, next_use);
    AssignStackSlot(current_interval);
}

template <bool IS_FP>
void RegAllocLinearScan::SplitBeforeUse(LifeIntervals *current_interval, LifeNumber use_pos)
{
    if (use_pos == INVALID_LIFE_NUMBER) {
        return;
    }
    COMPILER_LOG(DEBUG, REGALLOC) << "Split at " << std::to_string(use_pos - 1);
    auto split = current_interval->SplitAt(use_pos - 1, GetGraph()->GetAllocator());
    AddToQueue<IS_FP>(split);
}

void RegAllocLinearScan::BlockOverlappedRegisters(const LifeIntervals *current_interval)
{
    if (!current_interval->HasInst()) {
        // current_interval - is additional life interval for an instruction required temp, block fixed registers of
        // that instruction
        auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
        la.EnumerateFixedLocationsOverlappingTemp(current_interval, [this](Location location) {
            ASSERT(location.IsFixedRegister());
            auto reg = reg_map_.CodegenToRegallocReg(location.GetValue());
            if (reg_map_.IsRegAvailable(reg, GetGraph()->GetArch())) {
                ASSERT(regs_use_positions_.size() > reg);
                regs_use_positions_[reg] = 0;
            }
        });
    }
}

/// Returns true if the interval corresponds to Constant instruction and it could not be spilled to a stack.
bool RegAllocLinearScan::IsNonSpillableConstInterval(LifeIntervals *interval)
{
    if (interval->IsSplitSibling() || interval->IsPhysical()) {
        return false;
    }
    auto inst = interval->GetInst();
    return inst != nullptr && inst->IsConst() && remat_constants_ &&
           inst->CastToConstant()->GetImmTableSlot() == INVALID_IMM_TABLE_SLOT &&
           !GetGraph()->HasAvailableConstantSpillSlots();
}

/**
 * If constant rematerialization is enabled then constant intervals won't have use position
 * at the beginning of an interval. If there are available immediate slots then it is not an issue
 * as the interval will have a slot assigned to it during the spill. But if there are not available
 * immediate slots then the whole interval will be spilled (which is incorrect) unless we add a use
 * position at the beginning and thus force split creation right after it.
 */
void RegAllocLinearScan::BeforeConstantIntervalSpill(LifeIntervals *interval, LifeNumber split_pos)
{
    if (!IsNonSpillableConstInterval(interval)) {
        return;
    }
    if (interval->GetPrevUsage(split_pos) != INVALID_LIFE_NUMBER) {
        return;
    }
    interval->PrependUsePosition(interval->GetBegin());
}

}  // namespace panda::compiler
