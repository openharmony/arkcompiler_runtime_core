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

#include <charconv>
#include <cstddef>
#include "cross_values.h"
#include <endian.h>
#include "runtime/include/coretypes/string.h"
#include "intrinsics.h"
#include "libpandabase/utils/utf.h"
#include "libpandabase/utils/utils.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "utils/utils.h"

namespace ark::ets::intrinsics {

EtsObject *EtsEscompatArrayGet(EtsEscompatArray *array, int32_t index)
{
    ASSERT(array != nullptr);
    auto actualLength = array->GetActualLength();
    if (UNLIKELY(static_cast<uint32_t>(index) >= actualLength)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "Out of bounds");
        return nullptr;
    }
    return array->GetData()->Get(index);
}

void EtsEscompatArraySet(EtsEscompatArray *array, int32_t index, EtsObject *value)
{
    ASSERT(array != nullptr);
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
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    auto valueString = coretypes::String::Cast(value->GetCoreType());
    VMHandle<coretypes::String> strHandle(coroutine, valueString);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->IsStringClass() &&
            strHandle->Compare(coretypes::String::Cast(element->GetCoreType()), ctx) == 0) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfString(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    auto valueString = coretypes::String::Cast(value->GetCoreType());
    VMHandle<coretypes::String> strHandle(coroutine, valueString);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->IsStringClass() &&
            strHandle->Compare(coretypes::String::Cast(element->GetCoreType()), ctx) == 0) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfEnum(EtsObjectArray *buffer, EtsCoroutine *coro, EtsObject *value, EtsInt fromIndex,
                                      EtsInt actualLength)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *valueEnum = EtsBaseEnum::FromEtsObject(value)->GetValue();
    VMHandle<EtsBaseEnum> valueEnumHandle(coro, valueEnum->GetCoreType());
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        auto elementClass = element->GetClass();
        if (elementClass->IsEtsEnum()) {
            auto *elementEnum = EtsBaseEnum::FromEtsObject(element)->GetValue();
            if (EtsReferenceEquals(coro, valueEnumHandle.GetPtr(), elementEnum)) {
                return index;
            }
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfEnum(EtsObjectArray *buffer, EtsCoroutine *coro, EtsObject *value,
                                          EtsInt fromIndex)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *valueEnum = EtsBaseEnum::FromEtsObject(value)->GetValue();
    VMHandle<EtsBaseEnum> valueEnumHandle(coro, valueEnum->GetCoreType());
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        auto elementClass = element->GetClass();
        if (elementClass->IsEtsEnum()) {
            auto *elementEnum = EtsBaseEnum::FromEtsObject(element)->GetValue();
            if (EtsReferenceEquals(coro, valueEnumHandle.GetPtr(), elementEnum)) {
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
            EtsBigIntEquality(valueBigInt, EtsBigInt::FromEtsObject(element))) {
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
            EtsBigIntEquality(valueBigInt, EtsBigInt::FromEtsObject(element))) {
            return index;
        }
    }
    return -1;
}

template <typename T>
static auto GetNumericValue(EtsObject *element)
{
    return EtsBoxPrimitive<T>::Unbox(element);
}

static bool IsIntegerClass(EtsClass *objClass, const EtsPlatformTypes *ptypes)
{
    return (objClass == ptypes->coreLong || objClass == ptypes->coreByte || objClass == ptypes->coreShort ||
            objClass == ptypes->coreInt);
}

static bool IsNullOrUndefined(EtsHandle<EtsObject> elementHandle)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    return (elementHandle.GetPtr() == nullptr ||
            elementHandle.GetPtr() == EtsObject::FromCoreType(coroutine->GetNullValue()));
}

