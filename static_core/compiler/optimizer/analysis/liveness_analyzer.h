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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALIZER_H
#define COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALIZER_H

#include "utils/arena_containers.h"
#include "optimizer/analysis/liveness_use_table.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/marker.h"
#include "optimizer/pass.h"
#include "optimizer/ir/locations.h"
#include "compiler_logger.h"

namespace panda::compiler {
class BasicBlock;
class Graph;
class Inst;
class Loop;

class LiveRange {
public:
    LiveRange(LifeNumber begin, LifeNumber end) : begin_(begin), end_(end) {}
    LiveRange() = default;
    DEFAULT_MOVE_SEMANTIC(LiveRange);
    DEFAULT_COPY_SEMANTIC(LiveRange);
    ~LiveRange() = default;

    // Check if range contains other range
    bool Contains(const LiveRange &other) const
    {
        return (begin_ <= other.begin_ && other.end_ <= end_);
    }
    // Check if range contains point
    bool Contains(LifeNumber number) const
    {
        return (begin_ <= number && number <= end_);
    }
    // Check if ranges are equal
    bool operator==(const LiveRange &other) const
    {
        return begin_ == other.begin_ && end_ == other.end_;
    }

    void SetBegin(LifeNumber begin)
    {
        begin_ = begin;
    }
    LifeNumber GetBegin() const
    {
        return begin_;
    }

    void SetEnd(LifeNumber end)
    {
        end_ = end;
    }
    LifeNumber GetEnd() const
    {
        return end_;
    }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "[" << begin_ << ":" << end_ << ")";
        return ss.str();
    }

private:
    LifeNumber begin_ = 0;
    LifeNumber end_ = 0;
};

class LifeIntervals {
public:
    explicit LifeIntervals(ArenaAllocator *allocator) : LifeIntervals(allocator, nullptr) {}

    LifeIntervals(ArenaAllocator *allocator, Inst *inst) : LifeIntervals(allocator, inst, {}) {}

    LifeIntervals(ArenaAllocator *allocator, Inst *inst, LiveRange live_range)
        : inst_(inst),
          live_ranges_(allocator->Adapter()),
          use_positions_(allocator->Adapter()),
          location_(Location::Invalid()),
          type_(DataType::NO_TYPE),
          is_preassigned_(),
          is_physical_(),
          is_split_sibling_()
    {
#ifndef NDEBUG
        finalized_ = 0;
#endif
        if (live_range.GetEnd() != 0) {
            live_ranges_.push_back(live_range);
        }
    }

    DEFAULT_MOVE_SEMANTIC(LifeIntervals);
    DEFAULT_COPY_SEMANTIC(LifeIntervals);
    ~LifeIntervals() = default;

    /*
     * Basic blocks are visiting in descending lifetime order, so there are 3 ways to
     * update lifetime:
     * - append the first LiveRange
     * - extend the first LiveRange
     * - append a new one LiveRange due to lifetime hole
     */
    void AppendRange(LiveRange live_range)
    {
        ASSERT(live_range.GetEnd() >= live_range.GetBegin());
        // [live_range],[back]
        if (live_ranges_.empty() || live_range.GetEnd() < live_ranges_.back().GetBegin()) {
            live_ranges_.push_back(live_range);
            /*
             * [live_range]
             *         [front]
             * ->
             * [    front    ]
             */
        } else if (live_range.GetEnd() <= live_ranges_.back().GetEnd()) {
            live_ranges_.back().SetBegin(live_range.GetBegin());
            /*
             * [ live_range  ]
             * [front]
             * ->
             * [    front    ]
             */
        } else if (!live_ranges_.back().Contains(live_range)) {
            ASSERT(live_ranges_.back().GetBegin() == live_range.GetBegin());
            live_ranges_.back().SetEnd(live_range.GetEnd());
        }
    }

    void AppendRange(LifeNumber begin, LifeNumber end)
    {
        AppendRange({begin, end});
    }

