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

#ifndef COMMON_INTERFACES_OBJECTS_FIELD_H
#define COMMON_INTERFACES_OBJECTS_FIELD_H

#include <atomic>

namespace common {
class BaseObject;

using MemoryOrder = std::memory_order;
// T is primitive field
template <typename T, bool isAtomic = false>
class Field {
public:
    // the interfaces fellow are only used in atomic operation.
    bool CompareExchange(T expectedValue, T newValue, std::memory_order succOrder = std::memory_order_relaxed,
                         MemoryOrder failOrder = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        // compiling failure:
        //     std::atomic_compare_exchange_strong_explicit(&value, &expectedValue, newValue, order, order)
        return __atomic_compare_exchange(&value, &expectedValue, &newValue, false, succOrder, failOrder);
    }

    T Exchange(T newValue, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        T ret;
        __atomic_exchange(&value, &newValue, &ret, order);
        return ret;
    }

    T FetchAdd(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        return __atomic_fetch_add(&value, val, order);
    }

    T FetchSub(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        return __atomic_fetch_sub(&value, val, order);
    }

    T FetchAnd(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        return __atomic_fetch_and(&value, val, order);
    }

    T FetchOr(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        return __atomic_fetch_or(&value, val, order);
    }

    T FetchXor(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        return __atomic_fetch_xor(&value, val, order);
    }

private:
    T value;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_REF_FIELD_H
