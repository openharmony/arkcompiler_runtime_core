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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_LINEAR_SCAN_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_LINEAR_SCAN_H_

#include "utils/arena_containers.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/code_generator/registers_description.h"
#include "reg_alloc_base.h"
#include "reg_map.h"
#include "compiler_logger.h"

namespace panda::compiler {
class BasicBlock;
class Graph;
class LifeIntervals;

using InstructionsIntervals = ArenaList<LifeIntervals *>;
using EmptyRegMask = std::bitset<0>;

/**
 * Algorithm is based on paper "Linear Scan Register Allocation" by Christian Wimmer
 *
 * LinearScan works with instructions lifetime intervals, computed by the LivenessAnalyzer.
 * It scans forward through the intervals, ordered by increasing starting point and assign registers while the number of
 * the active intervals at the point is less then the number of available registers. When there are no available
 * registers for some interval, LinearScan assigns one of the blocked ones. Previous holder of this blocked register is
 * spilled to the memory at this point. Blocked register to be assigned is selected according to the usage information,
 * LinearScan selects register with the most distant use point from the current one.
 */
class RegAllocLinearScan : public RegAllocBase {
    struct WorkingIntervals {
        explicit WorkingIntervals(ArenaAllocator *allocator)
            : active(allocator->Adapter()),
              inactive(allocator->Adapter()),
              stack(allocator->Adapter()),
              handled(allocator->Adapter()),
              fixed(allocator->Adapter())
        {
        }

        void Clear()
        {
            active.clear();
            inactive.clear();
            stack.clear();
            handled.clear();
            fixed.clear();
        }

        InstructionsIntervals active;        // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsIntervals inactive;      // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsIntervals stack;         // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsIntervals handled;       // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<LifeIntervals *> fixed;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    struct PendingIntervals {
        explicit PendingIntervals(ArenaAllocator *allocator)
            : regular(allocator->Adapter()), fixed(allocator->Adapter())
        {
        }
        InstructionsIntervals regular;       // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<LifeIntervals *> fixed;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

public:
    explicit RegAllocLinearScan(Graph *graph);
    RegAllocLinearScan(Graph *graph, [[maybe_unused]] EmptyRegMask mask);
    ~RegAllocLinearScan() override = default;
    NO_MOVE_SEMANTIC(RegAllocLinearScan);
    NO_COPY_SEMANTIC(RegAllocLinearScan);

    const char *GetPassName() const override
    {
        return "RegAllocLinearScan";
    }

    bool AbortIfFailed() const override
    {
        return true;
    }

protected:
    bool Allocate() override;
    void InitIntervals() override;
    void PrepareInterval(LifeIntervals *interval) override;

private:
    template <bool IS_FP>
    const LocationMask &GetLocationMask() const
    {
        return IS_FP ? GetVRegMask() : GetRegMask();
    }

    template <bool IS_FP>
    PendingIntervals &GetIntervals()
    {
        return IS_FP ? vector_intervals_ : general_intervals_;
    }

    template <bool IS_FP>
    void AssignLocations();
    template <bool IS_FP>
    void PreprocessPreassignedIntervals();
    template <bool IS_FP>
    void ExpireIntervals(LifeNumber current_position);
    template <bool IS_FP>
    void WalkIntervals();
    template <bool IS_FP>
    bool TryToAssignRegister(LifeIntervals *current_interval);
    template <bool IS_FP>
    void SplitAndSpill(const InstructionsIntervals *intervals, const LifeIntervals *current_interval);
    template <bool IS_FP>
    void SplitActiveInterval(LifeIntervals *interval, LifeNumber split_pos);
    template <bool IS_FP>
    void AddToQueue(LifeIntervals *interval);
    template <bool IS_FP>
    void SplitBeforeUse(LifeIntervals *current_interval, LifeNumber use_pos);
    bool IsIntervalRegFree(const LifeIntervals *current_interval, Register reg) const;
    void AssignStackSlot(LifeIntervals *interval);
    void RemapRegallocReg(LifeIntervals *interval);
    void RemapRegistersIntervals();
    template <bool IS_FP>
    void AddFixedIntervalsToWorkingIntervals();
    template <bool IS_FP>
    void HandleFixedIntervalIntersection(LifeIntervals *current_interval);

    Register GetSuitableRegister(const LifeIntervals *current_interval);
    Register GetFreeRegister(const LifeIntervals *current_interval);
    std::pair<Register, LifeNumber> GetBlockedRegister(const LifeIntervals *current_interval);

    template <class T, class Callback>
    void IterateIntervalsWithErasion(T &intervals, const Callback &callback) const
    {
        for (auto it = intervals.begin(); it != intervals.end();) {
            auto interval = *it;
            if (callback(interval)) {
                it = intervals.erase(it);
            } else {
                ++it;
            }
        }
    }

    template <class T, class Callback>
    void EnumerateIntervals(const T &intervals, const Callback &callback) const
    {
        for (const auto &interval : intervals) {
            if (interval == nullptr || interval->GetReg() >= reg_map_.GetAvailableRegsCount()) {
                continue;
            }
            callback(interval);
        }
    }

    template <class T, class Callback>
    void EnumerateIntersectedIntervals(const T &intervals, const LifeIntervals *current, const Callback &callback) const
    {
        for (const auto &interval : intervals) {
            if (interval == nullptr || interval->GetReg() >= reg_map_.GetAvailableRegsCount()) {
                continue;
            }
            auto intersection = interval->GetFirstIntersectionWith(current);
            if (intersection != INVALID_LIFE_NUMBER) {
                callback(interval, intersection);
            }
        }
    }

    void BlockOverlappedRegisters(const LifeIntervals *current_interval);
    bool IsNonSpillableConstInterval(LifeIntervals *interval);
    void BeforeConstantIntervalSpill(LifeIntervals *interval, LifeNumber split_pos);

private:
    WorkingIntervals working_intervals_;
    ArenaVector<LifeNumber> regs_use_positions_;
    PendingIntervals general_intervals_;
    PendingIntervals vector_intervals_;
    RegisterMap reg_map_;
    bool remat_constants_;
    bool success_ {true};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_LINEAR_SCAN_H_
