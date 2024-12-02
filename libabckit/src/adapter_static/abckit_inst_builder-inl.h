/*
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

#ifndef ABCKIT_INST_BUILDER_INL_H
#define ABCKIT_INST_BUILDER_INL_H

#include "abckit_inst_builder.h"
#include <inst_builder_abckit_intrinsics.inc>

namespace ark::compiler {

template <bool IS_ACC_WRITE>
void AbcKitInstBuilder::AbcKitBuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitLoadObjectIntrinsic<IS_ACC_WRITE>(bcInst, type);
}

template <bool IS_ACC_READ>
void AbcKitInstBuilder::AbcKitBuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitStoreObjectIntrinsic<IS_ACC_READ>(bcInst, type);
}

void AbcKitInstBuilder::BuildIsUndefined(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_IS_UNDEFINED);
}

void AbcKitInstBuilder::BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitLoadStaticIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitStoreStaticIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitLoadArrayIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildLoadConstArray(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_CONST_ARRAY);
}

void AbcKitInstBuilder::BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildAbcKitStoreArrayIntrinsic(bcInst, type);
}

void AbcKitInstBuilder::BuildNewArray(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_NEW_ARRAY);
}

void AbcKitInstBuilder::BuildNewObject(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_NEW_OBJECT);
}

void AbcKitInstBuilder::BuildInitObject(const BytecodeInstruction *bcInst, bool isRange)
{
    BuildAbcKitInitObjectIntrinsic(bcInst, isRange);
}

void AbcKitInstBuilder::BuildCheckCast(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_CHECK_CAST);
}

void AbcKitInstBuilder::BuildIsInstance(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_IS_INSTANCE);
}

void AbcKitInstBuilder::BuildThrow(const BytecodeInstruction *bcInst)
{
    BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_THROW);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
void AbcKitInstBuilder::AbcKitBuildLoadFromPool(const BytecodeInstruction *bcInst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t typeId;
    Inst *inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (OPCODE == Opcode::LoadType) {
        auto typeIndex = bcInst->GetId(0).AsIndex();
        typeId = GetRuntime()->ResolveTypeIndex(method, typeIndex);
        if (GetRuntime()->ResolveType(method, typeId) == nullptr) {
            inst = GetGraph()->CreateInstUnresolvedLoadType(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
            if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
                GetRuntime()->GetUnresolvedTypes()->AddTableSlot(method, typeId,
                                                                 UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
            }
        } else {
            inst = GetGraph()->CreateInstLoadType(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers)
        static_assert(OPCODE == Opcode::LoadString);
        BuildDefaultAbcKitIntrinsic(bcInst, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING);
        return;
    }
    if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
        // Create SaveState instruction
        auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        inst->SetInput(0, saveState);
        static_cast<LoadFromPool *>(inst)->SetTypeId(typeId);
        static_cast<LoadFromPool *>(inst)->SetMethod(method);
        AddInstruction(saveState);
    } else {
        inst->SetInput(0, GetEnvDefinition(CONST_POOL_IDX));
        inst->CastToLoadFromConstantPool()->SetTypeId(typeId);
        inst->CastToLoadFromConstantPool()->SetMethod(method);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::LoadString) {
        if (GetGraph()->IsDynamicMethod() && GetGraph()->IsBytecodeOptimizer()) {
            BuildCastToAnyString(bcInst);
        }
    }
}

}  // namespace ark::compiler

#endif  // ABCKIT_INST_BUILDER_INL_H
