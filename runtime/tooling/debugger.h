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
#ifndef PANDA_RUNTIME_DEBUG_DEBUG_H
#define PANDA_RUNTIME_DEBUG_DEBUG_H

#include <atomic>
#include <functional>
#include <memory>
#include <string_view>

#include "include/method.h"
#include "include/runtime.h"
#include "pt_hooks_wrapper.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/mem/panda_containers.h"
#include "include/runtime_notification.h"
#include "include/tooling/debug_interface.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/utils/span.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/tooling/debug_interface.h"
#include "runtime/thread_manager.h"

namespace panda::tooling {
// Deprecated API
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class Breakpoint {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    Breakpoint(Method *method, uint32_t bcOffset) : method_(method), bc_offset_(bcOffset) {}
    ~Breakpoint() = default;

    Method *GetMethod() const
    {
        return method_;
    }

    uint32_t GetBytecodeOffset() const
    {
        return bc_offset_;
    }

    bool operator==(const Breakpoint &bpoint) const
    {
        return GetMethod() == bpoint.GetMethod() && GetBytecodeOffset() == bpoint.GetBytecodeOffset();
    }

    DEFAULT_COPY_SEMANTIC(Breakpoint);
    DEFAULT_MOVE_SEMANTIC(Breakpoint);

private:
    Method *method_;
    uint32_t bc_offset_;
};

// Deprecated API
class HashBreakpoint {
public:
    size_t operator()(const Breakpoint &bpoint) const
    {
        return (std::hash<Method *>()(bpoint.GetMethod())) ^ (std::hash<uint32_t>()(bpoint.GetBytecodeOffset()));
    }
};

class HashLocation {
public:
    size_t operator()(const PtLocation &location) const
    {
        return std::hash<std::string>()(location.GetPandaFile()) ^
               std::hash<uint32_t>()(location.GetMethodId().GetOffset()) ^  // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
               std::hash<uint32_t>()(location.GetBytecodeOffset());         // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
    }
};

class PropertyWatch {
public:
    enum class Type { ACCESS, MODIFY };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    PropertyWatch(panda_file::File::EntityId classId, panda_file::File::EntityId fieldId, Type type)
        : class_id_(classId), field_id_(fieldId), type_(type)
    {
    }

    ~PropertyWatch() = default;

    panda_file::File::EntityId GetClassId() const
    {
        return class_id_;
    }

    panda_file::File::EntityId GetFieldId() const
    {
        return field_id_;
    }

    Type GetType() const
    {
        return type_;
    }

private:
    NO_COPY_SEMANTIC(PropertyWatch);
    NO_MOVE_SEMANTIC(PropertyWatch);

