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

#ifndef PANDA_RUNTIME_THREAD_INL_H_
#define PANDA_RUNTIME_THREAD_INL_H_

#include "runtime/handle_base.h"
#include "runtime/global_handle_storage-inl.h"
#include "runtime/handle_storage-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"

namespace panda {

template <>
inline void ManagedThread::PushHandleScope<coretypes::TaggedType>(HandleScope<coretypes::TaggedType> *handle_scope)
{
    tagged_handle_scopes_.push_back(handle_scope);
}

template <>
inline HandleScope<coretypes::TaggedType> *ManagedThread::PopHandleScope<coretypes::TaggedType>()
{
    HandleScope<coretypes::TaggedType> *scope = tagged_handle_scopes_.back();
    tagged_handle_scopes_.pop_back();
    return scope;
}

template <>
inline HandleScope<coretypes::TaggedType> *ManagedThread::GetTopScope<coretypes::TaggedType>() const
{
    if (tagged_handle_scopes_.empty()) {
        return nullptr;
    }
    return tagged_handle_scopes_.back();
}

template <>
inline HandleStorage<coretypes::TaggedType> *ManagedThread::GetHandleStorage<coretypes::TaggedType>() const
{
    return tagged_handle_storage_;
}

template <>
inline GlobalHandleStorage<coretypes::TaggedType> *ManagedThread::GetGlobalHandleStorage<coretypes::TaggedType>() const
{
    return tagged_global_handle_storage_;
}

template <>
inline void ManagedThread::PushHandleScope<ObjectHeader *>(HandleScope<ObjectHeader *> *handle_scope)
{
    object_header_handle_scopes_.push_back(handle_scope);
}

template <>
inline HandleScope<ObjectHeader *> *ManagedThread::PopHandleScope<ObjectHeader *>()
{
    HandleScope<ObjectHeader *> *scope = object_header_handle_scopes_.back();
    object_header_handle_scopes_.pop_back();
    return scope;
}

template <>
inline HandleScope<ObjectHeader *> *ManagedThread::GetTopScope<ObjectHeader *>() const
{
    if (object_header_handle_scopes_.empty()) {
        return nullptr;
    }
    return object_header_handle_scopes_.back();
}

template <>
inline HandleStorage<ObjectHeader *> *ManagedThread::GetHandleStorage<ObjectHeader *>() const
{
    return object_header_handle_storage_;
}

template <bool check_native_stack, bool check_iframe_stack>
ALWAYS_INLINE inline bool ManagedThread::StackOverflowCheck()
{
    if (!StackOverflowCheckResult<check_native_stack, check_iframe_stack>()) {
        // we're going to throw exception that will use the reserved stack space, so disable check
        DisableStackOverflowCheck();
        panda::ThrowStackOverflowException(this);
        // after finish throw exception, restore overflow check
        EnableStackOverflowCheck();
        return false;
    }
    return true;
}

ALWAYS_INLINE inline MonitorPool *MTManagedThread::GetMonitorPool()
{
    return GetVM()->GetMonitorPool();
}

ALWAYS_INLINE inline int32_t MTManagedThread::GetMonitorCount()
{
    // Atomic with relaxed order reason: data race with monitor_count_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    return monitor_count_.load(std::memory_order_relaxed);
}

ALWAYS_INLINE inline void MTManagedThread::AddMonitor(Monitor *monitor)
{
    // Atomic with relaxed order reason: data race with monitor_count_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    monitor_count_.fetch_add(1, std::memory_order_relaxed);
    LOG(DEBUG, RUNTIME) << "Adding monitor " << monitor->GetId();
}

ALWAYS_INLINE inline void MTManagedThread::RemoveMonitor(Monitor *monitor)
{
    // Atomic with relaxed order reason: data race with monitor_count_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    monitor_count_.fetch_sub(1, std::memory_order_relaxed);
    LOG(DEBUG, RUNTIME) << "Removing monitor " << monitor->GetId();
}

ALWAYS_INLINE inline void MTManagedThread::ReleaseMonitors()
{
    if (GetMonitorCount() > 0) {
        GetMonitorPool()->ReleaseMonitors(this);
    }
    ASSERT(GetMonitorCount() == 0);
}

}  // namespace panda

#endif  // PANDA_RUNTIME_THREAD_INL_H_
