/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MONITOR_POOL_H_
#define PANDA_RUNTIME_MONITOR_POOL_H_

#include "libpandabase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/object_header.h"
#include "runtime/mark_word.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/monitor.h"

namespace panda {

class MonitorPool {
public:
    // Likely, we do not need to copy monitor pool
    NO_COPY_SEMANTIC(MonitorPool);
    NO_MOVE_SEMANTIC(MonitorPool);

    static constexpr Monitor::MonitorId MAX_MONITOR_ID = MarkWord::MONITOR_POINTER_MAX_COUNT;

    template <class Callback>
    void EnumerateMonitors(const Callback &cb)
    {
        os::memory::LockHolder lock(pool_lock_);
        for (auto &iter : monitors_) {
            if (!cb(iter.second)) {
                break;
            }
        }
    }

    template <class Callback>
    void DeflateMonitorsWithCallBack(const Callback &cb)
    {
        os::memory::LockHolder lock(pool_lock_);
        for (auto monitor_iter = monitors_.begin(); monitor_iter != monitors_.end();) {
            auto monitor = monitor_iter->second;
            if (cb(monitor) && monitor->DeflateInternal()) {
                monitor_iter = monitors_.erase(monitor_iter);
                allocator_->Delete(monitor);
            } else {
                monitor_iter++;
            }
        }
    }

    explicit MonitorPool(mem::InternalAllocatorPtr allocator) : monitors_(allocator->Adapter())
    {
        last_id_ = 0;
        allocator_ = allocator;
    }

    ~MonitorPool()
    {
#if defined(PANDA_TSAN_ON)
        // There is a potential false data race between destroying monitors and daemon threads which are got stuck in a
        // deadlock. The race is reported for accesses in Mutex::Lock() before going to endless sleep on futex call in
        // a deadlock. In this case the data race is reported with the main thread, which destroys the mutex.
        if (os::memory::Mutex::DoNotCheckOnDeadlock()) {
            TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
        }
#endif
        for (auto &iter : monitors_) {
            if (iter.second != nullptr) {
                allocator_->Delete(iter.second);
            }
        }
#if defined(PANDA_TSAN_ON)
        if (os::memory::Mutex::DoNotCheckOnDeadlock()) {
            TSAN_ANNOTATE_IGNORE_WRITES_END();
        }
#endif
    }

    Monitor *CreateMonitor(ObjectHeader *obj);

    Monitor *LookupMonitor(Monitor::MonitorId id);

    void FreeMonitor(Monitor::MonitorId id);

    void DeflateMonitors();

    void ReleaseMonitors(MTManagedThread *thread);

    PandaSet<Monitor::MonitorId> GetEnteredMonitorsIds(MTManagedThread *thread);

private:
    mem::InternalAllocatorPtr allocator_;
    // Lock for private data protection.
    os::memory::Mutex pool_lock_;

    Monitor::MonitorId last_id_ GUARDED_BY(pool_lock_);
    PandaUnorderedMap<Monitor::MonitorId, Monitor *> monitors_ GUARDED_BY(pool_lock_);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_MONITOR_POOL_H_
