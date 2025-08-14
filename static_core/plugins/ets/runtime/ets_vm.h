/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "include/mem/panda_smart_pointers.h"
#include "libpandabase/macros.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/utils/expected.h"
#include "libpandabase/os/mutex.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/external_callback_poster.h"
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
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ets_native_library_provider.h"
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/mem/root_provider.h"
#include "plugins/ets/runtime/ets_object_state_table.h"
#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"

namespace ark::ets {
class DoubleToStringCache;
class FloatToStringCache;
class LongToStringCache;
class EtsAbcRuntimeLinker;
class EtsFinalizableWeakRef;

using WalkEventLoopCallback = std::function<void(void *, void *)>;

enum class EventLoopRunMode : int { RUN_DEFAULT = 0, RUN_ONCE, RUN_NOWAIT };

class PandaEtsVM final : public PandaVM, public EtsVM, public ani_vm {  // NOLINT(fuchsia-multiple-inheritance)
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
    void VisitVmRoots(const GCRootVisitor &visitor) override;
    void UpdateVmRefs(const GCRootUpdater &gcRootUpdater) override;
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

    FinalizationRegistryManager *GetFinalizationRegistryManager() const
    {
        return finalizationRegistryManager_;
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

    PANDA_PUBLIC_API ObjectHeader *GetNullValue() const;

    compiler::RuntimeInterface *GetCompilerRuntimeInterface() const override
    {
        return runtimeIface_;
    }

    bool LoadNativeLibrary(EtsEnv *env, const PandaString &name, bool shouldVerifyPermission,
                           const PandaString &fileName);

    void ResolveNativeMethod(Method *method);

    static PandaEtsVM *FromEtsVM(EtsVM *vm)
    {
        return static_cast<PandaEtsVM *>(vm);
    }

    static PandaEtsVM *FromAniVM(ani_vm *vm)
    {
        return static_cast<PandaEtsVM *>(vm);
    }

    [[noreturn]] static void Abort(const char *message = nullptr);

    std::mt19937 &GetRandomEngine()
    {
        ASSERT(randomEngine_);
        return *randomEngine_;
    }

    bool IsStaticProfileEnabled() const override
    {
        return true;
    }

    void CleanupCompiledFrameResources(Frame *frame) override
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *ifaces = coro->GetExternalIfaceTable();
        if (ifaces->GetClearInteropHandleScopesFunction()) {
            ifaces->GetClearInteropHandleScopesFunction()(frame);
        }
    }

    os::memory::Mutex &GetAniBindMutex()
    {
        return aniBindMutex_;
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

    /// @brief Method creates CallBackPoster using factory.
    PandaUniquePtr<CallbackPoster> CreateCallbackPoster()
    {
        if (callbackPosterFactory_ == nullptr) {
            return nullptr;
        }
        return callbackPosterFactory_->CreatePoster();
    }

    using RunEventLoopFunction = std::function<void(EventLoopRunMode)>;

    void SetRunEventLoopFunction(RunEventLoopFunction &&cb)
    {
        ASSERT(!runEventLoop_);
        runEventLoop_ = std::move(cb);
    }

    void RunEventLoop(EventLoopRunMode mode)
    {
        if (runEventLoop_) {
            runEventLoop_(mode);
        }
    }

    using WalkEventLoopFunction = std::function<void(WalkEventLoopCallback &, void *)>;
    void SetWalkEventLoopFunction(WalkEventLoopFunction &&cb)
    {
        ASSERT(!walkEventLoop_);
        walkEventLoop_ = std::move(cb);
    }

    void WalkEventLoop(WalkEventLoopCallback &callback, void *args)
    {
        if (walkEventLoop_) {
            walkEventLoop_(callback, args);
        }
    }

    EtsObjectStateTable *GetEtsObjectStateTable() const
    {
        return objStateTable_.get();
    }

    void FreeInternalResources() override
    {
        objStateTable_->DeflateInfo();
    }

    /// @brief Adds failed job to an internal storage
    void AddUnhandledFailedJob(EtsJob *job);

    /// @brief Removes failed job from internal storage
    void RemoveUnhandledFailedJob(EtsJob *job);

    /// @brief Invokes managed method to apply custom handler on stored failed jobs
    void ListUnhandledFailedJobs();

    /// @brief Adds rejected promise to an internal storage
    void AddUnhandledRejectedPromise(EtsPromise *promise);

    /// @brief Removes rejected promise from internal storage
    void RemoveUnhandledRejectedPromise(EtsPromise *promise);

    /// @brief Invokes managed method to apply custom handler on stored rejected promises
    void ListUnhandledRejectedPromises();

    PANDA_PUBLIC_API void AddRootProvider(mem::RootProvider *provider);
    PANDA_PUBLIC_API void RemoveRootProvider(mem::RootProvider *provider);

    /// @brief Create application `AbcRuntimeLinker` in managed scope.
    ClassLinkerContext *CreateApplicationRuntimeLinker(const PandaVector<PandaString> &abcFiles);

    /// @brief print stack and exit the program
    [[noreturn]] void HandleUncaughtException() override;

protected:
    bool CheckEntrypointSignature(Method *entrypoint) override;
    Expected<int, Runtime::Error> InvokeEntrypointImpl(Method *entrypoint,
                                                       const std::vector<std::string> &args) override;

private:
    /**
     * @brief Update a VM root that has been moved by GC.
     * @param ref a reference to update, should hold an ObjectHeader pointer
     * @tparam REF_CAN_BE_NULL true iff it is legal for @param ref to hold a null pointer
     */
    template <bool REF_CAN_BE_NULL>
    static void UpdateMovedVmRef(Value &ref, const GCRootUpdater &gcRootUpdater);

    static void UpdateManagedEntrypointArgRefs(EtsCoroutine *coroutine, const GCRootUpdater &gcRootUpdater);

    void InitializeRandomEngine()
    {
        ASSERT(!randomEngine_);
        std::random_device rd;
        randomEngine_.emplace(rd());
    }

    explicit PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm);

