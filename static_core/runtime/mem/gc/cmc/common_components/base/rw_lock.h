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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_BASE_RWLOCK_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_BASE_RWLOCK_H

#include <atomic>

#include "libarkbase/utils/logger.h"

namespace ark::common_vm {
class RwLock {
public:
    void LockRead()
    {
        // Atomic with acquire order reason: data race with lockCount_ with dependecies on reads after the load
        int count = lockCount_.load(std::memory_order_acquire);
        do {
            while (count == WRITE_LOCKED) {
                sched_yield();
                // Atomic with acquire order reason: data race with lockCount_ with dependecies on reads after the load
                count = lockCount_.load(std::memory_order_acquire);
            }
            // Atomic with release order reason: lock acquisition requires release semantics to make prior writes
            // visible
        } while (!lockCount_.compare_exchange_weak(count, count + 1, std::memory_order_release));
    }

    void LockWrite()
    {
        // Atomic with release order reason: lock acquisition requires release semantics to make prior writes visible
        for (int count = 0; !lockCount_.compare_exchange_weak(count, WRITE_LOCKED, std::memory_order_release);
             count = 0) {
            sched_yield();
        }
    }

    bool TryLockWrite()
    {
        int count = 0;
        // Atomic with release order reason: lock acquisition requires release semantics to make prior writes visible
        if (lockCount_.compare_exchange_weak(count, WRITE_LOCKED, std::memory_order_release)) {
            return true;
        }
        return false;
    }

    bool TryLockRead()
    {
        // Atomic with acquire order reason: data race with lockCount_ with dependecies on reads after the load
        int count = lockCount_.load(std::memory_order_acquire);
        do {
            if (count == WRITE_LOCKED) {
                return false;
            }
            // Atomic with release order reason: lock acquisition requires release semantics to make prior writes
            // visible
        } while (!lockCount_.compare_exchange_weak(count, count + 1, std::memory_order_release));
        return true;
    }

    void UnlockRead()
    {
        // Atomic with acq_rel order reason: lock release requires acquire-release semantics to synchronize with
        // other lock/unlock operations
        int count = lockCount_.fetch_sub(1, std::memory_order_acq_rel);
        if (count < 0) {  // LCOV_EXCL_BR_LINE
            LOG(FATAL, COMMON) << "Unresolved fatal";
            UNREACHABLE();
        }
    }

    void UnlockWrite()
    {
        // Atomic with seq_cst order reason: debug assertion to verify write lock is held with requirement for
        // sequentially consistent order
        CHECK(lockCount_.load(std::memory_order_seq_cst) == WRITE_LOCKED);
        // Atomic with release order reason: lock release requires release semantics to make prior writes visible
        lockCount_.store(0, std::memory_order_release);
    }

private:
    // 0: unlocked
    // -1: write locked
    // inc 1 for each read lock
    static constexpr int WRITE_LOCKED = -1;
    std::atomic<int> lockCount_ {0};
};
}  // namespace ark::common_vm
#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_BASE_SPINLOCK_H
