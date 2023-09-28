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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_

#include <atomic>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <libpandafile/include/source_lang_enum.h>

#include "libpandabase/macros.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/utils/expected.h"
#include "libpandabase/os/mutex.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/language_context.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "runtime/include/object_header.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/gc_stats.h"
#include "runtime/mem/gc/gc_trigger.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/memory_manager.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/monitor_pool.h"
#include "runtime/string_table.h"
#include "runtime/thread_manager.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "plugins/ets/runtime/ets_native_library_provider.h"
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/job_queue.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"

namespace panda::ets {

class PromiseListener {
public:
    PromiseListener() = default;
    virtual ~PromiseListener() = default;

    virtual void OnPromiseStateChanged(EtsHandle<EtsPromise> &) = 0;

    DEFAULT_COPY_SEMANTIC(PromiseListener);
    DEFAULT_MOVE_SEMANTIC(PromiseListener);
};

class PandaEtsVM final : public PandaVM, public EtsVM {  // NOLINT(fuchsia-multiple-inheritance)
public:
    static Expected<PandaEtsVM *, PandaString> Create(Runtime *runtime, const RuntimeOptions &options);
    static bool Destroy(PandaEtsVM *vm);

    ~PandaEtsVM() override;

    PANDA_PUBLIC_API static PandaEtsVM *GetCurrent();

    bool Initialize() override;
    bool InitializeFinish() override;

    void PreStartup() override;
    void PreZygoteFork() override;
    void PostZygoteFork() override;
    void InitializeGC() override;
    void StartGC() override;
    void StopGC() override;
    void SweepVmRefs(const GCObjectVisitor &gc_object_visitor) override;
    void VisitVmRoots(const GCRootVisitor &visitor) override;
    void UpdateVmRefs() override;
    void UninitializeThreads() override;

    void HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred) override;
    void HandleGCRoutineInMutator() override;
    void HandleGCFinished() override;

    const RuntimeOptions &GetOptions() const override
    {
        return Runtime::GetOptions();
    }

    LanguageContext GetLanguageContext() const override
    {
        return Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    }

    coretypes::String *ResolveString(const panda_file::File &pf, panda_file::File::EntityId id) override
    {
        coretypes::String *str = GetStringTable()->GetInternalStringFast(pf, id);
        if (str != nullptr) {
            return str;
        }
        str = GetStringTable()->GetOrInternInternalString(pf, id, GetLanguageContext());
        return str;
    }

    coretypes::String *ResolveString(Frame *frame, panda_file::File::EntityId id) override
    {
        return PandaEtsVM::ResolveString(*frame->GetMethod()->GetPandaFile(), id);
    }

    coretypes::String *CreateString(Method *ctor, ObjectHeader *obj) override;

    Runtime *GetRuntime() const
    {
        return runtime_;
    }

    mem::HeapManager *GetHeapManager() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetHeapManager();
    }

    mem::MemStatsType *GetMemStats() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetMemStats();
    }

    mem::GC *GetGC() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGC();
    }

    mem::GCTrigger *GetGCTrigger() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGCTrigger();
    }

    mem::GCStats *GetGCStats() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGCStats();
    }

    EtsClassLinker *GetClassLinker() const
    {
        return class_linker_.get();
    }

    mem::GlobalObjectStorage *GetGlobalObjectStorage() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGlobalObjectStorage();
    }

    void DeleteGlobalRef(ets_object global_ref);

    void DeleteWeakGlobalRef(ets_weak weak_ref);

    mem::ReferenceProcessor *GetReferenceProcessor() const override
    {
        ASSERT(reference_processor_ != nullptr);
        return reference_processor_;
    }

    StringTable *GetStringTable() const override
    {
        return string_table_;
    }

    MonitorPool *GetMonitorPool() const override
    {
        return monitor_pool_;
    }

    ManagedThread *GetAssociatedThread() const override
    {
        return ManagedThread::GetCurrent();
    }

    ThreadManager *GetThreadManager() const override
    {
        return coroutine_manager_;
    }

    CoroutineManager *GetCoroutineManager() const
    {
        return static_cast<CoroutineManager *>(GetThreadManager());
    }

    Rendezvous *GetRendezvous() const override
    {
        return rendezvous_;
    }

    CompilerInterface *GetCompiler() const override
    {
        return compiler_;
    }

    PANDA_PUBLIC_API ObjectHeader *GetOOMErrorObject() override;

    compiler::RuntimeInterface *GetCompilerRuntimeInterface() const override
    {
        return runtime_iface_;
    }

    bool LoadNativeLibrary(EtsEnv *env, const PandaString &name);

    void ResolveNativeMethod(Method *method);

    static PandaEtsVM *FromEtsVM(EtsVM *vm)
    {
        return static_cast<PandaEtsVM *>(vm);
    }

    void RegisterFinalizationQueueInstance(EtsObject *instance);

    [[noreturn]] static void Abort(const char *message = nullptr);

    void *GetExternalData()
    {
        return external_data_.data;
    }

    static PandaEtsVM *FromExternalData(void *external_data)
    {
        ASSERT(external_data != nullptr);
        return reinterpret_cast<PandaEtsVM *>(ToUintPtr(external_data) - MEMBER_OFFSET(PandaEtsVM, external_data_));
    }

    struct alignas(16) ExternalData {  // NOLINT(readability-magic-numbers)
        static constexpr size_t SIZE = 512U;
        uint8_t data[SIZE];  // NOLINT(modernize-avoid-c-arrays)
    };

    JobQueue *GetJobQueue()
    {
        return job_queue_.get();
    }

    void InitJobQueue(JobQueue *job_queue)
    {
        ASSERT(job_queue_ == nullptr);
        job_queue_.reset(job_queue);
    }

    PANDA_PUBLIC_API void AddPromiseListener(EtsPromise *promise, PandaUniquePtr<PromiseListener> &&listener);

    void FirePromiseStateChanged(EtsHandle<EtsPromise> &promise);

    std::mt19937 &GetRandomEngine()
    {
        ASSERT(random_engine_);
        return *random_engine_;
    }

    bool IsStaticProfileEnabled() const override
    {
        return true;
    }

    void SetClearInteropHandleScopesFunction(const std::function<void(Frame *)> &func)
    {
        clear_interop_handle_scopes_ = func;
    }

    void ClearInteropHandleScopes(Frame *frame) override
    {
        if (clear_interop_handle_scopes_) {
            clear_interop_handle_scopes_(frame);
        }
    }

    os::memory::Mutex &GetAtomicsMutex()
    {
        return atomics_mutex_;
    }

    PandaVector<int8_t> *AllocateAtomicsSharedMemory(size_t byte_length);

