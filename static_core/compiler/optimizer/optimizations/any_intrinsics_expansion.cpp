/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "compiler_logger.h"
#include "any_intrinsics_expansion.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/runtime_interface.h"

namespace ark::compiler {

bool AnyIntrinsicsExpansion::RunImpl()
{
    VisitGraph();
    for (auto inst : toRemove_) {
        inst->GetBasicBlock()->RemoveInst(inst);
        SetApplied();
    }
    return IsApplied();
}

Inst *AnyIntrinsicsExpansion::CreateLoadClassWithGuard(Inst *inst, Inst *objInst, RuntimeInterface::ClassPtr cls)
{
    auto pc = inst->GetPc();
    auto saveState = inst->GetSaveState();
    auto nullCheck = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, pc, objInst, saveState);

    auto getClsInst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, pc, nullCheck);
    auto loadClsInst = GetGraph()->CreateInstLoadImmediate(DataType::REFERENCE, pc, cls);
    auto cmpInst = GetGraph()->CreateInstCompare(DataType::BOOL, pc, getClsInst, loadClsInst, DataType::REFERENCE,
                                                 ConditionCode::CC_NE);
    auto deoptInst = GetGraph()->CreateInstDeoptimizeIf(pc, cmpInst, saveState, DeoptimizeType::ANY_IC);

    auto bb = inst->GetBasicBlock();
    bb->InsertBefore(nullCheck, inst);
    bb->InsertBefore(getClsInst, inst);
    bb->InsertBefore(loadClsInst, inst);
    bb->InsertBefore(cmpInst, inst);
    bb->InsertBefore(deoptInst, inst);

    return nullCheck;
}

Inst *AnyIntrinsicsExpansion::BoxValue(Inst *inst, Inst *val)
{
    auto pc = inst->GetPc();
    auto saveState = inst->GetSaveState();
    auto runtime = GetGraph()->GetRuntime();
    auto type = val->GetType();
    auto boxedClass = runtime->GetDataTypeBoxedClass(type);
    if (boxedClass == nullptr) {
        ASSERT(type == DataType::REFERENCE);
        return val;
    }
    auto boxedClassId = runtime->GetClassIdWithinFile(GetGraph()->GetMethod(), boxedClass);
    auto cctor = runtime->GetBoxedClassConstructor(boxedClass);
    ASSERT(cctor != nullptr);
    auto cctorId = runtime->GetMethodId(cctor);
    auto initBoxedClass = GetGraph()->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, nullptr,
                                                                 TypeIdMixin {boxedClassId, cctor}, boxedClass);
    auto newObject = GetGraph()->CreateInstNewObject(DataType::REFERENCE, pc, initBoxedClass, saveState,
                                                     TypeIdMixin {boxedClassId, GetGraph()->GetMethod()});
    initBoxedClass->SetInput(0, saveState);

    auto callCctor = GetGraph()->CreateInstCallStatic(runtime->GetMethodReturnType(cctor, cctorId), pc, cctorId, cctor);
    callCctor->ReserveInputs(3U);
    callCctor->AllocateInputTypes(GetGraph()->GetAllocator(), 3U);
    callCctor->AppendInput(newObject);
    callCctor->AddInputType(DataType::REFERENCE);
    callCctor->AppendInput(val);
    callCctor->AddInputType(type);
    callCctor->AppendInput(saveState);
    callCctor->AddInputType(DataType::NO_TYPE);

    auto bb = inst->GetBasicBlock();
    bb->InsertBefore(initBoxedClass, inst);
    bb->InsertBefore(newObject, inst);
    bb->InsertBefore(callCctor, inst);
    return newObject;
}

void AnyIntrinsicsExpansion::HandleAnyLdbyname(IntrinsicInst *inst)
{
    auto runtime = GetGraph()->GetRuntime();
    auto cls = runtime->GetAnyInstInlineCaches()->GetClass(GetGraph()->GetMethod(), inst->GetSlotId());
    if (cls == nullptr) {
        return;
    }

    auto propName = runtime->GetStringValue(GetGraph()->GetMethod(), inst->GetImm(0));
    auto field = runtime->GetFieldPtrByName(cls, propName);
    auto getter = runtime->GetFieldGetterByName(cls, propName);
    if (field == nullptr && getter == nullptr) {
        // Leave it as intrinsic to throw runtime exception when executed
        return;
    }

    auto nullCheck = CreateLoadClassWithGuard(inst, inst->GetInput(0).GetInst(), cls);
    if (field != nullptr) {
        // Object has a field with this name, create a load object instruction
        auto loadField =
            GetGraph()->CreateInstLoadObject(runtime->GetFieldType(field), inst->GetPc(), nullCheck,
                                             TypeIdMixin {field->GetFileId().GetOffset(), GetGraph()->GetMethod()},
                                             field, runtime->IsFieldVolatile(field));

        inst->GetBasicBlock()->InsertBefore(loadField, inst);
        auto boxedValue = BoxValue(inst, loadField);
        inst->ReplaceUsers(boxedValue);
    } else {
        auto gId = runtime->GetMethodId(getter);
        auto callGetter =
            GetGraph()->CreateInstCallVirtual(runtime->GetMethodReturnType(getter, gId), inst->GetPc(), gId, getter);
        callGetter->ReserveInputs(2U);
        callGetter->AllocateInputTypes(GetGraph()->GetAllocator(), 2U);
        callGetter->AppendInput(nullCheck);
        callGetter->AddInputType(DataType::REFERENCE);
        callGetter->AppendInput(inst->GetSaveState());
        callGetter->AddInputType(DataType::NO_TYPE);

        inst->GetBasicBlock()->InsertBefore(callGetter, inst);
        auto boxedValue = BoxValue(inst, callGetter);
        inst->ReplaceUsers(boxedValue);
    }

    toRemove_.push_back(inst);
}

