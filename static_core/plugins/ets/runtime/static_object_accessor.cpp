/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <array>
#include "static_object_accessor.h"
#include "objects/base_object.h"
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"

namespace ark::ets {

StaticObjectAccessor StaticObjectAccessor::stcObjAccessor_;

bool StaticObjectAccessor::HasProperty([[maybe_unused]] panda::ThreadHolder *thread, const panda::BaseObject *obj,
                                       const char *name)
{
    const EtsObject *etsObj = reinterpret_cast<const EtsObject *>(obj);  // NOLINT(modernize-use-auto)
    return etsObj->GetClass()->GetFieldIDByName(name) != nullptr;
}

panda::BoxedValue GetPropertyValue(EtsCoroutine *coro, const EtsObject *etsObj, EtsField *field)
{
    EtsType type = field->GetEtsType();
    switch (type) {
        case EtsType::BOOLEAN: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsBoolean>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::BYTE: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsByte>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::CHAR: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsChar>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::SHORT: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsShort>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::INT: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsInt>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::LONG: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsLong>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::FLOAT: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsFloat>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::DOUBLE: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsDouble>(field);
            return reinterpret_cast<panda::BoxedValue>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::OBJECT: {
            return reinterpret_cast<panda::BoxedValue>(etsObj->GetFieldObject(field));
        }
        default:
            return nullptr;
    }
}

panda::BoxedValue StaticObjectAccessor::GetProperty([[maybe_unused]] panda::ThreadHolder *thread,
                                                    const panda::BaseObject *obj, const char *name)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    const EtsObject *etsObj = reinterpret_cast<const EtsObject *>(obj);  // NOLINT(modernize-use-auto)
    if (etsObj == nullptr) {
        return nullptr;
    }
    auto fieldIndex = etsObj->GetClass()->GetFieldIndexByName(name);
    EtsField *field = etsObj->GetClass()->GetFieldByIndex(fieldIndex);
    if (field == nullptr) {
        return nullptr;
    }
    return GetPropertyValue(coro, etsObj, field);
}

bool StaticObjectAccessor::SetProperty([[maybe_unused]] panda::ThreadHolder *thread, panda::BaseObject *obj,
                                       const char *name, panda::BoxedValue value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *etsObj = reinterpret_cast<EtsObject *>(obj);
    if (etsObj == nullptr) {
        return false;
    }
    ark::Value etsVal = GetUnboxedValue(coro, reinterpret_cast<EtsObject *>(value));
    auto fieldIndex = etsObj->GetClass()->GetFieldIndexByName(name);
    EtsField *field = etsObj->GetClass()->GetFieldByIndex(fieldIndex);
    if (field == nullptr) {
        return false;
    }
    EtsType type = field->GetEtsType();
    switch (type) {
        case EtsType::BOOLEAN:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsBoolean>());
            break;
        case EtsType::BYTE:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsByte>());
            break;
        case EtsType::CHAR:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsChar>());
            break;
        case EtsType::SHORT:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsShort>());
            break;
        case EtsType::INT:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsInt>());
            break;
        case EtsType::LONG:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsLong>());
            break;
        case EtsType::FLOAT:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsFloat>());
            break;
        case EtsType::DOUBLE:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsDouble>());
            break;
        case EtsType::OBJECT:
            etsObj->SetFieldObject(field, reinterpret_cast<EtsObject *>(value));
            break;
        default:
            return false;
    }
    return true;
}

bool StaticObjectAccessor::HasElementByIdx([[maybe_unused]] panda::ThreadHolder *thread,
                                           [[maybe_unused]] const panda::BaseObject *obj,
                                           [[maybe_unused]] const uint32_t index)
{
    LOG(ERROR, RUNTIME) << "HasElementByIdx has no meaning for static object";
    return false;
}

panda::BoxedValue StaticObjectAccessor::GetElementByIdx([[maybe_unused]] panda::ThreadHolder *thread,
                                                        const panda::BaseObject *obj, const uint32_t index)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    const EtsObject *etsObj = reinterpret_cast<const EtsObject *>(obj);  // NOLINT(modernize-use-auto)
    if (etsObj == nullptr) {
        return nullptr;
    }
    EtsObject *etsObject = const_cast<EtsObject *>(etsObj);  // NOLINT(modernize-use-auto)
    EtsMethod *method = etsObj->GetClass()->GetDirectMethod(GET_INDEX_METHOD, "I:Lstd/core/Object;");
    std::array args {ark::Value(reinterpret_cast<ObjectHeader *>(etsObject)), ark::Value(index)};
    ark::Value value = method->GetPandaMethod()->Invoke(coro, args.data());
    return reinterpret_cast<panda::BoxedValue>(EtsObject::FromCoreType(value.GetAs<ObjectHeader *>()));
}

bool StaticObjectAccessor::SetElementByIdx([[maybe_unused]] panda::ThreadHolder *thread, panda::BaseObject *obj,
                                           uint32_t index, const panda::BoxedValue value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *etsObj = reinterpret_cast<EtsObject *>(obj);
    if (etsObj == nullptr) {
        return false;
    }
    EtsMethod *method = etsObj->GetClass()->GetDirectMethod(SET_INDEX_METHOD, "ILstd/core/Object;:V");

    std::array args {ark::Value(reinterpret_cast<ObjectHeader *>(etsObj)), ark::Value(index),
                     ark::Value(reinterpret_cast<ObjectHeader *>(value))};
    method->GetPandaMethod()->Invoke(coro, args.data());
    if (UNLIKELY(coro->HasPendingException())) {
        return false;  // NOLINT(readability-simplify-boolean-expr)
    }
    return true;
}
}  // namespace ark::ets