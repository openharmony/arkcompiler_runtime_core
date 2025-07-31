/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "bytecodeopt_peepholes.h"

#include "libpandafile/bytecode_instruction-inl.h"

namespace ark::bytecodeopt {
bool BytecodeOptPeepholes::RunImpl()
{
    VisitGraph();
    return IsApplied();
}

CallInst *FindCtorCall(Inst *newObject)
{
    auto *graph = newObject->GetBasicBlock()->GetGraph();
    auto *adapter = graph->GetRuntime();
    for (auto &user : newObject->GetUsers()) {
        auto *inst = user.GetInst();
        if (inst->GetOpcode() == Opcode::NullCheck) {
            return FindCtorCall(inst);
        }
        if (inst->GetOpcode() != Opcode::CallStatic) {
            continue;
        }
        auto call = inst->CastToCallStatic();
        auto callFirstArg = call->GetInput(0U).GetInst();
        if (callFirstArg->GetOpcode() == Opcode::NewObject) {
            auto newObjectAsArg = callFirstArg->CastToNewObject();
            if (newObjectAsArg != newObject) {
                continue;
            }
        }
        if (adapter->IsConstructor(call->GetCallMethod(), graph->GetLanguage())) {
            return call;
        }
    }

    return nullptr;
}

CallInst *CreateInitObject(compiler::GraphVisitor *v, compiler::ClassInst *load, const CallInst *callInit)
{
    auto *graph = static_cast<BytecodeOptPeepholes *>(v)->GetGraph();
    auto *initObject = static_cast<CallInst *>(graph->CreateInst(compiler::Opcode::InitObject));
    initObject->SetType(compiler::DataType::REFERENCE);

    auto inputTypesCount = callInit->GetInputsCount();
    initObject->AllocateInputTypes(graph->GetAllocator(), inputTypesCount);
    initObject->AppendInput(load, compiler::DataType::REFERENCE);
    for (size_t i = 1; i < inputTypesCount; ++i) {
        auto inputInst = callInit->GetInput(i).GetInst();
        initObject->AppendInput(inputInst, inputInst->GetType());
    }

    initObject->SetCallMethodId(callInit->GetCallMethodId());
    initObject->SetCallMethod(static_cast<const CallInst *>(callInit)->GetCallMethod());
    return initObject;
}

void ReplaceNewObjectUsers(Inst *newObject, Inst *nullCheck, CallInst *initObject)
{
    for (auto it = newObject->GetUsers().begin(); it != newObject->GetUsers().end();
         it = newObject->GetUsers().begin()) {
        auto user = it->GetInst();
        if (user != nullCheck) {
            user->SetInput(it->GetIndex(), initObject);
        } else {
            newObject->RemoveUser(&(*it));
        }
    }

    // Update throwable instructions data
    auto graph = newObject->GetBasicBlock()->GetGraph();
    if (graph->IsInstThrowable(newObject)) {
        graph->ReplaceThrowableInst(newObject, initObject);
    }
}

void BytecodeOptPeepholes::VisitNewObject(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst != nullptr);
    CallInst *callInit = FindCtorCall(inst);
    if (callInit == nullptr || inst->GetBasicBlock() != callInit->GetBasicBlock()) {
        return;
    }

    // The optimization is correct only if there are no side-effects between NewObject and constructor call.
    // For simplicity, we abort it if any instruction except NullCheck and SaveState appears in-between.
    // Moreover, when we are inside a try block, local register state also matters, because it may be used inside
    // catch blocks. In such case we also abort if there are any instructions in corresponding bytecode.
    const auto graph = static_cast<BytecodeOptPeepholes *>(v)->GetGraph();
    const size_t newobjSize =
        BytecodeInstruction::Size(  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            BytecodeInstruction(graph->GetRuntime()->GetMethodCode(graph->GetMethod()) + inst->GetPc()).GetFormat());
    if (inst->GetBasicBlock()->IsTry() && callInit->GetPc() - inst->GetPc() > newobjSize) {
        return;
    }

    Inst *nullCheck = nullptr;
    for (auto *i = inst->GetNext(); i != callInit; i = i->GetNext()) {
        if (i->GetOpcode() != Opcode::SaveState && i->GetOpcode() != Opcode::NullCheck) {
            return;
        }

        if (i->GetOpcode() == Opcode::SaveState) {
            continue;
        }

        for (auto nullCheckInput : i->GetInputs()) {
            if (nullCheckInput.GetInst() == inst) {
                ASSERT(nullCheck == nullptr);
                nullCheck = i;
            }
        }
        if (nullCheck == nullptr) {
            return;
        }
    }

    auto load = static_cast<compiler::ClassInst *>(inst->GetInput(0U).GetInst());
    auto *initObject = CreateInitObject(v, load, callInit);
    callInit->InsertBefore(initObject);
    initObject->SetPc(callInit->GetPc());

    ReplaceNewObjectUsers(inst, nullCheck, initObject);
    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    if (nullCheck != nullptr) {
        nullCheck->ReplaceUsers(initObject);
        nullCheck->ClearFlag(compiler::inst_flags::NO_DCE);
        nullCheck->RemoveInputs();
        nullCheck->GetBasicBlock()->ReplaceInst(nullCheck,
                                                static_cast<BytecodeOptPeepholes *>(v)->GetGraph()->CreateInstNOP());
    }
    ASSERT(!callInit->HasUsers());
    callInit->ClearFlag(compiler::inst_flags::NO_DCE);

    static_cast<BytecodeOptPeepholes *>(v)->SetIsApplied();
}

}  // namespace ark::bytecodeopt
