/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_external_callback_poster.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_taskpool.h"

namespace ark::ets {

class DoubleToStringCache;
class FloatToStringCache;
class LongToStringCache;
class EtsFinalizableWeakRef;

class PandaEtsVM final : public PandaVM, public EtsVM {  // NOLINT(fuchsia-multiple-inheritance)
public:
    static Expected<PandaEtsVM *, PandaString> Create(Runtime *runtime, const RuntimeOptions &options);
    static bool Destroy(PandaEtsVM *vm);

    ~PandaEtsVM() override;

    PANDA_PUBLIC_API static PandaEtsVM *GetCurrent();

    /**
     * @brief Create TaskManager if needed and set it to runtime
     * @note It's temporary solution, TaskManager will be created outside VM in the future
     *
     * @param options runtime options
     *
     * @return true if TaskManager was created, false - otherwise
     */
    static bool CreateTaskManagerIfNeeded(const RuntimeOptions &options);

    bool Initialize() override;
    bool InitializeFinish() override;

    void PreStartup() override;
    void PreZygoteFork() override;
    void PostZygoteFork() override;
    void InitializeGC() override;
    void StartGC() override;
    void StopGC() override;
    void SweepVmRefs(const GCObjectVisitor &gcObjectVisitor) override;
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
        return classLinker_.get();
    }

    EtsClassLinkerExtension *GetEtsClassLinkerExtension() const
    {
        return classLinker_.get()->GetEtsClassLinkerExtension();
    }

