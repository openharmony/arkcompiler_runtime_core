/**
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

#include "optimizer/optimizations/simplify_string_builder.h"

#include "compiler_logger.h"

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"

namespace panda::compiler {
/**
 * Replaces
 *      let s = new StringBuilder(str).toString();
 * or
 *      let sb = new StringBuilder(str);
 *      let s = sb.toString();
 * with
 *      let s = str;
 * Skips toString calls dominated by other usages of StringBuilder instance
 * Removes StringBuilder instance construction if it was used for toString calls only
 */
SimplifyStringBuilder::SimplifyStringBuilder(Graph *graph) : Optimization(graph) {}

bool SimplifyStringBuilder::RunImpl()
{
    isApplied_ = false;
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsEmpty()) {
            continue;
        }
        VisitBlock(block);
    }
    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Simplify StringBuilder complete";
    return isApplied_;
}

void SimplifyStringBuilder::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool SimplifyStringBuilder::IsMethodStringBuilderConstructorWithStringArg(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderConstructorWithStringArg(call->GetCallMethod());
}

bool SimplifyStringBuilder::IsMethodStringBuilderToString(Inst *inst)
{
    if (inst->GetOpcode() == Opcode::CallVirtual) {
        auto call = inst->CastToCallVirtual();
        if (call->IsInlined()) {
            return false;
        }
        return GetGraph()->GetRuntime()->IsMethodStringBuilderToString(call->GetCallMethod());
    }
    if (inst->IsIntrinsic()) {
        auto intrinsic = inst->CastToIntrinsic();
        return GetGraph()->GetRuntime()->IsIntrinsicStringBuilderToString(intrinsic->GetIntrinsicId());
    }
    return false;
}

InstIter SimplifyStringBuilder::SkipToStringBuilderConstructor(InstIter begin, InstIter end)
{
    return std::find_if(std::move(begin), std::move(end),
                        [this](auto inst) { return IsMethodStringBuilderConstructorWithStringArg(inst); });
}

bool IsDataFlowInput(Inst *inst, Inst *input)
{
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        if (inst->GetDataFlowInput(i) == input) {
            return true;
        }
    }
    return false;
}

bool IsUsedOutsideBasicBlock(Inst *inst, BasicBlock *bb)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsCheck()) {
            if (!userInst->HasSingleUser()) {
                // In case of multi user check-instruction we assume it is used outside current basic block without
                // actually testing it.
                return true;
            }
            // In case of signle user check-instruction we test its the only user.
            userInst = userInst->GetUsers().Front().GetInst();
        }
        if (userInst->GetBasicBlock() != bb) {
            return true;
        }
    }
    return false;
}

void SimplifyStringBuilder::VisitBlock(BasicBlock *block)
{
    ASSERT(block != nullptr);
    ASSERT(block->GetGraph() == GetGraph());

    // Walk through a basic block, find every StringBuilder instance and constructor call,
    // and check it we can remove/replace them
    InstIter inst = block->Insts().begin();
    while ((inst = SkipToStringBuilderConstructor(inst, block->Insts().end())) != block->Insts().end()) {
        ASSERT((*inst)->IsStaticCall());
        auto ctorCall = (*inst)->CastToCallStatic();

        // void StringBuilder::<ctor> instance, arg, save_state
        ASSERT(ctorCall->GetInputsCount() == CONSTRUCTOR_WITH_STRING_ARG_TOTAL_ARGS_NUM);
        auto instance = ctorCall->GetInput(0).GetInst();
        auto arg = ctorCall->GetInput(1).GetInst();

        // Look for StringBuilder usages within current basic block
        auto nextInst = block->Insts().end();
        bool removeCtor = true;
        for (++inst; inst != block->Insts().end(); ++inst) {
            // Skip SaveState instructions
            if ((*inst)->IsSaveState()) {
                continue;
            }

            // Skip check instructions, like NullCheck, RefTypeCheck, etc.
            if ((*inst)->IsCheck()) {
                continue;
            }

            // Continue (outer loop) with the next StringBuilder constructor,
            // in case we met one in inner loop
            if (IsMethodStringBuilderConstructorWithStringArg(*inst)) {
                nextInst = nextInst != block->Insts().end() ? nextInst : inst;
            }

            if (!IsDataFlowInput(*inst, instance)) {
                continue;
            }

            // Process usages of StringBuilder instance:
            // replace toString()-calls until we met something else
            if (IsMethodStringBuilderToString(*inst)) {
                (*inst)->ReplaceUsers(arg);
                (*inst)->ClearFlag(compiler::inst_flags::NO_DCE);
                COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                    << "Remove StringBuilder toString()-call (id=" << (*inst)->GetId() << ")";
                isApplied_ = true;
            } else {
                removeCtor = false;
                break;
            }
        }

        // Remove StringBuilder constructor unless it has usages
        if (removeCtor && !IsUsedOutsideBasicBlock(instance, instance->GetBasicBlock())) {
            ctorCall->ClearFlag(compiler::inst_flags::NO_DCE);
            COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Remove StringBuilder constructor (id=" << ctorCall->GetId() << ")";
            isApplied_ = true;
        }

        // Proceed to the next StringBuilder constructor
        inst = nextInst != block->Insts().end() ? nextInst : inst;
    }
}

}  // namespace panda::compiler
