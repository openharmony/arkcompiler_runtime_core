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
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/ir/inst.h"
#include "bytecode_instruction.h"
#include "bytecode_instruction-inl.h"

namespace ark::compiler {

template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ>
void InstBuilder::BuildLaunch(const BytecodeInstruction *bcInst)
{
    if (graph_->GetArch() == Arch::AARCH32) {
        failed_ = true;
        return;
    }
    auto pc = GetPc(bcInst->GetAddress());
    auto inst = graph_->CreateInstLoadRuntimeClass(
        DataType::REFERENCE, pc, TypeIdMixin {TypeIdMixin::MEM_PROMISE_CLASS_ID, GetGraph()->GetMethod()}, nullptr);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto newObj = CreateNewObjectInst(pc, TypeIdMixin::MEM_PROMISE_CLASS_ID, saveState, inst);
    AddInstruction(saveState, inst, newObj);
    BuildCallHelper<OPCODE, IS_RANGE, ACC_READ>(bcInst, this, newObj);
    UpdateDefinitionAcc(newObj);
}

template void InstBuilder::BuildLaunch<Opcode::CallLaunchStatic, false, true>(const BytecodeInstruction *bc_inst);
template void InstBuilder::BuildLaunch<Opcode::CallLaunchStatic, true, false>(const BytecodeInstruction *bcInst);
template void InstBuilder::BuildLaunch<Opcode::CallLaunchStatic, false, false>(const BytecodeInstruction *bcInst);

template void InstBuilder::BuildLaunch<Opcode::CallLaunchVirtual, false, true>(const BytecodeInstruction *bc_inst);
template void InstBuilder::BuildLaunch<Opcode::CallLaunchVirtual, true, false>(const BytecodeInstruction *bcInst);
template void InstBuilder::BuildLaunch<Opcode::CallLaunchVirtual, false, false>(const BytecodeInstruction *bcInst);

// CC-OFFNXT(huge_method) big switch-case
void InstBuilder::BuildLdObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type)
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

std::pair<RuntimeInterface::IntrinsicId, DataType::Type> ExtractIntrinsicIdByType(DataType::Type type)
{
    switch (type) {
        case DataType::REFERENCE:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_OBJ, type};
        case DataType::FLOAT64:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F64, type};
        case DataType::FLOAT32:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F32, type};
        case DataType::UINT64:
        case DataType::INT64:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I64, DataType::INT64};
        case DataType::UINT8:
        case DataType::INT8:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I8, DataType::INT8};
        case DataType::UINT16:
        case DataType::INT16:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I16, DataType::INT16};
        case DataType::UINT32:
        case DataType::INT32:
            return {RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I32, DataType::INT32};
        default:
            UNREACHABLE();
            break;
    }
    UNREACHABLE();
}

IntrinsicInst *InstBuilder::CreateStObjByNameIntrinsic(size_t pc, compiler::DataType::Type type)
{
    auto [id, extractedType] = ExtractIntrinsicIdByType(type);
    type = extractedType;
    auto *intrinsic = GetGraph()->CreateInstIntrinsic(DataType::VOID, pc, id);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 3_I);
    intrinsic->AddInputType(DataType::REFERENCE);
    intrinsic->AddInputType(type);
    intrinsic->AddInputType(DataType::NO_TYPE);
    return intrinsic;
}

void InstBuilder::BuildStObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type)
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

    auto *intrinsic = CreateStObjByNameIntrinsic(pc, type);
    intrinsic->AppendInput(nullCheck);
    intrinsic->AppendInput(storeVal);
    intrinsic->AppendInput(saveState);
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
