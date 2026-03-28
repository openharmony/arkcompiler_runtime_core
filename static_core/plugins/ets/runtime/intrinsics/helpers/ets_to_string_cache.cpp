/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include <charconv>
#include "ets_class_linker_extension.h"
#include "ets_to_string_cache.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "libarkbase/mem/mem.h"

namespace ark::ets::detail {

template <typename T>
ToStringResult EtsToStringCacheElement<T>::TryStore(EtsExecutionContext *executionCtx, EtsString *string, T number)
{
    ASSERT(executionCtx != nullptr);
    SetString(executionCtx, string);
    SetNumber(number);
    return ToStringResult::STORE_UPDATE;
}

/* static */
template <typename T>
EtsClass *EtsToStringCacheElement<T>::GetClass(EtsExecutionContext *executionCtx)
{
    auto *pt = PlatformTypes(executionCtx);
    if constexpr (std::is_same_v<T, EtsDouble>) {
        return pt->coreDoubleToStringCacheElement;
    } else if constexpr (std::is_same_v<T, EtsFloat>) {
        return pt->coreFloatToStringCacheElement;
    } else if constexpr (std::is_same_v<T, EtsLong>) {
        return pt->coreLongToStringCacheElement;
    } else {
        UNREACHABLE();
    }
}

/* static */
template <typename T>
EtsToStringCacheElement<T> *EtsToStringCacheElement<T>::Create(EtsExecutionContext *executionCtx,
                                                               EtsHandle<EtsString> &stringHandle, T number,
                                                               EtsClass *klass)
{
    auto *instance = FromCoreType(EtsObject::Create(executionCtx, klass)->GetCoreType());
    instance->SetString(executionCtx, stringHandle.GetPtr());
    instance->SetNumber(number);
    return instance;
}

template <typename T>
void EtsToStringCacheElement<T>::SetString(EtsExecutionContext *executionCtx, EtsString *string)
{
    ASSERT(string != nullptr);
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, GetStringOffset(), string->GetCoreType());
}

template <typename T>
void EtsToStringCacheElement<T>::SetNumber(T number)
{
    number_ = number;
}

template <typename T>
EtsString *ToString(T number)
{
    if constexpr (std::is_same_v<T, EtsLong>) {
        // 1 for first digit and 1 for zero
        constexpr auto MAX_DIGITS = std::numeric_limits<int64_t>::digits10 + 2U;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        std::array<char, MAX_DIGITS + 1> buf;
        auto [stringEnd, result] = std::to_chars(buf.begin(), buf.end(), number);
        ASSERT(result == std::errc());
        return EtsString::CreateFromAscii(buf.begin(), stringEnd - buf.begin());
    } else {
        static_assert(std::is_floating_point_v<T>);
        return intrinsics::helpers::FpToStringDecimalRadix(
            number, [](std::string_view str) { return EtsString::CreateFromAscii(str.data(), str.length()); });
    }
}

template <typename T>
struct SimpleHash {
    static constexpr uint32_t CACHE_SIZE_SHIFT = 8U;
    static constexpr uint32_t T_BITS = sizeof(T) * BITS_PER_BYTE;
    using UnsignedIntType = ark::helpers::TypeHelperT<T_BITS, false>;
    static constexpr uint32_t SHIFT = T_BITS - CACHE_SIZE_SHIFT;

    static constexpr UnsignedIntType GetMul()
    {
        if constexpr (std::is_same_v<UnsignedIntType, uint32_t>) {
            constexpr auto MUL = 0x5bd1e995U;
            return MUL;
        } else {
            constexpr auto MUL = 0xc6a4a7935bd1e995ULL;
            return MUL;
        }
    }

    size_t operator()(T number) const
    {
        auto res = (bit_cast<UnsignedIntType>(number) * GetMul()) >> SHIFT;
        ASSERT(res < (1U << CACHE_SIZE_SHIFT));
        return static_cast<size_t>(res);
    }
};

