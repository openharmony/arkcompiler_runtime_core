/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_BASE_ATOMIC_SPINLOCK_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_BASE_ATOMIC_SPINLOCK_H

#include <atomic>

#include "common_interfaces/base/common.h"

namespace ark::common_vm {
class AtomicSpinLock {
public:
    AtomicSpinLock() {}
    ~AtomicSpinLock() = default;

    void Lock()
    {
        // Atomic with acquire order reason: lock acquisition requires acquire semantics to see prior releases
        while (state.test_and_set(std::memory_order_acquire)) {
        }
    }

    void Unlock()
    {
        // Atomic with release order reason: lock release requires release semantics to make prior writes visible
        state.clear(std::memory_order_release);
    }

    bool TryLock()
    {
        // Atomic with acquire order reason: lock acquisition requires acquire semantics to see prior releases
        return (state.test_and_set(std::memory_order_acquire) == false);
    }

private:
    std::atomic_flag state = ATOMIC_FLAG_INIT;

    NO_COPY_SEMANTIC(AtomicSpinLock);
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_BASE_ATOMIC_SPINLOCK_H