protected:
    bool CheckEntrypointSignature(Method *entrypoint) override;
    Expected<int, Runtime::Error> InvokeEntrypointImpl(Method *entrypoint,
                                                       const std::vector<std::string> &args) override;
    void HandleUncaughtException() override;

private:
    void InitializeRandomEngine()
    {
        ASSERT(!random_engine_);
        std::random_device rd;
        random_engine_.emplace(rd());
    }

    class PromiseListenerInfo {
    public:
        PromiseListenerInfo(EtsPromise *promise, PandaUniquePtr<PromiseListener> &&listener)
            : promise_(promise), listener_(std::move(listener))
        {
        }

        EtsPromise *GetPromise() const
        {
            return promise_;
        }

        void UpdateRefToMovedObject();
        void OnPromiseStateChanged(EtsHandle<EtsPromise> &promise);

    private:
        EtsPromise *promise_;
        PandaUniquePtr<PromiseListener> listener_;
    };

    explicit PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm);

    Runtime *runtime_ {nullptr};
    mem::MemoryManager *mm_ {nullptr};
    PandaUniquePtr<EtsClassLinker> class_linker_;
    mem::ReferenceProcessor *reference_processor_ {nullptr};
    PandaVector<ObjectHeader *> gc_roots_;
    Rendezvous *rendezvous_ {nullptr};
    CompilerInterface *compiler_ {nullptr};
    StringTable *string_table_ {nullptr};
    MonitorPool *monitor_pool_ {nullptr};
    CoroutineManager *coroutine_manager_ {nullptr};
    mem::Reference *oom_obj_ref_ {nullptr};
    compiler::RuntimeInterface *runtime_iface_ {nullptr};
    NativeLibraryProvider native_library_provider_;
    os::memory::Mutex finalization_queue_lock_;
    PandaList<EtsObject *> registered_finalization_queue_instances_ GUARDED_BY(finalization_queue_lock_);
    PandaUniquePtr<JobQueue> job_queue_;
    os::memory::Mutex promise_listeners_lock_;
    // TODO(audovichenko) Should be refactored #12030
    PandaList<PromiseListenerInfo> promise_listeners_ GUARDED_BY(promise_listeners_lock_);
    // optional for lazy initialization
    std::optional<std::mt19937> random_engine_;
    std::function<void(Frame *)> clear_interop_handle_scopes_;
    // for JS Atomics
    os::memory::Mutex atomics_mutex_;
    // Shared memory for SharedArrayBuffer
    std::atomic<PandaVector<int8_t> *> atomics_shared_memory_ {std::atomic(nullptr)};

    ExternalData external_data_ {};

    NO_MOVE_SEMANTIC(PandaEtsVM);
    NO_COPY_SEMANTIC(PandaEtsVM);

    friend class mem::Allocator;
};

}  // namespace panda::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_
