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
#include "bounds_analysis.h"
#include "dominators_tree.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "compiler/optimizer/ir/analysis.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace panda::compiler {
BoundsRange::BoundsRange(int64_t val, DataType::Type type) : BoundsRange(val, val, nullptr, type) {}

BoundsRange::BoundsRange(int64_t left, int64_t right, const Inst *inst, [[maybe_unused]] DataType::Type type)
    : left_(left), right_(right), len_array_(inst)
{
    ASSERT(inst == nullptr);
    ASSERT(left <= right);
    ASSERT(GetMin(type) <= left);
    ASSERT(right <= GetMax(type));
}

int64_t BoundsRange::GetLeft() const
{
    return left_;
}

int64_t BoundsRange::GetRight() const
{
    return right_;
}

bool BoundsRange::IsConst() const
{
    return left_ == right_;
}

bool BoundsRange::IsMaxRange(DataType::Type type) const
{
    return left_ <= GetMin(type) && right_ >= GetMax(type);
}

bool BoundsRange::IsEqual(const BoundsRange &range) const
{
    return left_ == range.GetLeft() && right_ == range.GetRight();
}

bool BoundsRange::IsLess(const BoundsRange &range) const
{
    return right_ < range.GetLeft();
}

bool BoundsRange::IsLess(const Inst *inst) const
{
    return false;
}

bool BoundsRange::IsMore(const BoundsRange &range) const
{
    return left_ > range.GetRight();
}

bool BoundsRange::IsMoreOrEqual(const BoundsRange &range) const
{
    return left_ >= range.GetRight();
}

bool BoundsRange::IsNotNegative() const
{
    return left_ >= 0;
}

bool BoundsRange::IsNegative() const
{
    return right_ < 0;
}
/**
 * Return the minimal value for a type.
 *
 * We consider that REFERENCE type has only non-negative address values
 */
int64_t BoundsRange::GetMin(DataType::Type type)
{
    ASSERT(!IsFloatType(type));
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::UINT16:
        case DataType::UINT32:
        case DataType::UINT64:
        case DataType::REFERENCE:
            return 0;
        case DataType::INT8:
            return INT8_MIN;
        case DataType::INT16:
            return INT16_MIN;
        case DataType::INT32:
            return INT32_MIN;
        case DataType::INT64:
            return INT64_MIN;
        default:
            UNREACHABLE();
    }
}

/**
 * Return the maximal value for a type.
 *
 * For REFERENCE we are interested in whether it is NULL or not.  Set the
 * maximum to INT64_MAX regardless the real architecture bitness.
 */
int64_t BoundsRange::GetMax(DataType::Type type)
{
    ASSERT(!IsFloatType(type));
    ASSERT(type != DataType::UINT64);
    switch (type) {
        case DataType::BOOL:
            return 1;
        case DataType::UINT8:
            return UINT8_MAX;
        case DataType::UINT16:
            return UINT16_MAX;
        case DataType::UINT32:
            return UINT32_MAX;
        case DataType::INT8:
            return INT8_MAX;
        case DataType::INT16:
            return INT16_MAX;
        case DataType::INT32:
            return INT32_MAX;
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::INT64:
            return INT64_MAX;
        case DataType::REFERENCE:
            return INT64_MAX;
        default:
            UNREACHABLE();
    }
}

BoundsRange BoundsRange::FitInType(DataType::Type type) const
{
    auto type_min = BoundsRange::GetMin(type);
    auto type_max = BoundsRange::GetMax(type);
    if (left_ < type_min || left_ > type_max || right_ < type_min || right_ > type_max) {
        return BoundsRange(type_min, type_max);
    }
    return *this;
}

BoundsRange BoundsRange::Union(const ArenaVector<BoundsRange> &ranges)
{
    int64_t min = MAX_RANGE_VALUE;
    int64_t max = MIN_RANGE_VALUE;
    for (const auto &range : ranges) {
        if (range.GetLeft() < min) {
            min = range.GetLeft();
        }
        if (range.GetRight() > max) {
            max = range.GetRight();
        }
    }
    return BoundsRange(min, max);
}