static bool IsNumericClass(EtsClass *objClass, const EtsPlatformTypes *ptypes)
{
    return (objClass == ptypes->coreDouble || objClass == ptypes->coreFloat);
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

EtsInt EtsEscompatArrayInternalIndexOfImpl(EtsEscompatArray *array, EtsObject *value, EtsInt fromIndex)
{
    auto actualLength = static_cast<int32_t>(array->GetActualLength());
    fromIndex = NormalizeArrayIndex(fromIndex, actualLength);
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
    EtsObjectArray *buffer = array->GetData();

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

extern "C" EtsInt EtsEscompatArrayInternalIndexOf(EtsEscompatArray *array, EtsObject *value, EtsInt fromIndex)
{
    return EtsEscompatArrayInternalIndexOfImpl(array, value, fromIndex);
}

extern "C" EtsDouble EtsEscompatArrayIndexOf(EtsEscompatArray *array, EtsObject *value)
{
    return EtsEscompatArrayInternalIndexOfImpl(array, value, 0);
}

EtsInt EtsEscompatArrayInternalLastIndexOfImpl(EtsEscompatArray *array, EtsObject *value, EtsInt fromIndex)
{
    auto actualLength = static_cast<EtsInt>(array->GetActualLength());
    if (actualLength == 0) {
        return -1;
    }

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
    EtsObjectArray *buffer = array->GetData();
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

extern "C" EtsInt EtsEscompatArrayInternalLastIndexOf(EtsEscompatArray *array, EtsObject *value, EtsInt fromIndex)
{
    return EtsEscompatArrayInternalLastIndexOfImpl(array, value, fromIndex);
}

extern "C" EtsDouble EtsEscompatArrayLastIndexOf(EtsEscompatArray *array, EtsObject *value)
{
    auto actualLength = static_cast<EtsInt>(array->GetActualLength());
    return EtsEscompatArrayInternalLastIndexOfImpl(array, value, actualLength - 1);
}

static uint32_t NormalizeIndex(int32_t idx, int64_t len)
{
    if (idx < 0) {
        return idx < -len ? 0 : idx + len;
    }
    return idx > len ? len : idx;
}

extern "C" EtsEscompatArray *EtsEscompatArrayFill(EtsEscompatArray *array, EtsObject *value, int32_t start, int32_t end)
{
    ASSERT(array != nullptr);
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsEscompatArray> arrayHandle(coro, array);
    auto actualLength = static_cast<int64_t>(array->GetActualLength());
    auto startInd = NormalizeIndex(start, actualLength);
    auto endInd = NormalizeIndex(end, actualLength);
    arrayHandle->GetData()->Fill(value, startInd, endInd);
    return arrayHandle.GetPtr();
}

struct ElementComputeResult {
    EtsInt intCount = 0;
    EtsInt doubleCount = 0;
    EtsInt stringCount = 0;
    EtsInt utf8Size = 0;
    EtsInt utf16Size = 0;
};

ark::ets::EtsString *GetObjStr(EtsHandle<EtsObject> elementHandle)
{
    auto cls = elementHandle->GetClass();
    auto coro = EtsCoroutine::GetCurrent();
    auto const performCall = [coro, &elementHandle](EtsMethod *method) {
        ASSERT(method != nullptr);
        std::array<Value, 1> args = {Value(elementHandle->GetCoreType())};
        EtsObject *callRes = EtsObject::FromCoreType(
            EtsMethod::ToRuntimeMethod(method)->Invoke(coro, args.data()).GetAs<ObjectHeader *>());
        if (coro->HasPendingException() || callRes == EtsObject::FromCoreType(coro->GetNullValue())) {
            return static_cast<EtsObject *>(nullptr);
        }
        return callRes;
    };

    auto toStrMethod = cls->GetInstanceMethod("toString", ":Lstd/core/String;");
    if (toStrMethod == nullptr) {
        return EtsString::CreateNewEmptyString();
    }

    auto etsStr = performCall(toStrMethod);
    if (etsStr == nullptr) {
        return nullptr;
    }

    auto objStr = EtsString::FromEtsObject(etsStr);
    return objStr;
}

void ComputeObjStrSize(EtsHandle<EtsObject> elementHandle, ElementComputeResult &res)
{
    auto objStr = GetObjStr(elementHandle);
    if (objStr->IsUtf16()) {
        auto utf16StrSize = objStr->GetUtf16Length();
        res.utf16Size += utf16StrSize;
    } else {
        auto utf8StrSize = objStr->GetUtf8Length();
        res.utf8Size += utf8StrSize;
    }
}

template <typename T>
constexpr size_t MaxChars()
{
    static_assert(std::numeric_limits<T>::radix == 2U);
    if constexpr (std::is_integral_v<T>) {
        auto bits = std::numeric_limits<T>::digits;
        auto digitPerBit = std::log10(std::numeric_limits<T>::radix);
        auto digits = std::ceil(bits * digitPerBit) + static_cast<int>(std::is_signed_v<T>);
        return digits;
    } else {
        static_assert(std::is_floating_point_v<T>);
        // Forced to treat T as double
        auto bits = std::numeric_limits<double>::digits;
        auto digitPerBit = std::log10(std::numeric_limits<double>::radix);
        auto digits = std::ceil(bits * digitPerBit) + static_cast<int>(std::is_signed_v<double>);
        // MaxChars for double
        // integer part + decimal point + digits + e+308
        //     1        +     1         +   17   +   5
        return 1U + 1U + digits + std::char_traits<char>::length("+e308");
    }
}

void ComputeElementCharSize(ElementComputeResult &res, EtsObject *element, const EtsPlatformTypes *ptypes)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    EtsHandle<EtsObject> elementHandle(coroutine, element);
    const int booleanUtf8Size = 5;
    if (IsNullOrUndefined(elementHandle)) {
        return;
    }

    auto elementCls = elementHandle->GetClass();
    if (elementCls->IsStringClass()) {
        auto strElement = coretypes::String::Cast(elementHandle->GetCoreType());
        if (strElement->IsUtf16()) {
            auto utf16StrSize = strElement->GetUtf16Length();
            res.utf16Size += utf16StrSize;
        } else {
            auto utf8StrSize = strElement->GetUtf8Length();
            res.utf8Size += utf8StrSize;
        }
        res.stringCount++;
    } else if (IsIntegerClass(elementCls, ptypes)) {
        res.intCount++;
    } else if (IsNumericClass(elementCls, ptypes)) {
        res.doubleCount++;
    } else if (elementCls == ptypes->coreChar) {
        res.utf16Size++;
    } else if (elementCls == ptypes->coreBoolean) {
        res.utf8Size += booleanUtf8Size;
    } else {
        ComputeObjStrSize(elementHandle, res);
    }
}

template <typename T>
size_t NumberToChars(PandaVector<EtsChar> &buf, size_t pos, T number)
{
    if constexpr (std::is_integral_v<T>) {
        auto num = number < 0 ? -static_cast<uint64_t>(number) : static_cast<uint64_t>(number);
        auto nDigits = (CountDigits(num) + static_cast<uint32_t>(number < 0));
        ASSERT(pos + nDigits <= buf.size());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf::UInt64ToUtf16Array(num, buf.data() + pos, nDigits, number < 0);
        return nDigits;
    } else {
        static_assert(std::is_floating_point_v<T>);
        static_assert(std::is_unsigned_v<EtsChar>);
        // Forced to treat T as double
        auto asDouble = static_cast<double>(number);
        return intrinsics::helpers::FpToStringDecimalRadix(asDouble, [&buf, &pos](std::string_view str) {
            ASSERT(str.length() <= MaxChars<T>());
            for (size_t i = 0; i < str.length(); i++) {
                buf[pos + i] = static_cast<EtsChar>(str[i]);
            }
            return str.length();
        });
    }
}

template <typename T>
size_t NumberToChars(PandaVector<char> &buf, size_t pos, T number)
{
    if constexpr (std::is_integral_v<T>) {
        auto [strEnd, result] = std::to_chars(&buf[pos], &buf[pos + MaxChars<T>()], number);
        ASSERT(result == std::errc());
        return strEnd - &buf[pos];
    } else {
        static_assert(std::is_floating_point_v<T>);
        // Forced to treat T as double
        auto asDouble = static_cast<double>(number);
        return intrinsics::helpers::FpToStringDecimalRadix(asDouble, [&buf, &pos](std::string_view str) {
            ASSERT(str.length() <= MaxChars<T>());
            auto err = memcpy_s(&buf[pos], MaxChars<T>(), str.data(), str.length());
            if (err != EOK) {
                UNREACHABLE();
            }
            return str.length();
        });
    }
}

template <typename T, typename BufferType>
void HandleNumericType(EtsHandle<EtsObject> elementHandle, BufferType &buf, size_t &pos)
{
    auto value = GetNumericValue<T>(elementHandle.GetPtr());
    pos += NumberToChars<T>(buf, pos, value);
}

template <typename BufferType>
using NumericHandler = void (*)(EtsHandle<EtsObject>, BufferType &, size_t &);

template <typename BufferType>
static constexpr auto GetNumericHandlerTable(const EtsPlatformTypes *ptypes)
{
    return std::array {std::pair {ptypes->coreInt, &HandleNumericType<EtsInt, BufferType>},
                       std::pair {ptypes->coreDouble, &HandleNumericType<EtsDouble, BufferType>},
                       std::pair {ptypes->coreByte, &HandleNumericType<EtsByte, BufferType>},
                       std::pair {ptypes->coreShort, &HandleNumericType<EtsShort, BufferType>},
                       std::pair {ptypes->coreLong, &HandleNumericType<EtsLong, BufferType>},
                       std::pair {ptypes->coreFloat, &HandleNumericType<EtsFloat, BufferType>}};
}

void ComputeCharSize(EtsObjectArray *buffer, EtsInt actualLength, ElementComputeResult &res,
                     const EtsPlatformTypes *ptypes, EtsHandle<EtsString> separatorHandle)
{
    res.intCount = 0;
    res.doubleCount = 0;
    res.stringCount = 0;
    res.utf8Size = 0;
    res.utf16Size = 0;

    for (EtsInt index = 0; index < actualLength; index++) {
        auto element = buffer->Get(index);
        ComputeElementCharSize(res, element, ptypes);
    }

    res.utf8Size = res.utf8Size + res.intCount * MaxChars<EtsLong>() + res.doubleCount * MaxChars<EtsDouble>();
    if (separatorHandle.GetPtr()->IsUtf16()) {
        res.utf16Size += separatorHandle.GetPtr()->GetUtf16Length() * (actualLength - 1);
    } else {
        res.utf8Size += separatorHandle.GetPtr()->GetUtf8Length() * (actualLength - 1);
    }
}

ark::ets::EtsString *EtsEscompatArrayJoinUtf8String(EtsObjectArray *buffer, EtsInt &actualLength, EtsInt &utf8Size,
                                                    EtsHandle<EtsString> separatorHandle)
{
    EtsString *s = EtsString::AllocateNonInitializedString(utf8Size, true);
    uint8_t *dstData = s->GetDataMUtf8();
    const size_t sepSize = separatorHandle.GetPtr()->GetUtf8Length();

    for (EtsInt i = 0; i < actualLength - 1; i++) {
        EtsObject *str = buffer->Get(i);
        coretypes::String *srcString = coretypes::String::Cast(str->GetCoreType());
        uint32_t n = srcString->CopyDataRegionMUtf8(dstData, 0, srcString->GetLength(), utf8Size);
        dstData += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf8Size -= n;
        if (sepSize > 0) {
            uint32_t len = separatorHandle.GetPtr()->CopyDataRegionMUtf8(
                dstData, 0, separatorHandle.GetPtr()->GetLength(), utf8Size);
            dstData += len;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            utf8Size -= len;
        }
    }

    EtsObject *lastStr = buffer->Get(actualLength - 1);
    coretypes::String *lastString = coretypes::String::Cast(lastStr->GetCoreType());
    lastString->CopyDataRegionMUtf8(dstData, 0, lastString->GetLength(), utf8Size);
    return s;
}

void ProcessUtf8Element(EtsObject *element, PandaVector<char> &buf, const EtsPlatformTypes *ptypes, size_t &pos,
                        EtsInt &utf8Size)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    EtsHandle<EtsObject> elementHandle(coroutine, element);
    if (IsNullOrUndefined(elementHandle)) {
        return;
    }

    auto elementCls = elementHandle->GetClass();
    const auto &numericHandlerTable = GetNumericHandlerTable<PandaVector<char>>(ptypes);
    for (const auto &entry : numericHandlerTable) {
        if (entry.first == elementCls) {
            entry.second(elementHandle, buf, pos);
            return;
        }
    }

    if (elementCls->IsStringClass()) {
        auto strElement = coretypes::String::Cast(elementHandle->GetCoreType());
        auto str = strElement->GetDataUtf8();
        auto elementSize = strElement->GetUtf8Length();
        if (elementSize > 0) {
            memcpy_s(&buf[pos], utf8Size, str, elementSize);
            pos += elementSize;
        }
    } else if (elementCls == ptypes->coreBoolean) {
        auto value = GetNumericValue<EtsBoolean>(elementHandle.GetPtr());
        const char *text = (value == 1) ? "true" : "false";
        const size_t len = (value == 1) ? 4 : 5;
        memcpy_s(&buf[pos], utf8Size, text, len);
        pos += len;
    } else {
        auto objStr = GetObjStr(elementHandle);
        auto objStrData = objStr->GetDataUtf8();
        auto objStrSize = objStr->GetUtf8Length();
        if (objStrSize > 0) {
            memcpy_s(&buf[pos], utf8Size, objStrData, objStrSize);
            pos += objStrSize;
        }
    }
}

ark::ets::EtsString *EtsEscompatArrayJoinUtf8(EtsObjectArray *buffer, EtsInt &actualLength, ElementComputeResult &res,
                                              const EtsPlatformTypes *ptypes, EtsHandle<EtsString> separatorHandle)
{
    if (res.utf8Size == 0) {
        return EtsString::CreateNewEmptyString();
    }

    if (res.stringCount == actualLength) {
        return EtsEscompatArrayJoinUtf8String(buffer, actualLength, res.utf8Size, separatorHandle);
    }

    PandaVector<char> buf(res.utf8Size + 1);
    const size_t sepSize = separatorHandle.GetPtr()->GetUtf8Length();
    size_t pos = 0;
    for (EtsInt index = 0; index < actualLength - 1; index++) {
        auto element = buffer->Get(index);
        ProcessUtf8Element(element, buf, ptypes, pos, res.utf8Size);

        if (sepSize > 0) {
            MemcpyUnsafe(&buf[pos], separatorHandle.GetPtr()->GetDataUtf8(), sepSize);
            pos += sepSize;
        }
    }

    auto lastElement = buffer->Get(actualLength - 1);
    ProcessUtf8Element(lastElement, buf, ptypes, pos, res.utf8Size);

    return EtsString::CreateFromUtf8(buf.data(), pos);
}

bool ProcessUtf16Element(EtsObject *element, PandaVector<EtsChar> &buf, const EtsPlatformTypes *ptypes, size_t &pos)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    EtsHandle<EtsObject> elementHandle(coroutine, element);
    if (IsNullOrUndefined(elementHandle)) {
        return true;
    }

    auto elementCls = elementHandle->GetClass();
    const auto &numericHandlerTable = GetNumericHandlerTable<PandaVector<EtsChar>>(ptypes);
    for (const auto &entry : numericHandlerTable) {
        if (entry.first == elementCls) {
            entry.second(elementHandle, buf, pos);
            return true;
        }
    }

    if (elementCls->IsStringClass()) {
        auto strElement = coretypes::String::Cast(elementHandle->GetCoreType());
        auto strSize = strElement->IsUtf16() ? strElement->GetUtf16Length() : strElement->GetUtf8Length();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        strElement->CopyDataUtf16(buf.data() + pos, strSize);
        pos += strSize;
    } else if (elementCls == ptypes->coreChar) {
        auto value = GetNumericValue<EtsChar>(elementHandle.GetPtr());
        buf[pos] = value;
        pos++;
    } else if (elementCls == ptypes->coreBoolean) {
        auto value = GetNumericValue<EtsBoolean>(elementHandle.GetPtr());
        const char *text = (value == 1) ? "true" : "false";
        const size_t len = (value == 1) ? 4 : 5;
        for (uint32_t i = 0; i < len; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            buf[i + pos] = text[i];
        }
        pos += len;
    } else {
        auto objStr = GetObjStr(elementHandle);
        if (objStr == nullptr) {
            return false;
        }
        auto objStrSize = objStr->IsUtf16() ? objStr->GetUtf16Length() : objStr->GetUtf8Length();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        objStr->CopyDataUtf16(buf.data() + pos, objStrSize);
        pos += objStrSize;
    }
    return true;
}

