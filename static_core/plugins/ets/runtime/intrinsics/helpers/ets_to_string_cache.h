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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TO_STRING_CACHE
#define PANDA_PLUGINS_ETS_RUNTIME_TO_STRING_CACHE

#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_string.h"
namespace ark::ets {

namespace test {
class EtsToStringCacheTest;
class EtsToStringCacheElementTest;
}  // namespace test

enum class ToStringResult : size_t { LOAD_CACHED = 0, STORE_NEW, STORE_UPDATE };

inline std::ostream &operator<<(std::ostream &out, ToStringResult res)
{
    static constexpr auto NAMES = std::array {"LOAD_CACHED", "STORE_NEW", "STORE_UPDATE"};
    ASSERT(static_cast<size_t>(res) < NAMES.size());
    return out << NAMES[static_cast<size_t>(res)];
}

namespace detail {

template <typename T>
struct SimpleHash;

template <typename T>
class EtsToStringCacheElement;

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

    EtsObject *AsObject()
    {
        return this;
    }

    static constexpr uint32_t GetStringOffset()
    {
        return MEMBER_OFFSET(EtsToStringCacheElement<T>, string_);
    }

    static constexpr uint32_t GetNumberOffset()
    {
        return MEMBER_OFFSET(EtsToStringCacheElement<T>, number_);
    }

    ToStringResult TryStore(EtsCoroutine *coro, EtsString *string, T number);

    EtsString *GetString(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetStringOffset());
        return EtsString::FromEtsObject(EtsObject::FromCoreType(obj));
    }

    T GetNumber() const
    {
        return number_;
    }

private:
    void SetString(EtsCoroutine *coro, EtsString *string);
    void SetNumber(T number);

private:
    ObjectPointer<EtsString> string_;
    T number_;

    friend class ark::ets::test::EtsToStringCacheTest;
};

template <typename T, typename Derived, typename Hash>
class EtsToStringCache;

template <typename T, typename Derived, typename Hash = SimpleHash<T>>
class EtsToStringCache : public EtsTypedObjectArray<EtsToStringCacheElement<T>> {
public:
    static Derived *Create(EtsCoroutine *coro);
    static Derived *FromCoreType(ObjectHeader *objectHeader)
    {
        return reinterpret_cast<Derived *>(objectHeader);
    }

    /**
     * @brief Compute representation of number and store to cache if possible
     * @param coro         Pointer to current coroutine
     * @param number       Number (double, float or int64) to get representation for
     * @param elem         Pointer to `EtsToStringCacheElement` loaded from cache
     */
    EtsString *CacheAndGetNoCheck(EtsCoroutine *coro, T number, ObjectHeader *elem);

    /**
     * @brief Load string representation of number from cache, or compute it and store to cache if possible
     * @param coro         Pointer to current coroutine
     * @param number       Number (double, float or int64) to get representation for
     */
    EtsString *GetOrCache(EtsCoroutine *coro, T number)
    {
        return GetOrCacheImpl(coro, number).first;
    }

    /**
     * @brief Get string representation of number ignoring the cache
     * @param number       Number (double, float or int64) to get representation for
     */
    static EtsString *GetNoCache(T number);

private:
    static constexpr uint32_t CACHE_SIZE_SHIFT = 8U;
    static constexpr size_t SIZE = 1U << CACHE_SIZE_SHIFT;

    using Elem = EtsToStringCacheElement<T>;
    using Base = EtsTypedObjectArray<Elem>;

    static uint32_t GetIndex(T number);
    void StoreToCache(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle, T number, uint32_t index);
    std::pair<EtsString *, ToStringResult> FinishUpdate(EtsCoroutine *coro, T number, EtsToStringCacheElement<T> *elem);
    std::pair<EtsString *, ToStringResult> GetOrCacheImpl(EtsCoroutine *coro, T number);

    friend class ark::ets::test::EtsToStringCacheTest;
};

}  // namespace detail

class DoubleToStringCache : public detail::EtsToStringCache<EtsDouble, DoubleToStringCache> {};
class FloatToStringCache : public detail::EtsToStringCache<EtsFloat, FloatToStringCache> {};
class LongToStringCache : public detail::EtsToStringCache<EtsLong, LongToStringCache> {};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TO_STRING_CACHE