BoundsRange::RangePair BoundsRange::NarrowBoundsByNE(BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    // We can narrow bounds of a range if another is a constant and matches one of the bounds
    // Mostly needed for a reference comparison with null
    if (left_range.IsConst() && !right_range.IsConst()) {
        if (ll == rl) {
            return {left_range, BoundsRange(rl + 1, rr)};
        }
        if (ll == rr) {
            return {left_range, BoundsRange(rl, rr - 1)};
        }
    }
    if (!left_range.IsConst() && right_range.IsConst()) {
        if (rl == ll) {
            return {BoundsRange(ll + 1, lr), right_range};
        }
        if (rl == lr) {
            return {BoundsRange(ll, lr - 1), right_range};
        }
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase1(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_A) {
        // With equal rl and lr left_range cannot be greater than right_range
        if (rl == lr) {
            return {BoundsRange(), BoundsRange()};
        }
        return {BoundsRange(rl + 1, lr), BoundsRange(rl, lr - 1)};
    }
    if (cc == ConditionCode::CC_GE || cc == ConditionCode::CC_AE || cc == ConditionCode::CC_EQ) {
        return {BoundsRange(rl, lr), BoundsRange(rl, lr)};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase2(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_GE || cc == ConditionCode::CC_EQ ||
        cc == ConditionCode::CC_A || cc == ConditionCode::CC_AE) {
        return {BoundsRange(), BoundsRange()};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase3(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_A) {
        // rl == lr handled in case 1
        return {BoundsRange(rl + 1, lr), right_range};
    }
    if (cc == ConditionCode::CC_GE || cc == ConditionCode::CC_AE) {
        return {BoundsRange(rl, lr), right_range};
    }
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_B) {
        // With equal ll and rr left_range cannot be less than right_range
        if (ll == rr) {
            return {BoundsRange(), BoundsRange()};
        }
        return {BoundsRange(ll, rr - 1), right_range};
    }
    if (cc == ConditionCode::CC_LE || cc == ConditionCode::CC_BE) {
        return {BoundsRange(ll, rr), right_range};
    }
    if (cc == ConditionCode::CC_EQ) {
        return {BoundsRange(rl, rr), right_range};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase4(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t rr = right_range.GetRight();
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_B) {
        // With equal ll and rr left_range cannot be less than right_range
        if (ll == rr) {
            return {BoundsRange(), BoundsRange()};
        }
        return {BoundsRange(ll, rr - 1), BoundsRange(ll + 1, rr)};
    }
    if (cc == ConditionCode::CC_LE || cc == ConditionCode::CC_BE || cc == ConditionCode::CC_EQ) {
        return {BoundsRange(ll, rr), BoundsRange(ll, rr)};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase5(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_LE || cc == ConditionCode::CC_EQ ||
        cc == ConditionCode::CC_B || cc == ConditionCode::CC_BE) {
        return {BoundsRange(), BoundsRange()};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase6(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_A) {
        // rl == lr handled in case 1
        return {left_range, BoundsRange(rl, lr - 1)};
    }
    if (cc == ConditionCode::CC_GE || cc == ConditionCode::CC_AE) {
        return {left_range, BoundsRange(rl, lr)};
    }
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_B) {
        // ll == rr handled in case 4
        return {left_range, BoundsRange(ll + 1, rr)};
    }
    if (cc == ConditionCode::CC_LE || cc == ConditionCode::CC_BE) {
        return {left_range, BoundsRange(ll, rr)};
    }
    if (cc == ConditionCode::CC_EQ) {
        return {left_range, BoundsRange(ll, lr)};
    }
    return ranges;
}

/**
 * Try narrow bounds range for <if (left CC right)> situation
 * Return a pair of narrowed left and right intervals
 */
BoundsRange::RangePair BoundsRange::TryNarrowBoundsByCC(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    if (cc == ConditionCode::CC_NE) {
        return NarrowBoundsByNE(ranges);
    }
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    // For further description () is for left_range bounds and [] is for right_range bounds
    // case 1: ( [ ) ]
    if (ll <= rl && rl <= lr && lr <= rr) {
        return NarrowBoundsCase1(cc, ranges);
    }
    // case 2: ( ) [ ]
    if (ll <= lr && lr < rl && rl <= rr) {
        return NarrowBoundsCase2(cc, ranges);
    }
    // case 3: ( [ ] )
    if (ll <= rl && rl <= rr && rr <= lr) {
        return NarrowBoundsCase3(cc, ranges);
    }
    // case 4: [ ( ] )
    if (rl <= ll && ll <= rr && rr <= lr) {
        return NarrowBoundsCase4(cc, ranges);
    }
    // case 5: [ ] ( )
    if (rl <= rr && rr < ll && ll <= lr) {
        return NarrowBoundsCase5(cc, ranges);
    }
    // case 6: [ ( ) ]
    if (rl <= ll && ll <= lr && lr <= rr) {
        return NarrowBoundsCase6(cc, ranges);
    }
    return ranges;
}

BoundsRange BoundsRangeInfo::FindBoundsRange(const BasicBlock *block, Inst *inst) const
{
    ASSERT(block != nullptr && inst != nullptr);
    ASSERT(!IsFloatType(inst->GetType()));
    ASSERT(inst->GetType() == DataType::REFERENCE || DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (inst->GetOpcode() == Opcode::NullPtr) {
        ASSERT(inst->GetType() == DataType::REFERENCE);
        return BoundsRange(0);
    }
    if (IsInstNotNull(inst)) {
        ASSERT(inst->GetType() == DataType::REFERENCE);
        return BoundsRange(1, BoundsRange::GetMax(DataType::REFERENCE));
    }
    while (block != nullptr) {
        if (bounds_range_info_.find(block) != bounds_range_info_.end() &&
            bounds_range_info_.at(block).find(inst) != bounds_range_info_.at(block).end()) {
            return bounds_range_info_.at(block).at(inst);
        }
        block = block->GetDominator();
    }
    if (inst->IsConst()) {
        ASSERT(inst->GetType() == DataType::INT64);
        auto val = static_cast<int64_t>(inst->CastToConstant()->GetIntValue());
        return BoundsRange(val);
    }
    // if we know nothing about inst return the complete range of type
    return BoundsRange(inst->GetType());
}

void BoundsRangeInfo::SetBoundsRange(const BasicBlock *block, const Inst *inst, BoundsRange range)
{
    if (inst->IsConst() && range.GetLenArray() == nullptr) {
        return;
    }
    if (inst->IsConst()) {
        auto val = static_cast<int64_t>(static_cast<const ConstantInst *>(inst)->GetIntValue());
        range = BoundsRange(val, val, range.GetLenArray());
    }
    ASSERT(inst->GetType() == DataType::REFERENCE || DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    ASSERT(range.GetLeft() >= BoundsRange::GetMin(inst->GetType()));
    ASSERT(range.GetRight() <= BoundsRange::GetMax(inst->GetType()));
    if (!range.IsMaxRange() || range.GetLenArray() != nullptr) {
        if (bounds_range_info_.find(block) == bounds_range_info_.end()) {
            auto it1 = bounds_range_info_.emplace(block, aa_.Adapter());
            ASSERT(it1.second);
            it1.first->second.emplace(inst, range);
        } else if (bounds_range_info_.at(block).find(inst) == bounds_range_info_.at(block).end()) {
            bounds_range_info_.at(block).emplace(inst, range);
        } else {
            bounds_range_info_.at(block).at(inst) = range;
        }
    }
}

BoundsAnalysis::BoundsAnalysis(Graph *graph) : Analysis(graph), bounds_range_info_(graph->GetAllocator()) {}

bool BoundsAnalysis::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();

    VisitGraph();

    return true;
}

const ArenaVector<BasicBlock *> &BoundsAnalysis::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

void BoundsAnalysis::VisitIf([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    UNREACHABLE();
}

void BoundsAnalysis::VisitIfImm(GraphVisitor *v, Inst *inst)
{
    auto if_inst = inst->CastToIfImm();
    ASSERT(if_inst->GetOperandsType() == DataType::BOOL);
    ASSERT(if_inst->GetCc() == ConditionCode::CC_NE || if_inst->GetCc() == ConditionCode::CC_EQ);
    ASSERT(if_inst->GetImm() == 0);

    auto input = inst->GetInput(0).GetInst();
    if (input->GetOpcode() != Opcode::Compare) {
        return;
    }
    auto compare = input->CastToCompare();
    if (compare->GetOperandsType() == DataType::UINT64) {
        return;
    }
    auto op0 = compare->GetInput(0).GetInst();
    auto op1 = compare->GetInput(1).GetInst();

    if ((DataType::GetCommonType(op0->GetType()) != DataType::INT64 && op0->GetType() != DataType::REFERENCE) ||
        (DataType::GetCommonType(op1->GetType()) != DataType::INT64 && op1->GetType() != DataType::REFERENCE)) {
        return;
    }

    auto cc = compare->GetCc();
    auto block = inst->GetBasicBlock();
    BasicBlock *true_block;
    BasicBlock *false_block;
    if (if_inst->GetCc() == ConditionCode::CC_NE) {
        // Corresponds to Compare result
        true_block = block->GetTrueSuccessor();
        false_block = block->GetFalseSuccessor();
    } else if (if_inst->GetCc() == ConditionCode::CC_EQ) {
        // Corresponds to inversion of Compare result
        true_block = block->GetFalseSuccessor();
        false_block = block->GetTrueSuccessor();
    } else {
        UNREACHABLE();
    }
    CalcNewBoundsRangeForCompare(v, block, cc, op0, op1, true_block);
    CalcNewBoundsRangeForCompare(v, block, GetInverseConditionCode(cc), op0, op1, false_block);
}

void BoundsAnalysis::VisitPhi(GraphVisitor *v, Inst *inst)
{
    if (IsFloatType(inst->GetType()) || inst->GetType() == DataType::REFERENCE || inst->GetType() == DataType::UINT64) {
        return;
    }
    auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
    auto phi = inst->CastToPhi();
    auto phi_block = inst->GetBasicBlock();
    auto phi_type = phi->GetType();
    ArenaVector<BoundsRange> ranges(phi_block->GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &block : phi_block->GetPredsBlocks()) {
        ranges.emplace_back(bri->FindBoundsRange(block, phi->GetPhiInput(block)));
    }
    bri->SetBoundsRange(phi_block, phi, BoundsRange::Union(ranges).FitInType(phi_type));
}

bool BoundsAnalysis::CheckTriangleCase(const BasicBlock *block, const BasicBlock *tgt_block)
{
    auto &preds_blocks = tgt_block->GetPredsBlocks();
    auto loop = tgt_block->GetLoop();
    auto &back_edges = loop->GetBackEdges();
    if (preds_blocks.size() == 1) {
        return true;
    }
    if (!loop->IsRoot() && back_edges.size() == 1 && preds_blocks.size() == 2U) {
        if (preds_blocks[0] == block && preds_blocks[1] == back_edges[0]) {
            return true;
        }
        if (preds_blocks[1] == block && preds_blocks[0] == back_edges[0]) {
            return true;
        }
        return false;
    }
    return false;
}

void BoundsAnalysis::CalcNewBoundsRangeForCompare(GraphVisitor *v, BasicBlock *block, ConditionCode cc, Inst *left,
                                                  Inst *right, BasicBlock *tgt_block)
{
    auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
    auto left_range = bri->FindBoundsRange(block, left);
    auto right_range = bri->FindBoundsRange(block, right);
    // try to skip triangle:
    /* [block]
     *    |  \
     *    |   \
     *    |   [BB]
     *    |   /
     *    |  /
     * [tgt_block]
     */
    if (CheckTriangleCase(block, tgt_block)) {
        auto ranges = BoundsRange::TryNarrowBoundsByCC(cc, {left_range, right_range});
        ASSERT(left_range.GetLenArray() == nullptr);
        ASSERT(right_range.GetLenArray() == nullptr);
        bri->SetBoundsRange(tgt_block, left, ranges.first.FitInType(left->GetType()));
        bri->SetBoundsRange(tgt_block, right, ranges.second.FitInType(right->GetType()));
    }
}
}  // namespace panda::compiler
