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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TO_STRING_CACHE
#define PANDA_PLUGINS_ETS_RUNTIME_TO_STRING_CACHE

#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_string.h"
namespace ark::ets {

namespace test {
class EtsToStringCacheTest;
}  // namespace test

namespace detail {

template <typename T>
class EtsToStringCacheElement;

template <typename T>
struct SimpleHash;

template <typename T, typename Derived, typename Hash = SimpleHash<T>>
class EtsToStringCache : public EtsTypedObjectArray<EtsToStringCacheElement<T>> {
public:
    static Derived *Create(EtsCoroutine *coro);

    EtsString *GetOrCache(EtsCoroutine *coro, T number);

private:
    static constexpr size_t SIZE = 256U;

    using Base = EtsTypedObjectArray<EtsToStringCacheElement<T>>;

    static uint32_t GetIndex(T number);
    EtsString *GetCached(EtsCoroutine *coro, T number, uint32_t index);
    void StoreToCache(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle, T number, uint32_t index);

    friend class ark::ets::test::EtsToStringCacheTest;
};

}  // namespace detail

class DoubleToStringCache : public detail::EtsToStringCache<EtsDouble, DoubleToStringCache> {};
class FloatToStringCache : public detail::EtsToStringCache<EtsFloat, FloatToStringCache> {};
class LongToStringCache : public detail::EtsToStringCache<EtsLong, LongToStringCache> {};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TO_STRING_CACHE
