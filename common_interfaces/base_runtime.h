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

#ifndef COMMON_INTERFACES_BASE_RUNTIME_H
#define COMMON_INTERFACES_BASE_RUNTIME_H

#include <atomic>
#include <functional>
#include <mutex>

#include "base/runtime_param.h"

namespace panda {
class BaseObject;
class HeapManager;
class LogManager;
class MutatorManager;
class ThreadHolderManager;
class ThreadHolder;

enum class GcType : uint8_t {
    ASYNC,
    SYNC,
    FULL,  // Waiting finish
};
using HeapVisitor = const std::function<void(BaseObject*)>;

class PUBLIC_API BaseRuntime {
public:
    BaseRuntime() = default;
    ~BaseRuntime() = default;

    static BaseRuntime *GetInstance();
    static void DestroyInstance();

    void PreFork(ThreadHolder *holder);
    void PostFork();

    void Init(const RuntimeParam &param);   // Support setting custom parameters
    void Init();                            // Use default parameters
    void Fini();

    // Need refactor, move to other file
    static void WriteBarrier(void* obj, void* field, void* ref);
    static void* ReadBarrier(void* obj, void* field);
    static void* ReadBarrier(void* field);
    static void* AtomicReadBarrier(void* obj, void* field, std::memory_order order);
    static void RequestGC(GcType type);
    static bool ForEachObj(HeapVisitor& visitor, bool safe);

    HeapParam &GetHeapParam()
    {
        return param_.heapParam;
    }

    GCParam &GetGCParam()
    {
        return param_.gcParam;
    }

    MutatorManager &GetMutatorManager()
    {
        return *mutatorManager_;
    }

    ThreadHolderManager &GetThreadHolderManager()
    {
        return *threadHolderManager_;
    }

private:
    RuntimeParam param_ {};

    HeapManager* heapManager_ = nullptr;
    LogManager* logManager_ = nullptr;
    MutatorManager* mutatorManager_ = nullptr;
    ThreadHolderManager* threadHolderManager_  = nullptr;

    static std::mutex vmCreationLock_;
    static BaseRuntime *baseRuntimeInstance_;
    static bool initialized_;
};
}  // namespace panda
#endif // COMMON_INTERFACES_BASE_RUNTIME_H
