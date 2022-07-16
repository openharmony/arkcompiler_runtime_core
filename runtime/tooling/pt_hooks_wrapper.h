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

#ifndef PANDA_TOOLING_PT_HOOKS_WRAPPER_H
#define PANDA_TOOLING_PT_HOOKS_WRAPPER_H

#include <atomic>
#include "runtime/include/tooling/debug_interface.h"
#include "os/mutex.h"
#include "runtime/include/mtmanaged_thread.h"
#include "pt_thread_info.h"
#include "pt_hook_type_info.h"

namespace panda::tooling {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class PtHooksWrapper : public PtHooks {
public:
    void SetHooks(PtHooks *hooks)
    {
        // Atomic with release order reason: data race with hooks_
        hooks_.store(hooks, std::memory_order_release);
    }

    void EnableGlobalHook(PtHookType hookType)
    {
        global_hook_type_info_.Enable(hookType);
    }

    void DisableGlobalHook(PtHookType hookType)
    {
        global_hook_type_info_.Disable(hookType);
    }

    void EnableAllGlobalHook()
    {
        global_hook_type_info_.EnableAll();
    }

    void DisableAllGlobalHook()
    {
        global_hook_type_info_.DisableAll();
    }

    // Wrappers for hooks
    void Breakpoint(PtThread thread, Method *method, const PtLocation &location) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_BREAKPOINT)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->Breakpoint(thread, method, location);
    }

    void LoadModule(std::string_view pandaFile) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_LOAD_MODULE)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->LoadModule(pandaFile);
    }

    void Paused(PauseReason reason) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_PAUSED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->Paused(reason);
    }

    void Exception(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *exceptionObject,
                   Method *catchMethod, const PtLocation &catchLocation) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXCEPTION)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->Exception(thread, method, location, exceptionObject, catchMethod, catchLocation);
    }

    void ExceptionCatch(PtThread thread, Method *method, const PtLocation &location,
                        ObjectHeader *exceptionObject) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXCEPTION_CATCH)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ExceptionCatch(thread, method, location, exceptionObject);
    }

    void PropertyAccess(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                        PtProperty property) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_PROPERTY_ACCESS)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->PropertyAccess(thread, method, location, object, property);
    }

    void PropertyModification(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                              PtProperty property, VRegValue newValue) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_PROPERTY_MODIFICATION)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->PropertyModification(thread, method, location, object, property, newValue);
    }

    void FramePop(PtThread thread, Method *method, bool wasPoppedByException) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_FRAME_POP)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->FramePop(thread, method, wasPoppedByException);
    }

    void GarbageCollectionFinish() override
    {
        // TODO(dtrubenkov): Add an assertion when 2125 issue is resolved
        // ASSERT(ManagedThread::GetCurrent() == nullptr)
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !GlobalHookIsEnabled(PtHookType::PT_HOOK_TYPE_GARBAGE_COLLECTION_FINISH)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        // Called in an unmanaged thread
        loaded_hooks->GarbageCollectionFinish();
    }

    void GarbageCollectionStart() override
    {
        // TODO(dtrubenkov): Add an assertion when 2125 issue is resolved
        // ASSERT(ManagedThread::GetCurrent() == nullptr)
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !GlobalHookIsEnabled(PtHookType::PT_HOOK_TYPE_GARBAGE_COLLECTION_START)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        // Called in an unmanaged thread
        loaded_hooks->GarbageCollectionStart();
    }

    void MethodEntry(PtThread thread, Method *method) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_METHOD_ENTRY)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->MethodEntry(thread, method);
    }

    void MethodExit(PtThread thread, Method *method, bool wasPoppedByException, VRegValue returnValue) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_METHOD_EXIT)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->MethodExit(thread, method, wasPoppedByException, returnValue);
    }

    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_SINGLE_STEP)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->SingleStep(thread, method, location);
    }

    void ThreadStart(PtThread thread) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_THREAD_START)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ThreadStart(thread);
    }

    void ThreadEnd(PtThread thread) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_THREAD_END)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ThreadEnd(thread);
    }

    void VmStart() override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_VM_START)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->VmStart();
    }

    void VmInitialization(PtThread thread) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_VM_INITIALIZATION)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->VmInitialization(thread);
    }

    void VmDeath() override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_VM_DEATH)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
#ifndef NDEBUG
        // Atomic with release order reason: data race with vmdeath_did_not_happen_
        vmdeath_did_not_happen_.store(false, std::memory_order_release);
#endif
        loaded_hooks->VmDeath();
        SetHooks(nullptr);
    }

    void ExceptionRevoked(ExceptionWrapper reason, ExceptionID exceptionId) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXCEPTION_REVOKED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ExceptionRevoked(reason, exceptionId);
    }

    void ExecutionContextCreated(ExecutionContextWrapper context) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXECUTION_CONTEXT_CREATEED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ExecutionContextCreated(context);
    }

    void ExecutionContextDestroyed(ExecutionContextWrapper context) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXECUTION_CONTEXT_DESTROYED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ExecutionContextDestroyed(context);
    }

    void ExecutionContextsCleared() override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXECUTION_CONTEXTS_CLEARED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ExecutionContextsCleared();
    }

    void InspectRequested(PtObject object, PtObject hints) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_INSPECT_REQUESTED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->InspectRequested(object, hints);
    }

    void ClassLoad(PtThread thread, BaseClass *klass) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_CLASS_LOAD)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ClassLoad(thread, klass);
    }

    void ClassPrepare(PtThread thread, BaseClass *klass) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_CLASS_PREPARE)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ClassPrepare(thread, klass);
    }

    void MonitorWait(PtThread thread, ObjectHeader *object, int64_t timeout) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_WAIT)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->MonitorWait(thread, object, timeout);
    }

    void MonitorWaited(PtThread thread, ObjectHeader *object, bool timedOut) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_WAITED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->MonitorWaited(thread, object, timedOut);
    }

    void MonitorContendedEnter(PtThread thread, ObjectHeader *object) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_CONTENDED_ENTER)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->MonitorContendedEnter(thread, object);
    }

    void MonitorContendedEntered(PtThread thread, ObjectHeader *object) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_CONTENDED_ENTERED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->MonitorContendedEntered(thread, object);
    }

    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, PtThread thread, size_t size) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loaded_hooks = hooks_.load(std::memory_order_acquire);
        if (loaded_hooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_OBJECT_ALLOC)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeath_did_not_happen_.load(std::memory_order_acquire));
        loaded_hooks->ObjectAlloc(klass, object, thread, size);
    }

private:
    bool GlobalHookIsEnabled(PtHookType type) const
    {
        return global_hook_type_info_.IsEnabled(type);
    }

    bool HookIsEnabled(PtHookType type) const
    {
        if (GlobalHookIsEnabled(type)) {
            return true;
        }

        ManagedThread *managed_thread = ManagedThread::GetCurrent();
        ASSERT(managed_thread != nullptr);

        // Check local value
        return managed_thread->GetPtThreadInfo()->GetHookTypeInfo().IsEnabled(type);
    }

    std::atomic<PtHooks *> hooks_ {nullptr};

    PtHookTypeInfo global_hook_type_info_ {true};

#ifndef NDEBUG
    std::atomic_bool vmdeath_did_not_happen_ {true};
#endif
};
}  // namespace panda::tooling

#endif  // PANDA_TOOLING_PT_HOOKS_WRAPPER_H
