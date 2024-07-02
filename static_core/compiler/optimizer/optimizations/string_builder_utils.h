/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_BUILDER_UTILS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_BUILDER_UTILS_H

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"

namespace ark::compiler {

bool IsStringBuilderInstance(Inst *inst);
bool IsMethodStringConcat(Inst *inst);
bool IsMethodStringBuilderConstructorWithStringArg(Inst *inst);
bool IsMethodStringBuilderConstructorWithCharArrayArg(Inst *inst);
bool IsStringBuilderToString(Inst *inst);
bool IsMethodStringBuilderDefaultConstructor(Inst *inst);
void InsertBeforeWithSaveState(Inst *inst, Inst *before);
void InsertAfterWithSaveState(Inst *inst, Inst *after);
void InsertBeforeWithInputs(Inst *inst, Inst *before);
using FindInputPredicate = std::function<bool(Input &input)>;
bool HasInput(Inst *inst, const FindInputPredicate &predicate);
bool HasInputPhiRecursively(Inst *inst, Marker visited, const FindInputPredicate &predicate);
void ResetInputMarkersRecursively(Inst *inst, Marker visited);
using FindUserPredicate = std::function<bool(User &user)>;
bool HasUser(Inst *inst, const FindUserPredicate &predicate);
bool HasUserPhiRecursively(Inst *inst, Marker visited, const FindUserPredicate &predicate);
size_t CountUsers(Inst *inst, const FindUserPredicate &predicate);
void ResetUserMarkersRecursively(Inst *inst, Marker visited);
Inst *SkipSingleUserCheckInstruction(Inst *inst);

template <bool ALLOW_INLINED = false>
bool IsStringBuilderAppend(Inst *inst)
{
    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    if (inst->GetOpcode() == Opcode::CallStatic || inst->GetOpcode() == Opcode::CallVirtual) {
        auto callInst = static_cast<CallInst *>(inst);
        return (ALLOW_INLINED || !callInst->IsInlined()) &&
               runtime->IsMethodStringBuilderAppend(callInst->GetCallMethod());
    }
    if (inst->IsIntrinsic()) {
        auto intrinsic = inst->CastToIntrinsic();
        return runtime->IsIntrinsicStringBuilderAppend(intrinsic->GetIntrinsicId());
    }
    return false;
}

bool IsIntrinsicStringBuilderAppendString(Inst *inst);
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_BUILDER_UTILS_H