    /*
     * Group range extends the first LiveRange, because it is covering the hole group,
     * starting from its header
     */
    void AppendGroupRange(LiveRange loop_range)
    {
        auto first_range = live_ranges_.back();
        live_ranges_.pop_back();
        ASSERT(loop_range.GetBegin() == first_range.GetBegin());
        // extend the first LiveRange
        first_range.SetEnd(std::max(loop_range.GetEnd(), first_range.GetEnd()));

        // resolve overlapping
        while (!live_ranges_.empty()) {
            if (first_range.Contains(live_ranges_.back())) {
                live_ranges_.pop_back();
            } else if (first_range.Contains(live_ranges_.back().GetBegin())) {
                ASSERT(live_ranges_.back().GetEnd() > first_range.GetEnd());
                first_range.SetEnd(live_ranges_.back().GetEnd());
                live_ranges_.pop_back();
                break;
            } else {
                break;
            }
        }
        live_ranges_.push_back(first_range);
    }

    void Clear()
    {
        live_ranges_.clear();
    }

    /*
     * Shorten the first range or create it if instruction has no users
     */
    void StartFrom(LifeNumber from)
    {
        if (live_ranges_.empty()) {
            AppendRange(from, from + LIFE_NUMBER_GAP);
        } else {
            ASSERT(live_ranges_.back().GetEnd() >= from);
            live_ranges_.back().SetBegin(from);
        }
    }

    const ArenaVector<LiveRange> &GetRanges() const
    {
        return live_ranges_;
    }

    LifeNumber GetBegin() const
    {
        ASSERT(!GetRanges().empty());
        return GetRanges().back().GetBegin();
    }

    LifeNumber GetEnd() const
    {
        ASSERT(!GetRanges().empty());
        return GetRanges().front().GetEnd();
    }

    template <bool INCLUDE_BORDER = false>
    bool SplitCover(LifeNumber position) const
    {
        for (auto range : GetRanges()) {
            if (range.GetBegin() <= position && position < range.GetEnd()) {
                return true;
            }
            if constexpr (INCLUDE_BORDER) {  // NOLINT(readability-braces-around-statements,
                                             // bugprone-suspicious-semicolon)
                if (position == range.GetEnd()) {
                    return true;
                }
            }
        }
        return false;
    }

    void SetReg(Register reg)
    {
        SetLocation(Location::MakeRegister(reg, type_));
    }

    void SetPreassignedReg(Register reg)
    {
        SetReg(reg);
        is_preassigned_ = true;
    }

    void SetPhysicalReg(Register reg, DataType::Type type)
    {
        SetLocation(Location::MakeRegister(reg, type));
        SetType(type);
        is_physical_ = true;
    }

    Register GetReg() const
    {
        return location_.GetValue();
    }

    bool HasReg() const
    {
        return location_.IsFixedRegister();
    }

    void SetLocation(Location location)
    {
        location_ = location;
    }

    Location GetLocation() const
    {
        return location_;
    }

    void ClearLocation()
    {
        SetLocation(Location::Invalid());
    }

    void SetType(DataType::Type type)
    {
        type_ = type;
    }

    DataType::Type GetType() const
    {
        return type_;
    }

    Inst *GetInst() const
    {
        ASSERT(!is_physical_);
        return inst_;
    }

    bool HasInst() const
    {
        return inst_ != nullptr;
    }

    const auto &GetUsePositions() const
    {
        return use_positions_;
    }

    void AddUsePosition(LifeNumber ln)
    {
        ASSERT(ln != 0 && ln != INVALID_LIFE_NUMBER);
        ASSERT(!finalized_);
        use_positions_.push_back(ln);
    }

    void PrependUsePosition(LifeNumber ln)
    {
        ASSERT(ln != 0 && ln != INVALID_LIFE_NUMBER);
        ASSERT(finalized_);
        ASSERT(use_positions_.empty() || ln <= use_positions_.front());
        use_positions_.insert(use_positions_.begin(), ln);
    }

    LifeNumber GetNextUsage(LifeNumber pos) const
    {
        ASSERT(finalized_);
        auto it = std::lower_bound(use_positions_.begin(), use_positions_.end(), pos);
        if (it != use_positions_.end()) {
            return *it;
        }
        return INVALID_LIFE_NUMBER;
    }

    LifeNumber GetLastUsageBefore(LifeNumber pos) const
    {
        ASSERT(finalized_);
        auto it = std::lower_bound(use_positions_.begin(), use_positions_.end(), pos);
        if (it == use_positions_.begin()) {
            return INVALID_LIFE_NUMBER;
        }
        it = std::prev(it);
        return it == use_positions_.end() ? INVALID_LIFE_NUMBER : *it;
    }

