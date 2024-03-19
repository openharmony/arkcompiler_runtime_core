/**
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

#include <cstdint>
#include "libpandabase/utils/utils.h"
#include "compiler_logger.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/ir/inst.h"
#include "bytecode_instruction.h"
#include "bytecode_instruction-inl.h"

namespace ark::compiler {

template <Opcode OPCODE>
void InstBuilder::BuildLaunch(const BytecodeInstruction *bcInst, bool isRange, bool accRead)
{
    if (graph_->GetArch() == Arch::AARCH32) {
        failed_ = true;
        return;
    }
    auto pc = GetPc(bcInst->GetAddress());
    auto inst = graph_->CreateInstLoadRuntimeClass(DataType::REFERENCE, pc, TypeIdMixin::MEM_PROMISE_CLASS_ID,
                                                   GetGraph()->GetMethod(), nullptr);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto newObj = CreateNewObjectInst(pc, TypeIdMixin::MEM_PROMISE_CLASS_ID, saveState, inst);
    AddInstruction(saveState, inst, newObj);
    BuildCall<OPCODE>(bcInst, isRange, accRead, newObj);
    UpdateDefinitionAcc(newObj);
}

template void InstBuilder::BuildLaunch<Opcode::CallLaunchStatic>(const BytecodeInstruction *bc_inst, bool is_range,
                                                                 bool acc_read);
template void InstBuilder::BuildLaunch<Opcode::CallLaunchVirtual>(const BytecodeInstruction *bc_inst, bool is_range,
                                                                  bool acc_read);

void InstBuilder::BuildLdObjByName(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto pc = GetPc(bcInst->GetAddress());
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bcInst->GetVReg(0)), saveState);

    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    RuntimeInterface::IntrinsicId id;
    switch (type) {
        case DataType::REFERENCE:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_OBJ;
            break;
        case DataType::FLOAT64:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_F64;
            break;
        case DataType::FLOAT32:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_F32;
            break;
        case DataType::UINT64:
        case DataType::INT64:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_I64;
            break;
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_I32;
            break;
        default:
            UNREACHABLE();
            break;
    }
    auto intrinsic = GetGraph()->CreateInstIntrinsic(type, pc, id);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);

    intrinsic->AppendInput(nullCheck);
    intrinsic->AddInputType(DataType::REFERENCE);

    intrinsic->AppendInput(saveState);
    intrinsic->AddInputType(DataType::NO_TYPE);

    intrinsic->AddImm(GetGraph()->GetAllocator(), fieldId);
    intrinsic->AddImm(GetGraph()->GetAllocator(), pc);

    intrinsic->SetMethodFirstInput();
    intrinsic->SetMethod(GetMethod());

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(intrinsic);

    UpdateDefinitionAcc(intrinsic);
}

void InstBuilder::BuildStObjByName(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto pc = GetPc(bcInst->GetAddress());
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bcInst->GetVReg(0)), saveState);

    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    // Get a value to store
    Inst *storeVal = nullptr;
    storeVal = GetDefinitionAcc();

    RuntimeInterface::IntrinsicId id;
    switch (type) {
        case DataType::REFERENCE:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_OBJ;
            break;
        case DataType::FLOAT64:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F64;
            break;
        case DataType::FLOAT32:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F32;
            break;
        case DataType::UINT64:
        case DataType::INT64:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I64;
            type = DataType::INT64;
            break;
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I32;
            type = DataType::INT32;
            break;
        default:
            UNREACHABLE();
            break;
    }
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::VOID, pc, id);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 3_I);

    intrinsic->AppendInput(nullCheck);
    intrinsic->AddInputType(DataType::REFERENCE);

    intrinsic->AppendInput(storeVal);
    intrinsic->AddInputType(type);

    intrinsic->AppendInput(saveState);
    intrinsic->AddInputType(DataType::NO_TYPE);
    intrinsic->AddImm(GetGraph()->GetAllocator(), fieldId);
    intrinsic->AddImm(GetGraph()->GetAllocator(), pc);

    intrinsic->SetMethodFirstInput();
    intrinsic->SetMethod(GetMethod());

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(intrinsic);
}

void InstBuilder::BuildEquals(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());

    Inst *obj1 = GetDefinition(bcInst->GetVReg(0));
    Inst *obj2 = GetDefinition(bcInst->GetVReg(1));

    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc,
                                                     RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_EQUALS);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);

    intrinsic->AppendInput(obj1);
    intrinsic->AddInputType(DataType::REFERENCE);
    intrinsic->AppendInput(obj2);
    intrinsic->AddInputType(DataType::REFERENCE);

    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

}  // namespace ark::compiler