/* static */
template <typename T, typename Derived, typename Hash>
uint32_t EtsToStringCache<T, Derived, Hash>::GetIndex(T number)
{
    auto index = Hash()(number);
    ASSERT(index < SIZE);
    return index;
}

template <typename T, typename Derived, typename Hash>
std::pair<EtsString *, ToStringResult> EtsToStringCache<T, Derived, Hash>::FinishUpdate(
    EtsExecutionContext *executionCtx, T number, EtsToStringCacheElement<T> *elem)
{
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsToStringCacheElement<T>> elemHandle(executionCtx, elem);
    // may trigger GC
    auto *string = ToString(number);
    ASSERT(string != nullptr);
    ASSERT(elemHandle.GetPtr() != nullptr);
    auto storeRes = elemHandle->TryStore(executionCtx, string, number);
    return {string, storeRes};
}

template <typename T, typename Derived, typename Hash>
std::pair<EtsString *, ToStringResult> EtsToStringCache<T, Derived, Hash>::GetOrCacheImpl(
    EtsExecutionContext *executionCtx, T number)
{
    EVENT_ETS_CACHE("Fastpath: create string from number with cache");
    auto index = GetIndex(number);
    auto *elem = Base::Get(index);
    if (UNLIKELY(elem == nullptr)) {
        [[maybe_unused]] EtsHandleScope scope(executionCtx);
        EtsHandle<EtsString> string(executionCtx, ToString(number));
        ASSERT(string.GetPtr() != nullptr);
        // may trigger GC
        StoreToCache(executionCtx, string, number, index);
        ASSERT(string.GetPtr() != nullptr);
        return {string.GetPtr(), ToStringResult::STORE_NEW};
    }
    auto cachedNumber = elem->GetNumber();
    if (LIKELY(cachedNumber == number)) {
        return {elem->GetString(executionCtx), ToStringResult::LOAD_CACHED};
    }
    return FinishUpdate(executionCtx, number, elem);
}

template <typename T, typename Derived, typename Hash>
EtsString *EtsToStringCache<T, Derived, Hash>::CacheAndGetNoCheck(EtsExecutionContext *executionCtx, T number,
                                                                  ObjectHeader *elem)
{
    ASSERT(elem != nullptr);
    EVENT_ETS_CACHE("Fastpath: create string from number with cache");
    return FinishUpdate(executionCtx, number, Elem::FromCoreType(elem)).first;
}

/* static */
template <typename T, typename Derived, typename Hash>
EtsString *EtsToStringCache<T, Derived, Hash>::GetNoCache(T number)
{
    EVENT_ETS_CACHE("Slowpath: create string from number without cache");
    return ToString(number);
}

/* static */
template <typename T, typename Derived, typename Hash>
Derived *EtsToStringCache<T, Derived, Hash>::Create(EtsExecutionContext *executionCtx)
{
    auto *etsClass = Elem::GetClass(executionCtx);
    if (etsClass == nullptr) {
        return nullptr;
    }
    return static_cast<Derived *>(Base::Create(etsClass, SIZE, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT));
}

template <typename T, typename Derived, typename Hash>
void EtsToStringCache<T, Derived, Hash>::StoreToCache(EtsExecutionContext *executionCtx,
                                                      EtsHandle<EtsString> &stringHandle, T number, uint32_t index)
{
    auto *arrayClass = Base::GetCoreType()->template ClassAddr<Class>();
    auto *elemClass = arrayClass->GetComponentType();
    ASSERT(elemClass->GetObjectSize() == Elem::GetNumberOffset() + sizeof(T));
    auto *elem = Elem::Create(executionCtx, stringHandle, number, EtsClass::FromRuntimeClass(elemClass));
    Base::Set(index, elem);
}

template class EtsToStringCache<EtsDouble, DoubleToStringCache>;
template class EtsToStringCache<EtsFloat, FloatToStringCache>;
template class EtsToStringCache<EtsLong, LongToStringCache>;

}  // namespace ark::ets::detail