    LifeNumber GetPrevUsage(LifeNumber pos) const
    {
        ASSERT(finalized_);
        auto it = std::upper_bound(use_positions_.begin(), use_positions_.end(), pos);
        if (it != use_positions_.begin()) {
            return *std::prev(it);
        }
        return INVALID_LIFE_NUMBER;
    }

    bool NoUsageUntil(LifeNumber pos) const
    {
        ASSERT(finalized_);
        return use_positions_.empty() || (*use_positions_.begin() > pos);
    }

    bool NoDest() const
    {
        if (IsPseudoUserOfMultiOutput(inst_)) {
            return false;
        }
        return inst_->NoDest();
    }

    bool IsPreassigned() const
    {
        return is_preassigned_;
    }

    bool IsPhysical() const
    {
        return is_physical_;
    }

    template <bool WITH_INST_ID = true>
    std::string ToString() const
    {
        std::stringstream ss;
        auto delim = "";
        for (auto it = GetRanges().rbegin(); it != GetRanges().rend(); it++) {
            ss << delim << it->ToString();
            delim = " ";
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (WITH_INST_ID) {
            if (HasInst()) {
                ss << " {inst v" << std::to_string(GetInst()->GetId()) << "}";
            } else {
                ss << " {physical}";
            }
        }
        return ss.str();
    }

    // Split current interval at specified life number and return new interval starting at `ln`.
    // If interval has range [begin, end) then SplitAt call will truncate it to [begin, ln) and
    // returned interval will have range [ln, end).
    LifeIntervals *SplitAt(LifeNumber ln, ArenaAllocator *alloc);

    // Split current interval before or after the first use, then recursively do the same with splits.
    // The result will be
    // [use1---use2---useN) -> [use1),[---),[use2),[---)[useN)
    void SplitAroundUses(ArenaAllocator *alloc);

    // Helper to merge interval, which was splitted at the beginning: [a, a+1) [a+1, b) -> [a,b)
    void MergeSibling();

    // Return sibling interval created by SplitAt call or nullptr if there is no sibling for current interval.
    LifeIntervals *GetSibling() const
    {
        return sibling_;
    }

    // Return sibling interval covering specified life number or nullptr if there is no such sibling.
    LifeIntervals *FindSiblingAt(LifeNumber ln);

    bool Intersects(const LiveRange &range) const;
    // Return first point where `this` interval intersects with the `other
    LifeNumber GetFirstIntersectionWith(const LifeIntervals *other, LifeNumber search_from = 0) const;

    template <bool OTHER_IS_PHYSICAL = false>
    bool IntersectsWith(const LifeIntervals *other) const
    {
        auto intersection = GetFirstIntersectionWith(other);
        if constexpr (OTHER_IS_PHYSICAL) {
            ASSERT(other->IsPhysical());
            // Interval can intersect the physical one at the beginning of its live range only if that physical
            // interval's range was created for it. Try to find next intersection
            //
            // interval [--------------------------]
            // physical [-]       [-]           [-]
            //           ^         ^
            //          skip      intersection
            if (intersection == GetBegin()) {
                intersection = GetFirstIntersectionWith(other, intersection + 1U);
            }
        }
        return intersection != INVALID_LIFE_NUMBER;
    }

    bool IsSplitSibling() const
    {
        return is_split_sibling_;
    }

    bool FindRangeCoveringPosition(LifeNumber ln, LiveRange *dst) const;

    void Finalize()
    {
#ifndef NDEBUG
        ASSERT(!finalized_);
        finalized_ = true;
#endif
        std::sort(use_positions_.begin(), use_positions_.end());
    }

private:
    Inst *inst_ {nullptr};
    ArenaVector<LiveRange> live_ranges_;
    LifeIntervals *sibling_ {nullptr};
    ArenaVector<LifeNumber> use_positions_;
    Location location_;
    DataType::Type type_;
    uint8_t is_preassigned_ : 1;
    uint8_t is_physical_ : 1;
    uint8_t is_split_sibling_ : 1;
#ifndef NDEBUG
    uint8_t finalized_ : 1;
#endif
};

/*
 * Class to hold live instruction set
 */
class InstLiveSet {
public:
    explicit InstLiveSet(size_t size, ArenaAllocator *allocator) : bits_(size, allocator) {};
    NO_MOVE_SEMANTIC(InstLiveSet);
    NO_COPY_SEMANTIC(InstLiveSet);
    ~InstLiveSet() = default;