ark::ets::EtsString *EtsEscompatArrayJoinUtf16(EtsObjectArray *buffer, EtsInt &actualLength, ElementComputeResult &res,
                                               const EtsPlatformTypes *ptypes, EtsHandle<EtsString> separatorHandle)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    EtsHandle<EtsObjectArray> bufferHandle(coroutine, buffer);
    res.utf16Size =
        res.utf16Size + res.utf8Size + res.intCount * MaxChars<EtsLong>() + res.doubleCount * MaxChars<EtsDouble>();
    PandaVector<EtsChar> buf(res.utf16Size);
    auto separator = separatorHandle.GetPtr();
    auto sepSize = separator->IsUtf16() ? separator->GetUtf16Length() : separator->GetUtf8Length();
    size_t pos = 0;
    for (EtsInt index = 0; index < actualLength - 1; index++) {
        auto element = bufferHandle->Get(index);
        if (!ProcessUtf16Element(element, buf, ptypes, pos)) {
            return nullptr;
        }

        if (sepSize > 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            separatorHandle.GetPtr()->CopyDataUtf16(buf.data() + pos, sepSize);
            pos += sepSize;
        }
    }

    auto lastElement = bufferHandle->Get(actualLength - 1);
    if (!ProcessUtf16Element(lastElement, buf, ptypes, pos)) {
        return nullptr;
    }

    return EtsString::CreateFromUtf16(reinterpret_cast<EtsChar *>(buf.data()), pos);
}

