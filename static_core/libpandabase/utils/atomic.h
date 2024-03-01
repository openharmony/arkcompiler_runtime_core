/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_LIBPANDABASE_UTILS_ATOMIC_H
#define PANDA_LIBPANDABASE_UTILS_ATOMIC_H

#include <atomic>
#include "libpandabase/macros.h"

namespace ark {
template <typename T>
ALWAYS_INLINE inline void AtomicStore(T *addr, T val, std::memory_order order)
{
    // Atomic with parameterized order reason: memory order passed as argument
    reinterpret_cast<std::atomic<T> *>(addr)->store(val, order);
}

template <typename T>
ALWAYS_INLINE inline T AtomicLoad(T *addr, std::memory_order order)
{
    // Atomic with parameterized order reason: memory order passed as argument
    return reinterpret_cast<const std::atomic<T> *>(addr)->load(order);
}

template <typename T>
ALWAYS_INLINE inline T AtomicCmpxchgStrong(T *addr, T expected, T newValue, std::memory_order order)
{
    // Atomic with parameterized order reason: memory order passed as argument
    reinterpret_cast<std::atomic<T> *>(addr)->compare_exchange_strong(expected, newValue, order);
    return expected;
}
}  // namespace ark

#endif  // PANDA_LIBPANDABASE_UTILS_ATOMIC_H