    mem::GlobalObjectStorage *GetGlobalObjectStorage() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGlobalObjectStorage();
    }

    void DeleteGlobalRef(ets_object globalRef);

    void DeleteWeakGlobalRef(ets_weak weakRef);

    mem::ReferenceProcessor *GetReferenceProcessor() const override
    {
        ASSERT(referenceProcessor_ != nullptr);
        return referenceProcessor_;
    }

    StringTable *GetStringTable() const override
    {
        return stringTable_;
    }

    MonitorPool *GetMonitorPool() const override
    {
        return monitorPool_;
    }

    ManagedThread *GetAssociatedThread() const override
    {
        return ManagedThread::GetCurrent();
    }

    ThreadManager *GetThreadManager() const override
    {
        return coroutineManager_;
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

    PANDA_PUBLIC_API ObjectHeader *GetUndefinedObject();

    compiler::RuntimeInterface *GetCompilerRuntimeInterface() const override
    {
        return runtimeIface_;
    }

    bool LoadNativeLibrary(EtsEnv *env, const PandaString &name);

    void ResolveNativeMethod(Method *method);

    static PandaEtsVM *FromEtsVM(EtsVM *vm)
    {
        return static_cast<PandaEtsVM *>(vm);
    }

    void RegisterFinalizationRegistryInstance(EtsObject *instance);

    [[noreturn]] static void Abort(const char *message = nullptr);

    void *GetExternalData()
    {
        return externalData_.data;
    }

    static PandaEtsVM *FromExternalData(void *externalData)
    {
        ASSERT(externalData != nullptr);
        return reinterpret_cast<PandaEtsVM *>(ToUintPtr(externalData) - MEMBER_OFFSET(PandaEtsVM, externalData_));
    }

    struct alignas(16U) ExternalData {  // NOLINT(readability-magic-numbers)
        static constexpr size_t SIZE = 256U * 3;
        uint8_t data[SIZE];  // NOLINT(modernize-avoid-c-arrays)
    };

    JobQueue *GetJobQueue() const
    {
        return jobQueue_.get();
    }

    Taskpool *GetTaskpool()
    {
        return taskpool_;
    }

    const Taskpool *GetTaskpool() const
    {
        return taskpool_;
    }

    void InitJobQueue(JobQueue *jobQueue)
    {
        ASSERT(jobQueue_ == nullptr);
        jobQueue_.reset(jobQueue);
    }

    std::mt19937 &GetRandomEngine()
    {
        ASSERT(randomEngine_);
        return *randomEngine_;
    }

    bool IsStaticProfileEnabled() const override
    {
        return true;
    }

    void SetClearInteropHandleScopesFunction(const std::function<void(Frame *)> &func)
    {
        clearInteropHandleScopes_ = func;
    }

    void ClearInteropHandleScopes(Frame *frame) override
    {
        if (clearInteropHandleScopes_) {
            clearInteropHandleScopes_(frame);
        }
    }

    void SetDestroyExternalDataFunction(const std::function<void(void *)> &func)
    {
        ASSERT(!destroyExternalData_);
        destroyExternalData_ = func;
    }

    os::memory::Mutex &GetAtomicsMutex()
    {
        return atomicsMutex_;
    }

    bool SupportGCSinglePassCompaction() const override
    {
        return true;
    }

    DoubleToStringCache *GetDoubleToStringCache()
    {
        return doubleToStringCache_;
    }

    FloatToStringCache *GetFloatToStringCache()
    {
        return floatToStringCache_;
    }

    LongToStringCache *GetLongToStringCache()
    {
        return longToStringCache_;
    }

    static constexpr uint32_t GetDoubleToStringCacheOffset()
    {
        return MEMBER_OFFSET(PandaEtsVM, doubleToStringCache_);
    }

    void RegisterFinalizerForObject(EtsCoroutine *coro, const EtsHandle<EtsObject> &object, void (*finalizer)(void *),
                                    void *finalizerArg);

    void CleanFinalizableReferenceList();

    void BeforeShutdown() override;

    /**
     * @brief Method creates CallbackPosterFactory with your implementation. Here we use template to avoid
     * usage of InternalAllocator outside the virtual machine.
     */
    template <class FactoryImpl, class... Args>
    void CreateCallbackPosterFactory(Args... args)
    {
        static_assert(std::is_base_of_v<CallbackPosterFactoryIface, FactoryImpl>);
        static_assert(std::is_constructible_v<FactoryImpl, Args...>);
        ASSERT(callbackPosterFactory_ == nullptr);
        callbackPosterFactory_ = MakePandaUnique<FactoryImpl>(args...);
    }

    /// @brief Uses CallbackPosterFactory to create a CallbackPoster
    PandaUniquePtr<CallbackPoster> CreateCallbackPoster()
    {
        auto *coro = EtsCoroutine::GetCurrent();
        if (coro != coro->GetPandaVM()->GetCoroutineManager()->GetMainThread()) {
            return nullptr;
        }
        ASSERT(callbackPosterFactory_ != nullptr);
        return callbackPosterFactory_->CreatePoster();
    }

protected:
    bool CheckEntrypointSignature(Method *entrypoint) override;
    Expected<int, Runtime::Error> InvokeEntrypointImpl(Method *entrypoint,
                                                       const std::vector<std::string> &args) override;
    void HandleUncaughtException() override;

private:
    /**
     * @brief Update a VM root that has been moved by GC.
     * @param ref a reference to update, should hold an ObjectHeader pointer
     * @tparam REF_CAN_BE_NULL true iff it is legal for @param ref to hold a null pointer
     */
    template <bool REF_CAN_BE_NULL>
    static void UpdateMovedVmRef(Value &ref);

    void InitializeRandomEngine()
    {
        ASSERT(!randomEngine_);
        std::random_device rd;
        randomEngine_.emplace(rd());
    }

    explicit PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm);

    Runtime *runtime_ {nullptr};
    mem::MemoryManager *mm_ {nullptr};
    PandaUniquePtr<EtsClassLinker> classLinker_;
    mem::ReferenceProcessor *referenceProcessor_ {nullptr};
    PandaVector<ObjectHeader *> gcRoots_;
    Rendezvous *rendezvous_ {nullptr};
    CompilerInterface *compiler_ {nullptr};
    StringTable *stringTable_ {nullptr};
    MonitorPool *monitorPool_ {nullptr};
    CoroutineManager *coroutineManager_ {nullptr};
    mem::Reference *oomObjRef_ {nullptr};
    compiler::RuntimeInterface *runtimeIface_ {nullptr};
    mem::Reference *undefinedObjRef_ {nullptr};
    mem::Reference *finalizableWeakRefList_ {nullptr};
    os::memory::Mutex finalizableWeakRefListLock_;
    NativeLibraryProvider nativeLibraryProvider_;
    os::memory::Mutex finalizationRegistryLock_;
    PandaList<EtsObject *> registeredFinalizationRegistryInstances_ GUARDED_BY(finalizationRegistryLock_);
    PandaUniquePtr<JobQueue> jobQueue_;
    Taskpool *taskpool_ {nullptr};
    PandaUniquePtr<CallbackPosterFactoryIface> callbackPosterFactory_;
    // optional for lazy initialization
    std::optional<std::mt19937> randomEngine_;
    std::function<void(Frame *)> clearInteropHandleScopes_;
    std::function<void(void *)> destroyExternalData_;
    // for JS Atomics
    os::memory::Mutex atomicsMutex_;
    DoubleToStringCache *doubleToStringCache_ {nullptr};
    FloatToStringCache *floatToStringCache_ {nullptr};
    LongToStringCache *longToStringCache_ {nullptr};

    ExternalData externalData_ {};

    NO_MOVE_SEMANTIC(PandaEtsVM);
    NO_COPY_SEMANTIC(PandaEtsVM);

    friend class mem::Allocator;
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_
