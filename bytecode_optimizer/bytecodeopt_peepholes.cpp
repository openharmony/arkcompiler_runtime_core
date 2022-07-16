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

#include "bytecodeopt_peepholes.h"

#include "libpandafile/bytecode_instruction-inl.h"

namespace panda::bytecodeopt {
bool BytecodeOptPeepholes::RunImpl()
{
    VisitGraph();
    return IsApplied();
}

CallInst *FindCtorCall(Inst *new_object, const compiler::ClassInst *load)
{
    auto *adapter = new_object->GetBasicBlock()->GetGraph()->GetRuntime();
    for (auto &user : new_object->GetUsers()) {
        auto *inst = user.GetInst();
        if (inst->GetOpcode() == Opcode::NullCheck) {
            return FindCtorCall(inst, load);
        }
        if (inst->GetOpcode() != Opcode::CallStatic) {
            continue;
        }
        auto call = inst->CastToCallStatic();
        if (adapter->IsConstructor(call->GetCallMethod(), load->GetTypeId())) {
            return call;
        }
    }

    return nullptr;
}

CallInst *CreateInitObject(compiler::GraphVisitor *v, compiler::ClassInst *load, const CallInst *call_init)
{
    auto *graph = static_cast<BytecodeOptPeepholes *>(v)->GetGraph();
    auto *init_object = static_cast<CallInst *>(graph->CreateInst(compiler::Opcode::InitObject));
    init_object->SetType(compiler::DataType::REFERENCE);

    auto input_types_count = call_init->GetInputsCount();
    init_object->AllocateInputTypes(graph->GetAllocator(), input_types_count);
    init_object->AddInputType(compiler::DataType::REFERENCE);
    init_object->AppendInput(load);
    for (size_t i = 1; i < input_types_count; ++i) {
        auto input_inst = call_init->GetInput(i).GetInst();
        init_object->AddInputType(input_inst->GetType());
        init_object->AppendInput(input_inst);
    }

    init_object->SetCallMethodId(call_init->GetCallMethodId());
    init_object->SetCallMethod(static_cast<const CallInst *>(call_init)->GetCallMethod());
    return init_object;
}

void ReplaceNewObjectUsers(Inst *new_object, Inst *null_check, CallInst *init_object)
{
    for (auto it = new_object->GetUsers().begin(); it != new_object->GetUsers().end();
         it = new_object->GetUsers().begin()) {
        auto user = it->GetInst();
        if (user != null_check) {
            user->SetInput(it->GetIndex(), init_object);
        } else {
            new_object->RemoveUser(&(*it));
        }
    }

    // Update throwable instructions data
    auto graph = new_object->GetBasicBlock()->GetGraph();
    if (graph->IsInstThrowable(new_object)) {
        graph->ReplaceThrowableInst(new_object, init_object);
    }
}

void BytecodeOptPeepholes::VisitNewObject(GraphVisitor *v, Inst *inst)
{
    auto load = static_cast<compiler::ClassInst *>(inst->GetInput(0).GetInst());
    ASSERT(load != nullptr);

    CallInst *call_init = FindCtorCall(inst, load);
    if (call_init == nullptr) {
        return;
    }

    if (inst->GetBasicBlock() != call_init->GetBasicBlock()) {
        return;
    }

    // The optimization is correct only if there are no side-effects between NewObject and constructor call.
    // For simplicity, we abort it if any instruction except NullCheck and SaveState appears in-between.
    // Moreover, when we are inside a try block, local register state also matters, because it may be used inside
    // catch blocks. In such case we also abort if there are any instructions in corresponding bytecode.
    const auto graph = static_cast<BytecodeOptPeepholes *>(v)->GetGraph();
    const size_t NEWOBJ_SIZE =
        BytecodeInstruction::Size(  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            BytecodeInstruction(graph->GetRuntime()->GetMethodCode(graph->GetMethod()) + inst->GetPc()).GetFormat());
    if (inst->GetBasicBlock()->IsTry() && call_init->GetPc() - inst->GetPc() > NEWOBJ_SIZE) {
        return;
    }

    Inst *null_check = nullptr;
    for (auto *i = inst->GetNext(); i != call_init; i = i->GetNext()) {
        if (i->GetOpcode() != Opcode::SaveState && i->GetOpcode() != Opcode::NullCheck) {
            return;
        }

        if (i->GetOpcode() == Opcode::SaveState) {
            continue;
        }

        for (auto null_check_input : i->GetInputs()) {
            if (null_check_input.GetInst() == inst) {
                ASSERT(null_check == nullptr);
                null_check = i;
            }
        }
        if (null_check == nullptr) {
            return;
        }
    }

    auto *init_object = CreateInitObject(v, load, call_init);
    call_init->InsertBefore(init_object);
    init_object->SetPc(call_init->GetPc());

    ReplaceNewObjectUsers(inst, null_check, init_object);
    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    if (null_check != nullptr) {
        null_check->ReplaceUsers(init_object);
        null_check->ClearFlag(compiler::inst_flags::NO_DCE);
        null_check->RemoveInputs();
        null_check->GetBasicBlock()->ReplaceInst(null_check,
                                                 static_cast<BytecodeOptPeepholes *>(v)->GetGraph()->CreateInstNOP());
    }
    ASSERT(!call_init->HasUsers());
    call_init->ClearFlag(compiler::inst_flags::NO_DCE);

    static_cast<BytecodeOptPeepholes *>(v)->SetIsApplied();
}

}  // namespace panda::bytecodeopt
