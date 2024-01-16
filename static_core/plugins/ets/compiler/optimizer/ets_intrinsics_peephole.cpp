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

#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/runtime_interface.h"

namespace ark::compiler {
bool Peepholes::PeepholeStringEquals([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    // Replaces
    //      Intrinsic.StdCoreStringEquals arg, NullPtr
    // with
    //      Compare EQ ref             arg, NullPtr

    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();
    if (input0->IsNullPtr() || input1->IsNullPtr()) {
        auto bb = intrinsic->GetBasicBlock();
        auto graph = bb->GetGraph();

        auto compare = graph->CreateInst(Opcode::Compare)->CastToCompare();
        compare->SetCc(ConditionCode::CC_EQ);
        compare->SetType(intrinsic->GetType());
        ASSERT(input0->GetType() == input1->GetType());
        compare->SetOperandsType(input0->GetType());

        compare->SetInput(0, input0);
        compare->SetInput(1, input1);
        bb->InsertAfter(compare, intrinsic);
        intrinsic->ReplaceUsers(compare);

        return true;
    }

    return false;
}

Inst *GetStringFromLength(Inst *inst)
{
    Inst *lenArray = inst;
    if (inst->GetBasicBlock()->GetGraph()->GetRuntime()->IsCompressedStringsEnabled()) {
        if (inst->GetOpcode() != Opcode::Shr || inst->GetType() != DataType::INT32) {
            return nullptr;
        }
        auto input1 = inst->GetInput(1).GetInst();
        if (!input1->IsConst() || input1->CastToConstant()->GetRawValue() != 1) {
            return nullptr;
        }
        lenArray = inst->GetInput(0).GetInst();
    }

    if (lenArray->GetOpcode() != Opcode::LenArray || !lenArray->CastToLenArray()->IsString()) {
        return nullptr;
    }
    return lenArray->GetDataFlowInput(0);
}

bool Peepholes::PeepholeStringSubstring([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    // Replaces
    //      string
    //      Intrinsic.StdCoreStringSubstring string, 0, string.Length -> .....
    // with
    //      string -> .....

    auto string = intrinsic->GetDataFlowInput(0);
    auto begin = intrinsic->GetInput(1).GetInst();
    auto end = intrinsic->GetInput(2).GetInst();
    if (!begin->IsConst() || begin->GetType() != DataType::INT64) {
        return false;
    }
    if (static_cast<int64_t>(begin->CastToConstant()->GetRawValue()) > 0) {
        return false;
    }
    if (GetStringFromLength(end) != string) {
        return false;
    }
    intrinsic->ReplaceUsers(string);
    intrinsic->ClearFlag(inst_flags::NO_DCE);

    return true;
}

template <bool IS_STORE>
bool TryInsertFieldInst(IntrinsicInst *intrinsic, RuntimeInterface::ClassPtr klassPtr,
                        RuntimeInterface::FieldPtr rawField, size_t fieldId)
{
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto field = runtime->ResolveLookUpField(rawField, klassPtr);
    if (field == nullptr) {
        return false;
    }
    Inst *memObj;
    auto type = intrinsic->GetType();
    auto pc = intrinsic->GetPc();
    if constexpr (IS_STORE) {
        auto storeField = graph->CreateInstStoreObject(type, pc);
        storeField->SetTypeId(fieldId);
        storeField->SetMethod(intrinsic->GetMethod());
        storeField->SetObjField(field);
        if (runtime->IsFieldVolatile(field)) {
            storeField->SetVolatile(true);
        }
        if (type == DataType::REFERENCE) {
            storeField->SetNeedBarrier(true);
        }
        storeField->SetInput(1, intrinsic->GetInput(1).GetInst());
        memObj = storeField;
    } else {
        auto loadField = graph->CreateInstLoadObject(type, pc);
        loadField->SetTypeId(fieldId);
        loadField->SetMethod(intrinsic->GetMethod());
        loadField->SetObjField(field);
        if (runtime->IsFieldVolatile(field)) {
            loadField->SetVolatile(true);
        }
        memObj = loadField;
        intrinsic->ReplaceUsers(loadField);
    }
    memObj->SetInput(0, intrinsic->GetInput(0).GetInst());
    intrinsic->InsertAfter(memObj);

    intrinsic->ClearFlag(inst_flags::NO_DCE);
    return true;
}

template <bool IS_STORE>
bool TryInsertCallInst(IntrinsicInst *intrinsic, RuntimeInterface::ClassPtr klassPtr,
                       RuntimeInterface::FieldPtr rawField)
{
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto method = runtime->ResolveLookUpCall(rawField, klassPtr, IS_STORE);
    if (method == nullptr) {
        return false;
    }
    auto type = IS_STORE ? DataType::VOID : intrinsic->GetType();
    auto pc = intrinsic->GetPc();

    auto call = graph->CreateInstCallVirtual(type, pc, runtime->GetMethodId(method));
    call->SetCallMethod(method);
    size_t numInputs = IS_STORE ? 3 : 2;
    call->ReserveInputs(numInputs);
    call->AllocateInputTypes(graph->GetAllocator(), numInputs);
    for (size_t i = 0; i < numInputs; ++i) {
        call->AppendInputAndType(intrinsic->GetInput(i).GetInst(), intrinsic->GetInputType(i));
    }
    intrinsic->InsertAfter(call);
    intrinsic->ReplaceUsers(call);
    intrinsic->ClearFlag(inst_flags::NO_DCE);
    return true;
}

bool Peepholes::PeepholeLdObjByName(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto klassPtr = GetClassPtrForObject(intrinsic);
    if (klassPtr == nullptr) {
        return false;
    }
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto method = intrinsic->GetMethod();
    auto runtime = graph->GetRuntime();
    auto fieldId = intrinsic->GetImm(0);
    auto rawField = runtime->ResolveField(method, fieldId, !graph->IsAotMode(), nullptr);
    ASSERT(rawField != nullptr);

    if (TryInsertFieldInst<false>(intrinsic, klassPtr, rawField, fieldId)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    if (TryInsertCallInst<false>(intrinsic, klassPtr, rawField)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    return false;
}

bool Peepholes::PeepholeStObjByName(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto klassPtr = GetClassPtrForObject(intrinsic);
    if (klassPtr == nullptr) {
        return false;
    }
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto method = intrinsic->GetMethod();
    auto runtime = graph->GetRuntime();
    auto fieldId = intrinsic->GetImm(0);
    auto rawField = runtime->ResolveField(method, fieldId, !graph->IsAotMode(), nullptr);
    ASSERT(rawField != nullptr);

    if (TryInsertFieldInst<true>(intrinsic, klassPtr, rawField, fieldId)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    if (TryInsertCallInst<true>(intrinsic, klassPtr, rawField)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    return false;
}

}  // namespace ark::compiler