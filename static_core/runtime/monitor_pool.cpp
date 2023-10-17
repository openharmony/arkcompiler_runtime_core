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

#include "runtime/monitor_pool.h"

#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/mark_word.h"
#include "runtime/monitor.h"

namespace panda {

Monitor *MonitorPool::CreateMonitor(ObjectHeader *obj)
{
    os::memory::LockHolder lock(pool_lock_);
    for (Monitor::MonitorId i = 0; i < MAX_MONITOR_ID; i++) {
        last_id_ = (last_id_ + 1) % MAX_MONITOR_ID;
        if (monitors_.count(last_id_) == 0) {
            auto monitor = allocator_->New<Monitor>(last_id_);
            if (monitor == nullptr) {
                return nullptr;
            }
            monitors_[last_id_] = monitor;
            monitor->SetObject(obj);
            return monitor;
        }
    }
    LOG(FATAL, RUNTIME) << "Out of MonitorPool indexes";
    UNREACHABLE();
}

Monitor *MonitorPool::LookupMonitor(Monitor::MonitorId id)
{
    os::memory::LockHolder lock(pool_lock_);
    auto it = monitors_.find(id);
    if (it != monitors_.end()) {
        return it->second;
    }
    return nullptr;
}

void MonitorPool::FreeMonitor(Monitor::MonitorId id)
{
    os::memory::LockHolder lock(pool_lock_);
    auto it = monitors_.find(id);
    if (it != monitors_.end()) {
        auto *monitor = it->second;
        monitors_.erase(it);
        allocator_->Delete(monitor);
    }
}

void MonitorPool::DeflateMonitors()
{
    os::memory::LockHolder lock(pool_lock_);
    for (auto monitor_iter = monitors_.begin(); monitor_iter != monitors_.end();) {
        auto monitor = monitor_iter->second;
        if (monitor->DeflateInternal()) {
            monitor_iter = monitors_.erase(monitor_iter);
            allocator_->Delete(monitor);
        } else {
            monitor_iter++;
        }
    }
}

void MonitorPool::ReleaseMonitors(MTManagedThread *thread)
{
    os::memory::LockHolder lock(pool_lock_);
    for (auto &it : monitors_) {
        auto *monitor = it.second;
        // Recursive lock is possible
        while (monitor->GetOwner() == thread) {
            monitor->Release(thread);
        }
    }
}

PandaSet<Monitor::MonitorId> MonitorPool::GetEnteredMonitorsIds(MTManagedThread *thread)
{
    PandaSet<Monitor::MonitorId> entered_monitors_ids;
    os::memory::LockHolder lock(pool_lock_);
    for (auto &it : monitors_) {
        auto *monitor = it.second;
        if (monitor->GetOwner() == thread) {
            entered_monitors_ids.insert(monitor->GetId());
        }
    }
    return entered_monitors_ids;
}

}  // namespace panda