    panda_file::File::EntityId class_id_;
    panda_file::File::EntityId field_id_;
    Type type_;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Debugger : public DebugInterface, RuntimeListener {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Debugger(const Runtime *runtime)
        : runtime_(runtime),
          breakpoints_(GetInternalAllocatorAdapter(runtime)),
          property_watches_(GetInternalAllocatorAdapter(runtime)),
          vm_started_(runtime->IsInitialized())
    {
        runtime_->GetNotificationManager()->AddListener(this, DEBUG_EVENT_MASK);
    }

    ~Debugger() override
    {
        runtime_->GetNotificationManager()->RemoveListener(this, DEBUG_EVENT_MASK);
    }

    PtLangExt *GetLangExtension() const override
    {
        PT_UNIMPLEMENTED();
        return nullptr;
    }

    Expected<PtMethod, Error> GetPtMethod(const PtLocation & /* location */) const override
    {
        PT_DEPRECATED();
        return Unexpected(Error(Error::Type::DEPRECATED, "Method is deprecated"));
    }

    std::optional<Error> RegisterHooks(PtHooks *hooks) override
    {
        hooks_.SetHooks(hooks);
        return {};
    }

    std::optional<Error> UnregisterHooks() override
    {
        hooks_.SetHooks(nullptr);
        return {};
    }

    std::optional<Error> EnableAllGlobalHook() override
    {
        hooks_.EnableAllGlobalHook();
        return {};
    }

    std::optional<Error> DisableAllGlobalHook() override
    {
        hooks_.DisableAllGlobalHook();
        return {};
    }

    std::optional<Error> SetNotification(PtThread thread, bool enable, PtHookType hookType) override;
    std::optional<Error> SetBreakpoint(const PtLocation &location) override;

    std::optional<Error> RemoveBreakpoint(const PtLocation &location) override;

    Expected<std::unique_ptr<PtFrame>, Error> GetCurrentFrame(PtThread thread) const override;

    std::optional<Error> EnumerateFrames(PtThread thread, std::function<bool(const PtFrame &)> callback) const override;

    // RuntimeListener methods

    void LoadModule(std::string_view filename) override
    {
        hooks_.LoadModule(filename);
    }

    void ThreadStart(ManagedThread *managed_thread) override
    {
        hooks_.ThreadStart(PtThread(managed_thread));
    }

    void ThreadEnd(ManagedThread *managed_thread) override
    {
        hooks_.ThreadEnd(PtThread(managed_thread));
    }

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override;

    void VmStart() override
    {
        vm_started_ = true;
        hooks_.VmStart();
    }

    void VmInitialization(ManagedThread *managed_thread) override
    {
        hooks_.VmInitialization(PtThread(managed_thread));
    }

    void VmDeath() override
    {
        hooks_.VmDeath();
    }

    void GarbageCollectorStart() override
    {
        hooks_.GarbageCollectionStart();
    }

    void GarbageCollectorFinish() override
    {
        panda::MTManagedThread *self = panda::MTManagedThread::GetCurrent();
        if (self == nullptr) {
            return;
        }
        hooks_.GarbageCollectionFinish();
    }

    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size) override;

    void ExceptionThrow(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject,
                        uint32_t bcOffset) override;
    void ExceptionCatch(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject,
                        uint32_t bcOffset) override;

    void MethodEntry(ManagedThread *thread, Method *method) override;
    void MethodExit(ManagedThread *thread, Method *method) override;

    void ClassLoad(Class *klass) override;
    void ClassPrepare(Class *klass) override;

    void MonitorWait(ObjectHeader *object, int64_t timeout) override;
    void MonitorWaited(ObjectHeader *object, bool timedOut) override;
    void MonitorContendedEnter(ObjectHeader *object) override;
    void MonitorContendedEntered(ObjectHeader *object) override;

    /*
     * Mock API for debug interphase starts:
     *
     * API's function should be revorked and input parameters should be added
     */
    std::optional<Error> GetThreadList(PandaVector<PtThread> *threadList) const override
    {
        runtime_->GetPandaVM()->GetThreadManager()->EnumerateThreads(
            [threadList](MTManagedThread *mt_managed_thread) {
                ASSERT(mt_managed_thread && "thread is null");
                threadList->push_back(PtThread(mt_managed_thread));
                return true;
            },
            static_cast<unsigned int>(panda::EnumerationFlag::ALL),
            static_cast<unsigned int>(panda::EnumerationFlag::VM_THREAD));

        return {};
    }

    std::optional<Error> GetThreadInfo([[maybe_unused]] PtThread thread,
                                       [[maybe_unused]] ThreadInfo *infoPtr) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> SuspendThread(PtThread thread) const override;

    std::optional<Error> ResumeThread(PtThread thread) const override;

    std::optional<Error> SetVariable([[maybe_unused]] PtThread thread, [[maybe_unused]] uint32_t frameDepth,
                                     [[maybe_unused]] int32_t regNumber,
                                     [[maybe_unused]] const PtValue &value) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> SetVariable(PtThread thread, uint32_t frameDepth, int32_t regNumber,
                                     const VRegValue &value) const override;