void AnyIntrinsicsExpansion::CallSetter(IntrinsicInst *inst, Inst *val, Inst *obj, RuntimeInterface::MethodPtr setter)
{
    ASSERT(setter != nullptr);
    auto runtime = GetGraph()->GetRuntime();
    auto sId = runtime->GetMethodId(setter);
    auto valType = runtime->GetMethodArgumentType(setter, sId, 0U);

    auto callSetter =
        GetGraph()->CreateInstCallVirtual(runtime->GetMethodReturnType(setter, sId), inst->GetPc(), sId, setter);
    callSetter->ReserveInputs(3U);
    callSetter->AllocateInputTypes(GetGraph()->GetAllocator(), 3U);
    callSetter->AppendInput(obj);
    callSetter->AddInputType(DataType::REFERENCE);
    callSetter->AppendInput(val);
    callSetter->AddInputType(valType);
    callSetter->AppendInput(inst->GetSaveState());
    callSetter->AddInputType(DataType::NO_TYPE);

    inst->GetBasicBlock()->InsertBefore(callSetter, inst);
    inst->ReplaceUsers(callSetter);
}

void AnyIntrinsicsExpansion::HandleAnyStbyname(IntrinsicInst *inst)
{
    auto runtime = GetGraph()->GetRuntime();
    auto cls = runtime->GetAnyInstInlineCaches()->GetClass(GetGraph()->GetMethod(), inst->GetSlotId());
    if (cls == nullptr) {
        return;
    }

    auto val = inst->GetInput(1).GetInst();
    ASSERT(val->GetType() == DataType::REFERENCE);

    auto propName = runtime->GetStringValue(GetGraph()->GetMethod(), inst->GetImm(0));
    auto field = runtime->GetFieldPtrByName(cls, propName);
    auto setter = runtime->GetFieldSetterByName(cls, propName);
    if (field == nullptr && setter == nullptr) {
        // Leave it as intrinsic to throw runtime exception when executed
        return;
    }

    auto valType = field != nullptr ? runtime->GetFieldType(field)
                                    : runtime->GetMethodArgumentType(setter, runtime->GetMethodId(setter), 0U);
    if (valType != DataType::REFERENCE) {
        auto boxedClass = runtime->GetDataTypeBoxedClass(valType);
        ASSERT(boxedClass != nullptr);
        auto boxedClassId = runtime->GetClassIdWithinFile(GetGraph()->GetMethod(), boxedClass);
        auto boxedClassField = runtime->GetFieldPtrByName(boxedClass, "value");
        if (boxedClassField == nullptr) {
            ASSERT_PRINT(false, "Expected box class to have `value` property");
            return;
        }

        val = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, inst->GetPc(), val, inst->GetSaveState());
        val->SetFlag(inst_flags::CAN_DEOPTIMIZE);
        val = GetGraph()->CreateInstLoadObject(valType, inst->GetPc(), val,
                                               TypeIdMixin {boxedClassId, GetGraph()->GetMethod()}, boxedClassField,
                                               runtime->IsFieldVolatile(boxedClassField));
    }
    auto nullCheck = CreateLoadClassWithGuard(inst, inst->GetInput(0).GetInst(), cls);
    if (valType != DataType::REFERENCE) {
        ASSERT(val->GetInputsCount() > 0 && val->GetInput(0).GetInst()->IsNullCheck());
        // Insert the null check inst
        inst->GetBasicBlock()->InsertBefore(val->GetInput(0).GetInst(), inst);
        // Insert unboxed value after the class load
        inst->GetBasicBlock()->InsertBefore(val, inst);
    }

    if (field != nullptr) {
        // Object has a field with this name, create a store object instruction
        auto storeField =
            GetGraph()->CreateInstStoreObject(valType, inst->GetPc(), nullCheck, val,
                                              TypeIdMixin {field->GetFileId().GetOffset(), GetGraph()->GetMethod()},
                                              field, runtime->IsFieldVolatile(field));

        inst->GetBasicBlock()->InsertBefore(storeField, inst);
        inst->ReplaceUsers(storeField);
    } else {
        CallSetter(inst, val, nullCheck, setter);
    }

    toRemove_.push_back(inst);
}

void AnyIntrinsicsExpansion::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    auto intrinsic = inst->CastToIntrinsic();
    auto visitor = static_cast<AnyIntrinsicsExpansion *>(v);
    switch (intrinsic->GetIntrinsicId()) {
        default:
            return;
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_LDBYNAME:
            visitor->HandleAnyLdbyname(intrinsic);
            return;
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_STBYNAME:
            visitor->HandleAnyStbyname(intrinsic);
            return;
    }
}

}  // namespace ark::compiler
