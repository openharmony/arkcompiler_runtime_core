/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "libarkbase/utils/utils.h"
#include "compiler_logger.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/ir/inst.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/bytecode_instruction-inl.h"

namespace ark::compiler {
namespace {
void PlaceAsyncContextInput(SaveStateInst *saveStateSuspend, Inst *asyncContext)
{
    size_t actualAsyncContextInputIdx = saveStateSuspend->GetInputsCount();
    for (size_t idx = 0; idx < saveStateSuspend->GetInputsCount(); ++idx) {
        if (saveStateSuspend->GetInput(idx).GetInst() == asyncContext) {
            actualAsyncContextInputIdx = idx;
            break;
        }
    }

    ASSERT(actualAsyncContextInputIdx < saveStateSuspend->GetInputsCount());
    auto expectedAsyncContextInputIdx = saveStateSuspend->GetAsyncContextIndex();
    if (actualAsyncContextInputIdx == expectedAsyncContextInputIdx) {
        return;
    }

    auto expectedInput = saveStateSuspend->GetInput(expectedAsyncContextInputIdx).GetInst();
    auto expectedVreg = saveStateSuspend->GetVirtualRegister(expectedAsyncContextInputIdx);
    auto actualAsyncContextVreg = saveStateSuspend->GetVirtualRegister(actualAsyncContextInputIdx);

    saveStateSuspend->SetInput(expectedAsyncContextInputIdx, asyncContext);
    saveStateSuspend->SetInput(actualAsyncContextInputIdx, expectedInput);
    saveStateSuspend->SetVirtualRegister(expectedAsyncContextInputIdx, actualAsyncContextVreg);
    saveStateSuspend->SetVirtualRegister(actualAsyncContextInputIdx, expectedVreg);
}
}  // namespace

void InstBuilder::BuildIsNullValue(const BytecodeInstruction *bcInst)
{
    auto uniqueObjInst = graph_->GetOrCreateUniqueObjectInst();
    auto cmpInst = graph_->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), GetDefinitionAcc(),
                                             uniqueObjInst, DataType::REFERENCE, ConditionCode::CC_EQ);
    AddInstruction(cmpInst);
    UpdateDefinitionAcc(cmpInst);
}

template <bool IS_STRICT>
void InstBuilder::BuildEquals(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());

    Inst *obj1 = GetDefinition(bcInst->GetVReg(0));
    Inst *obj2 = GetDefinition(bcInst->GetVReg(1));

    auto intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_EQUALS;
    if constexpr (IS_STRICT) {
        intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_STRICT_EQUALS;
    }
#if defined(ENABLE_LIBABCKIT)
    if (GetGraph()->IsAbcKit()) {
        if constexpr (IS_STRICT) {
            intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_STRICT_EQUALS;
        } else {
            intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_EQUALS;
        }
    }
#else
#endif
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc, intrinsicId);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), intrinsic->RequireState() ? 3_I : 2_I);

    intrinsic->AppendInput(obj1);
    intrinsic->AddInputType(DataType::REFERENCE);
    intrinsic->AppendInput(obj2);
    intrinsic->AddInputType(DataType::REFERENCE);

    if (intrinsic->RequireState()) {
        // Create SaveState instruction
        auto *saveState = CreateSaveState(Opcode::SaveState, pc);
        intrinsic->AppendInput(saveState);
        intrinsic->AddInputType(DataType::NO_TYPE);
        AddInstruction(saveState);
    }

    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

template void InstBuilder::BuildEquals<true>(const BytecodeInstruction *bcInst);
template void InstBuilder::BuildEquals<false>(const BytecodeInstruction *bcInst);

void InstBuilder::BuildTypeof(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinition(bcInst->GetVReg(0));

    RuntimeInterface::IntrinsicId intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_TYPEOF;
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc, intrinsicId);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
    intrinsic->AppendInput(obj);
    intrinsic->AddInputType(DataType::REFERENCE);
    intrinsic->AppendInput(saveState);
    intrinsic->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

void InstBuilder::BuildIstrue(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinition(bcInst->GetVReg(0));

    RuntimeInterface::IntrinsicId intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ISTRUE;
    auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc, intrinsicId);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 1_I);
    intrinsic->AppendInput(obj);
    intrinsic->AddInputType(DataType::REFERENCE);

    AddInstruction(intrinsic);
    UpdateDefinitionAcc(intrinsic);
}