    void Union(const InstLiveSet *other)
    {
        if (other == nullptr) {
            return;
        }
        ASSERT(bits_.size() == other->bits_.size());
        bits_ |= other->bits_;
    }

    void Add(size_t index)
    {
        ASSERT(index < bits_.size());
        bits_.SetBit(index);
    }

    void Remove(size_t index)
    {
        ASSERT(index < bits_.size());
        bits_.ClearBit(index);
    }

    bool IsSet(size_t index)
    {
        ASSERT(index < bits_.size());
        return bits_.GetBit(index);
    }

private:
    ArenaBitVector bits_;
};

using LocationHints = ArenaMap<LifeNumber, Register>;
/*
 * `LivenessAnalyzer` is based on algorithm, published by Christian Wimmer and Michael Franz in
 * "Linear Scan Register Allocation on SSA Form" paper. ACM, 2010.
 */
class LivenessAnalyzer : public Analysis {
public:
    explicit LivenessAnalyzer(Graph *graph);

    NO_MOVE_SEMANTIC(LivenessAnalyzer);
    NO_COPY_SEMANTIC(LivenessAnalyzer);
    ~LivenessAnalyzer() override = default;

    bool RunImpl() override;

    const ArenaVector<BasicBlock *> &GetLinearizedBlocks() const
    {
        return linear_blocks_;
    }
    LifeIntervals *GetInstLifeIntervals(const Inst *inst) const;

    Inst *GetInstByLifeNumber(LifeNumber ln) const
    {
        return insts_by_life_number_[ln / LIFE_NUMBER_GAP];
    }

    BasicBlock *GetBlockCoversPoint(LifeNumber ln) const
    {
        for (auto bb : linear_blocks_) {
            if (GetBlockLiveRange(bb).Contains(ln)) {
                return bb;
            }
        }
        return nullptr;
    }

    void Cleanup()
    {
        for (auto *interv : inst_life_intervals_) {
            if (!interv->IsPhysical() && !interv->IsPreassigned()) {
                interv->ClearLocation();
            }
            if (interv->GetSibling() != nullptr) {
                interv->MergeSibling();
            }
        }
    }

    void Finalize()
    {
        for (auto *interv : inst_life_intervals_) {
            interv->Finalize();
        }
        for (auto *interv : intervals_for_temps_) {
            interv->Finalize();
        }
#ifndef NDEBUG
        finalized_ = true;
#endif
    }

    const ArenaVector<LifeIntervals *> &GetLifeIntervals() const
    {
        return inst_life_intervals_;
    }

    LiveRange GetBlockLiveRange(const BasicBlock *block) const;

    template <typename Func>
    void EnumerateLiveIntervalsForInst(Inst *inst, Func func)
    {
        auto inst_number = GetInstLifeNumber(inst);
        for (auto &li : GetLifeIntervals()) {
            if (!li->HasInst()) {
                continue;
            }
            auto li_inst = li->GetInst();
            // phi-inst could be removed after regalloc
            if (li_inst->GetBasicBlock() == nullptr) {
                ASSERT(li_inst->IsPhi());
                continue;
            }
            if (li_inst == inst || li->NoDest()) {
                continue;
            }
            auto sibling = li->FindSiblingAt(inst_number);
            if (sibling != nullptr && sibling->SplitCover(inst_number)) {
                func(sibling);
            }
        }
    }

    /*
     * 'interval_for_temp' - is additional life interval for an instruction required temp;
     * - Find instruction for which was created 'interval_for_temp'
     * - Enumerate instruction's inputs with fixed locations
     */
    template <typename Func>
    void EnumerateFixedLocationsOverlappingTemp(const LifeIntervals *interval_for_temp, Func func) const
    {
        ASSERT(!interval_for_temp->HasInst());
        ASSERT(interval_for_temp->GetBegin() + 1 == interval_for_temp->GetEnd());

        auto ln = interval_for_temp->GetEnd();
        auto inst = GetInstByLifeNumber(ln);
        ASSERT(inst != nullptr);

        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            auto location = inst->GetLocation(i);
            if (location.IsFixedRegister()) {
                func(location);
            }
        }
    }

