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
#ifndef RUNTIME_INCLUDE_CALLBACK_QUEUE_H
#define RUNTIME_INCLUDE_CALLBACK_QUEUE_H

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark {
class Callback {
public:
    Callback() = default;
    DEFAULT_COPY_SEMANTIC(Callback);
    DEFAULT_MOVE_SEMANTIC(Callback);
    virtual ~Callback() = default;

    virtual void Run() = 0;
};

class CallbackQueue {
public:
    CallbackQueue() = default;
    NO_COPY_SEMANTIC(CallbackQueue);
    NO_MOVE_SEMANTIC(CallbackQueue);
    virtual ~CallbackQueue() = default;

    /// Post callback to the queue
    virtual void Post(PandaUniquePtr<Callback> callback) = 0;

    /// Post sequence of callbacks to the queue
    virtual void PostSequence(PandaList<PandaUniquePtr<Callback>> callbacks)
    {
        while (!callbacks.empty()) {
            Post(std::move(callbacks.front()));
            callbacks.pop_front();
        }
    }

    /// Process the queue callbacks until it is empty
    virtual void Process() {}

    /// Get status of queue
    virtual bool IsEmpty()
    {
        return true;
    }

    /// Destroy callback queue
    virtual void Destroy() {}
};

}  // namespace ark

#endif  // RUNTIME_INCLUDE_CALLBACK_QUEUE_H