extern "C" ark::ets::EtsString *EtsEscompatArrayJoinInternal(EtsEscompatArray *array, ark::ets::EtsString *separator)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
    EtsObjectArray *buffer = array->GetData();
    auto actualLength = static_cast<EtsInt>(array->GetActualLength());
    ElementComputeResult res;
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsString> separatorHandle(coroutine, separator);

    ComputeCharSize(buffer, actualLength, res, ptypes, separatorHandle);

    if (res.utf16Size > 0) {
        return EtsEscompatArrayJoinUtf16(buffer, actualLength, res, ptypes, separatorHandle);
    }

    return EtsEscompatArrayJoinUtf8(buffer, actualLength, res, ptypes, separatorHandle);
}

extern "C" EtsObject *EtsEscompatArrayGetUnsafe(EtsEscompatArray *array, int32_t index)
{
    ASSERT(array != nullptr);
    [[maybe_unused]] auto length = array->GetData()->GetLength();
    ASSERT(static_cast<uint32_t>(index) < length);
    return array->GetData()->Get(index);
}

extern "C" void EtsEscompatArraySetUnsafe(EtsEscompatArray *array, int32_t index, EtsObject *value)
{
    ASSERT(array != nullptr);
    [[maybe_unused]] auto actualLength = array->GetActualLength();
    ASSERT(static_cast<uint32_t>(index) < actualLength);
    array->GetData()->Set(index, value);
}
}  // namespace ark::ets::intrinsics
