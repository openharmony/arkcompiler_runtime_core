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

#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H

#include "runtime/include/runtime.h"
#include "runtime/coroutines/local_storage.h"

namespace ark {

/// Represents a coroutine worker, which can host multiple coroutines and schedule them.
class CoroutineWorker {
public:
    enum class DataIdx { INTEROP_CTX_PTR, EXTERNAL_IFACES, LAST_ID };
    using LocalStorage = StaticLocalStorage<DataIdx>;

    NO_COPY_SEMANTIC(CoroutineWorker);
    NO_MOVE_SEMANTIC(CoroutineWorker);

    CoroutineWorker(Runtime *runtime, PandaVM *vm) : runtime_(runtime), vm_(vm) {}
    virtual ~CoroutineWorker() = default;

    Runtime *GetRuntime()
    {
        return runtime_;
    }

    PandaVM *GetPandaVM() const
    {
        return vm_;
    }

    LocalStorage &GetLocalStorage()
    {
        return localStorage_;
    }

private:
    Runtime *runtime_ = nullptr;
    PandaVM *vm_ = nullptr;
    LocalStorage localStorage_;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H */
