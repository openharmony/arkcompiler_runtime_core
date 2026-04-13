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

#include "plugins/ets/runtime/ets_vm.h"
#include <atomic>

#include "compiler/optimizer/ir/runtime_interface.h"
#include "execution/job_execution_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_execution_context_wrapper.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/mem/panda_string.h"
#include "jit/profile_saver_worker.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani_vm_api.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_vm_api.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_vm_options.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_taskpool.h"
#include "runtime/compiler.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/value-inl.h"
#include "runtime/init_icu.h"
#include "runtime/execution/coroutines/stackful/stackful_coroutine_manager.h"
#include "runtime/execution/stackless/stackless_job_manager.h"
#include "runtime/execution/stackless/job_manager_config.h"
#include "runtime/mem/lock_config_helper.h"
#include "runtime/class_lock.h"
#include "plugins/ets/stdlib/native/init_native_methods.h"
#include "plugins/ets/runtime/types/ets_error.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref_list.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"
#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"

#include "plugins/ets/runtime/ets_object_state_table.h"
#include "libarkbase/taskmanager/task_manager.h"

namespace ark::mem::ets {
common_vm::VMInterface *RegisterCmcGcCallbacks();
}  // namespace ark::mem::ets

namespace ark::ets {

static PandaEtsVM *g_pandaEtsVM = nullptr;

// Create MemoryManager by RuntimeOptions
static std::pair<mem::MemoryManager *, common_vm::VMInterface *> CreateMM(Runtime *runtime,
                                                                          const RuntimeOptions &options)
{
    mem::MemoryManager::HeapOptions heapOptions {
        nullptr,                                      // is_object_finalizeble_func
        nullptr,                                      // register_finalize_reference_func
        options.GetMaxGlobalRefSize(),                // max_global_ref_size
        options.IsGlobalReferenceSizeCheckEnabled(),  // is_global_reference_size_check_enabled
        MT_MODE_TASK,                                 // multithreading mode
        options.IsUseTlabForAllocations(),            // is_use_tlab_for_allocations
        options.IsStartAsZygote(),                    // is_start_as_zygote
    };

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);
    auto allocator = runtime->GetInternalAllocator();

    mem::GCTriggerConfig gcTriggerConfig(options, panda_file::SourceLang::ETS);

    mem::GCSettings gcSettings(options, panda_file::SourceLang::ETS);

    auto gcType = Runtime::GetGCType(options, panda_file::SourceLang::ETS);

    common_vm::VMInterface *vmIface {nullptr};
#if defined(ARK_USE_COMMON_RUNTIME)
    vmIface = ark::mem::ets::RegisterCmcGcCallbacks();
#endif

    auto *memMgr = mem::MemoryManager::Create(ctx, allocator, gcType, gcSettings, gcTriggerConfig, heapOptions);
    return std::make_pair(memMgr, vmIface);
}

