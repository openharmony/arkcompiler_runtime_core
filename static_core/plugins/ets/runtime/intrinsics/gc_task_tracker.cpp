/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/intrinsics/gc_task_tracker.h"

namespace ark::ets::intrinsics {

GCTaskTracker g_gcTaskTracker;           // NOLINT(fuchsia-statically-constructed-objects)
os::memory::Mutex GCTaskTracker::lock_;  // NOLINT(fuchsia-statically-constructed-objects)
bool GCTaskTracker::initialized_ = false;

/* static */
GCTaskTracker &GCTaskTracker::InitIfNeededAndGet(mem::GC *gc)
{
    os::memory::LockHolder lh(lock_);
    if (initialized_) {
        return g_gcTaskTracker;
    }
    gc->AddListener(&g_gcTaskTracker);
    initialized_ = true;
    return g_gcTaskTracker;
}

/* static */
bool GCTaskTracker::IsInitialized()
{
    os::memory::LockHolder lh(lock_);
    return initialized_;
}

void GCTaskTracker::AddTaskId(uint64_t id)
{
    os::memory::LockHolder lock(lock_);
    callbackRefs_[id] = nullptr;
}

bool GCTaskTracker::HasId(uint64_t id)
{
    os::memory::LockHolder lock(lock_);
    return callbackRefs_.find(id) != callbackRefs_.end();
}

void GCTaskTracker::SetCallbackForTask(uint64_t taskId, mem::Reference *callbackRef)
{
    os::memory::LockHolder lock(lock_);
    callbackRefs_[taskId] = callbackRef;
}

void GCTaskTracker::GCStarted(const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    os::memory::LockHolder lock(lock_);
    currentTaskId_ = task.GetId();
}

void GCTaskTracker::GCPhaseStarted(mem::GCPhase phase)
{
    if (phase != mem::GCPhase::GC_PHASE_MARK) {
        return;
    }
    mem::Reference *callbackRef = nullptr;
    {
        os::memory::LockHolder lock(lock_);
        auto it = callbackRefs_.find(currentTaskId_);
        if (it == callbackRefs_.end() || it->second == nullptr) {
            return;
        }
        callbackRef = it->second;
    }
    auto *mThread = ManagedThread::GetCurrent();
    ASSERT(mThread != nullptr);
    auto *obj = reinterpret_cast<EtsObject *>(mThread->GetVM()->GetGlobalObjectStorage()->Get(callbackRef));
    Value arg(obj->GetCoreType());
    os::memory::ReadLockHolder lock(*mThread->GetVM()->GetRendezvous()->GetMutatorLock());
    LambdaUtils::InvokeVoid(mThread, obj);
}

void GCTaskTracker::GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                               [[maybe_unused]] size_t heapSize)
{
    RemoveId(task.GetId());
}

void GCTaskTracker::RemoveId(uint64_t id)
{
    mem::Reference *callbackRef = nullptr;
    {
        os::memory::LockHolder lock(lock_);
        if (currentTaskId_ == id) {
            currentTaskId_ = 0;
        }
        auto callbackIt = callbackRefs_.find(id);
        if (callbackIt != callbackRefs_.end()) {
            callbackRef = callbackIt->second;
            callbackRefs_.erase(callbackIt);
        }
    }
    if (callbackRef != nullptr) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        ASSERT(executionCtx != nullptr);
        executionCtx->GetPandaVM()->GetGlobalObjectStorage()->Remove(callbackRef);
    }
}

}  // namespace ark::ets::intrinsics
