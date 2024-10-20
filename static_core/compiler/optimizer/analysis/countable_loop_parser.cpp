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

#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"

namespace ark::compiler {
/**
 * Check if loop is countable
 *
 * [Loop]
 * Phi(init, update)
 * ...
 * update(phi, 1)
 * Compare(Add/Sub, test)
 *
 * where `update` is Add or Sub instruction
 */
std::optional<CountableLoopInfo> CountableLoopParser::Parse()
{
    if (loop_.IsIrreducible() || loop_.IsOsrLoop() || loop_.IsTryCatchLoop() || loop_.GetBackEdges().size() != 1 ||
        loop_.IsRoot() || loop_.IsInfinite()) {
        return std::nullopt;
    }

    if (!ParseLoopExit()) {
        return std::nullopt;
    }

    if (!SetUpdateAndTestInputs()) {
        return std::nullopt;
    }

    if (!IsInstIncOrDec(loopInfo_.update)) {
        return std::nullopt;
    }
    SetIndexAndConstStep();
    if (loopInfo_.index->GetBasicBlock() != loop_.GetHeader()) {
        return std::nullopt;
    }

    if (!TryProcessBackEdge()) {
        return std::nullopt;
    }
    return loopInfo_;
}

bool CountableLoopParser::ParseLoopExit()
{
    auto loopExit = FindLoopExitBlock();
    if (loopExit == nullptr) {
        return false;
    }
    if (loopExit->IsEmpty() || (loopExit != loop_.GetHeader() && loopExit != loop_.GetBackEdges()[0])) {
        return false;
    }
    isHeadLoopExit_ = (loopExit == loop_.GetHeader() && loopExit != loop_.GetBackEdges()[0]);
    loopInfo_.ifImm = loopExit->GetLastInst();
    if (loopInfo_.ifImm->GetOpcode() != Opcode::IfImm && loopInfo_.ifImm->GetOpcode() != Opcode::If) {
        return false;
    }
    auto loopExitCmp = loopInfo_.ifImm->GetInput(0).GetInst();
    if (loopExitCmp->GetOpcode() != Opcode::Compare) {
        return false;
    }
    if (isHeadLoopExit_ && !loopExitCmp->GetInput(0).GetInst()->IsPhi() &&
        !loopExitCmp->GetInput(1).GetInst()->IsPhi()) {
        return false;
    }
    auto cmpType = loopExitCmp->CastToCompare()->GetOperandsType();
    return DataType::GetCommonType(cmpType) == DataType::INT64;
}

bool CountableLoopParser::TryProcessBackEdge()
{
    ASSERT(loopInfo_.index->IsPhi());
    auto backEdge {loop_.GetBackEdges()[0]};
    auto backEdgeIdx {loopInfo_.index->CastToPhi()->GetPredBlockIndex(backEdge)};
    if (loopInfo_.index->GetInput(backEdgeIdx).GetInst() != loopInfo_.update) {
        return false;
    }
    ASSERT(loopInfo_.index->GetInputsCount() == MAX_SUCCS_NUM);
    loopInfo_.init = loopInfo_.index->GetInput(1 - backEdgeIdx).GetInst();
    SetNormalizedConditionCode();
    return IsConditionCodeAcceptable();
}

bool CountableLoopParser::HasPreHeaderCompare(Loop *loop, const CountableLoopInfo &loopInfo)
{
    auto preHeader = loop->GetPreHeader();
    auto backEdge = loop->GetBackEdges()[0];
    if (loopInfo.ifImm->GetBasicBlock() != backEdge || preHeader->IsEmpty() ||
        preHeader->GetLastInst()->GetOpcode() != Opcode::IfImm) {
        return false;
    }
    auto preHeaderIfImm = preHeader->GetLastInst();
    ASSERT(preHeaderIfImm->GetOpcode() == Opcode::IfImm);
    auto preHeaderCmp = preHeaderIfImm->GetInput(0).GetInst();
    if (preHeaderCmp->GetOpcode() != Opcode::Compare) {
        return false;
    }
    auto backEdgeCmp = loopInfo.ifImm->GetInput(0).GetInst();
    ASSERT(backEdgeCmp->GetOpcode() == Opcode::Compare);

    // Compare condition codes
    if (preHeaderCmp->CastToCompare()->GetCc() != backEdgeCmp->CastToCompare()->GetCc()) {
        return false;
    }

    if (loopInfo.ifImm->CastToIfImm()->GetCc() != preHeaderIfImm->CastToIfImm()->GetCc() ||
        loopInfo.ifImm->CastToIfImm()->GetImm() != preHeaderIfImm->CastToIfImm()->GetImm()) {
        return false;
    }

    // Compare control-flow
    if (preHeader->GetTrueSuccessor() != backEdge->GetTrueSuccessor() ||
        preHeader->GetFalseSuccessor() != backEdge->GetFalseSuccessor()) {
        return false;
    }

    // Compare test inputs
    auto testInputIdx = 1;
    if (backEdgeCmp->GetInput(0) == loopInfo.test) {
        testInputIdx = 0;
    } else {
        ASSERT(backEdgeCmp->GetInput(1) == loopInfo.test);
    }

    return preHeaderCmp->GetInput(testInputIdx).GetInst() == loopInfo.test &&
           preHeaderCmp->GetInput(1 - testInputIdx).GetInst() == loopInfo.init;
}

// Returns exact number of iterations for loop with constant boundaries
// if its index does not overflow
std::optional<uint64_t> CountableLoopParser::GetLoopIterations(const CountableLoopInfo &loopInfo)
{
    if (!loopInfo.init->IsConst() || !loopInfo.test->IsConst() || loopInfo.constStep == 0) {
        return std::nullopt;
    }
    uint64_t initValue = loopInfo.init->CastToConstant()->GetInt64Value();
    uint64_t testValue = loopInfo.test->CastToConstant()->GetInt64Value();
    auto type = loopInfo.index->GetType();

    if (loopInfo.isInc) {
        int64_t maxTest = BoundsRange::GetMax(type) - static_cast<int64_t>(loopInfo.constStep);
        if (loopInfo.normalizedCc == CC_LE) {
            maxTest--;
        }
        if (static_cast<int64_t>(testValue) > maxTest) {
            // index may overflow
            return std::nullopt;
        }
    } else {
        int64_t minTest = BoundsRange::GetMin(type) + static_cast<int64_t>(loopInfo.constStep);
        if (loopInfo.normalizedCc == CC_GE) {
            minTest++;
        }
        if (static_cast<int64_t>(testValue) < minTest) {
            // index may overflow
            return std::nullopt;
        }
        std::swap(initValue, testValue);
    }
    if (static_cast<int64_t>(initValue) > static_cast<int64_t>(testValue)) {
        return 0;
    }
    uint64_t diff = testValue - initValue;
    uint64_t count = diff + loopInfo.constStep;
    if (diff > std::numeric_limits<uint64_t>::max() - loopInfo.constStep) {
        // count may overflow
        return std::nullopt;
    }
    if (loopInfo.normalizedCc == CC_LT || loopInfo.normalizedCc == CC_GT) {
        count--;
    }
    return count / loopInfo.constStep;
}

/*
 * Check if instruction is Add or Sub with constant and phi inputs
 */
bool CountableLoopParser::IsInstIncOrDec(Inst *inst)
{
    if (!inst->IsAddSub()) {
        return false;
    }
    ConstantInst *cnst = nullptr;
    if (inst->GetInput(0).GetInst()->IsConst() && inst->GetInput(1).GetInst()->IsPhi()) {
        cnst = inst->GetInput(0).GetInst()->CastToConstant();
    } else if (inst->GetInput(1).GetInst()->IsConst() && inst->GetInput(0).GetInst()->IsPhi()) {
        cnst = inst->GetInput(1).GetInst()->CastToConstant();
    }
    return cnst != nullptr;
}

// NOTE(a.popov) Suppot 'GetLoopExit()' method in the 'Loop' class
BasicBlock *CountableLoopParser::FindLoopExitBlock()
{
    auto outerLoop = loop_.GetOuterLoop();
    BasicBlock *loopExit = nullptr;
    for (auto block : loop_.GetBlocks()) {
        const auto &succs = block->GetSuccsBlocks();
        auto it = std::find_if(succs.begin(), succs.end(),
                               [&outerLoop](const BasicBlock *bb) { return bb->GetLoop() == outerLoop; });
        if (it != succs.end()) {
            // Countable loop must have a single exit:
            if (loopExit != nullptr) {
                return nullptr;
            }
            loopExit = block;
        }
    }
    return loopExit;
}

bool CountableLoopParser::SetUpdateAndTestInputs()
{
    auto loopExitCmp = loopInfo_.ifImm->GetInput(0).GetInst();
    ASSERT(loopExitCmp->GetOpcode() == Opcode::Compare);
    loopInfo_.update = loopExitCmp->GetInput(0).GetInst();
    loopInfo_.test = loopExitCmp->GetInput(1).GetInst();
    if (isHeadLoopExit_) {
        if (!loopInfo_.update->IsPhi()) {
            std::swap(loopInfo_.update, loopInfo_.test);
        }
        ASSERT(loopInfo_.update->IsPhi());
        if (loopInfo_.update->GetBasicBlock() != loop_.GetHeader()) {
            return false;
        }
        auto backEdge {loop_.GetBackEdges()[0]};
        loopInfo_.update = loopInfo_.update->CastToPhi()->GetPhiInput(backEdge);
    } else {
        if (!IsInstIncOrDec(loopInfo_.update)) {
            std::swap(loopInfo_.update, loopInfo_.test);
        }
    }

    return true;
}

void CountableLoopParser::SetIndexAndConstStep()
{
    loopInfo_.index = loopInfo_.update->GetInput(0).GetInst();
    auto constInst = loopInfo_.update->GetInput(1).GetInst();
    if (loopInfo_.index->IsConst()) {
        loopInfo_.index = loopInfo_.update->GetInput(1).GetInst();
        constInst = loopInfo_.update->GetInput(0).GetInst();
    }

    ASSERT(constInst->GetType() == DataType::INT64);
    auto cnst = constInst->CastToConstant()->GetIntValue();
    const uint64_t mask = (1ULL << 63U);
    auto isNeg = DataType::IsTypeSigned(loopInfo_.update->GetType()) && (cnst & mask) != 0;
    loopInfo_.isInc = loopInfo_.update->IsAdd();
    if (isNeg) {
        cnst = ~cnst + 1;
        loopInfo_.isInc = !loopInfo_.isInc;
    }
    loopInfo_.constStep = cnst;
}

void CountableLoopParser::SetNormalizedConditionCode()
{
    auto loopExit = loopInfo_.ifImm->GetBasicBlock();
    ASSERT(loopExit != nullptr);
    auto loopExitCmp = loopInfo_.ifImm->GetInput(0).GetInst();
    ASSERT(loopExitCmp->GetOpcode() == Opcode::Compare);
    auto cc = loopExitCmp->CastToCompare()->GetCc();
    if (loopInfo_.test == loopExitCmp->GetInput(0).GetInst()) {
        cc = SwapOperandsConditionCode(cc);
    }
    ASSERT(loopInfo_.ifImm->CastToIfImm()->GetImm() == 0);
    if (loopInfo_.ifImm->CastToIfImm()->GetCc() == CC_EQ) {
        cc = GetInverseConditionCode(cc);
    } else {
        ASSERT(loopInfo_.ifImm->CastToIfImm()->GetCc() == CC_NE);
    }
    auto loop = loopExit->GetLoop();
    if (loopExit->GetFalseSuccessor()->GetLoop() == loop ||
        loopExit->GetFalseSuccessor()->GetLoop()->GetOuterLoop() == loop) {
        cc = GetInverseConditionCode(cc);
    } else {
        ASSERT(loopExit->GetTrueSuccessor()->GetLoop() == loop ||
               loopExit->GetTrueSuccessor()->GetLoop()->GetOuterLoop() == loop);
    }
    loopInfo_.normalizedCc = cc;
}

bool CountableLoopParser::IsConditionCodeAcceptable()
{
    auto cc = loopInfo_.normalizedCc;
    // Condition should be: inc <= test | inc < test
    if (loopInfo_.isInc && cc != CC_LE && cc != CC_LT) {
        return false;
    }
    // Condition should be: dec >= test | dec > test
    if (!loopInfo_.isInc && cc != CC_GE && cc != CC_GT) {
        return false;
    }
    return true;
}
}  // namespace ark::compiler
