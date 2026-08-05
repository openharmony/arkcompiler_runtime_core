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

Inst *AnyIntrinsicsExpansion::BoxValue(Inst *inst, Inst *val, Inst *saveState)
{
    auto pc = inst->GetPc();
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

    auto saveStateInst = saveState->CastToSaveState();
    auto ctorSaveState = CopySaveState(GetGraph(), saveStateInst);
    ctorSaveState->AppendBridge(newObject);

    auto callCctor = GetGraph()->CreateInstCallStatic(runtime->GetMethodReturnType(cctor, cctorId), pc, cctorId, cctor);
    callCctor->ReserveInputs(3U);
    callCctor->AllocateInputTypes(GetGraph()->GetAllocator(), 3U);
    callCctor->AppendInput(newObject);
    callCctor->AddInputType(DataType::REFERENCE);
    callCctor->AppendInput(val);
    callCctor->AddInputType(type);
    callCctor->AppendInput(ctorSaveState);
    callCctor->AddInputType(DataType::NO_TYPE);

    auto bb = inst->GetBasicBlock();
    bb->InsertBefore(initBoxedClass, inst);
    bb->InsertBefore(newObject, inst);
    bb->InsertBefore(ctorSaveState, inst);
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
        auto boxedValue = BoxValue(inst, loadField, inst->GetSaveState());
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
        auto boxedValue = BoxValue(inst, callGetter, inst->GetSaveState());
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

std::optional<std::vector<Inst *>> AnyIntrinsicsExpansion::GetBoxedArgs(IntrinsicInst *inst,
                                                                        RuntimeInterface::MethodPtr method)
{
    auto runtime = GetGraph()->GetRuntime();
    auto methodId = runtime->GetMethodId(method);
    std::vector<Inst *> boxedArgs;
    if (inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_CALL_THIS_SHORT) {
        // Input 1 is the arg
        boxedArgs.push_back(inst->GetInput(1).GetInst());
    }
    if (inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_CALL_THIS_RANGE) {
        // Input 1 is argc, last input is SaveState, between them are actual args.
        for (size_t i = 2U; i < inst->GetInputsCount() - 1U; i++) {
            boxedArgs.push_back(inst->GetInput(i).GetInst());
        }
    }
    if (runtime->GetMethodArgumentsCount(method, methodId) != boxedArgs.size()) {
        return std::nullopt;
    }
    // Run checks here so we don't generate code if we can't
    for (size_t i = 0; i < boxedArgs.size(); i++) {
        auto argType = runtime->GetMethodArgumentType(method, methodId, i);
        if (argType != DataType::REFERENCE) {
            auto boxedClass = runtime->GetDataTypeBoxedClass(argType);
            ASSERT(boxedClass != nullptr);
            auto boxedClassField = runtime->GetFieldPtrByName(boxedClass, "value");
            if (boxedClassField == nullptr) {
                ASSERT_PRINT(false, "Expected box class to have `value` property");
                return std::nullopt;
            }
        }
    }

    return boxedArgs;
}

Inst *AnyIntrinsicsExpansion::UnboxValue(IntrinsicInst *inst, Inst *boxedValue, DataType::Type valueType)
{
    ASSERT(valueType != DataType::REFERENCE);
    // This method doesn't do static checks as it assumes that they were done beforehand
    auto runtime = GetGraph()->GetRuntime();
    auto boxedClass = runtime->GetDataTypeBoxedClass(valueType);
    ASSERT(boxedClass != nullptr);
    auto boxedClassId = runtime->GetClassIdWithinFile(GetGraph()->GetMethod(), boxedClass);
    auto boxedClassField = runtime->GetFieldPtrByName(boxedClass, "value");
    ASSERT(boxedClassField != nullptr);

    // Guard that checks that we're loading from the correct box class
    auto nullCheck = CreateLoadClassWithGuard(inst, boxedValue, boxedClass);
    nullCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
    auto unboxedValue = GetGraph()->CreateInstLoadObject(valueType, inst->GetPc(), boxedValue,
                                                         TypeIdMixin {boxedClassId, GetGraph()->GetMethod()},
                                                         boxedClassField, runtime->IsFieldVolatile(boxedClassField));
    inst->GetBasicBlock()->InsertBefore(unboxedValue, inst);
    return unboxedValue;
}

Inst *AnyIntrinsicsExpansion::CreateCallThis(IntrinsicInst *inst, Inst *thisObject, std::vector<Inst *> boxedArgs,
                                             RuntimeInterface::MethodPtr method)
{
    auto runtime = GetGraph()->GetRuntime();
    auto saveState = inst->GetSaveState();
    auto methodId = runtime->GetMethodId(method);
    auto retType = runtime->GetMethodReturnType(method, methodId);
    ASSERT(!runtime->IsMethodStatic(method));
    CallInst *call = GetGraph()->CreateInstCallVirtual(retType, inst->GetPc(), methodId, method);
    call->ReserveInputs(2U + boxedArgs.size());
    call->AllocateInputTypes(GetGraph()->GetAllocator(), 2U + boxedArgs.size());
    call->AppendInput(thisObject);
    call->AddInputType(DataType::REFERENCE);
    auto refCounter = retType == DataType::REFERENCE ? 1U : 0;
    for (size_t i = 0; i < boxedArgs.size(); i++) {
        auto arg = boxedArgs[i];
        auto argType = runtime->GetMethodArgumentType(method, methodId, i);
        if (argType != DataType::REFERENCE) {
            arg = UnboxValue(inst, arg, argType);
        } else {
            // Create a guard that this reference is assignable to argument type
            auto refTypeId = runtime->GetMethodArgReferenceTypeId(method, refCounter++);
            auto refClass = runtime->ResolveType(method, refTypeId);
            auto loadClass = GetGraph()->CreateInstLoadClass(DataType::REFERENCE, inst->GetPc(), saveState,
                                                             TypeIdMixin {refTypeId, method}, refClass);

            auto isInstance =
                GetGraph()->CreateInstIsInstance(DataType::BOOL, inst->GetPc(), arg, loadClass, saveState,
                                                 TypeIdMixin {refTypeId, method}, runtime->GetClassType(refClass));
            // IsInstance returns `false` if input is `nullptr`, so handle this separately
            auto isNull =
                GetGraph()->CreateInstCompare(DataType::BOOL, inst->GetPc(), arg, GetGraph()->GetOrCreateNullPtr(),
                                              DataType::REFERENCE, ConditionCode::CC_EQ);
            auto isInstanceOrNull = GetGraph()->CreateInstOr(DataType::BOOL, inst->GetPc(), isInstance, isNull);
            auto isNotInstanceAndNotNull = GetGraph()->CreateInstNot(DataType::BOOL, inst->GetPc(), isInstanceOrNull);
            auto deoptInst = GetGraph()->CreateInstDeoptimizeIf(inst->GetPc(), isNotInstanceAndNotNull, saveState,
                                                                DeoptimizeType::ANY_IC);

            inst->GetBasicBlock()->InsertBefore(loadClass, inst);
            inst->GetBasicBlock()->InsertBefore(isInstance, inst);
            inst->GetBasicBlock()->InsertBefore(isNull, inst);
            inst->GetBasicBlock()->InsertBefore(isInstanceOrNull, inst);
            inst->GetBasicBlock()->InsertBefore(isNotInstanceAndNotNull, inst);
            inst->GetBasicBlock()->InsertBefore(deoptInst, inst);
        }
        call->AppendInput(arg);
        call->AddInputType(argType);
    }
    call->AppendInput(inst->GetSaveState());
    call->AddInputType(DataType::NO_TYPE);
    inst->GetBasicBlock()->InsertBefore(call, inst);
    return call;
}

void AnyIntrinsicsExpansion::HandleAnyCallThis(IntrinsicInst *inst)
{
    auto runtime = GetGraph()->GetRuntime();
    auto cls = runtime->GetAnyInstInlineCaches()->GetClass(GetGraph()->GetMethod(), inst->GetSlotId());
    if (cls == nullptr) {
        return;
    }

    auto methodName = runtime->GetStringValue(GetGraph()->GetMethod(), inst->GetImm(0));
    auto method = runtime->GetUniqueInstanceMethodByName(cls, methodName);
    if (method == nullptr) {
        return;
    }
    auto methodId = runtime->GetMethodId(method);
    auto argsOpt = GetBoxedArgs(inst, method);
    if (argsOpt == std::nullopt) {
        return;
    }
    auto boxedArgs = argsOpt.value();

    auto nullCheck = CreateLoadClassWithGuard(inst, inst->GetInput(0).GetInst(), cls);
    auto call = CreateCallThis(inst, nullCheck, boxedArgs, method);

    auto retType = runtime->GetMethodReturnType(method, methodId);
    if (retType == DataType::VOID) {
        inst->ReplaceUsers(GetGraph()->GetOrCreateNullPtr());
    } else {
        // We've made a call, so we need a new savestate
        auto afterCallSaveState = CopySaveState(GetGraph(), inst->GetSaveState());
        if (retType == DataType::REFERENCE) {
            afterCallSaveState->AppendBridge(call);
        }
        inst->GetBasicBlock()->InsertBefore(afterCallSaveState, inst);
        auto boxedValue = BoxValue(inst, call, afterCallSaveState);
        inst->ReplaceUsers(boxedValue);
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
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_CALL_THIS0:
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_CALL_THIS_SHORT:
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ANY_CALL_THIS_RANGE:
            visitor->HandleAnyCallThis(intrinsic);
            return;
    }
}

}  // namespace ark::compiler
