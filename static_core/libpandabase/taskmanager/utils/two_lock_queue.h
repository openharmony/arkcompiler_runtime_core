/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_TWO_LOCK_QUEUE
#define LIBPANDABASE_TASKMANAGER_UTILS_TWO_LOCK_QUEUE

#include "libpandabase/taskmanager/utils/sp_sc_lock_free_queue.h"
#include "libpandabase/os/mutex.h"

namespace ark::taskmanager::internal {

template <class T, class Allocator>
class TwoLockQueue {
public:
    void Push(T &&val)
    {
        os::memory::LockHolder lh(pushLock_);
        queue_.Push(std::move(val));
    }

    T Pop()
    {
        os::memory::LockHolder lh(popLock_);
        return queue_.Pop();
    }

    bool TryPop(T *val)
    {
        os::memory::LockHolder lh(popLock_);
        return queue_.TryPop(val);
    }

    bool IsEmpty() const
    {
        return queue_.IsEmpty();
    }

    size_t Size() const
    {
        return queue_.Size();
    }

private:
    using InternalTaskQueue = SPSCLockFreeQueue<T, Allocator>;

    mutable os::memory::Mutex pushLock_;
    mutable os::memory::Mutex popLock_;
    InternalTaskQueue queue_;
};

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_TWO_LOCK_QUEUE
