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
#ifndef RUNTIME_INCLUDE_EVENT_CORO_CALLBACK_H
#define RUNTIME_INCLUDE_EVENT_CORO_CALLBACK_H

#include "libpandabase/os/mutex.h"
#include "runtime/include/callback_queue.h"
#include "runtime/include/runtime.h"

namespace ark {

class CoroCallbackQueue final : public CallbackQueue {
public:
    using CallbackPtr = PandaUniquePtr<Callback>;

    void Process() override
    {
        while (!IsEmpty()) {
            PandaList<CallbackPtr> readyCallbacks;
            {
                os::memory::LockHolder lh(queueLock_);
                readyCallbacks = std::move(queue_);
            }
            while (!readyCallbacks.empty()) {
                auto callback = std::move(readyCallbacks.front());
                callback->Run();
                readyCallbacks.pop_front();
            }
        }
    }

    void Post(CallbackPtr callback) override
    {
        os::memory::LockHolder lh(queueLock_);
        queue_.push_back(std::move(callback));
    }

    void PostSequence(PandaList<CallbackPtr> callbacks) override
    {
        os::memory::LockHolder lh(queueLock_);
        queue_.splice(std::prev(queue_.end()), std::move(callbacks));
    }

    bool IsEmpty() override
    {
        os::memory::LockHolder lh(queueLock_);
        return queue_.empty();
    }

    void Destroy() override
    {
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(this);
    }

private:
    os::memory::Mutex queueLock_;
    PandaList<CallbackPtr> queue_;
};

}  // namespace ark

#endif  // RUNTIME_INCLUDE_EVENT_CORO_CALLBACK_H