    std::optional<Error> GetVariable([[maybe_unused]] PtThread thread, [[maybe_unused]] uint32_t frameDepth,
                                     [[maybe_unused]] int32_t regNumber,
                                     [[maybe_unused]] PtValue *result) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> GetVariable(PtThread thread, uint32_t frameDepth, int32_t regNumber,
                                     VRegValue *result) const override;

    std::optional<Error> GetProperty([[maybe_unused]] PtObject object, [[maybe_unused]] PtProperty property,
                                     [[maybe_unused]] PtValue *value) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> SetProperty([[maybe_unused]] PtObject object, [[maybe_unused]] PtProperty property,
                                     [[maybe_unused]] const PtValue &value) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> EvaluateExpression([[maybe_unused]] PtThread thread, [[maybe_unused]] uint32_t frameNumber,
                                            [[maybe_unused]] ExpressionWrapper expr,
                                            [[maybe_unused]] PtValue *result) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> RetransformClasses([[maybe_unused]] int classCount,
                                            [[maybe_unused]] const PtClass *classes) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> RedefineClasses([[maybe_unused]] int classCount,
                                         [[maybe_unused]] const PandaClassDefinition *classes) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> RestartFrame([[maybe_unused]] PtThread thread,
                                      [[maybe_unused]] uint32_t frameNumber) const override;

    std::optional<Error> SetAsyncCallStackDepth([[maybe_unused]] uint32_t maxDepth) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> AwaitPromise([[maybe_unused]] PtObject promiseObject,
                                      [[maybe_unused]] PtValue *result) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> CallFunctionOn([[maybe_unused]] PtObject object, [[maybe_unused]] PtMethod method,
                                        [[maybe_unused]] const PandaVector<PtValue> &arguments,
                                        [[maybe_unused]] PtValue *returnValue) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> GetProperties([[maybe_unused]] uint32_t *countPtr,
                                       [[maybe_unused]] char ***propertyPtr) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> NotifyFramePop(PtThread thread, uint32_t depth) const override;

    std::optional<Error> SetPropertyAccessWatch([[maybe_unused]] PtClass klass,
                                                [[maybe_unused]] PtProperty property) override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> ClearPropertyAccessWatch([[maybe_unused]] PtClass klass,
                                                  [[maybe_unused]] PtProperty property) override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> SetPropertyModificationWatch([[maybe_unused]] PtClass klass,
                                                      [[maybe_unused]] PtProperty property) override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> ClearPropertyModificationWatch([[maybe_unused]] PtClass klass,
                                                        [[maybe_unused]] PtProperty property) override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> GetThisVariableByFrame([[maybe_unused]] PtThread thread, [[maybe_unused]] uint32_t frameDepth,
                                                [[maybe_unused]] PtValue *result) override
    {
        PT_UNIMPLEMENTED();
        return {};
    }
    std::optional<Error> GetThisVariableByFrame(PtThread thread, uint32_t frameDepth, ObjectHeader **thisPtr) override;

    std::optional<Error> SetPropertyAccessWatch(BaseClass *klass, PtProperty property) override;

    std::optional<Error> ClearPropertyAccessWatch(BaseClass *klass, PtProperty property) override;

    std::optional<Error> SetPropertyModificationWatch(BaseClass *klass, PtProperty property) override;

    std::optional<Error> ClearPropertyModificationWatch(BaseClass *klass, PtProperty property) override;

private:
    Expected<interpreter::StaticVRegisterRef, Error> GetVRegByPandaFrame(panda::Frame *frame, int32_t regNumber) const;
    Expected<interpreter::DynamicVRegisterRef, Error> GetVRegByPandaFrameDyn(panda::Frame *frame,
                                                                             int32_t regNumber) const;
    std::optional<Error> CheckLocation(const PtLocation &location);
    bool IsBreakpoint(const PtLocation &location) const REQUIRES_SHARED(rwlock_);
    bool EraseBreakpoint(const PtLocation &location);

