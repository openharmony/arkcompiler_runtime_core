/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/string_builder_utils.h"

#include "hybrid_strings_optimization.h"

namespace ark::compiler {

HybridStringsOptimization::HybridStringsOptimization(Graph *graph)
    : Optimization(graph), inputDescriptors_ {graph->GetLocalAllocator()->Adapter()}
{
}

void HybridStringsOptimization::MarkMaybeDeoptimizedConcatStringsInputs(Inst *inst, Marker visited,
                                                                        Marker maybeDeoptimized)
{
    if (inst->SetMarker(visited)) {
        return;
    }

    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (IsMethodStringBuilderConcatStrings(inputInst)) {
            inputInst->SetMarker(maybeDeoptimized);
        }

        if (inputInst->IsPhi()) {
            MarkMaybeDeoptimizedConcatStringsInputs(inputInst, visited, maybeDeoptimized);
        }
    }
}

void HybridStringsOptimization::MarkMaybeDeoptimizedInstruction(Inst *inst, Marker visited, Marker maybeDeoptimized)
{
    ASSERT(inst->CanDeoptimize());

    if (inst->RequireState()) {
        auto saveState = inst->GetSaveState();
        while (saveState != nullptr) {
            MarkMaybeDeoptimizedConcatStringsInputs(saveState, visited, maybeDeoptimized);
            saveState = saveState->GetCallerInst() != nullptr ? saveState->GetCallerInst()->GetSaveState() : nullptr;
        }
    }

    return MarkMaybeDeoptimizedConcatStringsInputs(inst, visited, maybeDeoptimized);
}

void HybridStringsOptimization::MarkMaybeDeoptimizedInstructions(Marker maybeDeoptimized)
{
    MarkerHolder visited {GetGraph()};
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (!inst->CanDeoptimize()) {
                continue;
            }

            MarkMaybeDeoptimizedInstruction(inst, visited.GetMarker(), maybeDeoptimized);
        }
    }
}

bool HybridStringsOptimization::RunImpl()
{
    isApplied_ = false;

    GetGraph()->RunPass<DominatorsTree>();
    Marker maybeDeoptimized = GetGraph()->NewMarker();
    MarkMaybeDeoptimizedInstructions(maybeDeoptimized);

    Marker visited = GetGraph()->NewMarker();
    auto &blocksRPO = GetGraph()->GetBlocksRPO();
    for (auto block = blocksRPO.rbegin(); block != blocksRPO.rend(); block++) {
        for (auto inst : (*block)->AllInstsSafeReverse()) {
            if (inst->IsMarked(visited)) {
                continue;
            }

            if (!IsMethodStringBuilderConcatStrings(inst) && !inst->IsPhi()) {
                continue;
            }

            ConcatenationMatch match {GetGraph()};
            MatchConcatenationExpression(inst, match, visited, maybeDeoptimized);
            if (!match.IsValid()) {
                continue;
            }

            ReplaceEachWithStringBuilder(match);
            Cleanup(match);
            isApplied_ = true;
        }
    }
    GetGraph()->EraseMarker(visited);
    GetGraph()->EraseMarker(maybeDeoptimized);

    isApplied_ |= ManuallyInlineMethods();
    if (isApplied_) {
        GetGraph()->RunPass<compiler::Cleanup>();
    }

    return isApplied_;
}

bool ManuallyInlineStringBuilderConcatStrings(Graph *graph, Inst *concatStrings)
{
    ASSERT(IsMethodStringBuilderConcatStrings(concatStrings));

    auto lhs = concatStrings->GetInput(0).GetInst();
    auto rhs = concatStrings->GetInput(1).GetInst();

    std::array<Inst *, 4U> args {lhs, rhs, nullptr, nullptr};
    auto concatIntrinsic = CreateConcatIntrinsic(graph, args, 2, DataType::REFERENCE, concatStrings->GetSaveState());

    InsertBeforeWithSaveState(concatIntrinsic, concatStrings);

    concatStrings->ReplaceUsers(concatIntrinsic);
    concatStrings->ClearFlag(inst_flags::NO_DCE);

    return true;
}

