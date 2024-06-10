/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ets_intrinsics_helpers.h"
#include "utils/logger.h"

namespace ark::ets::detail {

template <typename T>
class EtsToStringCacheElement : public EtsObject {
public:
    ~EtsToStringCacheElement() = default;
    NO_COPY_SEMANTIC(EtsToStringCacheElement);
    NO_MOVE_SEMANTIC(EtsToStringCacheElement);

    static EtsClass *GetClass(EtsCoroutine *coro);

    static EtsToStringCacheElement<T> *Create(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle, T number,
                                              EtsClass *klass);

    static EtsToStringCacheElement<T> *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsToStringCacheElement<T> *>(obj);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    EtsString *GetString(EtsCoroutine *coro);

    T GetNumber() const
    {
        return number_;
    }

private:
    void SetString(EtsCoroutine *coro, EtsString *string);

    void SetNumber(T number)
    {
        number_ = number;
    }

private:
    ObjectPointer<EtsString> string_;
    T number_;
};

/* static */
template <typename T>
EtsClass *EtsToStringCacheElement<T>::GetClass(EtsCoroutine *coro)
{
    auto *classLinker = coro->GetPandaVM()->GetClassLinker();
    auto *ext = classLinker->GetEtsClassLinkerExtension();

    std::string_view classDescriptor;
    if constexpr (std::is_same_v<T, EtsDouble>) {
        classDescriptor = panda_file_items::class_descriptors::DOUBLE_TO_STRING_CACHE_ELEMENT;
    } else if constexpr (std::is_same_v<T, EtsFloat>) {
        classDescriptor = panda_file_items::class_descriptors::FLOAT_TO_STRING_CACHE_ELEMENT;
    } else if constexpr (std::is_same_v<T, EtsLong>) {
        classDescriptor = panda_file_items::class_descriptors::LONG_TO_STRING_CACHE_ELEMENT;
    } else {
        UNREACHABLE();
    }
    return classLinker->GetClass(classDescriptor.data(), false, ext->GetBootContext());
}

/* static */
template <typename T>
EtsToStringCacheElement<T> *EtsToStringCacheElement<T>::Create(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle,
                                                               T number, EtsClass *klass)
{
    auto *instance = FromCoreType(EtsObject::Create(coro, klass)->GetCoreType());
    instance->SetString(coro, stringHandle.GetPtr());
    instance->SetNumber(number);
    return instance;
}

template <typename T>
EtsString *EtsToStringCacheElement<T>::GetString(EtsCoroutine *coro)
{
    auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsToStringCacheElement<T>, string_));
    return EtsString::FromEtsObject(EtsObject::FromCoreType(obj));
}

template <typename T>
void EtsToStringCacheElement<T>::SetString(EtsCoroutine *coro, EtsString *string)
{
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsToStringCacheElement<T>, string_), string->GetCoreType());
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
    size_t operator()(T number) const
    {
        return std::hash<T>()(number);
    }
};

/* static */
template <typename T, typename Derived, typename Hash>
uint32_t EtsToStringCache<T, Derived, Hash>::GetIndex(T number)
{
    return Hash()(number) & (SIZE - 1);
}

template <typename T, typename Derived, typename Hash>
EtsString *EtsToStringCache<T, Derived, Hash>::GetOrCache(EtsCoroutine *coro, T number)
{
    auto index = GetIndex(number);
    if (auto string = GetCached(coro, number, index); string != nullptr) {
        return string;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsString> string(coro, ToString(number));
    ASSERT(string.GetPtr() != nullptr);
    StoreToCache(coro, string, number, index);
    ASSERT(string.GetPtr() != nullptr);
    return string.GetPtr();
}

/* static */
template <typename T, typename Derived, typename Hash>
Derived *EtsToStringCache<T, Derived, Hash>::Create(EtsCoroutine *coro)
{
    auto *etsClass = detail::EtsToStringCacheElement<T>::GetClass(coro);
    if (etsClass == nullptr) {
        return nullptr;
    }
    return static_cast<Derived *>(Base::Create(etsClass, SIZE, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT));
}

template <typename T, typename Derived, typename Hash>
EtsString *EtsToStringCache<T, Derived, Hash>::GetCached(EtsCoroutine *coro, T number, uint32_t index)
{
    auto *elem = Base::Get(index, std::memory_order_acquire);
    if (elem != nullptr && elem->GetNumber() == number) {
        LOG(DEBUG, RUNTIME) << "ToString cache hit: " << number;
        return elem->GetString(coro);
    }
    if (elem == nullptr) {
        LOG(DEBUG, RUNTIME) << "ToString cache miss: " << number;
    } else {
        LOG(DEBUG, RUNTIME) << "ToString cache update, old: " << elem->GetNumber() << "new: " << number;
    }
    return nullptr;
}

template <typename T, typename Derived, typename Hash>
void EtsToStringCache<T, Derived, Hash>::StoreToCache(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle, T number,
                                                      uint32_t index)
{
    auto *arrayClass = Base::GetCoreType()->template ClassAddr<Class>();
    auto *elemClass = arrayClass->GetComponentType();
    ASSERT(elemClass->GetObjectSize() == sizeof(EtsToStringCacheElement<T>));
    auto *elem = EtsToStringCacheElement<T>::Create(coro, stringHandle, number, EtsClass::FromRuntimeClass(elemClass));
    // Atomic with release order reason: writes to elem should be visible in other threads
    Base::Set(index, elem, std::memory_order_release);
}

template class EtsToStringCache<EtsDouble, DoubleToStringCache>;
template class EtsToStringCache<EtsFloat, FloatToStringCache>;
template class EtsToStringCache<EtsLong, LongToStringCache>;

}  // namespace ark::ets::detail
