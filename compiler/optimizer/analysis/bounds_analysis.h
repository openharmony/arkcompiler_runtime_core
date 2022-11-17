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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_BOUNDSRANGE_ANALYSIS_H
#define COMPILER_OPTIMIZER_ANALYSIS_BOUNDSRANGE_ANALYSIS_H

#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
/**
 * Represents a range of values that a variable might have.
 *
 * It is used to represent variables of integral types according to their size
 * and sign.
 * It is used for REFERENCE type as well but only for reasoning whether a
 * variable is NULL or not.
 */
class BoundsRange {
public:
    using RangePair = std::pair<BoundsRange, BoundsRange>;

    explicit BoundsRange(DataType::Type type = DataType::INT64) : left_(GetMin(type)), right_(GetMax(type)) {};

    explicit BoundsRange(int64_t left, int64_t right, const Inst *inst = nullptr,
                         DataType::Type type = DataType::INT64);

    explicit BoundsRange(int64_t val, DataType::Type type = DataType::INT64);

    DEFAULT_COPY_SEMANTIC(BoundsRange);
    DEFAULT_MOVE_SEMANTIC(BoundsRange);
    ~BoundsRange() = default;

    const Inst *GetLenArray()
    {
        return len_array_;
    }
    int64_t GetLeft() const;

    int64_t GetRight() const;

    BoundsRange FitInType(DataType::Type type) const;

    bool IsConst() const;

    bool IsMaxRange(DataType::Type type = DataType::INT64) const;

    bool IsEqual(const BoundsRange &range) const;

    bool IsLess(const BoundsRange &range) const;

    bool IsLess(const Inst *inst) const;

    bool IsMore(const BoundsRange &range) const;

    bool IsMoreOrEqual(const BoundsRange &range) const;

    bool IsNotNegative() const;

    bool IsNegative() const;

    static int64_t GetMin(DataType::Type type);

    static int64_t GetMax(DataType::Type type);

    static BoundsRange Union(const ArenaVector<BoundsRange> &ranges);

    static RangePair NarrowBoundsByNE(RangePair const &ranges);
    static RangePair NarrowBoundsCase1(ConditionCode cc, RangePair const &ranges);
    static RangePair NarrowBoundsCase2(ConditionCode cc, RangePair const &ranges);
    static RangePair NarrowBoundsCase3(ConditionCode cc, RangePair const &ranges);
    static RangePair NarrowBoundsCase4(ConditionCode cc, RangePair const &ranges);
    static RangePair NarrowBoundsCase5(ConditionCode cc, RangePair const &ranges);
    static RangePair NarrowBoundsCase6(ConditionCode cc, RangePair const &ranges);

    static RangePair TryNarrowBoundsByCC(ConditionCode cc, RangePair const &ranges);

    static constexpr int64_t MAX_RANGE_VALUE = INT64_MAX;
    static constexpr int64_t MIN_RANGE_VALUE = INT64_MIN;

private:
    int64_t left_ = MIN_RANGE_VALUE;
    int64_t right_ = MAX_RANGE_VALUE;
    const Inst *len_array_ {nullptr};
};

class BoundsRangeInfo {
public:
    explicit BoundsRangeInfo(ArenaAllocator *aa) : aa_(*aa), bounds_range_info_(aa->Adapter()) {}
    NO_COPY_SEMANTIC(BoundsRangeInfo);
    NO_MOVE_SEMANTIC(BoundsRangeInfo);
    ~BoundsRangeInfo() = default;

    BoundsRange FindBoundsRange(const BasicBlock *block, Inst *inst) const;

    void SetBoundsRange(const BasicBlock *block, const Inst *inst, BoundsRange range);

private:
    ArenaAllocator &aa_;
    ArenaDoubleUnorderedMap<const BasicBlock *, const Inst *, BoundsRange> bounds_range_info_;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BoundsAnalysis : public Analysis, public GraphVisitor {
public:
    explicit BoundsAnalysis(Graph *graph);
    NO_MOVE_SEMANTIC(BoundsAnalysis);
    NO_COPY_SEMANTIC(BoundsAnalysis);
    ~BoundsAnalysis() override = default;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "BoundsAnalysis";
    }

    BoundsRangeInfo *GetBoundsRangeInfo()
    {
        return &bounds_range_info_;
    }

    const BoundsRangeInfo *GetBoundsRangeInfo() const
    {
        return &bounds_range_info_;
    }

    static void VisitIf([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitIfImm(GraphVisitor *v, Inst *inst);
    static void VisitPhi(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"
private:
    static bool CheckTriangleCase(const BasicBlock *block, const BasicBlock *tgt_block);

    static void CalcNewBoundsRangeForCompare(GraphVisitor *v, BasicBlock *block, ConditionCode cc, Inst *left,
                                             Inst *right, BasicBlock *tgt_block);
private:
    BoundsRangeInfo bounds_range_info_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_BOUNDS_ANALYSIS_H
