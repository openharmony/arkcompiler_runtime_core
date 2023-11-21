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

#include <cstdint>
#include "compiler_logger.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/ir/inst.h"
#include "bytecode_instruction.h"
#include "bytecode_instruction-inl.h"

namespace panda::compiler {

template <Opcode OPCODE>
void InstBuilder::BuildLaunch(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    if (graph_->GetArch() == Arch::AARCH32) {
        failed_ = true;
        return;
    }
    auto pc = GetPc(bc_inst->GetAddress());
    auto inst = graph_->CreateInstLoadRuntimeClass(DataType::REFERENCE, pc, TypeIdMixin::MEM_PROMISE_CLASS_ID,
                                                   GetGraph()->GetMethod(), nullptr);
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    auto new_obj = CreateNewObjectInst(pc, TypeIdMixin::MEM_PROMISE_CLASS_ID, save_state, inst);
    AddInstruction(save_state, inst, new_obj);
    BuildCall<OPCODE>(bc_inst, is_range, acc_read, new_obj);
    UpdateDefinitionAcc(new_obj);
}

template void InstBuilder::BuildLaunch<Opcode::CallLaunchStatic>(const BytecodeInstruction *bc_inst, bool is_range,
                                                                 bool acc_read);
template void InstBuilder::BuildLaunch<Opcode::CallLaunchVirtual>(const BytecodeInstruction *bc_inst, bool is_range,
                                                                  bool acc_read);

void InstBuilder::BuildLdObjByName(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    auto pc = GetPc(bc_inst->GetAddress());
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    auto null_check =
        graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bc_inst->GetVReg(0)), save_state);

    auto runtime = GetRuntime();
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = runtime->ResolveFieldIndex(GetMethod(), field_index);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), field_id);
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
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2);

    intrinsic->AppendInput(null_check);
    intrinsic->AddInputType(DataType::REFERENCE);

    intrinsic->AppendInput(save_state);
    intrinsic->AddInputType(DataType::NO_TYPE);

    intrinsic->AddImm(GetGraph()->GetAllocator(), field_id);
    intrinsic->AddImm(GetGraph()->GetAllocator(), pc);

    intrinsic->SetMethodFirstInput();
    intrinsic->SetMethod(GetMethod());

    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(intrinsic);

    UpdateDefinitionAcc(intrinsic);
}

void InstBuilder::BuildStObjByName(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    auto pc = GetPc(bc_inst->GetAddress());
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    auto null_check =
        graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bc_inst->GetVReg(0)), save_state);

    auto runtime = GetRuntime();
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = runtime->ResolveFieldIndex(GetMethod(), field_index);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), field_id);
    }

    // Get a value to store
    Inst *store_val = nullptr;
    store_val = GetDefinitionAcc();

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
            break;
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I32;
            break;
        default:
            UNREACHABLE();
            break;
    }
    auto intrinsic = GetGraph()->CreateInstIntrinsic(type, pc, id);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 3);

    intrinsic->AppendInput(null_check);
    intrinsic->AddInputType(DataType::REFERENCE);

    intrinsic->AppendInput(store_val);
    intrinsic->AddInputType(type);

    intrinsic->AppendInput(save_state);
    intrinsic->AddInputType(DataType::NO_TYPE);
    intrinsic->AddImm(GetGraph()->GetAllocator(), field_id);
    intrinsic->AddImm(GetGraph()->GetAllocator(), pc);

    intrinsic->SetMethodFirstInput();
    intrinsic->SetMethod(GetMethod());

    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(intrinsic);
}

template <bool IS_ACC_WRITE>
void InstBuilder::BuildLdundefined(const BytecodeInstruction *bc_inst)
{
    auto const pc = GetPc(bc_inst->GetAddress());
    auto const intrin_id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LDUNDEFINED;

    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc, intrin_id);

    AddInstruction(intrinsic);

    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_WRITE) {
        UpdateDefinitionAcc(intrinsic);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        UpdateDefinition(bc_inst->GetVReg(0), intrinsic);
    }
}
template void InstBuilder::BuildLdundefined<false>(const BytecodeInstruction *bc_inst);
template void InstBuilder::BuildLdundefined<true>(const BytecodeInstruction *bc_inst);

void InstBuilder::BuildIsundefined(const BytecodeInstruction *bc_inst)
{
    auto const pc = GetPc(bc_inst->GetAddress());
    auto const intrin_id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LDUNDEFINED;

    auto undef_inst = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc, intrin_id);

    auto cmp_inst = GetGraph()->CreateInstCompare(DataType::BOOL, pc, GetDefinitionAcc(), undef_inst,
                                                  DataType::REFERENCE, ConditionCode::CC_EQ);

    AddInstruction(undef_inst);
    AddInstruction(cmp_inst);
    UpdateDefinitionAcc(cmp_inst);
}

}  // namespace panda::compiler
