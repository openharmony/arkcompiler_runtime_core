/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstddef>
#include "cross_values.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"

namespace ark::ets::intrinsics {

EtsObject *EtsEscompatArrayGet(ObjectHeader *arrayHeader, int32_t index)
{
    ASSERT(arrayHeader != nullptr);
    auto *array = EtsArrayObject<EtsObject>::FromEtsObject(EtsObject::FromCoreType(arrayHeader));
    auto actualLength = array->GetActualLength();
    if (UNLIKELY(static_cast<uint32_t>(index) >= actualLength)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "Out of bounds");
        return nullptr;
    }
    return array->GetData()->Get(index);
}

void EtsEscompatArraySet(ObjectHeader *arrayHeader, int32_t index, EtsObject *value)
{
    ASSERT(arrayHeader != nullptr);
    auto *array = EtsArrayObject<EtsObject>::FromEtsObject(EtsObject::FromCoreType(arrayHeader));
    auto actualLength = array->GetActualLength();
    if (UNLIKELY(static_cast<uint32_t>(index) >= actualLength)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "Out of bounds");
        return;
    }
    array->GetData()->Set(index, value);
}

template <typename Func>
constexpr auto CreateTypeMap(const EtsPlatformTypes *ptypes)
{
    return std::array {std::pair {ptypes->coreDouble, &Func::template Call<EtsDouble>},
                       std::pair {ptypes->coreInt, &Func::template Call<EtsInt>},
                       std::pair {ptypes->coreByte, &Func::template Call<EtsByte>},
                       std::pair {ptypes->coreShort, &Func::template Call<EtsShort>},
                       std::pair {ptypes->coreLong, &Func::template Call<EtsLong>},
                       std::pair {ptypes->coreFloat, &Func::template Call<EtsFloat>},
                       std::pair {ptypes->coreChar, &Func::template Call<EtsChar>},
                       std::pair {ptypes->coreBoolean, &Func::template Call<EtsBoolean>}};
}

EtsInt NormalizeArrayIndex(EtsInt index, EtsInt actualLength)
{
    if (index < -actualLength) {
        return 0;
    }

    if (index < 0) {
        return actualLength + index;
    }

    if (index > actualLength) {
        return actualLength;
    }

    return index;
}

EtsDouble EtsEscompatArrayIndexOfString(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex, EtsInt actualLength)
{
    auto valueString = coretypes::String::Cast(value->GetCoreType());
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->IsStringClass() &&
            valueString->Compare(coretypes::String::Cast(element->GetCoreType())) == 0) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfString(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    auto valueString = coretypes::String::Cast(value->GetCoreType());
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->IsStringClass() &&
            valueString->Compare(coretypes::String::Cast(element->GetCoreType())) == 0) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfEnum(EtsObjectArray *buffer, EtsCoroutine *coro, EtsObject *value, EtsInt fromIndex,
                                      EtsInt actualLength)
{
    auto *valueEnum = EtsBaseEnum::FromEtsObject(value)->GetValue();
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        auto elementClass = element->GetClass();
        if (elementClass->IsEtsEnum()) {
            auto *elementEnum = EtsBaseEnum::FromEtsObject(element)->GetValue();
            if (EtsReferenceEquals(coro, valueEnum, elementEnum)) {
                return index;
            }
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfEnum(EtsObjectArray *buffer, EtsCoroutine *coro, EtsObject *value,
                                          EtsInt fromIndex)
{
    auto *valueEnum = EtsBaseEnum::FromEtsObject(value)->GetValue();
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        auto elementClass = element->GetClass();
        if (elementClass->IsEtsEnum()) {
            auto *elementEnum = EtsBaseEnum::FromEtsObject(element)->GetValue();
            if (EtsReferenceEquals(coro, valueEnum, elementEnum)) {
                return index;
            }
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfNull(EtsObjectArray *buffer, EtsCoroutine *coroutine, EtsInt fromIndex,
                                      EtsInt actualLength)
{
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element == EtsObject::FromCoreType(coroutine->GetNullValue())) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfUndefined(EtsObjectArray *buffer, EtsInt fromIndex, EtsInt actualLength)
{
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element == nullptr) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfUndefined(EtsObjectArray *buffer, EtsInt fromIndex)
{
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element == nullptr) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfNull(EtsObjectArray *buffer, EtsCoroutine *coroutine, EtsInt fromIndex)
{
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element == EtsObject::FromCoreType(coroutine->GetNullValue())) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfBigInt(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex, EtsInt actualLength)
{
    auto valueBigInt = EtsBigInt::FromEtsObject(value);
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        auto elementClass = element == nullptr ? nullptr : element->GetClass();
        if (elementClass != nullptr && elementClass->IsBigInt() &&
            EtsBigIntEquality(valueBigInt, EtsBigInt::FromEtsObject(value))) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfBigInt(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    auto valueBigInt = EtsBigInt::FromEtsObject(value);
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        auto elementClass = element == nullptr ? nullptr : element->GetClass();
        if (elementClass != nullptr && elementClass->IsBigInt() &&
            EtsBigIntEquality(valueBigInt, EtsBigInt::FromEtsObject(value))) {
            return index;
        }
    }
    return -1;
}

template <typename T>
static auto GetNumericValue(EtsObject *element)
{
    return EtsBoxPrimitive<T>::FromCoreType(element)->GetValue();
}

template <typename T>
EtsDouble EtsEscompatArrayIndexOfNumeric(EtsObjectArray *buffer, EtsClass *valueCls, EtsObject *value, EtsInt fromIndex,
                                         EtsInt actualLength)
{
    auto valTyped = GetNumericValue<T>(value);
    if (std::isnan(valTyped)) {
        return -1;
    }

    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->GetClass() == valueCls && valTyped == GetNumericValue<T>(element)) {
            return index;
        }
    }
    return -1;
}

template <typename T>
EtsDouble EtsEscompatArrayLastIndexOfNumeric(EtsObjectArray *buffer, EtsClass *valueCls, EtsObject *value,
                                             EtsInt fromIndex)
{
    auto valTyped = GetNumericValue<T>(value);
    if (std::isnan(valTyped)) {
        return -1;
    }

    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->GetClass() == valueCls && valTyped == GetNumericValue<T>(element)) {
            return index;
        }
    }
    return -1;
}

