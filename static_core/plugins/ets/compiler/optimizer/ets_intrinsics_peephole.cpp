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

#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "compiler/optimizer/optimizations/const_folding.h"

namespace ark::compiler {

static void ReplaceWithCompareEQ(IntrinsicInst *intrinsic)
{
    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();

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
}

bool Peepholes::PeepholeStringEquals([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    // Replaces
    //      Intrinsic.StdCoreStringEquals arg, NullPtr
    // with
    //      Compare EQ ref             arg, NullPtr

    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();
    if (input0->IsNullPtr() || input1->IsNullPtr()) {
        ReplaceWithCompareEQ(intrinsic);
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
    auto pc = intrinsic->GetPc();
    if constexpr (IS_STORE) {
        auto type = intrinsic->GetInputType(1);
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
        auto type = intrinsic->GetType();
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

bool Peepholes::PeepholeLdObjByName([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
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
        return true;
    }
    if (TryInsertCallInst<false>(intrinsic, klassPtr, rawField)) {
        return true;
    }
    return false;
}

bool Peepholes::PeepholeStObjByName([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
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
        return true;
    }
    if (TryInsertCallInst<true>(intrinsic, klassPtr, rawField)) {
        return true;
    }
    return false;
}

static void ReplaceWithCompareNullish(IntrinsicInst *intrinsic, Inst *input)
{
    auto bb = intrinsic->GetBasicBlock();
    auto graph = bb->GetGraph();

    auto compareNull = graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), input, graph->GetOrCreateNullPtr(),
                                                DataType::REFERENCE, ConditionCode::CC_EQ);
    auto compareUndef =
        graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), input, graph->GetOrCreateUndefinedInst(),
                                 DataType::REFERENCE, ConditionCode::CC_EQ);

    auto orInst = graph->CreateInstOr(DataType::BOOL, intrinsic->GetPc(), compareNull, compareUndef);

    bb->InsertAfter(compareNull, intrinsic);
    bb->InsertAfter(compareUndef, compareNull);
    bb->InsertAfter(orInst, compareUndef);

    intrinsic->ReplaceUsers(orInst);
}

bool Peepholes::PeepholeEquals([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();

    const auto isNullish = [](Inst *inst) { return inst->IsNullPtr() || inst->IsLoadUndefined(); };
    if (input0 == input1 || (isNullish(input0) && isNullish(input1))) {
        intrinsic->ReplaceUsers(ConstFoldingCreateIntConst(intrinsic, 1));
        return true;
    }
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return false;
    }
    if (isNullish(input0) || isNullish(input1)) {
        auto nonNullishInput = isNullish(input0) ? input1 : input0;
        ReplaceWithCompareNullish(intrinsic, nonNullishInput);
        return true;
    }

    auto klass1 = GetClassPtrForObject(intrinsic, 0);
    auto klass2 = GetClassPtrForObject(intrinsic, 1);
    if ((klass1 != nullptr && !graph->GetRuntime()->IsClassValueTyped(klass1)) ||
        (klass2 != nullptr && !graph->GetRuntime()->IsClassValueTyped(klass2))) {
        ReplaceWithCompareEQ(intrinsic);
        return true;
    }
    // NOTE(vpukhov): #16340 try ObjectTypePropagation
    return false;
}

#ifdef PANDA_ETS_INTEROP_JS

bool Peepholes::TryFuseGetPropertyAndCast(IntrinsicInst *intrinsic, RuntimeInterface::IntrinsicId newId)
{
    auto input = intrinsic->GetInput(0).GetInst();
    if (!input->HasSingleUser() || input->GetBasicBlock() != intrinsic->GetBasicBlock()) {
        return false;
    }
    if (!input->IsIntrinsic() || input->CastToIntrinsic()->GetIntrinsicId() !=
                                     RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_JS_VALUE) {
        return false;
    }
    for (auto inst = input->GetNext(); inst != intrinsic; inst = inst->GetNext()) {
        ASSERT(inst != nullptr);
        if (inst->IsNotRemovable()) {
            return false;
        }
    }
    input->CastToIntrinsic()->SetIntrinsicId(newId);
    input->SetType(intrinsic->GetType());
    intrinsic->ReplaceUsers(input);
    intrinsic->GetBasicBlock()->RemoveInst(intrinsic);
    intrinsic->SetNext(input->GetNext());  // Fix for InstForwardIterator in Peepholes visitor
    return true;
}

bool Peepholes::PeepholeJSRuntimeGetValueString([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseGetPropertyAndCast(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_STRING);
}

bool Peepholes::PeepholeJSRuntimeGetValueDouble([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseGetPropertyAndCast(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_DOUBLE);
}

bool Peepholes::PeepholeJSRuntimeGetValueBoolean([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseGetPropertyAndCast(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_BOOLEAN);
}

bool Peepholes::TryFuseCastAndSetProperty(IntrinsicInst *intrinsic, RuntimeInterface::IntrinsicId newId)
{
    size_t userCount = 0;
    constexpr size_t STORE_VALUE_IDX = 2;
    constexpr auto SET_PROP_ID = RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_JS_VALUE;
    for (auto &user : intrinsic->GetUsers()) {
        ++userCount;
        if (user.GetIndex() != STORE_VALUE_IDX || !user.GetInst()->IsIntrinsic() ||
            user.GetInst()->CastToIntrinsic()->GetIntrinsicId() != SET_PROP_ID) {
            return false;
        }
    }
    auto userIt = intrinsic->GetUsers().begin();
    for (size_t userIdx = 0; userIdx < userCount; ++userIdx) {
        ASSERT(userIt != intrinsic->GetUsers().end());
        auto *storeInst = userIt->GetInst();
        auto newValue = intrinsic->GetInput(0).GetInst();
        storeInst->CastToIntrinsic()->SetIntrinsicId(newId);
        storeInst->ReplaceInput(intrinsic, newValue);
        storeInst->CastToIntrinsic()->SetInputType(STORE_VALUE_IDX, newValue->GetType());
        userIt = intrinsic->GetUsers().begin();
    }
    return true;
}

bool Peepholes::PeepholeJSRuntimeNewJSValueString(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseCastAndSetProperty(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_STRING);
}

bool Peepholes::PeepholeJSRuntimeNewJSValueDouble(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseCastAndSetProperty(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_DOUBLE);
}

bool Peepholes::PeepholeJSRuntimeNewJSValueBoolean(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseCastAndSetProperty(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_BOOLEAN);
}

static std::pair<Inst *, Inst *> BuildLoadPropertyChain(IntrinsicInst *intrinsic, uint64_t qnameStart,
                                                        uint64_t qnameLen)
{
    auto *jsThis = intrinsic->GetInput(0).GetInst();
    auto *jsFn = jsThis;
    auto saveState = intrinsic->GetSaveState();
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto klass = runtime->GetClass(intrinsic->GetMethod());
    auto pc = intrinsic->GetPc();
    auto jsConstPool = graph->CreateInstIntrinsic(
        DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_LOAD_JS_CONSTANT_POOL);
    jsConstPool->SetInputs(graph->GetAllocator(), {{saveState, DataType::NO_TYPE}});
    intrinsic->InsertBefore(jsConstPool);
    Inst *cpOffsetForClass = intrinsic->GetInput(3U).GetInst();
    for (uint32_t strIndexInAnnot = qnameStart; strIndexInAnnot < qnameStart + qnameLen; strIndexInAnnot++) {
        IntrinsicInst *jsProperty = nullptr;
        auto uniqueStrIndex = graph->GetRuntime()->GetAnnotationElementUniqueIndex(
            klass, "Lets/annotation/DynamicCall;", strIndexInAnnot);
        auto strIndexInAnnotConst = graph->FindOrCreateConstant(uniqueStrIndex);
        auto indexInst = graph->CreateInstAdd(DataType::INT32, pc, cpOffsetForClass, strIndexInAnnotConst);
        intrinsic->InsertBefore(indexInst);
        jsProperty = graph->CreateInstIntrinsic(DataType::POINTER, pc,
                                                RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_JS_ELEMENT);
        // ConstantPool elements are immutable
        jsProperty->ClearFlag(inst_flags::NO_CSE);
        jsProperty->ClearFlag(inst_flags::NO_DCE);

        jsProperty->SetInputs(
            graph->GetAllocator(),
            {{jsConstPool, DataType::POINTER}, {indexInst, DataType::INT32}, {saveState, DataType::NO_TYPE}});

        auto getProperty = graph->CreateInstIntrinsic(
            DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_JS_PROPERTY);
        getProperty->SetInputs(
            graph->GetAllocator(),
            {{jsFn, DataType::POINTER}, {jsProperty, DataType::POINTER}, {saveState, DataType::NO_TYPE}});
        intrinsic->InsertBefore(jsProperty);
        intrinsic->InsertBefore(getProperty);
        jsThis = jsFn;
        jsFn = getProperty;
    }
    return {jsThis, jsFn};
}

bool Peepholes::PeepholeResolveQualifiedJSCall([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    if (!intrinsic->HasUsers()) {
        return false;
    }
    auto qnameStartInst = intrinsic->GetInput(1).GetInst();
    auto qnameLenInst = intrinsic->GetInput(2U).GetInst();

    if (!qnameStartInst->IsConst() || !qnameLenInst->IsConst()) {
        // qnameStart and qnameLen are always constant, but may be e.g. Phi instructions after BCO
        return false;
    }
    auto qnameStart = qnameStartInst->CastToConstant()->GetIntValue();
    auto qnameLen = qnameLenInst->CastToConstant()->GetIntValue();
    ASSERT(qnameLen > 0);
    auto [jsThis, jsFn] = BuildLoadPropertyChain(intrinsic, qnameStart, qnameLen);
    constexpr auto FN_PSEUDO_USER = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_LOAD_RESOLVED_JS_FUNCTION;
    for (auto &user : intrinsic->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsIntrinsic() && userInst->CastToIntrinsic()->GetIntrinsicId() == FN_PSEUDO_USER) {
            userInst->ReplaceUsers(jsFn);
        }
    }
    intrinsic->ReplaceUsers(jsThis);
    intrinsic->ClearFlag(inst_flags::NO_DCE);
    return true;
}

#endif

}  // namespace ark::compiler