bool HybridStringsOptimization::ManuallyInlineMethods()
{
    bool isInlined = false;
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsEmpty()) {
            continue;
        }

        for (Inst *inst : block->Insts()) {
            if (!inst->GetFlag(inst_flags::NO_DCE)) {
                continue;
            }

            if (!IsMethodStringBuilderConcatStrings(inst)) {
                continue;
            }

            if (IsMethodInInliningBlackList(GetGraph()->GetRuntime(), inst)) {
                continue;
            }

            isInlined |= ManuallyInlineStringBuilderConcatStrings(GetGraph(), inst);
        }

        bool blockStillHasConcatStrings = false;
        for (Inst *inst : block->Insts()) {
            if (!IsMethodStringBuilderConcatStrings(inst)) {
                continue;
            }

            if (inst->GetFlag(inst_flags::NO_DCE)) {
                blockStillHasConcatStrings = true;
            }
        }

        if (blockStillHasConcatStrings) {
            return isInlined;
        }

        for (Inst *inst : block->Insts()) {
            if (inst->Is(Opcode::InitClass) &&
                GetGraph()->GetRuntime()->IsClassStringBuilder(inst->CastToInitClass()->GetClass())) {
                inst->ClearFlag(inst_flags::NO_DCE);
            }
        }
    }

    return isInlined;
}

static bool IsCheckCastWithoutUsers(Inst *inst)
{
    return inst->GetOpcode() == Opcode::CheckCast && !inst->HasUsers();
}

static bool HasUserOtherThanConcat([[maybe_unused]] Inst *inst)
{
    return HasUser(inst, [](auto &user) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        bool isSaveState = userInst->IsSaveState();
        bool isCheckCast = IsCheckCastWithoutUsers(userInst);
        bool isReturn = userInst->IsReturn();
        bool isPhi = userInst->IsPhi();
        bool isLenArray = userInst->GetOpcode() == Opcode::LenArray;
        bool isConcatStrings = IsMethodStringBuilderConcatStrings(userInst);
        return !isSaveState && !isCheckCast && !isReturn && !isPhi && !isLenArray && !isConcatStrings;
    });
}

void HybridStringsOptimization::MatchConcatenationExpression(Inst *inst, ConcatenationMatch &match, Marker visited,
                                                             Marker maybeDeoptimized)
{
    ASSERT(IsMethodStringBuilderConcatStrings(inst) || inst->IsPhi());
    if (inst->SetMarker(visited)) {
        return;
    }

    if (inst->IsMarked(maybeDeoptimized)) {
        return;
    }

    size_t numConcatStringsInputs = 0;
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        auto inputInst = inst->GetDataFlowInput(i);
        if (inputInst->IsSaveState()) {
            continue;
        }

        if (inputInst->IsPhi()) {
            MatchConcatenationExpression(inputInst, match, visited, maybeDeoptimized);
            continue;
        }
        if (!IsMethodStringBuilderConcatStrings(inputInst)) {
            continue;
        }

        numConcatStringsInputs++;

        MatchConcatenationExpression(inputInst, match, visited, maybeDeoptimized);
    }

    if (IsMethodStringBuilderConcatStrings(inst) && numConcatStringsInputs > 0) {
        match.width += numConcatStringsInputs - 1;
    }

    if (inst->IsPhi()) {
        return;
    }

    auto requireFlattening = HasUserRequireFlattening(inst);
    match.requireFlattening |= requireFlattening;
    match.usedByConcatOnly &= !HasUserOtherThanConcat(inst) || requireFlattening;
    match.usedInTryCatch |= inst->GetBasicBlock()->IsTryCatch();
    match.concatInsts.push_back(inst);

    return;
}

bool HybridStringsOptimization::HasUserRequireFlattening(Inst *inst)
{
    {
        MarkerHolder visited {GetGraph()};
        if (HasUserPhiRecursively(inst, visited.GetMarker(),
                                  [](auto &user) { return user.GetInst()->GetOpcode() == Opcode::StringFlatCheck; })) {
            return true;
        }
    }

    auto isInstRequireFlattening = [runtime = GetGraph()->GetRuntime()](Inst *userInst) {
        if (userInst->IsCall()) {
            auto callInst = static_cast<CallInst *>(userInst);
            return runtime->IsMethodRequireFlattening(callInst->GetCallMethod());
        }

        if (userInst->IsIntrinsic()) {
            auto intrinsicInst = userInst->CastToIntrinsic();
            return runtime->IsIntrinsicRequireFlattening(intrinsicInst->GetIntrinsicId());
        }

        return false;
    };

    {
        MarkerHolder visited {GetGraph()};
        if (HasUserPhiRecursively(inst, visited.GetMarker(), [&isInstRequireFlattening](auto &user) {
                auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
                return isInstRequireFlattening(userInst);
            })) {
            return true;
        }
    }

    {
        MarkerHolder visited {GetGraph()};
        if (HasUserRecursively(inst, visited.GetMarker(), [&isInstRequireFlattening](auto &user) {
                auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
                return isInstRequireFlattening(userInst);
            })) {
            return true;
        }
    }

    return false;
}

