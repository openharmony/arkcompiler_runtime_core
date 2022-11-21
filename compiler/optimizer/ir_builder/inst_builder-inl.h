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

#ifndef PANDA_INST_BUILDER_INL_H
#define PANDA_INST_BUILDER_INL_H

#include "inst_builder.h"

namespace panda::compiler {
// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildAnyTypeCheckInst(size_t bc_addr, Inst *input, Inst *save_state, AnyBaseType type)
{
    auto any_check = graph_->CreateInstAnyTypeCheck(DataType::ANY, bc_addr);
    any_check->SetInput(0, input);
    any_check->SetInput(1, save_state);
    any_check->SetAnyType(type);
    AddInstruction(any_check);

    return any_check;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode opcode>
void InstBuilder::BuildLoadFromPool(const BytecodeInstruction *bc_inst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t type_id;
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    LoadFromPool *inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (opcode == Opcode::LoadType) {
        auto type_index = bc_inst->GetId(0).AsIndex();
        type_id = GetRuntime()->ResolveTypeIndex(method, type_index);
        if (GetRuntime()->ResolveType(method, type_id) == nullptr) {
            inst = GetGraph()->CreateInstUnresolvedLoadType(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
            if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
                GetRuntime()->GetUnresolvedTypes()->AddTableSlot(method, type_id,
                                                                 UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
            }
        } else {
            inst = GetGraph()->CreateInstLoadType(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers)
        static_assert(opcode == Opcode::LoadString);
        type_id = GetRuntime()->ResolveOffsetByIndex(GetGraph()->GetMethod(), bc_inst->GetId(0).AsIndex());
        inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    }
    inst->SetTypeId(type_id);
    inst->SetMethod(method);
    inst->SetInput(0, save_state);

    AddInstruction(save_state);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (opcode == Opcode::LoadString) {
        if (GetGraph()->IsDynamicMethod()) {
            BuildCastToAnyString(bc_inst);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyString(const BytecodeInstruction *bc_inst)
{
    auto input = GetDefinitionAcc();
    ASSERT(input->GetType() == DataType::REFERENCE);

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto any_type = GetAnyStringType(language);
    ASSERT(any_type != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()));
    box->SetAnyType(any_type);
    box->SetInput(0, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyNumber(const BytecodeInstruction *bc_inst)
{
    auto input = GetDefinitionAcc();
    auto type = input->GetType();

    if (input->IsConst() && !DataType::IsFloatType(type)) {
        auto const_insn = input->CastToConstant();
        if (const_insn->GetType() == DataType::INT64) {
            auto value = input->CastToConstant()->GetInt64Value();
            if (value == static_cast<uint32_t>(value)) {
                type = DataType::INT32;
            }
        }
    }

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto any_type = NumericDataTypeToAnyType(type, language);
    ASSERT(any_type != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()));
    box->SetAnyType(any_type);
    box->SetInput(0, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

}  // namespace panda::compiler

#endif  // PANDA_INST_BUILDER_INL_H