    const char *GetPassName() const override
    {
        return "LivenessAnalysis";
    }

    size_t GetBlocksCount() const
    {
        return block_live_ranges_.size();
    }

    bool IsCallBlockingRegisters(Inst *inst) const;

    void DumpLifeIntervals(std::ostream &out = std::cout) const;
    void DumpLocationsUsage(std::ostream &out = std::cout) const;

    const UseTable &GetUseTable() const
    {
        return use_table_;
    }

    LifeIntervals *GetTmpRegInterval(const Inst *inst)
    {
        auto inst_ln = GetInstLifeNumber(inst);
        auto it = std::find_if(intervals_for_temps_.begin(), intervals_for_temps_.end(),
                               [inst_ln](auto li) { return li->GetBegin() == inst_ln - 1; });
        return it == intervals_for_temps_.end() ? nullptr : *it;
    }

private:
    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    void ResetLiveness();

    /*
     * Blocks linearization methods
     */
    bool AllForwardEdgesVisited(BasicBlock *block);
    void BuildBlocksLinearOrder();
    template <bool USE_PC_ORDER>
    void LinearizeBlocks();
    bool CheckLinearOrder();

    /*
     * Lifetime analysis methods
     */
    void BuildInstLifeNumbers();
    void BuildInstLifeIntervals();
    void ProcessBlockLiveInstructions(BasicBlock *block, InstLiveSet *live_set);
    void AdjustInputsLifetime(Inst *inst, LiveRange live_range, InstLiveSet *live_set);
    void SetInputRange(const Inst *inst, const Inst *input, LiveRange live_range) const;
    void CreateLifeIntervals(Inst *inst);
    void CreateIntervalForTemp(LifeNumber ln);
    InstLiveSet *GetInitInstLiveSet(BasicBlock *block);
    LifeNumber GetInstLifeNumber(const Inst *inst) const;
    void SetInstLifeNumber(const Inst *inst, LifeNumber number);
    void SetBlockLiveRange(BasicBlock *block, LiveRange life_range);
    void SetBlockLiveSet(BasicBlock *block, InstLiveSet *live_set);
    InstLiveSet *GetBlockLiveSet(BasicBlock *block) const;
    LifeNumber GetLoopEnd(Loop *loop);
    LiveRange GetPropagatedLiveRange(Inst *inst, LiveRange live_range);
    void AdjustCatchPhiInputsLifetime(Inst *inst);
    void SetUsePositions(Inst *user_inst, LifeNumber life_number);

    void BlockFixedRegisters(Inst *inst);
    template <bool IS_FP>
    void BlockReg(Register reg, LifeNumber block_from, LifeNumber block_to, bool is_use);
    template <bool IS_FP>
    void BlockPhysicalRegisters(LifeNumber block_from);
    void BlockFixedLocationRegister(Location location, LifeNumber ln)
    {
        BlockFixedLocationRegister(location, ln, ln + 1U, true);
    }
    void BlockFixedLocationRegister(Location location, LifeNumber block_from, LifeNumber block_to, bool is_use);

private:
    ArenaAllocator *allocator_;
    ArenaVector<BasicBlock *> linear_blocks_;
    ArenaVector<LifeNumber> inst_life_numbers_;
    ArenaVector<LifeIntervals *> inst_life_intervals_;
    InstVector insts_by_life_number_;
    ArenaVector<LiveRange> block_live_ranges_;
    ArenaVector<InstLiveSet *> block_live_sets_;
    ArenaMultiMap<Inst *, Inst *> pending_catch_phi_inputs_;
    ArenaVector<LifeIntervals *> physical_general_intervals_;
    ArenaVector<LifeIntervals *> physical_vector_intervals_;
    ArenaVector<LifeIntervals *> intervals_for_temps_;
    UseTable use_table_;
    bool has_safepoint_during_call_;

    Marker marker_ {UNDEF_MARKER};
#ifndef NDEBUG
    bool finalized_ {};
#endif
};

float CalcSpillWeight(const LivenessAnalyzer &la, LifeIntervals *interval);
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALIZER_H
