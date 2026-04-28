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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_FINALIZER_PROCESSOR_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_FINALIZER_PROCESSOR_H

#include <climits>
#include <list>

#include "common_components/common/page_allocator.h"
#include "common_components/common/type_def.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/log/log.h"

#include "libarkbase/os/mutex.h"

namespace ark::common_vm {
template <typename T>
using ManagedList = std::list<T, StdContainerAllocator<T, FINALIZER_PROCESSOR>>;

class FinalizerProcessor {
public:
    FinalizerProcessor();
    ~FinalizerProcessor() = default;

    // mainly for resurrection.
    uint32_t VisitFinalizers(const RootVisitor &visitor)
    {
        uint32_t count = 0;
        ark::os::memory::LockHolder l(listLock_);
        for (BaseObject *&obj : finalizers_) {
            visitor(reinterpret_cast<ObjectRef &>(obj));
            ++count;
        }
        return count;
    }

    // for : *finalizers* are not proper gc roots.
    void VisitGCRoots(const RootVisitor &visitor)
    {
        ark::os::memory::LockHolder l(listLock_);
        for (BaseObject *&obj : finalizables_) {
            visitor(reinterpret_cast<ObjectRef &>(obj));
        }
        for (BaseObject *&obj : workingFinalizables_) {
            visitor(reinterpret_cast<ObjectRef &>(obj));
        }
    }

    // mainly for fixing old pointers
    void VisitRawPointers(const RootVisitor &visitor)
    {
        ark::os::memory::LockHolder l(listLock_);
        for (BaseObject *&obj : finalizables_) {
            visitor(reinterpret_cast<ObjectRef &>(obj));
        }
        for (BaseObject *&obj : workingFinalizables_) {
            visitor(reinterpret_cast<ObjectRef &>(obj));
        }
        for (BaseObject *&obj : finalizers_) {
            visitor(reinterpret_cast<ObjectRef &>(obj));
        }
    }

    // notify for finalizer processing loop, invoked after GC
    void Notify();
    // wait started flag set, call after create finalizerProcessor thread
    void WaitStarted();

    void Start();
    void Stop();
    void Run();
    void Init();
    void Fini();
    void WaitStop();

    void EnqueueFinalizables(const std::function<bool(BaseObject *)> &finalizable, uint32_t countLimit = UINT_MAX);
    bool IsRunning() const
    {
        return running_;
    }
    uint32_t GetTid() const
    {
        return tid_;
    }

    Mutator *GetMutator() const
    {
        return fpMutator_;
    }

    void NotifyToReclaimGarbage()
    {
        // Atomic with release order reason: data race with shouldReclaimHeapGarbage_ with dependecies on writes
        // before the store
        shouldReclaimHeapGarbage_.store(true, std::memory_order_release);
    }
    void NotifyToFeedAllocBuffers()
    {
        // Atomic with release order reason: data race with shouldFeedHungryBuffers_ with dependecies on writes
        // before the store
        shouldFeedHungryBuffers_.store(true, std::memory_order_release);
        Notify();
    }

private:
    void NotifyStarted();
    void Wait(uint32_t timeoutMilliSeconds);
    void ProcessFinalizables();
    void ProcessFinalizableList();
    void ReclaimHeapGarbage();
    void FeedHungryBuffers();

    ark::os::memory::Mutex wakeLock_;
    ark::os::memory::ConditionVariable wakeCondition_;  // notify finalizer processing continue

    ark::os::memory::Mutex startedLock_;
    ark::os::memory::ConditionVariable startedCondition_;  // notify finalizerProcessor thread is started
    volatile bool started_;

    volatile bool running_;  // Initially false and set true after finalizerProcessor thread start, set false when stop

    uint32_t iterationWaitTime_;

    // finalization
    ark::os::memory::Mutex listLock_;       // lock for finalizers & finalizables & workingFinalizables
    ManagedList<BaseObject *> finalizers_;  // created finalizer record, accessed by mutator & GC

    // a dead finalizer is moved into finalizable by GC, then run finalize method by FP thread
    ManagedList<BaseObject *> finalizables_;

    ManagedList<BaseObject *> workingFinalizables_;  // FP working list, swap from finalizables

    std::atomic<bool> hasFinalizableJob_;
    std::atomic<bool> shouldReclaimHeapGarbage_;
    std::atomic<bool> shouldFeedHungryBuffers_;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    // stats
    void LogAfterProcess();
#endif
    uint64_t timeProcessorBegin_;
    uint64_t timeProcessUsed_;
    uint64_t timeCurrentProcessBegin_;
    uint32_t tid_ = 0;
    pthread_t threadHandle_ = 0;  // thread handle to thread
    Mutator *fpMutator_ = nullptr;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_FINALIZER_PROCESSOR_H