template <typename T>
static EtsDouble DispatchIndexOf(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx, EtsInt len)
{
    return EtsEscompatArrayIndexOfNumeric<T>(buf, cls, val, idx, len);
}

template <typename T>
static EtsDouble DispatchLastIndexOf(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx)
{
    return EtsEscompatArrayLastIndexOfNumeric<T>(buf, cls, val, idx);
}

struct IndexOfDispatcher {
    template <typename T>
    static EtsDouble Call(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx, EtsInt len)
    {
        return DispatchIndexOf<T>(buf, cls, val, idx, len);
    }
};

struct LastIndexOfDispatcher {
    template <typename T>
    static EtsDouble Call(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx)
    {
        return DispatchLastIndexOf<T>(buf, cls, val, idx);
    }
};

EtsDouble EtsEscompatArrayIndexOfCommon(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex, EtsInt actualLength)
{
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element == value) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfCommon(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element == value) {
            return index;
        }
    }
    return -1;
}

EtsInt EtsEscompatArrayInternalIndexOfImpl(ObjectHeader *arrayHeader, EtsObject *value, EtsInt fromIndex)
{
    auto actualLength = ObjectAccessor::GetPrimitive<EtsInt>(
        arrayHeader, cross_values::GetEscompatArrayActualLengthOffset(RUNTIME_ARCH));
    fromIndex = NormalizeArrayIndex(fromIndex, actualLength);
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
    ObjectHeader *bufferObjectHeader =
        ObjectAccessor::GetObject(arrayHeader, cross_values::GetEscompatArrayBufferOffset(RUNTIME_ARCH));
    EtsObjectArray *buffer = EtsObjectArray::FromCoreType(bufferObjectHeader);

    if (value == nullptr) {
        return EtsEscompatArrayIndexOfUndefined(buffer, fromIndex, actualLength);
    }

    auto valueCls = value->GetClass();
    static const auto TYPE_MAP = CreateTypeMap<IndexOfDispatcher>(ptypes);
    for (const auto &[type, handler] : TYPE_MAP) {
        if (valueCls == type) {
            return handler(buffer, valueCls, value, fromIndex, actualLength);
        }
    }

    if (valueCls->IsBigInt()) {
        return EtsEscompatArrayIndexOfBigInt(buffer, value, fromIndex, actualLength);
    }

    if (valueCls->IsStringClass()) {
        return EtsEscompatArrayIndexOfString(buffer, value, fromIndex, actualLength);
    }

    if (valueCls->IsEtsEnum()) {
        return EtsEscompatArrayIndexOfEnum(buffer, coroutine, value, fromIndex, actualLength);
    }

    if (value == EtsObject::FromCoreType(coroutine->GetNullValue())) {
        return EtsEscompatArrayIndexOfNull(buffer, coroutine, fromIndex, actualLength);
    }

    return EtsEscompatArrayIndexOfCommon(buffer, value, fromIndex, actualLength);
}

