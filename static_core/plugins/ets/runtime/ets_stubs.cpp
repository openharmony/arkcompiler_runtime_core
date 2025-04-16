
/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

template <typename T>
static std::optional<T> GetBoxedNumericValue(EtsPlatformTypes const *ptypes, EtsObject *obj)
{
    auto *cls = obj->GetClass();

    auto const getValue = [obj](auto typeId) {
        using Type = typename decltype(typeId)::type;
        return static_cast<T>(EtsBoxPrimitive<Type>::FromCoreType(obj)->GetValue());
    };

    if (cls == ptypes->coreDouble) {
        return getValue(helpers::TypeIdentity<EtsDouble>());
    }
    if (cls == ptypes->coreInt) {
        return getValue(helpers::TypeIdentity<EtsInt>());
    }

    if (cls == ptypes->coreByte) {
        return getValue(helpers::TypeIdentity<EtsByte>());
    }
    if (cls == ptypes->coreShort) {
        return getValue(helpers::TypeIdentity<EtsShort>());
    }
    if (cls == ptypes->coreLong) {
        return getValue(helpers::TypeIdentity<EtsLong>());
    }
    if (cls == ptypes->coreFloat) {
        return getValue(helpers::TypeIdentity<EtsFloat>());
    }
    if (cls == ptypes->coreChar) {
        return getValue(helpers::TypeIdentity<EtsChar>());
    }
    return std::nullopt;
}

static bool EtsBigIntEquality(EtsBigInt *obj1, EtsBigInt *obj2)
{
    auto bytes1 = obj1->GetBytes();
    auto bytes2 = obj2->GetBytes();
    ASSERT(bytes1 != nullptr && bytes2 != nullptr);

    auto size1 = bytes1->GetLength();
    auto size2 = bytes2->GetLength();
    if (size1 != size2) {
        return false;
    }

    if (obj1->GetSign() != obj2->GetSign()) {
        return false;
    }

    return (std::memcmp(bytes1->GetCoreType()->GetData(), bytes2->GetCoreType()->GetData(), size1 * sizeof(EtsInt)) ==
            0);
}

template <typename BoxedType>
static bool CompareBoxedPrimitive(EtsObject *obj1, EtsObject *obj2)
{
    return EtsBoxPrimitive<BoxedType>::FromCoreType(obj1)->GetValue() ==
           EtsBoxPrimitive<BoxedType>::FromCoreType(obj2)->GetValue();
}

bool EtsValueTypedEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2)
{
    auto cls1 = obj1->GetClass();
    auto cls2 = obj2->GetClass();
    ASSERT(cls1->IsValueTyped() && cls1->IsValueTyped());

    auto ptypes = PlatformTypes(coro);
    ASSERT(ptypes != nullptr);

    if (cls1->IsStringClass()) {
        return cls2->IsStringClass() &&
               coretypes::String::Cast(obj1->GetCoreType())->Compare(coretypes::String::Cast(obj2->GetCoreType())) == 0;
    }
    if (cls1 == ptypes->coreBoolean) {
        return cls2 == ptypes->coreBoolean && CompareBoxedPrimitive<EtsBoolean>(obj1, obj2);
    }
    if (cls1 == ptypes->coreChar) {
        return cls2 == ptypes->coreChar && CompareBoxedPrimitive<EtsChar>(obj1, obj2);
    }
    if (UNLIKELY(cls1->IsBigInt())) {
        return cls2->IsBigInt() && EtsBigIntEquality(EtsBigInt::FromEtsObject(obj1), EtsBigInt::FromEtsObject(obj2));
    }
    if (cls1 == ptypes->coreLong && cls2 == ptypes->coreLong) {
        return CompareBoxedPrimitive<EtsLong>(obj1, obj2);
    }
    if (auto num1 = GetBoxedNumericValue<EtsDouble>(ptypes, obj1); num1.has_value()) {
        auto num2 = GetBoxedNumericValue<EtsDouble>(ptypes, obj2);
        return num2.has_value() && num2.value() == num1.value();
    }
    if (cls1->IsEtsEnum()) {
        if (UNLIKELY(!cls2->IsEtsEnum())) {
            return false;
        }
        auto *value1 = EtsBaseEnum::FromEtsObject(obj1)->GetValue();
        auto *value2 = EtsBaseEnum::FromEtsObject(obj2)->GetValue();
        if (UNLIKELY(value1->GetClass()->IsEtsEnum() || value2->GetClass()->IsEtsEnum())) {
            return false;
        }
        return EtsReferenceEquals(coro, value1, value2);
    }
    UNREACHABLE();
}

