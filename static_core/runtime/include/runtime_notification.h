/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_RUNTIME_NOTIFICATION_H_
#define PANDA_RUNTIME_RUNTIME_NOTIFICATION_H_

#include <cstdint>
#include <optional>
#include <string_view>

#include "libarkbase/os/mutex.h"
#include "libarkbase/mem/space.h"
#include "runtime/include/console_call_type.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_scope-inl.h"

namespace ark {

class Method;
class Class;
class Rendezvous;

class RuntimeListener {
public:
    RuntimeListener() = default;
    virtual ~RuntimeListener() = default;
    DEFAULT_COPY_SEMANTIC(RuntimeListener);
    DEFAULT_MOVE_SEMANTIC(RuntimeListener);

    virtual void LoadModule([[maybe_unused]] std::string_view name) {}

    virtual void ThreadStart([[maybe_unused]] ManagedThread *managedThread) {}
    virtual void ThreadEnd([[maybe_unused]] ManagedThread *managedThread) {}

    virtual void BytecodePcChanged([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method,
                                   [[maybe_unused]] uint32_t bcOffset)
    {
    }

    virtual void GarbageCollectorStart() {}
    virtual void GarbageCollectorFinish() {}

    virtual void ExceptionThrow([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method,
                                [[maybe_unused]] ObjectHeader *exceptionObject, [[maybe_unused]] uint32_t bcOffset)
    {
    }

    virtual void ExceptionCatch([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method,
                                [[maybe_unused]] ObjectHeader *exceptionObject, [[maybe_unused]] uint32_t bcOffset)
    {
    }

    virtual void ConsoleCall([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] ConsoleCallType type,
                             [[maybe_unused]] uint64_t timestamp,
                             [[maybe_unused]] const PandaVector<TypedValue> &arguments)
    {
    }

    virtual void VmStart() {}
    virtual void VmInitialization([[maybe_unused]] ManagedThread *managedThread) {}
    virtual void VmDeath() {}

    virtual void MethodEntry([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method) {}
    virtual void MethodExit([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method) {}

    virtual void ClassLoad([[maybe_unused]] Class *klass) {}
    virtual void ClassPrepare([[maybe_unused]] Class *klass) {}

    virtual void MonitorWait([[maybe_unused]] ObjectHeader *object, [[maybe_unused]] int64_t timeout) {}
    virtual void MonitorWaited([[maybe_unused]] ObjectHeader *object, [[maybe_unused]] bool timedOut) {}
    virtual void MonitorContendedEnter([[maybe_unused]] ObjectHeader *object) {}
    virtual void MonitorContendedEntered([[maybe_unused]] ObjectHeader *object) {}

    virtual void ObjectAlloc([[maybe_unused]] BaseClass *klass, [[maybe_unused]] ObjectHeader *object,
                             [[maybe_unused]] ManagedThread *thread, [[maybe_unused]] size_t size)
    {
    }

    virtual void OutOfMemory([[maybe_unused]] size_t size, [[maybe_unused]] SpaceType spaceType) {}

    virtual void NativeMethodCall([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method) {}

    // Deprecated events
    virtual void ThreadStart([[maybe_unused]] ManagedThread::ThreadId threadId) {}
    virtual void ThreadEnd([[maybe_unused]] ManagedThread::ThreadId threadId) {}
    virtual void VmInitialization([[maybe_unused]] ManagedThread::ThreadId threadId) {}
    virtual void ExceptionCatch([[maybe_unused]] const ManagedThread *thread, [[maybe_unused]] const Method *method,
                                [[maybe_unused]] uint32_t bcOffset)
    {
    }
};

class DebuggerListener {
public:
    DebuggerListener() = default;
    virtual ~DebuggerListener() = default;
    DEFAULT_COPY_SEMANTIC(DebuggerListener);
    DEFAULT_MOVE_SEMANTIC(DebuggerListener);

    virtual void StartDebugger() = 0;
    virtual void StopDebugger() = 0;
    virtual bool IsDebuggerConfigured() = 0;
};

class RuntimeNotificationManager {
public:
    // Each event is a single bit in a bitmask, values follow the pattern 1 << n (0x01, 0x02, 0x04, ...)
    enum Event : uint32_t {
        BYTECODE_PC_CHANGED = 0x01,
        LOAD_MODULE = 0x02,
        THREAD_EVENTS = 0x04,
        GARBAGE_COLLECTOR_EVENTS = 0x08,
        EXCEPTION_EVENTS = 0x10,
        VM_EVENTS = 0x20,
        METHOD_EVENTS = 0x40,
        CLASS_EVENTS = 0x80,
        MONITOR_EVENTS = 0x100,
        ALLOCATION_EVENTS = 0x200,
        CONSOLE_EVENTS = 0x400,
        OUT_OF_MEMORY_EVENTS = 0x800,
        NATIVE_METHOD_CALL_EVENTS = 0x1000,
        ALL = 0xFFFFFFFF
    };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit RuntimeNotificationManager(mem::AllocatorPtr<mem::AllocatorPurpose::ALLOCATOR_PURPOSE_INTERNAL> allocator)
        : bytecodePcListeners_(allocator->Adapter()),
          loadModuleListeners_(allocator->Adapter()),
          threadEventsListeners_(allocator->Adapter()),
          garbageCollectorListeners_(allocator->Adapter()),
          exceptionListeners_(allocator->Adapter()),
          vmEventsListeners_(allocator->Adapter()),
          methodListeners_(allocator->Adapter()),
          classListeners_(allocator->Adapter()),
          monitorListeners_(allocator->Adapter()),
          allocationFailedListeners_(allocator->Adapter()),
          nativeMethodCallListeners_(allocator->Adapter())
    {
    }

    void AddListener(RuntimeListener *listener, uint32_t eventMask)
    {
        AddListenerIfMatches(listener, eventMask, &bytecodePcListeners_, Event::BYTECODE_PC_CHANGED,
                             &hasBytecodePcListeners_, bytecodePcLock_);

        AddListenerIfMatches(listener, eventMask, &loadModuleListeners_, Event::LOAD_MODULE, &hasLoadModuleListeners_,
                             loadModuleLock_);

        AddListenerIfMatches(listener, eventMask, &threadEventsListeners_, Event::THREAD_EVENTS,
                             &hasThreadEventsListeners_, threadEventsLock_);

        AddListenerIfMatches(listener, eventMask, &exceptionListeners_, Event::EXCEPTION_EVENTS,
                             &hasExceptionListeners_, exceptionLock_);

        AddListenerIfMatches(listener, eventMask, &vmEventsListeners_, Event::VM_EVENTS, &hasVmEventsListeners_,
                             vmEventsLock_);

        AddListenerIfMatches(listener, eventMask, &methodListeners_, Event::METHOD_EVENTS, &hasMethodListeners_,
                             methodLock_);

        AddListenerIfMatches(listener, eventMask, &classListeners_, Event::CLASS_EVENTS, &hasClassListeners_,
                             classLock_);

        AddListenerIfMatches(listener, eventMask, &monitorListeners_, Event::MONITOR_EVENTS, &hasMonitorListeners_,
                             monitorLock_);

        AddListenerIfMatches(listener, eventMask, &allocationListeners_, Event::ALLOCATION_EVENTS,
                             &hasAllocationListeners_, allocationLock_);

        AddListenerIfMatches(listener, eventMask, &consoleListeners_, Event::CONSOLE_EVENTS, &hasConsoleListeners_,
                             consoleLock_);

        AddListenerIfMatches(listener, eventMask, &allocationFailedListeners_, Event::OUT_OF_MEMORY_EVENTS,
                             &hasAllocationFailedListeners_, allocationFailedLock_);

        AddListenerIfMatches(listener, eventMask, &nativeMethodCallListeners_, Event::NATIVE_METHOD_CALL_EVENTS,
                             &hasNativeMethodCallListeners_, nativeMethodCallLock_);

        AddListenerIfMatches(listener, eventMask, &garbageCollectorListeners_, Event::GARBAGE_COLLECTOR_EVENTS,
                             &hasGarbageCollectorListeners_, garbageCollectorLock_);
    }

    void RemoveListener(RuntimeListener *listener, uint32_t eventMask)
    {
        RemoveListenerIfMatches(listener, eventMask, &bytecodePcListeners_, Event::BYTECODE_PC_CHANGED,
                                &hasBytecodePcListeners_, bytecodePcLock_);

        RemoveListenerIfMatches(listener, eventMask, &loadModuleListeners_, Event::LOAD_MODULE,
                                &hasLoadModuleListeners_, loadModuleLock_);

        RemoveListenerIfMatches(listener, eventMask, &threadEventsListeners_, Event::THREAD_EVENTS,
                                &hasThreadEventsListeners_, threadEventsLock_);

        RemoveListenerIfMatches(listener, eventMask, &exceptionListeners_, Event::EXCEPTION_EVENTS,
                                &hasExceptionListeners_, exceptionLock_);

        RemoveListenerIfMatches(listener, eventMask, &vmEventsListeners_, Event::VM_EVENTS, &hasVmEventsListeners_,
                                vmEventsLock_);

        RemoveListenerIfMatches(listener, eventMask, &methodListeners_, Event::METHOD_EVENTS, &hasMethodListeners_,
                                methodLock_);

        RemoveListenerIfMatches(listener, eventMask, &classListeners_, Event::CLASS_EVENTS, &hasClassListeners_,
                                classLock_);

        RemoveListenerIfMatches(listener, eventMask, &monitorListeners_, Event::MONITOR_EVENTS, &hasMonitorListeners_,
                                monitorLock_);

        RemoveListenerIfMatches(listener, eventMask, &allocationListeners_, Event::ALLOCATION_EVENTS,
                                &hasAllocationListeners_, allocationLock_);

        RemoveListenerIfMatches(listener, eventMask, &consoleListeners_, Event::CONSOLE_EVENTS, &hasConsoleListeners_,
                                consoleLock_);

        RemoveListenerIfMatches(listener, eventMask, &allocationFailedListeners_, Event::OUT_OF_MEMORY_EVENTS,
                                &hasAllocationFailedListeners_, allocationFailedLock_);

        RemoveListenerIfMatches(listener, eventMask, &nativeMethodCallListeners_, Event::NATIVE_METHOD_CALL_EVENTS,
                                &hasNativeMethodCallListeners_, nativeMethodCallLock_);

        RemoveListenerIfMatches(listener, eventMask, &garbageCollectorListeners_, Event::GARBAGE_COLLECTOR_EVENTS,
                                &hasGarbageCollectorListeners_, garbageCollectorLock_);
    }

    void LoadModuleEvent(std::string_view name)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasLoadModuleListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(loadModuleLock_);
            for (auto *listener : loadModuleListeners_) {
                if (listener != nullptr) {
                    listener->LoadModule(name);
                }
            }
        }
    }

    void ThreadStartEvent(ManagedThread *managedThread)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasThreadEventsListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(threadEventsLock_);
            for (auto *listener : threadEventsListeners_) {
                if (listener != nullptr) {
                    listener->ThreadStart(managedThread);
                }
            }
        }
    }

    void ThreadEndEvent(ManagedThread *managedThread)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasThreadEventsListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(threadEventsLock_);
            for (auto *listener : threadEventsListeners_) {
                if (listener != nullptr) {
                    listener->ThreadEnd(managedThread);
                }
            }
        }
    }