void InstBuilder::BuildNullcheck(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    Inst *obj = GetDefinitionAcc();

    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(saveState);

    if (GetGraph()->IsBytecodeOptimizer()) {
        RuntimeInterface::IntrinsicId intrinsicId = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_NULLCHECK;
        auto intrinsic = GetGraph()->CreateInstIntrinsic(DataType::BOOL, pc, intrinsicId);

        intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
        intrinsic->AppendInput(obj);
        intrinsic->AddInputType(DataType::REFERENCE);
        intrinsic->AppendInput(saveState);
        intrinsic->AddInputType(DataType::NO_TYPE);

        AddInstruction(intrinsic);
    } else {
        auto nullCheck = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, pc, obj, saveState);
        AddInstruction(nullCheck);
        UpdateDefinitionAcc(nullCheck);
    }
}

void InstBuilder::BuildAwait(const BytecodeInstruction *bcInst)
{
    // NOTE(compiler_team): support stackless in BCO #33857.
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->IsAbcKit()) {
        failed_ = true;
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto promise = GetDefinition(bcInst->GetVReg(0));
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto asyncContext = GetGraph()->CreateInstIntrinsic(
        DataType::REFERENCE, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_AWAIT_RESOLUTION);

    asyncContext->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
    asyncContext->AppendInput(promise);
    asyncContext->AddInputType(DataType::REFERENCE);
    asyncContext->AppendInput(saveState);
    asyncContext->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(asyncContext);

    auto nextPc = GetPc(bcInst->GetNext().GetAddress());
    auto saveStateSuspend = CreateSaveState(Opcode::SaveStateSuspend, nextPc);
    // NOTE (asidorov): remove workaround for SaveStateSuspend (#34570)
    saveStateSuspend->AppendBridge(asyncContext);
    PlaceAsyncContextInput(saveStateSuspend, asyncContext);
    AddInstruction(saveStateSuspend);
}

void InstBuilder::BuildDispatch(const BytecodeInstruction *bcInst)
{
    // NOTE(compiler_team): support stackless in BCO #33857.
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->IsAbcKit()) {
        failed_ = true;
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto asyncContext = GetGraph()->CreateInstIntrinsic(
        DataType::REFERENCE, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ASYNC_CONTEXT_CURRENT);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    asyncContext->AllocateInputTypes(GetGraph()->GetAllocator(), 1_I);
    asyncContext->AppendInput(saveState);
    asyncContext->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(asyncContext);

    saveState = CreateSaveState(Opcode::SaveStateDeoptimize, pc);
    AddInstruction(saveState);
    auto dispatchInst = GetGraph()->CreateInstDispatch(DataType::NO_TYPE, pc, asyncContext, saveState);
    AddInstruction(dispatchInst);
    GetGraph()->SetDispatchInst(dispatchInst);
}

void InstBuilder::BuildUnpack(const BytecodeInstruction *bcInst)
{
    // NOTE(compiler_team): support stackless in BCO #33857.
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->IsAbcKit()) {
        failed_ = true;
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto promise = GetDefinition(bcInst->GetVReg(0));
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto result = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc,
                                                  RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ASYNC_UNPACK);

    result->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
    result->AppendInput(promise);
    result->AddInputType(DataType::REFERENCE);
    result->AppendInput(saveState);
    result->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

void InstBuilder::BuildResolve(const BytecodeInstruction *bcInst)
{
    // NOTE(compiler_team): support stackless in BCO #33857.
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->IsAbcKit()) {
        failed_ = true;
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto value = GetDefinition(bcInst->GetVReg(0));
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto promise = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc,
                                                   RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ASYNC_RESOLVE);

    promise->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
    promise->AppendInput(value);
    promise->AddInputType(DataType::REFERENCE);
    promise->AppendInput(saveState);
    promise->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(promise);
    UpdateDefinitionAcc(promise);
}

void InstBuilder::BuildReject(const BytecodeInstruction *bcInst)
{
    // NOTE(compiler_team): support stackless in BCO #33857.
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->IsAbcKit()) {
        failed_ = true;
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto error = GetDefinition(bcInst->GetVReg(0));
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto promise = GetGraph()->CreateInstIntrinsic(DataType::REFERENCE, pc,
                                                   RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ASYNC_REJECT);

    promise->AllocateInputTypes(GetGraph()->GetAllocator(), 2_I);
    promise->AppendInput(error);
    promise->AddInputType(DataType::REFERENCE);
    promise->AppendInput(saveState);
    promise->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState);
    AddInstruction(promise);
    UpdateDefinitionAcc(promise);
}

}  // namespace ark::compiler