static CoroutineManagerConfig CreateCoroutineManagerConfig(const RuntimeOptions &options)
{
    auto &taskPoolMode = options.GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));
    CoroutineManagerConfig cfg {
        // enable drain queue interface
        options.IsCoroutineEnableFeaturesAniDrainQueue(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable migration
        options.IsCoroutineEnableFeaturesMigration(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable migrate awakened coroutines
        options.IsCoroutineEnableFeaturesMigrateAwakened(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // workers_count
        options.GetCoroutineWorkersCount(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // exclusive workers limit
        options.GetCoroutineEWorkersLimit(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable perf stats
        options.IsCoroutineDumpStats(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable external timer implementation
        options.IsCoroutineEnableFeaturesEnableExternalTimer(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // number of reserved workers for taskpool
        taskPoolMode == ets::intrinsics::taskpool::TASKPOOL_EAWORKER_MODE
            ? ets::intrinsics::taskpool::TASKPOOL_EAWORKER_INIT_NUM
            : 0};
    return cfg;
}

/* static */
bool PandaEtsVM::CreateTaskManagerIfNeeded(const RuntimeOptions &options)
{
    if (options.GetWorkersType() == "taskmanager" && !Runtime::IsTaskManagerUsed()) {
        taskmanager::TaskManager::Start(options.GetTaskmanagerWorkersCount(),
                                        taskmanager::StringToTaskTimeStats(options.GetTaskStatsType()));
        taskmanager::TaskManager::EnableTimerThread();
        Runtime::SetTaskManagerUsed(true);
    }
    return true;
}

/* static */
Expected<PandaEtsVM *, PandaString> PandaEtsVM::Create(Runtime *runtime, const RuntimeOptions &options)
{
    ASSERT(runtime != nullptr);

    // NOTE(dslynko, #33090): re-enable or remove
    if (verifier::VerificationModeFromString(options.GetVerificationMode()) == verifier::VerificationMode::ON_THE_FLY) {
        return Unexpected(PandaString("on-the-fly verification mode is not supported in ets"));
    }

    if (!PandaEtsVM::CreateTaskManagerIfNeeded(options)) {
        return Unexpected(PandaString("Cannot create TaskManager"));
    }

    auto [mm, vmIface] = CreateMM(runtime, options);
    if (mm == nullptr) {
        return Unexpected(PandaString("Cannot create MemoryManager"));
    }

    auto allocator = mm->GetHeapManager()->GetInternalAllocator();
    auto vm = allocator->New<PandaEtsVM>(runtime, options, mm, vmIface);
    if (vm == nullptr) {
        return Unexpected(PandaString("Cannot create PandaCoreVM"));
    }

    mem::StaticObjectOperator::Initialize();

    auto classLinker = EtsClassLinker::Create(runtime->GetClassLinker());
    if (!classLinker) {
        allocator->Delete(vm);
        mem::MemoryManager::Destroy(mm);
        return Unexpected(classLinker.Error());
    }
    vm->classLinker_ = std::move(classLinker.Value());

    vm->InitializeGC();
    vm->GetGC()->AddListener(vm->fullGCLongTimeListener_);

    const auto &icuPath = options.GetIcuDataPath();
    if (icuPath == "default") {
        SetIcuDirectory();
    } else {
        u_setDataDirectory(icuPath.c_str());
    }

    vm->jobManager_->InitializeScheduler(runtime, vm);

    g_pandaEtsVM = vm;
    return vm;
}

bool PandaEtsVM::Destroy(PandaEtsVM *vm)
{
    if (vm == nullptr) {
        return false;
    }
    g_pandaEtsVM = nullptr;

    vm->SaveProfileInfo();
    vm->UninitializeThreads();
    vm->StopGC();

    auto runtime = Runtime::GetCurrent();
    runtime->GetInternalAllocator()->Delete(vm);

    runtime->StopCoverageListener();

    return true;
}

void PandaEtsVM::InitializeANI(const RuntimeOptions &options)
{
    const EtsVmOptions *etsVmOptions = GetEtsVmOptions(options);
    bool isVerifyANI = etsVmOptions == nullptr ? false : etsVmOptions->IsVerifyANI();
    isVerifyANI |= options.WasSetVerifyAni();
    if (isVerifyANI) {
        bool useWorkaround =
            etsVmOptions == nullptr ? false : etsVmOptions->IsVerifyANIWorkaroundNoCrashIfInvalidUsage();
        aniVerifier_ = MakePandaUnique<ani::verify::ANIVerifier>();
        aniVerifier_->SetVerifyOptions(useWorkaround);
        c_api = ani::verify::GetVerifyVMAPI();
    }
}

// CC-OFFNXT(G.FUD.05) solid logic
PandaEtsVM::PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm,
                       common_vm::VMInterface *vmIface)
    : ani_vm {ani::GetVMAPI()}, runtime_(runtime), mm_(mm), vmIface_(vmIface)
{
    ASSERT(runtime_ != nullptr);
    ASSERT(mm_ != nullptr);

    InitializeANI(options);
    auto heapManager = mm_->GetHeapManager();
    auto allocator = heapManager->GetInternalAllocator();

    runtimeIface_ = allocator->New<EtsRuntimeInterface>();
    if (options.IsIncrementalProfilesaverEnabled()) {
        if (!taskmanager::TaskManager::IsUsed()) {
            LOG(WARNING, RUNTIME)
                << "[profile_saver] Cannot get current taskScheduler, disable incremental profile saver.";
        } else {
            saverWorker_ = allocator->New<ProfileSaverWorker>(allocator);
            LOG(INFO, RUNTIME) << "[profile_saver] Profile saver worker created.";
        }
    } else {
        LOG(INFO, RUNTIME) << "[profile_saver] Incremental profile saver disabled.";
    }
    compiler_ = allocator->New<Compiler>(heapManager->GetCodeAllocator(), allocator, options,
                                         heapManager->GetMemStats(), runtimeIface_);
    stringTable_ = allocator->New<StringTable>();
    monitorPool_ = allocator->New<MonitorPool>(allocator);
    finalizationRegistryManager_ = allocator->New<FinalizationRegistryManager>(this);
    referenceProcessor_ = allocator->New<mem::ets::EtsReferenceProcessor>(mm_->GetGC(), this);
    unhandledObjectManager_ = allocator->New<UnhandledObjectManager>(this);
    fullGCLongTimeListener_ = allocator->New<FullGCLongTimeListener>();

    auto langStr = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    auto cfg = CreateCoroutineManagerConfig(options);
    const auto &coroType = options.GetCoroutineImpl(langStr);
    if (coroType == "stackful") {
        jobManager_ = allocator->New<StackfulCoroutineManager>(cfg, EtsCoroutine::Create<Coroutine>);
    } else if (coroType == "stackless") {
        jobManager_ = allocator->New<StacklessJobManager>(JobManagerConfig {}, EtsExecutionContextWrapper::Create);
    } else {
        UNREACHABLE();
    }
    rendezvous_ = allocator->New<Rendezvous>(this);
    objStateTable_ = MakePandaUnique<EtsObjectStateTable>(allocator);
    InitializeRandomEngine();
}

PandaEtsVM::~PandaEtsVM()
{
    auto allocator = mm_->GetHeapManager()->GetInternalAllocator();
    ASSERT(allocator != nullptr);

    allocator->Delete(rendezvous_);
    allocator->Delete(runtimeIface_);
    allocator->Delete(jobManager_);
    allocator->Delete(referenceProcessor_);
    allocator->Delete(monitorPool_);
    allocator->Delete(finalizationRegistryManager_);
    allocator->Delete(stringTable_);
    allocator->Delete(compiler_);
    allocator->Delete(unhandledObjectManager_);
    allocator->Delete(fullGCLongTimeListener_);
    if (saverWorker_ != nullptr) {
        allocator->Delete(saverWorker_);
    }

    objStateTable_.reset();

    ASSERT(mm_ != nullptr);
    mm_->Finalize();
    mem::MemoryManager::Destroy(mm_);
}

PandaEtsVM *PandaEtsVM::GetCurrent()
{
    return g_pandaEtsVM;
}

static mem::Reference *PreallocSpecialReference(PandaEtsVM *vm, EtsClass *cls, bool nonMovable = false)
{
    EtsObject *obj = nonMovable ? EtsObject::CreateNonMovable(cls) : EtsObject::Create(cls);
    if (obj == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate a special object " << cls->GetDescriptor();
    }
    return vm->GetGlobalObjectStorage()->Add(obj->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
}

static mem::Reference *PreallocOOMError(EtsExecutionContext *executionCtx, PandaEtsVM *vm)
{
    ASSERT(executionCtx != nullptr);
    auto *oom = EtsOutOfMemoryError::Create(executionCtx);
    if (oom == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate OOM error";
    }

    return vm->GetGlobalObjectStorage()->Add(oom->AsObject()->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
}

static void CreateEtsObjects(PandaVector<CoroutineWorker::LocalObjectData> &objectRefs)
{
    if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        EtsHandleScope scope(executionCtx);
        EtsHandle<DoubleToStringCache> cacheDouble(executionCtx, DoubleToStringCache::Create(executionCtx));
        EtsHandle<FloatToStringCache> cacheFloat(executionCtx, FloatToStringCache::Create(executionCtx));
        EtsHandle<LongToStringCache> cacheLong(executionCtx, LongToStringCache::Create(executionCtx));
        if (cacheDouble.GetPtr() == nullptr || cacheFloat.GetPtr() == nullptr || cacheLong.GetPtr() == nullptr) {
            LOG(WARNING, ETS) << "Cannot initialize number-to-string caches";
            return;
        }

        LOG(DEBUG, ETS) << "Initialized number-to-string caches";
        auto *refStorage = executionCtx->GetPandaVM()->GetGlobalObjectStorage();
        auto *cacheRefDouble = refStorage->Add(cacheDouble->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        objectRefs.push_back(CoroutineWorker::LocalObjectData {
            CoroutineWorker::DataIdx::DOUBLE_TO_STRING_CACHE, cacheRefDouble,
            [refStorage](void *ref) { refStorage->Remove(static_cast<mem::Reference *>(ref)); }});

        auto *cacheRefFloat = refStorage->Add(cacheFloat->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        objectRefs.push_back(CoroutineWorker::LocalObjectData {
            CoroutineWorker::DataIdx::FLOAT_TO_STRING_CACHE, cacheRefFloat,
            [refStorage](void *ref) { refStorage->Remove(static_cast<mem::Reference *>(ref)); }});

        auto *cacheRefLong = refStorage->Add(cacheLong->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        objectRefs.push_back(CoroutineWorker::LocalObjectData {
            CoroutineWorker::DataIdx::LONG_TO_STRING_CACHE, cacheRefLong,
            [refStorage](void *ref) { refStorage->Remove(static_cast<mem::Reference *>(ref)); }});
    }
}

bool PandaEtsVM::Initialize()
{
    if (!ark::intrinsics::Initialize(ark::panda_file::SourceLang::ETS)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize eTS intrinsics";
        return false;
    }

    if (!classLinker_->Initialize()) {
        LOG(FATAL, ETS) << "Cannot initialize ets class linker";
    }
    classLinker_->GetEtsClassLinkerExtension()->InitializeBuiltinClasses();

    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        ASSERT(ManagedThread::GetCurrent() != nullptr);
        ASSERT(GetThreadManager()->GetMainThread() == ManagedThread::GetCurrent());
        auto *executionCtx = EtsExecutionContext::GetCurrent();

        ASSERT(executionCtx != nullptr);
        executionCtx->GetLocalStorage().Set<EtsExecutionContext::DataIdx::ETS_PLATFORM_TYPES_PTR>(
            ToUintPtr(classLinker_->GetEtsClassLinkerExtension()->GetPlatformTypes()));
        ASSERT(PlatformTypes(executionCtx) != nullptr);

        // Should be invoked after PlatformTypes is initialized in coroutine.
        oomObjRef_ = PreallocOOMError(executionCtx, this);
        nullValueRef_ = PreallocSpecialReference(this, PlatformTypes(executionCtx)->coreNull, true);
        finalizableWeakRefList_ = PreallocSpecialReference(this, PlatformTypes(executionCtx)->coreFinalizableWeakRef);

        executionCtx->SetupNullValue(GetNullValue());
        referenceProcessor_->Initialize();
        jobManager_->InitializeManagedStructures(CreateEtsObjects);
    }

    // Check if Intrinsics/native methods should be initialized, we don't want to attempt to
    // initialize  native methods in certain scenarios where we don't have ets stdlib at our disposal
    if (Runtime::GetOptions().ShouldInitializeIntrinsics()) {
        // NOTE(ksarychev, #18135): Implement napi module registration via loading a separate
        // library
        ani_env *env = EtsExecutionContext::GetCurrent()->GetPandaAniEnv();
        ark::ets::stdlib::InitNativeMethods(env);

        stdLibCache_ = CreateStdLibCache(env);
    }
    const auto lang = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    for (const auto &path : Runtime::GetOptions().GetNativeLibraryPath(lang)) {
        nativeLibraryProvider_.AddLibraryPath(ConvertToString(path));
    }

    return true;
}

bool PandaEtsVM::InitializeFinish()
{
    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        // Preinitialize StackOverflowError, so we don't need to do this when stack overflow occurred
        EtsClass *cls = PlatformTypes()->coreStackOverflowError;
        if (cls == nullptr) {
            LOG(FATAL, ETS) << "Cannot preinitialize StackOverflowError";
            return false;
        }
    }
    // Initialize platform classes only if intrinsics were loaded, because static initializers might call them
    if (!Runtime::GetOptions().IsMockRuntime() && Runtime::GetOptions().ShouldInitializeIntrinsics()) {
        classLinker_->GetEtsClassLinkerExtension()->InitializeFinish();
    }
    return true;
}

void PandaEtsVM::UninitializeThreads()
{
    // Wait until all threads finish the work
    jobManager_->WaitForDeregistration();
    jobManager_->Finalize();
}

void PandaEtsVM::PreStartup()
{
    ASSERT(mm_ != nullptr);

    mm_->PreStartup();
}

void PandaEtsVM::PreZygoteFork()
{
    ASSERT(mm_ != nullptr);
    ASSERT(compiler_ != nullptr);
    ASSERT(jobManager_ != nullptr);

    mm_->PreZygoteFork();
    compiler_->PreZygoteFork();

    if (saverWorker_ != nullptr) {
        saverWorker_->PreZygoteFork();
    }
    jobManager_->PreZygoteFork();

    if (taskmanager::TaskManager::IsUsed()) {
        preForkWorkerCount_ = taskmanager::TaskManager::GetWorkersCount();
        taskmanager::TaskManager::DisableTimerThread();
        taskmanager::TaskManager::SetWorkersCount(0U);
    }
}

void PandaEtsVM::PostZygoteFork()
{
    ASSERT(compiler_ != nullptr);
    ASSERT(mm_ != nullptr);
    ASSERT(jobManager_ != nullptr);

    if (taskmanager::TaskManager::IsUsed()) {
        taskmanager::TaskManager::SetWorkersCount(preForkWorkerCount_);
        taskmanager::TaskManager::EnableTimerThread();
    }
    jobManager_->PostZygoteFork();
    compiler_->PostZygoteFork();
    mm_->PostZygoteFork();
    if (saverWorker_ != nullptr) {
        saverWorker_->PostZygoteFork();
    }

    auto runtime = Runtime::GetCurrent();
    runtime->StartCoverageListener();

    // stop GCtask for application start-up
    PostForkStart();
}

void PandaEtsVM::PostForkStart()
{
    isPostFork_ = true;
    // Add a delay GCTask.
    if ((!Runtime::GetCurrent()->IsZygote()) && (!mm_->GetGC()->GetSettings()->RunGCInPlace())) {
        // set target footprint to a high value to disable GC during App startup.
        size_t startupTargetFootprint = Runtime::GetOptions().GetHeapSizeLimit() / 2;
        size_t startupLimit = startupTargetFootprint / 2;
        mm_->GetGCTrigger()->SetMinTargetFootprint(startupTargetFootprint);
        originSize_ = mm_->GetGC()->AdjustStartupLimit(startupLimit);
        constexpr uint64_t DISABLE_GC_DURATION_NS = 2000ULL * 1000 * 1000;
        auto task = MakePandaUnique<GCTask>(GCTaskCause::STARTUP_COMPLETE_CAUSE,
                                            time::GetCurrentTimeInNanos() + DISABLE_GC_DURATION_NS);
        mm_->GetGC()->AddGCTask(true, std::move(task));
        LOG(DEBUG, GC) << "Add Startup Complete GCTask";
    }
}

void PandaEtsVM::PostForkEnd()
{
    mm_->GetGC()->AdjustStartupLimit(originSize_);
    isPostFork_ = false;
}

void PandaEtsVM::InitializeGC()
{
    ASSERT(mm_ != nullptr);

    mm_->InitializeGC(this);
}

void PandaEtsVM::StartGC()
{
    ASSERT(mm_ != nullptr);

    mm_->StartGC();
}

void PandaEtsVM::StopGC()
{
    ASSERT(mm_ != nullptr);

#if defined(ARK_USE_COMMON_RUNTIME)
    // NOTE(ivagin): #33823 Introduce "IsGCRunning" interface for common_runtime
    mm_->StopGC();
#else
    if (GetGC()->IsGCRunning()) {
        mm_->StopGC();
    }
#endif
}

void PandaEtsVM::HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred)
{
    ASSERT(mm_ != nullptr);

    auto gc = mm_->GetGC();
    ASSERT(gc != nullptr);

    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    gc->ProcessReferences(gc->GetGCPhase(), task, pred);
    LOG(DEBUG, REF_PROC) << "Finish processing cleared references";

    GetGlobalObjectStorage()->ClearUnmarkedWeakRefs(gc, pred);
}

void PandaEtsVM::HandleGCRoutineInMutator()
{
    // Handle references only in coroutine
    ASSERT(ManagedThread::GetCurrent() != nullptr);
    ASSERT(GetMutatorLock()->HasLock());
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    GetFinalizationRegistryManager()->LaunchCleanupJobIfNeeded(executionCtx);
    executionCtx->GetPandaVM()->CleanFinalizableReferenceList();
}

void PandaEtsVM::HandleGCFinished() {}

bool PandaEtsVM::CheckEntrypointSignature(Method *entrypoint)
{
    ASSERT(entrypoint != nullptr);

    if (entrypoint->GetReturnType().GetId() != panda_file::Type::TypeId::I32 &&
        entrypoint->GetReturnType().GetId() != panda_file::Type::TypeId::VOID && !entrypoint->HasAsyncAnnotation()) {
        return false;
    }

    if (entrypoint->GetNumArgs() == 0) {
        return true;
    }

    if (entrypoint->GetNumArgs() > 1) {
        return false;
    }

    auto *pf = entrypoint->GetPandaFile();
    ASSERT(pf != nullptr);
    panda_file::MethodDataAccessor mda(*pf, entrypoint->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

    if (pda.GetArgType(0).GetId() != panda_file::Type::TypeId::REFERENCE) {
        return false;
    }

    auto name = pf->GetStringData(pda.GetReferenceType(0));
    auto *expectedClass = PlatformTypes()->coreStringFixedArray;

    return utf::IsEqual({name.data, name.utf16Length}, {utf::CStringAsMutf8(expectedClass->GetDescriptor()),
                                                        std::strlen(expectedClass->GetDescriptor())});
}

static EtsObjectArray *CreateArgumentsArray(const std::vector<std::string> &args, PandaEtsVM *etsVm)
{
    EtsClass *arrayKlass = PlatformTypes(etsVm)->coreStringFixedArray;

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsObjectArray *etsArray = EtsObjectArray::Create(arrayKlass, args.size());
    EtsHandle<EtsObjectArray> arrayHandle(executionCtx, etsArray);

    for (size_t i = 0; i < args.size(); i++) {
        EtsString *str = EtsString::CreateFromMUtf8(args[i].data(), args[i].length());
        ASSERT(arrayHandle.GetPtr() != nullptr);
        arrayHandle.GetPtr()->Set(i, str->AsObject());
    }

    return arrayHandle.GetPtr();
}

coretypes::String *PandaEtsVM::CreateString(Method *ctor, ObjectHeader *obj)
{
    EtsString *str = nullptr;
    ASSERT(ctor->GetNumArgs() > 0);  // must be at list this argument
    if (ctor->GetNumArgs() == 1) {
        str = EtsString::CreateNewEmptyString();
    } else if (ctor->GetNumArgs() == 2U) {
        ASSERT(ctor->GetArgType(1).GetId() == panda_file::Type::TypeId::REFERENCE);
        auto *strData = utf::Mutf8AsCString(ctor->GetRefArgType(1).data);
        if (std::strcmp("[C", strData) == 0) {
            auto *array = reinterpret_cast<EtsArray *>(obj);
            str = EtsString::CreateNewStringFromChars(0, array->GetLength(), array);
        } else if ((std::strcmp("Lstd/core/String;", strData) == 0)) {
            str = EtsString::CreateNewStringFromString(reinterpret_cast<EtsString *>(obj));
        } else {
            LOG(FATAL, ETS) << "Non-existent ctor";
        }
    } else {
        LOG(FATAL, ETS) << "Must be 1 or 2 ctor args";
    }
    return str == nullptr ? nullptr : str->GetCoreType();
}

static Expected<int, Runtime::Error> WaitForEntrypointCompletion(Value result)
{
    if (!result.IsReference()) {
        return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    EtsHandle<EtsPromise> promise(executionCtx, EtsPromise::FromCoreType(result.GetAs<ObjectHeader *>()));
    ASSERT(promise->GetClass() == PlatformTypes(executionCtx)->corePromise);

    while (promise->IsPending()) {
        ScopedNativeCodeThread nativeCode(executionCtx->GetMT());
        JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager()->ExecuteJobs();
    }

    ASSERT(promise->IsResolved() || promise->IsRejected());
    if (promise->IsRejected()) {
        executionCtx->GetMT()->SetException(promise->GetValue(executionCtx)->GetCoreType());
    } else {
        // NOTE(panferovi): try to obtain int from promise??
    }

    return 0;
}

Expected<int, Runtime::Error> PandaEtsVM::InvokeEntrypointImpl(Method *entrypoint, const std::vector<std::string> &args)
{
    ASSERT(Runtime::GetCurrent()->GetLanguageContext(*entrypoint).GetLanguage() == panda_file::SourceLang::ETS);

    if (entrypoint->GetNumArgs() > 1) {
        // What if entrypoint->GetNumArgs() > 1 ?
        LOG(ERROR, RUNTIME) << "ets entrypoint has args count more than 1 : " << entrypoint->GetNumArgs();
        return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    ScopedManagedCodeThread sj(executionCtx->GetMT());
    if (!classLinker_->InitializeClass(executionCtx, EtsClass::FromRuntimeClass(entrypoint->GetClass()))) {
        LOG(ERROR, RUNTIME) << "Cannot initialize class '" << entrypoint->GetClass()->GetName() << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_INITIALIZED);
    }

    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    Value *entrypointArgs = nullptr;
    Value argValue;

    if (entrypoint->GetNumArgs() == 1) {
        EtsObjectArray *etsObjectArray = CreateArgumentsArray(args, this);
        argValue = Value(etsObjectArray->GetCoreType());
        entrypointArgs = &argValue;
    }

    auto v = entrypoint->Invoke(executionCtx->GetMT(), entrypointArgs);
    if (entrypoint->HasAsyncAnnotation()) {
        return WaitForEntrypointCompletion(v);
    }

    return v.GetAs<int>();
}

ObjectHeader *PandaEtsVM::GetOOMErrorObject()
{
    auto obj = GetGlobalObjectStorage()->Get(oomObjRef_);
    ASSERT(obj != nullptr);
    return obj;
}

ObjectHeader *PandaEtsVM::GetNullValue() const
{
    auto obj = GetGlobalObjectStorage()->Get(nullValueRef_);
    ASSERT(obj != nullptr);
    return obj;
}

bool PandaEtsVM::LoadNativeLibrary(ani_env *env, const PandaString &name, bool shouldVerifyPermission,
                                   const PandaString &fileName)
{
    ASSERT_PRINT(ManagedThread::GetCurrent()->IsInNativeCode(), "LoadNativeLibrary must be called at native");

    if (auto error = nativeLibraryProvider_.LoadLibrary(env, name, shouldVerifyPermission, fileName)) {
        LOG(ERROR, RUNTIME) << "Cannot load library " << name << ": " << error.value();
        return false;
    }

    return true;
}

void PandaEtsVM::HandleUncaughtException()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    ScopedManagedCodeThread sj(executionCtx->GetMT());
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    EtsHandle<EtsObject> exception(executionCtx, EtsObject::FromCoreType(executionCtx->GetMT()->GetException()));

    GetUnhandledObjectManager()->InvokeErrorHandler(executionCtx, exception);
}

void HandleEmptyArguments(PandaVector<Value> &arguments, const GCRootVisitor &visitor, const Job *job)
{
    // arguments may be empty in the following cases:
    // 1. The entrypoint is static and doesn't accept any arguments
    // 2. The job is launched.
    // 3. The entrypoint is the main method
    Method *entrypoint = job->GetManagedEntrypoint();
    panda_file::ShortyIterator it(entrypoint->GetShorty());
    size_t argIdx = 0;
    ++it;  // skip return type
    if (!entrypoint->IsStatic()) {
        // handle 'this' argument
        ASSERT(arguments[argIdx].IsReference());
        ASSERT(arguments[argIdx].GetAs<ObjectHeader *>() != nullptr);
        visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arguments[argIdx].GetGCRoot()));
        ++argIdx;
    }
    while (it != panda_file::ShortyIterator()) {
        if ((*it).GetId() == panda_file::Type::TypeId::REFERENCE) {
            ASSERT(arguments[argIdx].IsReference());
            ObjectHeader *arg = arguments[argIdx].GetAs<ObjectHeader *>();
            if (arg != nullptr) {
                visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arguments[argIdx].GetGCRoot()));
            }
        }
        ++it;
        ++argIdx;
    }
}

void PandaEtsVM::AddRootProvider(mem::RootProvider *provider)
{
    os::memory::LockHolder lock(rootProviderlock_);
    ASSERT(rootProviders_.find(provider) == rootProviders_.end());
    rootProviders_.insert(provider);
}

void PandaEtsVM::RemoveRootProvider(mem::RootProvider *provider)
{
    os::memory::LockHolder lock(rootProviderlock_);
    ASSERT(rootProviders_.find(provider) != rootProviders_.end());
    rootProviders_.erase(provider);
}

void PandaEtsVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    PandaVM::VisitVmRoots(visitor);
    GetJobManager()->EnumerateThreads([visitor](ManagedThread *thread) {
        if (auto etsNapiEnv = EtsExecutionContext::FromMT(thread)->GetPandaAniEnv()) {
            auto etsStorage = etsNapiEnv->GetEtsReferenceStorage();
            etsStorage->GetAsReferenceStorage()->VisitObjects(visitor, mem::RootType::ROOT_NATIVE_LOCAL);
        }
        return true;
    });
    // NOTE(panferovi): Jobs as GC roots (issue #33719)
    GetJobManager()->EnumerateJobs([visitor](Job *job) {
        if (!job->HasManagedEntrypoint()) {
            return true;
        }
        PandaVector<Value> &arguments = job->GetManagedEntrypointArguments();
        if (!arguments.empty()) {
            HandleEmptyArguments(arguments, visitor, job);
        }
        return true;
    });
    if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
        PlatformTypes(this)->VisitRoots(visitor);
    }
    {
        os::memory::LockHolder lock(rootProviderlock_);
        for (auto *rootProvider : rootProviders_) {
            rootProvider->VisitRoots(visitor);
        }
    }
    GetUnhandledObjectManager()->VisitObjects(visitor);
}

void PandaEtsVM::UpdateAndSweepVmRefs(const ReferenceUpdater &updater)
{
    PandaVM::UpdateAndSweepVmRefs(updater);
    objStateTable_->EnumerateObjectStates([&updater](EtsObjectStateInfo *info) {
        auto *obj = info->GetEtsObject()->GetCoreType();
        [[maybe_unused]] ObjectStatus status = updater(&obj);
        ASSERT(status == ObjectStatus::ALIVE_OBJECT);
        info->SetEtsObject(EtsObject::FromCoreType(obj));
    });
    finalizationRegistryManager_->UpdateAndSweep(updater);
    {
        os::memory::LockHolder lock(rootProviderlock_);
        for (auto *rootProvider : rootProviders_) {
            rootProvider->UpdateRefs([&updater](ObjectHeader **ref) {
                auto status = updater(ref);
                return status == ObjectStatus::ALIVE_OBJECT;
            });
        }
    }
}

EtsFinalizableWeakRef *PandaEtsVM::RegisterFinalizerForObject(EtsExecutionContext *executionCtx,
                                                              const EtsHandle<EtsObject> &object,
                                                              void (*finalizer)(void *), void *finalizerArg)
{
    ASSERT_MANAGED_CODE();
    auto *weakRef = EtsFinalizableWeakRef::Create(executionCtx);
    weakRef->SetFinalizer(finalizer, finalizerArg);
    weakRef->SetReferent(object.GetPtr());
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    ASSERT(weakRefList != nullptr);
    weakRefList->Push(executionCtx, weakRef);
    return weakRef;
}

bool PandaEtsVM::UnregisterFinalizerForObject(EtsExecutionContext *executionCtx, EtsFinalizableWeakRef *weakRef)
{
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    ASSERT(weakRefList != nullptr);
    return weakRefList->Unlink(executionCtx, weakRef);
}

void PandaEtsVM::CleanFinalizableReferenceList()
{
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    ASSERT(weakRefList != nullptr);
    weakRefList->UnlinkClearedReferences(EtsExecutionContext::GetCurrent());
}

void PandaEtsVM::BeforeShutdown()
{
    ScopedManagedCodeThread managedScope(ManagedThread::GetCurrent());
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    ASSERT(weakRefList != nullptr);
    weakRefList->TraverseAndFinalize();
}

ClassLinkerContext *PandaEtsVM::CreateApplicationRuntimeLinker(const PandaVector<PandaString> &abcFiles)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    [[maybe_unused]] ScopedManagedCodeThread sj(executionCtx->GetMT());
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    const auto exceptionHandler = [this, executionCtx]() __attribute__((__noreturn__))
    // CC-OFFNXT(G.FMT.03-CPP) project code style
    {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        [[maybe_unused]] ScopedNativeCodeThread nj(executionCtx->GetMT());
        HandleUncaughtException();
        UNREACHABLE();
    };

    auto *klass = PlatformTypes(this)->coreAbcRuntimeLinker;
    EtsHandle<EtsAbcRuntimeLinker> linkerHandle(executionCtx,
                                                EtsAbcRuntimeLinker::FromEtsObject(EtsObject::Create(klass)));
    ASSERT(linkerHandle.GetPtr() != nullptr);

    EtsHandle<EtsEscompatArray> pathsHandle(executionCtx, EtsEscompatArray::Create(executionCtx, abcFiles.size()));
    for (size_t idx = 0; idx < abcFiles.size(); ++idx) {
        auto utf8Data = reinterpret_cast<const uint8_t *>(abcFiles[idx].data());
        auto *str = EtsString::CreateFromMUtf8(abcFiles[idx].data(), utf::MUtf8ToUtf16Size(utf8Data));
        if (UNLIKELY(str == nullptr)) {
            // Handle possible OOM
            exceptionHandler();
        }
        pathsHandle->EscompatArraySetUnsafe(idx, str->AsObject());
    }
    std::array args {Value(linkerHandle->GetCoreType()), Value(nullptr), Value(pathsHandle->GetCoreType())};

    auto *ctor =
        klass->GetDirectMethod(GetLanguageContext().GetCtorName(), "Lstd/core/RuntimeLinker;Lstd/core/Array;:V");
    ASSERT(ctor != nullptr);
    ctor->GetPandaMethod()->InvokeVoid(executionCtx->GetMT(), args.data());
    if (executionCtx->GetMT()->HasPendingException()) {
        // Print exceptions thrown in constructor (e.g. if file not found) and exit
        exceptionHandler();
    }

    // Save global reference to created application `AbcRuntimeLinker`
    GetGlobalObjectStorage()->Add(linkerHandle->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    // Safe to return a non-managed object
    return linkerHandle->GetClassLinkerContext();
}

/* static */
void PandaEtsVM::Abort(const char *message /* = nullptr */)
{
    Runtime::Abort(message);
}
}  // namespace ark::ets