    void BytecodePcChangedEvent(ManagedThread *thread, Method *method, uint32_t bcOffset)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasBytecodePcListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(bytecodePcLock_);
            for (auto *listener : bytecodePcListeners_) {
                if (listener != nullptr) {
                    listener->BytecodePcChanged(thread, method, bcOffset);
                }
            }
        }
    }

    void GarbageCollectorStartEvent()
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasGarbageCollectorListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(garbageCollectorLock_);
            for (auto *listener : garbageCollectorListeners_) {
                if (listener != nullptr) {
                    listener->GarbageCollectorStart();
                }
            }
        }
    }

    void GarbageCollectorFinishEvent()
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasGarbageCollectorListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(garbageCollectorLock_);
            for (auto *listener : garbageCollectorListeners_) {
                if (listener != nullptr) {
                    listener->GarbageCollectorFinish();
                }
            }
        }
    }

    void ExceptionThrowEvent(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject, uint32_t bcOffset)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasExceptionListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(exceptionLock_);
            [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
            VMHandle<ObjectHeader> objHandle(thread, exceptionObject);
            for (auto *listener : exceptionListeners_) {
                if (listener != nullptr) {
                    listener->ExceptionThrow(thread, method, objHandle.GetPtr(), bcOffset);
                }
            }
        }
    }

    void ExceptionCatchEvent(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject, uint32_t bcOffset)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasExceptionListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(exceptionLock_);
            [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
            VMHandle<ObjectHeader> objHandle(thread, exceptionObject);
            for (auto *listener : exceptionListeners_) {
                if (listener != nullptr) {
                    listener->ExceptionCatch(thread, method, objHandle.GetPtr(), bcOffset);
                }
            }
        }
    }

    void VmStartEvent()
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasVmEventsListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(vmEventsLock_);
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmStart();
                }
            }
        }
    }

    void VmInitializationEvent(ManagedThread *managedThread)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasVmEventsListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(vmEventsLock_);
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmInitialization(managedThread);
                }
            }
        }
    }

    // Deprecated API
    void VmInitializationEvent(ManagedThread::ThreadId threadId)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasVmEventsListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(vmEventsLock_);
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmInitialization(threadId);
                }
            }
        }
    }

    void VmDeathEvent()
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasVmEventsListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(vmEventsLock_);
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmDeath();
                }
            }
        }
    }

    void MethodEntryEvent(ManagedThread *thread, Method *method)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasMethodListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(methodLock_);
            for (auto *listener : methodListeners_) {
                if (listener != nullptr) {
                    listener->MethodEntry(thread, method);
                }
            }
        }
    }

    void MethodExitEvent(ManagedThread *thread, Method *method)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasMethodListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(methodLock_);
            for (auto *listener : methodListeners_) {
                if (listener != nullptr) {
                    listener->MethodExit(thread, method);
                }
            }
        }
    }

    void ClassLoadEvent(Class *klass)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasClassListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(classLock_);
            for (auto *listener : classListeners_) {
                if (listener != nullptr) {
                    listener->ClassLoad(klass);
                }
            }
        }
    }

    void ClassPrepareEvent(Class *klass)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasClassListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(classLock_);
            for (auto *listener : classListeners_) {
                if (listener != nullptr) {
                    listener->ClassPrepare(klass);
                }
            }
        }
    }

    void MonitorWaitEvent(ObjectHeader *object, int64_t timeout)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasMonitorListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(monitorLock_);
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                listener->MonitorWait(object, timeout);
            }
        }
    }

    void MonitorWaitedEvent(ObjectHeader *object, bool timedOut)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasMonitorListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(monitorLock_);
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                monitorListeners_.front()->MonitorWaited(object, timedOut);
            }
        }
    }

    void MonitorContendedEnterEvent(ObjectHeader *object)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasMonitorListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(monitorLock_);
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                monitorListeners_.front()->MonitorContendedEnter(object);
            }
        }
    }

    void MonitorContendedEnteredEvent(ObjectHeader *object)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasMonitorListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(monitorLock_);
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                monitorListeners_.front()->MonitorContendedEntered(object);
            }
        }
    }

    bool HasAllocationListeners() const
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        return hasAllocationListeners_.load(std::memory_order_relaxed);
    }

    bool HasAllocationFailedListeners() const
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        return hasAllocationFailedListeners_.load(std::memory_order_relaxed);
    }

    void OutOfMemoryEvent(size_t size, SpaceType spaceType)
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasAllocationFailedListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(allocationFailedLock_);
            for (auto *listener : allocationFailedListeners_) {
                if (listener != nullptr) {
                    listener->OutOfMemory(size, spaceType);
                }
            }
        }
    }

    void ObjectAllocEvent(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size) const
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasAllocationListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(allocationLock_);
            // If we need to support multiple allocation listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(allocationListeners_.size() == 1);
            auto *listener = allocationListeners_.front();
            if (listener != nullptr) {
                allocationListeners_.front()->ObjectAlloc(klass, object, thread, size);
            }
        }
    }

    void ConsoleCallEvent(ManagedThread *thread, ConsoleCallType type, uint64_t timestamp,
                          [[maybe_unused]] const PandaVector<TypedValue> &arguments) const
    {
        // Atomic with relaxed order reason: sync with adding/removing listeners
        if (UNLIKELY(hasConsoleListeners_.load(std::memory_order_relaxed))) {
            os::memory::ReadLockHolder rwlock(consoleLock_);
            for (auto listener : consoleListeners_) {
                if (listener != nullptr) {
                    listener->ConsoleCall(thread, type, timestamp, arguments);
                }
            }
        }
    }

    void NativeMethodCallEvent(ManagedThread *thread, Method *method)
    {
        if (UNLIKELY(hasNativeMethodCallListeners_)) {
            for (auto *listener : nativeMethodCallListeners_) {
                if (listener != nullptr) {
                    listener->NativeMethodCall(thread, method);
                }
            }
        }
    }

    void StartDebugger()
    {
        os::memory::ReadLockHolder holder(debuggerLock_);
        for (auto *listener : debuggerListeners_) {
            listener->StartDebugger();
        }
    }

    void StopDebugger()
    {
        os::memory::ReadLockHolder holder(debuggerLock_);
        for (auto *listener : debuggerListeners_) {
            listener->StopDebugger();
        }
    }

    bool IsDebuggerConfigured()
    {
        os::memory::ReadLockHolder holder(debuggerLock_);
        for (auto *listener : debuggerListeners_) {
            if (!listener->IsDebuggerConfigured()) {
                return false;
            }
        }
        return true;
    }

    void AddDebuggerListener(DebuggerListener *listener)
    {
        os::memory::WriteLockHolder holder(debuggerLock_);
        debuggerListeners_.push_back(listener);
    }

    void RemoveDebuggerListener(DebuggerListener *listener)
    {
        os::memory::WriteLockHolder holder(debuggerLock_);
        RemoveListener(debuggerListeners_, listener);
    }