void HybridStringsOptimization::ReplaceEachWithStringBuilder(const ConcatenationMatch &match)
{
    for (auto concatInst : match.concatInsts) {
        ASSERT(IsMethodStringBuilderConcatStrings(concatInst));

        auto saveState = concatInst->GetSaveState();
        auto instance = CreateInstructionStringBuilderInstance(GetGraph(), saveState->GetPc(), saveState);
        InsertBeforeWithInputs(instance, saveState);

        auto ctor = CreateStringBuilderDefaultConstructorCall(GetGraph(), instance, saveState);
        InsertBeforeWithInputs(ctor, saveState);

        for (size_t i = 0; i < concatInst->GetInputsCount(); ++i) {
            auto inputInst = concatInst->GetInput(i).GetInst();
            if (inputInst->IsSaveState()) {
                continue;
            }

            auto append = CreateStringBuilderAppendStringIntrinsic(GetGraph(), instance, inputInst, saveState);
            InsertBeforeWithInputs(append, saveState);
            FixBrokenSaveStates(inputInst, append);
            FixBrokenSaveStates(instance, append);
        }

        auto toString = CreateStringBuilderToStringIntrinsic(GetGraph(), instance, saveState);
        InsertBeforeWithSaveState(toString, saveState);
        FixBrokenSaveStates(instance, toString);

        concatInst->ReplaceUsers(toString);
        for (auto &user : toString->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsSaveState()) {
                continue;
            }

            FixBrokenSaveStates(toString, userInst);
        }
    }
}

void HybridStringsOptimization::Cleanup(const ConcatenationMatch &match)
{
    for (auto &concatInst : match.concatInsts) {
        RemoveFromSaveStateInputs(concatInst);
        concatInst->ClearFlag(inst_flags::NO_DCE);
    }

    for (auto &concatInst : match.concatInsts) {
        for (auto &user : concatInst->GetUsers()) {
            auto userInst = user.GetInst();
            if (IsCheckCastWithoutUsers(userInst)) {
                userInst->ClearFlag(compiler::inst_flags::NO_DCE);
            }
        }
    }

    for (auto &concatInst : match.concatInsts) {
        concatInst->RemoveUsers();
    }
}

void HybridStringsOptimization::RemoveFromSaveStateInputs(Inst *inst, bool doMark)
{
    inputDescriptors_.clear();

    for (auto &user : inst->GetUsers()) {
        if (!user.GetInst()->IsSaveState()) {
            continue;
        }
        inputDescriptors_.emplace_back(user.GetInst(), user.GetIndex());
    }

    RemoveFromInstructionInputs(inputDescriptors_, doMark);
}

void HybridStringsOptimization::FixBrokenSaveStates(Inst *source, Inst *target)
{
    if (source->IsMovableObject()) {
        ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), ObjCtx {source, target});
    }
}

bool HybridStringsOptimization::ConcatenationMatch::IsValid() const
{
    if (usedInTryCatch) {
        return false;
    }

    if (requireFlattening) {
        return true;
    }

    /* Check if pattern width and number of concat-instruction matches long chain concatenation
       Single instruction within a loop considered to be valid as well

       VALID data flow example 1:

            arg0   arg1
                \ /
                (+)  arg2
                  \ /
                  (+)  arg3
                    \ /
                    (+)  arg4
                      \ /
                      (+)  arg5
                        \ /
                        (+)  arg6
                          \ /
                         result
            Data flow width: 1
            Total number of concat instructions: 5

       VALID data flow example 2:

              result
            +--- \ --------------------+
            |     \   arg[i]           |
            |      \ /                 |
            |      (+)                 |
            +------ | --- Loop over i -+
                  result
            In-loop: yes
            Data flow width: 1
            Total number of concat instructions: 1

       INVALID data flow example 3:

            arg0   arg1
                \ /
                (+)  arg2
                  \ /
                  (+) arg3  arg4
                    \     \ /
                     \    (+)
                      \   /
                       \ /
                       [+] arg5  arg6
                         \     \ /
                          \    (+)
                           \   /
                            \ /
                            [+] arg7  arg8
                              \     \ /
                               \    (+)
                                \   /
                                 \ /
                                 [+]
                                result
            Data flow width: 4 (number of concat operations with more than one input being
                                other concat operation plus one, concat operations
                                incrementing data flow width are marked with square brakets)
            Total number of concat instructions: 8
    */
    return (width <= 2U && usedByConcatOnly) &&
           (concatInsts.size() > 1 ||
            (concatInsts.size() == 1 && !concatInsts.front()->GetBasicBlock()->GetLoop()->IsRoot()));
}

}  // namespace ark::compiler
