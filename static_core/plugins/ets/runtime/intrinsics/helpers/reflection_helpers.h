/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_REFLECTION_HELPERS_
#define PANDA_PLUGINS_ETS_REFLECTION_HELPERS_

#include "runtime/include/managed_thread.h"
#include "libarkbase/utils/utils.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics::helpers {

struct PrimitiveFieldInfo {
    EtsObject *thisObj = nullptr;
    EtsObject *arg = nullptr;
    EtsClass *argClass = nullptr;
    EtsType fieldType = EtsType::UNKNOWN;
    EtsField *field = nullptr;
};

bool CheckReceiverType(EtsExecutionContext *executionCtx, EtsObject *arg, EtsClass *paramClass, Value *argValue);
EtsObject *InvokeAndResolveReturnValue(EtsMethod *method, EtsExecutionContext *executionCtx, Value *args);
EtsMethod *ValidateAndResolveInstanceMethod(EtsExecutionContext *executionCtx, EtsObject *thisObj, EtsMethod *method);
bool CheckPrimitiveReciever(EtsExecutionContext *executionCtx, EtsObject *arg, EtsClass *argClass, EtsClass *paramClass,
                            Value *argValue);
bool ValidateInstanceField(EtsExecutionContext *executionCtx, EtsObject *thisObj, EtsField *field);

template <typename T>
bool CheckAndUnpackBoxedType(EtsClassLinkerExtension *linkExt, EtsObject *arg, EtsClass *paramClass, Value *argValue,
                             ClassRoot primitiveRoot)
{
    if (paramClass != EtsClass::FromRuntimeClass(linkExt->GetClassRoot(primitiveRoot))) {
        return false;
    }
    *argValue = Value(EtsBoxPrimitive<T>::Unbox(arg));
    return true;
}

inline void ReflectFieldSetReference(EtsExecutionContext *executionCtx, EtsObject *thisObj, EtsObject *arg,
                                     EtsField *field)
{
    EtsHandle argH(executionCtx, arg);
    EtsHandle thisObjH(executionCtx, thisObj);
    if (field->IsStatic()) {
        field->GetDeclaringClass()->SetStaticFieldObject(field, arg);
    } else if (ValidateInstanceField(executionCtx, thisObj, field)) {
        thisObjH->SetFieldObject(field, argH.GetPtr());
    }
}

template <typename T>
bool ReflectFieldSetPrimitive(EtsExecutionContext *executionCtx, EtsObject *thisObj, T primVal, EtsField *field)
{
    EtsHandle thisObjH(executionCtx, thisObj);
    if (field->IsStatic()) {
        field->GetDeclaringClass()->SetStaticFieldPrimitive<T>(field, primVal);
        return true;
    }
    if (ValidateInstanceField(executionCtx, thisObjH.GetPtr(), field)) {
        thisObjH->template SetFieldPrimitive<T>(field, primVal);
        return true;
    }
    return false;
}

inline EtsObject *ReflectFieldGetReference(EtsObject *thisObj, EtsField *field)
{
    if (field->IsStatic()) {
        return field->GetDeclaringClass()->GetStaticFieldObject(field);
    }
    return thisObj->GetFieldObject(field);
}

template <typename T>
Value ReflectFieldGetPrimitive(EtsObject *thisObj, EtsField *field)
{
    if (field->IsStatic()) {
        return Value(field->GetDeclaringClass()->GetStaticFieldPrimitive<T>(field));
    }
    return Value(thisObj->GetFieldPrimitive<T>(field));
}

bool ResolveAndSetPrimitive(EtsExecutionContext *executionCtx, const PrimitiveFieldInfo &info);
Value GetPrimitiveValue(EtsExecutionContext *executionCtx, EtsObject *thisObj, EtsType fieldType, EtsField *field);

EtsBoolean IsLiteralInitializedInterface(EtsObject *target);

class InstanceMethodsIterator final {
public:
    explicit InstanceMethodsIterator(EtsClass *cls, bool publicOnly) : publicOnly_(publicOnly)
    {
        auto *rtClass = cls->GetRuntimeClass();
        vMethods_ = rtClass->GetVirtualMethods();
        cMethods_ = rtClass->GetCopiedMethods();
        iter_ = vMethods_.begin();
        isCMethod_ = false;
        Advance();
    }

    bool IsEmpty() const
    {
        return isCMethod_ && iter_ == cMethods_.end();
    }

    EtsMethod *Value()
    {
        ASSERT(!IsEmpty());
        return EtsMethod::FromRuntimeMethod(iter_);
    }

    void Next()
    {
        ASSERT(!IsEmpty());
        ++iter_;
        Advance();
    }

private:
    void Advance()
    {
        if (!isCMethod_) {
            while (iter_ != vMethods_.end() && ShouldSkip()) {
                ++iter_;
            }
            if (iter_ == vMethods_.end()) {
                iter_ = cMethods_.begin();
                isCMethod_ = true;
            }
        }
        if (isCMethod_) {
            while (iter_ != cMethods_.end() && ShouldSkip()) {
                ++iter_;
            }
        }
    }

    bool ShouldSkip() const
    {
        return iter_->IsConstructor() || (publicOnly_ && !iter_->IsPublic());
    }

private:
    Span<Method> vMethods_ {};
    Span<Method> cMethods_ {};
    Span<Method>::Iterator iter_ {};
    bool isCMethod_ {};
    bool publicOnly_ {};
};

}  // namespace ark::ets::intrinsics::helpers

#endif  // PANDA_PLUGINS_ETS_REFLECTION_HELPERS_