    void AddUnhandledObjectImpl(PandaUnorderedSet<EtsObject *> &unhandledObjects, EtsObject *object);
    void RemoveUnhandledObjectImpl(PandaUnorderedSet<EtsObject *> &unhandledObjects, EtsObject *object);

    Runtime *runtime_ {nullptr};
    mem::MemoryManager *mm_ {nullptr};
    PandaUniquePtr<EtsClassLinker> classLinker_;
    mem::ReferenceProcessor *referenceProcessor_ {nullptr};
    PandaVector<ObjectHeader *> gcRoots_;
    Rendezvous *rendezvous_ {nullptr};
    CompilerInterface *compiler_ {nullptr};
    StringTable *stringTable_ {nullptr};
    MonitorPool *monitorPool_ {nullptr};
    FinalizationRegistryManager *finalizationRegistryManager_ {nullptr};
    CoroutineManager *coroutineManager_ {nullptr};
    mem::Reference *oomObjRef_ {nullptr};
    compiler::RuntimeInterface *runtimeIface_ {nullptr};
    mem::Reference *nullValueRef_ {nullptr};
    mem::Reference *finalizableWeakRefList_ {nullptr};
    os::memory::Mutex finalizableWeakRefListLock_;
    NativeLibraryProvider nativeLibraryProvider_;
    PandaUniquePtr<CallbackPosterFactoryIface> callbackPosterFactory_;
    os::memory::Mutex rootProviderlock_;
    PandaUnorderedSet<mem::RootProvider *> rootProviders_ GUARDED_BY(rootProviderlock_);
    os::memory::Mutex aniBindMutex_;
    // optional for lazy initialization
    std::optional<std::mt19937> randomEngine_;
    // for JS Atomics
    os::memory::Mutex atomicsMutex_;
    DoubleToStringCache *doubleToStringCache_ {nullptr};
    FloatToStringCache *floatToStringCache_ {nullptr};
    LongToStringCache *longToStringCache_ {nullptr};
    os::memory::Mutex unhandledMutex_;
    PandaUnorderedSet<EtsObject *> unhandledFailedJobs_;
    PandaUnorderedSet<EtsObject *> unhandledRejectedPromises_;

    PandaUniquePtr<EtsObjectStateTable> objStateTable_ {nullptr};
    RunEventLoopFunction runEventLoop_ = nullptr;
    WalkEventLoopFunction walkEventLoop_ = nullptr;

    size_t preForkWorkerCount_ = 0;

    NO_MOVE_SEMANTIC(PandaEtsVM);
    NO_COPY_SEMANTIC(PandaEtsVM);

    friend class mem::Allocator;
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_