    bool IsPropertyWatchActive() const
    {
        os::memory::ReadLockHolder rholder(rwlock_);
        return !property_watches_.empty();
    }
    const tooling::PropertyWatch *FindPropertyWatch(panda_file::File::EntityId classId,
                                                    panda_file::File::EntityId fieldId,
                                                    tooling::PropertyWatch::Type type) const REQUIRES_SHARED(rwlock_);
    bool RemovePropertyWatch(panda_file::File::EntityId classId, panda_file::File::EntityId fieldId,
                             tooling::PropertyWatch::Type type);

    bool HandleBreakpoint(ManagedThread *thread, Method *method, const PtLocation &location);
    void HandleNotifyFramePop(ManagedThread *thread, Method *method, bool wasPoppedByException);
    bool HandleStep(ManagedThread *thread, Method *method, const PtLocation &location);

    bool HandlePropertyAccess(ManagedThread *thread, Method *method, const PtLocation &location);
    bool HandlePropertyModify(ManagedThread *thread, Method *method, const PtLocation &location);

    static constexpr uint32_t DEBUG_EVENT_MASK =
        RuntimeNotificationManager::Event::LOAD_MODULE | RuntimeNotificationManager::Event::THREAD_EVENTS |
        RuntimeNotificationManager::Event::BYTECODE_PC_CHANGED | RuntimeNotificationManager::Event::EXCEPTION_EVENTS |
        RuntimeNotificationManager::Event::VM_EVENTS | RuntimeNotificationManager::Event::GARBAGE_COLLECTOR_EVENTS |
        RuntimeNotificationManager::Event::METHOD_EVENTS | RuntimeNotificationManager::Event::CLASS_EVENTS |
        RuntimeNotificationManager::Event::MONITOR_EVENTS | RuntimeNotificationManager::Event::ALLOCATION_EVENTS;

    const Runtime *runtime_;
    PtHooksWrapper hooks_;

    mutable os::memory::RWLock rwlock_;
    PandaUnorderedSet<PtLocation, HashLocation> breakpoints_ GUARDED_BY(rwlock_);
    PandaList<PropertyWatch> property_watches_ GUARDED_BY(rwlock_);
    // TODO(m.strizhak): research how to rework VM start to avoid atomic
    std::atomic_bool vm_started_ {false};

    NO_COPY_SEMANTIC(Debugger);
    NO_MOVE_SEMANTIC(Debugger);
};

class PtDebugFrame : public PtFrame {
public:
    explicit PtDebugFrame(Method *method, Frame *interpreterFrame);
    ~PtDebugFrame() override = default;

    bool IsInterpreterFrame() const override
    {
        return is_interpreter_frame_;
    }

    Method *GetMethod() const override
    {
        return method_;
    }

    uint64_t GetVReg(size_t i) const override
    {
        if (!is_interpreter_frame_) {
            return 0;
        }
        return vregs_[i];
    }

    size_t GetVRegNum() const override
    {
        return vregs_.size();
    }

    uint64_t GetArgument(size_t i) const override
    {
        if (!is_interpreter_frame_) {
            return 0;
        }
        return args_[i];
    }

    size_t GetArgumentNum() const override
    {
        return args_.size();
    }

    uint64_t GetAccumulator() const override
    {
        return acc_;
    }

    panda_file::File::EntityId GetMethodId() const override
    {
        return method_id_;
    }

    uint32_t GetBytecodeOffset() const override
    {
        return bc_offset_;
    }

    std::string GetPandaFile() const override
    {
        return panda_file_;
    }

    // mock API
    uint32_t GetFrameId() const override
    {
        return 0;
    }

private:
    NO_COPY_SEMANTIC(PtDebugFrame);
    NO_MOVE_SEMANTIC(PtDebugFrame);

    bool is_interpreter_frame_;
    Method *method_;
    uint64_t acc_ {0};
    PandaVector<uint64_t> vregs_;
    PandaVector<uint64_t> args_;
    panda_file::File::EntityId method_id_;
    uint32_t bc_offset_ {0};
    std::string panda_file_;
};
}  // namespace panda::tooling

#endif  // PANDA_RUNTIME_DEBUG_DEBUG_H