private:
    // NOLINTBEGIN(misc-unused-parameters)
    static void AddListenerIfMatches(RuntimeListener *listener, uint32_t eventMask,
                                     PandaList<RuntimeListener *> *listenerGroup, Event eventModifier,
                                     std::atomic<bool> *eventFlag, os::memory::RWLock &lock)
    {
        os::memory::WriteLockHolder holder(lock);
        if ((eventMask & eventModifier) != 0) {
            // If a free group item presents, use it, otherwise push back a new item
            auto it = std::find(listenerGroup->begin(), listenerGroup->end(), nullptr);
            if (it != listenerGroup->end()) {
                *it = listener;
            } else {
                listenerGroup->push_back(listener);
            }
            // Atomic with relaxed order reason: need other threads to see added listener
            eventFlag->store(true, std::memory_order_relaxed);
        }
    }

    template <typename Container, typename T>
    ALWAYS_INLINE void RemoveListener(Container &c, T &listener)
    {
        c.erase(std::remove_if(c.begin(), c.end(), [&listener](const T &elem) { return listener == elem; }));
    }

    static void RemoveListenerIfMatches(RuntimeListener *listener, uint32_t eventMask,
                                        PandaList<RuntimeListener *> *listenerGroup, Event eventModifier,
                                        std::atomic<bool> *eventFlag, os::memory::RWLock &lock)
    {
        os::memory::WriteLockHolder holder(lock);
        if ((eventMask & eventModifier) != 0) {
            auto it = std::find(listenerGroup->begin(), listenerGroup->end(), listener);
            if (it == listenerGroup->end()) {
                return;
            }
            // Removing a listener is not safe, because the iteration can not be completed in another thread.
            // We just null the item in the group
            *it = nullptr;

            // Check if any listener presents and update the flag if not
            if (std::find_if(listenerGroup->begin(), listenerGroup->end(),
                             [](RuntimeListener *item) { return item != nullptr; }) == listenerGroup->end()) {
                // Atomic with relaxed order reason: need other threads to see removed listener
                eventFlag->store(false, std::memory_order_relaxed);
            }
        }
    }
    // NOLINTEND(misc-unused-parameters)

    PandaList<RuntimeListener *> bytecodePcListeners_ GUARDED_BY(bytecodePcLock_);
    PandaList<RuntimeListener *> loadModuleListeners_ GUARDED_BY(loadModuleLock_);
    PandaList<RuntimeListener *> threadEventsListeners_ GUARDED_BY(threadEventsLock_);
    PandaList<RuntimeListener *> garbageCollectorListeners_ GUARDED_BY(garbageCollectorLock_);
    PandaList<RuntimeListener *> exceptionListeners_ GUARDED_BY(exceptionLock_);
    PandaList<RuntimeListener *> vmEventsListeners_ GUARDED_BY(vmEventsLock_);
    PandaList<RuntimeListener *> methodListeners_ GUARDED_BY(methodLock_);
    PandaList<RuntimeListener *> classListeners_ GUARDED_BY(classLock_);
    PandaList<RuntimeListener *> monitorListeners_ GUARDED_BY(monitorLock_);
    PandaList<RuntimeListener *> allocationListeners_ GUARDED_BY(allocationLock_);
    PandaList<RuntimeListener *> consoleListeners_ GUARDED_BY(consoleLock_);
    PandaList<RuntimeListener *> allocationFailedListeners_ GUARDED_BY(allocationFailedLock_);
    PandaList<RuntimeListener *> nativeMethodCallListeners_;

    std::atomic<bool> hasBytecodePcListeners_ = false;
    std::atomic<bool> hasLoadModuleListeners_ = false;
    std::atomic<bool> hasThreadEventsListeners_ = false;
    std::atomic<bool> hasGarbageCollectorListeners_ = false;
    std::atomic<bool> hasExceptionListeners_ = false;
    std::atomic<bool> hasVmEventsListeners_ = false;
    std::atomic<bool> hasMethodListeners_ = false;
    std::atomic<bool> hasClassListeners_ = false;
    std::atomic<bool> hasMonitorListeners_ = false;
    std::atomic<bool> hasAllocationListeners_ = false;
    std::atomic<bool> hasConsoleListeners_ = false;
    std::atomic<bool> hasAllocationFailedListeners_ = false;
    std::atomic<bool> hasNativeMethodCallListeners_ = false;

    os::memory::RWLock bytecodePcLock_;
    os::memory::RWLock loadModuleLock_;
    os::memory::RWLock threadEventsLock_;
    os::memory::RWLock garbageCollectorLock_;
    os::memory::RWLock exceptionLock_;
    os::memory::RWLock vmEventsLock_;
    os::memory::RWLock methodLock_;
    os::memory::RWLock classLock_;
    os::memory::RWLock monitorLock_;
    mutable os::memory::RWLock allocationLock_;
    mutable os::memory::RWLock consoleLock_;
    os::memory::RWLock allocationFailedLock_;
    os::memory::RWLock nativeMethodCallLock_;

    os::memory::RWLock debuggerLock_;
    PandaList<DebuggerListener *> debuggerListeners_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_RUNTIME_NOTIFICATION_H_
