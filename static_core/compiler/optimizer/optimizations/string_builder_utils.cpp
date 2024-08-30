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

#include "string_builder_utils.h"

namespace ark::compiler {

bool IsStringBuilderInstance(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::NewObject) {
        return false;
    }

    auto klass = GetObjectClass(inst->CastToNewObject());
    if (klass == nullptr) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsClassStringBuilder(klass);
}

bool IsMethodStringConcat(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic && inst->GetOpcode() != Opcode::CallVirtual) {
        return false;
    }

    auto call = static_cast<CallInst *>(inst);
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringConcat(call->GetCallMethod());
}

bool IsMethodStringBuilderConstructorWithStringArg(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderConstructorWithStringArg(call->GetCallMethod());
}

bool IsMethodStringBuilderConstructorWithCharArrayArg(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderConstructorWithCharArrayArg(call->GetCallMethod());
}

bool IsStringBuilderToString(Inst *inst)
{
    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    if (inst->GetOpcode() == Opcode::CallStatic || inst->GetOpcode() == Opcode::CallVirtual) {
        auto callInst = static_cast<CallInst *>(inst);
        return !callInst->IsInlined() && runtime->IsMethodStringBuilderToString(callInst->GetCallMethod());
    }
    if (inst->IsIntrinsic()) {
        auto intrinsic = inst->CastToIntrinsic();
        return runtime->IsIntrinsicStringBuilderToString(intrinsic->GetIntrinsicId());
    }
    return false;
}

bool IsMethodStringBuilderDefaultConstructor(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::CallStatic) {
        return false;
    }

    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsMethodStringBuilderDefaultConstructor(call->GetCallMethod());
}

void InsertBeforeWithSaveState(Inst *inst, Inst *before)
{
    if (inst->RequireState()) {
        before->InsertBefore(inst->GetSaveState());
    }
    before->InsertBefore(inst);
}

void InsertAfterWithSaveState(Inst *inst, Inst *after)
{
    after->InsertAfter(inst);
    if (inst->RequireState()) {
        after->InsertAfter(inst->GetSaveState());
    }
}

void InsertBeforeWithInputs(Inst *inst, Inst *before)
{
    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->GetBasicBlock() == nullptr) {
            InsertBeforeWithInputs(inputInst, before);
        }
    }

    if (inst->GetBasicBlock() == nullptr) {
        before->InsertBefore(inst);
    }
}

bool HasInput(Inst *inst, const FindInputPredicate &predicate)
{
    // Check if any instruction input satisfy predicate

    auto found = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(), predicate);
    return found != inst->GetInputs().end();
}

bool HasInputPhiRecursively(Inst *inst, Marker visited, const FindInputPredicate &predicate)
{
    // Check if any instruction input satisfy predicate
    // All Phi-instruction inputs are checked recursively

    if (HasInput(inst, predicate)) {
        return true;
    }

    inst->SetMarker(visited);

    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (!inputInst->IsPhi()) {
            continue;
        }
        if (inputInst->IsMarked(visited)) {
            continue;
        }
        if (HasInputPhiRecursively(inputInst, visited, predicate)) {
            return true;
        }
    }

    return false;
}

void ResetInputMarkersRecursively(Inst *inst, Marker visited)
{
    // Reset marker for an instruction and all it's inputs recursively

    if (inst->IsMarked(visited)) {
        inst->ResetMarker(visited);

        for (auto &input : inst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (inputInst->IsMarked(visited)) {
                ResetInputMarkersRecursively(inputInst, visited);
            }
        }
    }
}

bool HasUser(Inst *inst, const FindUserPredicate &predicate)
{
    // Check if instruction is used in a context defined by predicate

    auto found = std::find_if(inst->GetUsers().begin(), inst->GetUsers().end(), predicate);
    return found != inst->GetUsers().end();
}

bool HasUserPhiRecursively(Inst *inst, Marker visited, const FindUserPredicate &predicate)
{
    // Check if instruction is used in a context defined by predicate
    // All Phi-instruction users are checked recursively

    if (HasUser(inst, predicate)) {
        return true;
    }

    inst->SetMarker(visited);

    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (!userInst->IsPhi()) {
            continue;
        }
        if (userInst->IsMarked(visited)) {
            continue;
        }
        if (HasUserPhiRecursively(userInst, visited, predicate)) {
            return true;
        }
    }

    return false;
}

size_t CountUsers(Inst *inst, const FindUserPredicate &predicate)
{
    size_t count = 0;
    for (auto &user : inst->GetUsers()) {
        if (predicate(user)) {
            ++count;
        }

        auto userInst = user.GetInst();
        if (userInst->IsCheck()) {
            count += CountUsers(userInst, predicate);
        }
    }

    return count;
}

void ResetUserMarkersRecursively(Inst *inst, Marker visited)
{
    // Reset marker for an instruction and all it's users recursively

    if (inst->IsMarked(visited)) {
        inst->ResetMarker(visited);

        for (auto &user : inst->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsMarked(visited)) {
                ResetUserMarkersRecursively(userInst, visited);
            }
        }
    }
}

Inst *SkipSingleUserCheckInstruction(Inst *inst)
{
    if (inst->IsCheck() && inst->HasSingleUser()) {
        inst = inst->GetUsers().Front().GetInst();
    }
    return inst;
}

bool IsIntrinsicStringBuilderAppendString(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }

    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    return runtime->IsIntrinsicStringBuilderAppendString(inst->CastToIntrinsic()->GetIntrinsicId());
}

}  // namespace ark::compiler