extern "C" EtsInt EtsEscompatArrayInternalIndexOf(ObjectHeader *arrayHeader, EtsObject *value, EtsInt fromIndex)
{
    return EtsEscompatArrayInternalIndexOfImpl(arrayHeader, value, fromIndex);
}

extern "C" EtsDouble EtsEscompatArrayIndexOf(ObjectHeader *arrayHeader, EtsObject *value)
{
    return EtsEscompatArrayInternalIndexOfImpl(arrayHeader, value, 0);
}

EtsInt EtsEscompatArrayInternalLastIndexOfImpl(ObjectHeader *arrayHeader, EtsObject *value, EtsInt fromIndex)
{
    auto actualLength = ObjectAccessor::GetPrimitive<EtsInt>(
        arrayHeader, cross_values::GetEscompatArrayActualLengthOffset(RUNTIME_ARCH));
    if (actualLength == 0) {
        return -1;
    }

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
    ObjectHeader *bufferObjectHeader =
        ObjectAccessor::GetObject(arrayHeader, cross_values::GetEscompatArrayBufferOffset(RUNTIME_ARCH));
    EtsObjectArray *buffer = EtsObjectArray::FromCoreType(bufferObjectHeader);
    EtsInt startIndex = 0;

    if (fromIndex >= 0) {
        startIndex = std::min(actualLength - 1, fromIndex);
    } else {
        startIndex = actualLength + fromIndex;
    }

    if (value == nullptr) {
        return EtsEscompatArrayLastIndexOfUndefined(buffer, startIndex);
    }

    auto valueCls = value->GetClass();
    static const auto TYPE_MAP = CreateTypeMap<LastIndexOfDispatcher>(ptypes);
    for (const auto &[type, handler] : TYPE_MAP) {
        if (valueCls == type) {
            return handler(buffer, valueCls, value, startIndex);
        }
    }

    if (valueCls->IsBigInt()) {
        return EtsEscompatArrayLastIndexOfBigInt(buffer, value, startIndex);
    }

    if (valueCls->IsStringClass()) {
        return EtsEscompatArrayLastIndexOfString(buffer, value, startIndex);
    }

    if (valueCls->IsEtsEnum()) {
        return EtsEscompatArrayLastIndexOfEnum(buffer, coroutine, value, startIndex);
    }

    if (value == EtsObject::FromCoreType(coroutine->GetNullValue())) {
        return EtsEscompatArrayLastIndexOfNull(buffer, coroutine, startIndex);
    }

    return EtsEscompatArrayLastIndexOfCommon(buffer, value, startIndex);
}

extern "C" EtsInt EtsEscompatArrayInternalLastIndexOf(ObjectHeader *arrayHeader, EtsObject *value, EtsInt fromIndex)
{
    return EtsEscompatArrayInternalLastIndexOfImpl(arrayHeader, value, fromIndex);
}

extern "C" EtsDouble EtsEscompatArrayLastIndexOf(ObjectHeader *arrayHeader, EtsObject *value)
{
    auto actualLength = ObjectAccessor::GetPrimitive<EtsInt>(
        arrayHeader, cross_values::GetEscompatArrayActualLengthOffset(RUNTIME_ARCH));
    return EtsEscompatArrayInternalLastIndexOfImpl(arrayHeader, value, actualLength - 1);
}

static uint32_t NormalizeIndex(int32_t idx, int64_t len)
{
    if (idx < 0) {
        return idx < -len ? 0 : idx + len;
    }
    return idx > len ? len : idx;
}

extern "C" ObjectHeader *EtsEscompatArrayFill(ObjectHeader *arrayHeader, EtsObject *value, int32_t start, int32_t end)
{
    ASSERT(arrayHeader != nullptr);
    auto *array = EtsArrayObject<EtsObject>::FromEtsObject(EtsObject::FromCoreType(arrayHeader));
    auto actualLength = static_cast<int64_t>(array->GetActualLength());
    auto startInd = NormalizeIndex(start, actualLength);
    auto endInd = NormalizeIndex(end, actualLength);
    array->GetData()->Fill(value, startInd, endInd);
    return arrayHeader;
}
}  // namespace ark::ets::intrinsics
