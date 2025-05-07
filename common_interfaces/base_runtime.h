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
#include <mutex>

#include "thread/thread_holder_manager.h"
#include "common_interfaces/heap/heap.h"

namespace panda {
class BaseRuntime {
public:
    BaseRuntime() = default;
    ~BaseRuntime() = default;

    static BaseRuntime *GetInstance();
    static void DestroyInstance();

    void PreFork(ThreadHolder *holder);
    void PostFork();

    static void WriteBarrier(void* obj, void* field, void* ref);
    static void* ReadBarrier(void* obj, void* field);
    static void* ReadBarrier(void* field);
    static void* AtomicReadBarrier(void* obj, void* field, std::memory_order order);

    RuntimeParam GetRuntimeParam()
    {
        return param_;
    }

    HeapParam GetHeapParam()
    {
        return param_.heapParam;
    }

    GCParam GetGCParam()
    {
        return param_.gcParam;
    }

    panda::common::Heap &GetHeap()
    {
        return heap_;
    }

    ThreadHolderManager &GetThreadHolderManager()
    {
        return threadHolderManager_;
    }

private:
    RuntimeParam param_ {};
    ThreadHolderManager threadHolderManager_ {};
    panda::common::Heap heap_ {};

    static std::mutex vmCreationLock_;
    static BaseRuntime *baseRuntimeInstance_;
};
}  // namespace panda
#endif // COMMON_INTERFACES_BASE_RUNTIME_H