[[maybe_unused]] static bool DbgIsBoxedNumericClass(EtsCoroutine *coro, EtsClass *cls)
{
    auto ptypes = PlatformTypes(coro);
    return cls == ptypes->coreByte || cls == ptypes->coreChar || cls == ptypes->coreShort || cls == ptypes->coreInt ||
           cls == ptypes->coreLong || cls == ptypes->coreFloat || cls == ptypes->coreDouble;
}

EtsString *EtsGetTypeof(EtsCoroutine *coro, EtsObject *obj)
{
    // NOTE(vpukhov): #19799 use string constants
    if (obj == nullptr) {
        return EtsString::CreateFromMUtf8("undefined");
    }
    EtsClass *cls = obj->GetClass();

    // NOTE(vpukhov): re-encode subtyping flags if necessary
    if (!cls->IsValueTyped()) {
        if (cls->IsFunction()) {
            return EtsString::CreateFromMUtf8("function");
        }
        return EtsString::CreateFromMUtf8("object");
    }

    if (cls->IsNullValue()) {
        return EtsString::CreateFromMUtf8("object");
    }
    if (obj->IsStringClass()) {
        return EtsString::CreateFromMUtf8("string");
    }
    if (cls->IsBigInt()) {
        return EtsString::CreateFromMUtf8("bigint");
    }
    if (cls->IsEtsEnum()) {
        auto *value = EtsBaseEnum::FromEtsObject(obj)->GetValue();
        if (UNLIKELY(value->GetClass()->IsEtsEnum())) {
            // This situation is unexpected from language point of view. If BaseEnum object is contained
            // as value of enum, then it's treated as object
            return EtsString::CreateFromMUtf8("object");
        }
        return EtsGetTypeof(coro, EtsBaseEnum::FromEtsObject(obj)->GetValue());
    }

    ASSERT(cls->IsBoxed());

    if (cls == PlatformTypes(coro)->coreBoolean) {
        return EtsString::CreateFromMUtf8("boolean");
    }

    ASSERT(DbgIsBoxedNumericClass(coro, cls));
    return EtsString::CreateFromMUtf8("number");
}

bool EtsGetIstrue(EtsCoroutine *coro, EtsObject *obj)
{
    if (IsReferenceNullish(coro, obj)) {
        return false;
    }
    EtsClass *cls = obj->GetClass();

    if (!cls->IsValueTyped()) {
        return true;
    }
    if (obj->IsStringClass()) {
        return !EtsString::FromEtsObject(obj)->IsEmpty();
    }
    if (cls->IsBigInt()) {
        return EtsBigInt::FromEtsObject(obj)->GetSign() != 0;
    }
    if (cls->IsEtsEnum()) {
        auto *value = EtsBaseEnum::FromEtsObject(obj)->GetValue();
        if (UNLIKELY(value->GetClass()->IsEtsEnum())) {
            // This situation is unexpected from language point of view. If BaseEnum object is contained
            // as value of enum, then it's treated as object
            return true;
        }
        return EtsGetIstrue(coro, value);
    }

    ASSERT(cls->IsBoxed());

    auto ptypes = PlatformTypes(coro);
    if (cls == ptypes->coreBoolean) {
        return EtsBoxPrimitive<EtsBoolean>::FromCoreType(obj)->GetValue() != 0;
    }

    ASSERT(DbgIsBoxedNumericClass(coro, cls));
    if (auto num = GetBoxedNumericValue<EtsDouble>(ptypes, obj); num.has_value()) {
        return num.value() != 0 && !std::isnan(num.value());
    }
    UNREACHABLE();
}

}  // namespace ark::ets
