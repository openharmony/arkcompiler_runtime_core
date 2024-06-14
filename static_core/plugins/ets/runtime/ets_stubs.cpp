
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

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_bigint.h"

namespace ark::ets {

template <typename T>
static std::optional<T> GetBoxedNumericValue(EtsClassLinkerExtension *ext, EtsObject *obj)
{
    auto *cls = obj->GetClass()->GetRuntimeClass();

    auto const getValue = [obj](auto typeId) {
        using Type = typename decltype(typeId)::type;
        return static_cast<T>(EtsBoxPrimitive<Type>::FromCoreType(obj)->GetValue());
    };

    if (cls == ext->GetBoxDoubleClass()) {
        return getValue(helpers::TypeIdentity<EtsDouble>());
    }
    if (cls == ext->GetBoxIntClass()) {
        return getValue(helpers::TypeIdentity<EtsInt>());
    }

    if (cls == ext->GetBoxByteClass()) {
        return getValue(helpers::TypeIdentity<EtsByte>());
    }
    if (cls == ext->GetBoxShortClass()) {
        return getValue(helpers::TypeIdentity<EtsShort>());
    }
    if (cls == ext->GetBoxLongClass()) {
        return getValue(helpers::TypeIdentity<EtsLong>());
    }
    if (cls == ext->GetBoxFloatClass()) {
        return getValue(helpers::TypeIdentity<EtsFloat>());
    }
    return std::nullopt;
}

static bool EtsBigIntEquality(EtsBigInt *obj1, EtsBigInt *obj2)
{
    auto bytes1 = obj1->GetBytes();
    auto bytes2 = obj2->GetBytes();
    ASSERT(bytes1 != nullptr && bytes2 != nullptr);

    auto size1 = bytes1->GetActualLength();
    auto size2 = bytes2->GetActualLength();
    ASSERT(size1 != 0 && size2 != 0);
    auto data1 = bytes1->GetData();
    auto data2 = bytes2->GetData();
    ASSERT(data1 != nullptr && data2 != nullptr);
    ASSERT(data1->GetLength() >= size1 && data2->GetLength() >= size2);

    auto const readElem = [](auto *arr, uint32_t i) { return arr->Get(i)->GetValue(); };

    if (size1 != size2) {
        return false;
    }

    // start with check of MSD, which is sign.
    for (int i = size1 - 1; i >= 0; --i) {
        if (readElem(data1, i) != readElem(data2, i)) {
            return false;
        }
    }
    return true;
}

template <typename BoxedType>
static bool CompareBoxedPrimitive(EtsObject *obj1, EtsObject *obj2)
{
    return EtsBoxPrimitive<BoxedType>::FromCoreType(obj1)->GetValue() ==
           EtsBoxPrimitive<BoxedType>::FromCoreType(obj2)->GetValue();
}

bool EtsValueTypedEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2)
{
    auto ecls1 = obj1->GetClass();
    auto ecls2 = obj2->GetClass();
    auto cls1 = ecls1->GetRuntimeClass();
    auto cls2 = ecls2->GetRuntimeClass();
    ASSERT(ecls1->IsValueTyped() && ecls1->IsValueTyped());

    auto ext = coro->GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension();

    if (cls1->IsStringClass()) {
        return cls2->IsStringClass() &&
               coretypes::String::Cast(obj1->GetCoreType())->Compare(coretypes::String::Cast(obj2->GetCoreType())) == 0;
    }
    if (cls1 == ext->GetBoxBooleanClass()) {
        return cls2 == ext->GetBoxBooleanClass() && CompareBoxedPrimitive<EtsBoolean>(obj1, obj2);
    }
    if (cls1 == ext->GetBoxCharClass()) {
        return cls2 == ext->GetBoxCharClass() && CompareBoxedPrimitive<EtsChar>(obj1, obj2);
    }
    if (UNLIKELY(cls1 == ext->GetBigIntClass())) {
        return cls2 == ext->GetBigIntClass() &&
               EtsBigIntEquality(EtsBigInt::FromEtsObject(obj1), EtsBigInt::FromEtsObject(obj2));
    }
    if (cls1 == ext->GetBoxLongClass() && cls2 == ext->GetBoxLongClass()) {
        return CompareBoxedPrimitive<EtsLong>(obj1, obj2);
    }
    if (auto num1 = GetBoxedNumericValue<EtsDouble>(ext, obj1); num1.has_value()) {
        auto num2 = GetBoxedNumericValue<EtsDouble>(ext, obj2);
        return num2.has_value() && num2.value() == num1.value();
    }
    UNREACHABLE();
}

}  // namespace ark::ets
