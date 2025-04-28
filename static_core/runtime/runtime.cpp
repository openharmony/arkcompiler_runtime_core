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

#include "runtime/include/runtime.h"

#include "compiler_options.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "assembler/assembly-literals.h"
#include "core/core_language_context.h"
#include "intrinsics.h"
#include "libpandabase/events/events.h"
#include "libpandabase/mem/mem_config.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandabase/os/cpu_affinity.h"
#include "libpandabase/os/mem_hooks.h"
#include "libpandabase/os/native_stack.h"
#include "libpandabase/os/thread.h"
#include "libpandabase/utils/arena_containers.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/dfx.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/literal_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "plugins.h"
#include "public.h"
#include "runtime/cha.h"
#include "runtime/compiler.h"
#include "runtime/dprofiler/dprofiler.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/language_context.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/tooling/debug_inf.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "mem/refstorage/reference_storage.h"
#include "runtime/mem/gc/gc_stats.h"
#include "runtime/mem/gc/stw-gc/stw-gc.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/memory_manager.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/mem/gc/gc-hung/gc_hung.h"
#include "runtime/include/panda_vm.h"
#include "runtime/profilesaver/profile_saver.h"
#include "runtime/tooling/debugger.h"
#include "runtime/tooling/memory_allocation_dumper.h"
#include "runtime/include/file_manager.h"
#include "runtime/methodtrace/trace.h"
#include "trace/trace.h"
#include "runtime/tests/intrusive-tests/intrusive_test_option.h"
#include "runtime/jit/profiling_saver.h"

namespace ark {

using std::unique_ptr;

Runtime *Runtime::instance_ = nullptr;
RuntimeOptions Runtime::options_;   // NOLINT(fuchsia-statically-constructed-objects)
std::string Runtime::runtimeType_;  // NOLINT(fuchsia-statically-constructed-objects)
os::memory::Mutex Runtime::mutex_;  // NOLINT(fuchsia-statically-constructed-objects)
taskmanager::TaskScheduler *Runtime::taskScheduler_ = nullptr;

const LanguageContextBase *g_ctxsJsRuntime = nullptr;  // Deprecated. Only for capability with js_runtime.

class RuntimeInternalAllocator {
public:
    static mem::InternalAllocatorPtr Create(bool useMallocForInternalAllocation)
    {
        ASSERT(mem::InternalAllocator<>::GetInternalAllocatorFromRuntime() == nullptr);

        memStatsS_ = new (std::nothrow) mem::MemStatsType();
        ASSERT(memStatsS_ != nullptr);

        if (useMallocForInternalAllocation) {
            internalAllocatorS_ =
                new (std::nothrow) mem::InternalAllocatorT<mem::InternalAllocatorConfig::MALLOC_ALLOCATOR>(memStatsS_);
        } else {
            internalAllocatorS_ =
                new (std::nothrow) mem::InternalAllocatorT<mem::InternalAllocatorConfig::PANDA_ALLOCATORS>(memStatsS_);
        }
        ASSERT(internalAllocatorS_ != nullptr);
        mem::InternalAllocator<>::InitInternalAllocatorFromRuntime(static_cast<mem::Allocator *>(internalAllocatorS_));

        return internalAllocatorS_;
    }

    static void Finalize()
    {
        internalAllocatorS_->VisitAndRemoveAllPools(
            [](void *mem, size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    }

    static void Destroy()
    {
        ASSERT(mem::InternalAllocator<>::GetInternalAllocatorFromRuntime() != nullptr);

        mem::InternalAllocator<>::ClearInternalAllocatorFromRuntime();
        delete static_cast<mem::Allocator *>(internalAllocatorS_);
        internalAllocatorS_ = nullptr;

        if (daemonMemoryLeakThreshold_ == 0) {
            // One more check that we don't have memory leak in internal allocator.
            ASSERT(daemonThreadsCnt_ > 0 || memStatsS_->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL) == 0);
        } else {
            // There might be a memory leaks in daemon threads, which we intend to ignore (issue #6539).
            ASSERT(memStatsS_->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL) <= daemonMemoryLeakThreshold_);
        }

        delete memStatsS_;
        memStatsS_ = nullptr;
    }

    static mem::InternalAllocatorPtr Get()
    {
        ASSERT(internalAllocatorS_ != nullptr);
        return internalAllocatorS_;
    }

    static void SetDaemonMemoryLeakThreshold(uint32_t daemonMemoryLeakThreshold)
    {
        daemonMemoryLeakThreshold_ = daemonMemoryLeakThreshold;
    }

    static void SetDaemonThreadsCount(uint32_t daemonThreadsCnt)
    {
        daemonThreadsCnt_ = daemonThreadsCnt;
    }

private:
    static mem::MemStatsType *memStatsS_;
    static mem::InternalAllocatorPtr internalAllocatorS_;  // NOLINT(fuchsia-statically-constructed-objects)
    static uint32_t daemonMemoryLeakThreshold_;
    static uint32_t daemonThreadsCnt_;
};

uint32_t RuntimeInternalAllocator::daemonMemoryLeakThreshold_ = 0;
uint32_t RuntimeInternalAllocator::daemonThreadsCnt_ = 0;

mem::MemStatsType *RuntimeInternalAllocator::memStatsS_ = nullptr;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
mem::InternalAllocatorPtr RuntimeInternalAllocator::internalAllocatorS_ = nullptr;

Runtime::DebugSession::DebugSession(Runtime &runtime)
    : runtime_(runtime), isJitEnabled_(runtime.IsJitEnabled()), lock_(runtime.debugSessionUniquenessMutex_)
{
    ASSERT(runtime_.isDebugMode_);
    runtime_.ForceDisableJit();
    debugger_ = MakePandaUnique<tooling::Debugger>(&runtime_);
}

Runtime::DebugSession::~DebugSession()
{
    debugger_.reset();
    if (isJitEnabled_) {
        runtime_.ForceEnableJit();
    }
}

tooling::DebugInterface &Runtime::DebugSession::GetDebugger()
{
    return *debugger_;
}

// all GetLanguageContext(...) methods should be based on this one
LanguageContext Runtime::GetLanguageContext(panda_file::SourceLang lang)
{
    if (g_ctxsJsRuntime != nullptr) {
        // Deprecated. Only for capability with js_runtime.
        return LanguageContext(g_ctxsJsRuntime);
    }

    auto *ctx = plugins::GetLanguageContextBase(lang);
    ASSERT(ctx != nullptr);
    return LanguageContext(ctx);
}

LanguageContext Runtime::GetLanguageContext(const Method &method)
{
    // Check class source lang
    auto *cls = method.GetClass();
    if (cls != nullptr) {
        return GetLanguageContext(cls->GetSourceLang());
    }

    panda_file::MethodDataAccessor mda(*method.GetPandaFile(), method.GetFileId());
    auto res = mda.GetSourceLang();
    return GetLanguageContext(res.value());
}

LanguageContext Runtime::GetLanguageContext(const Class &cls)
{
    return GetLanguageContext(cls.GetSourceLang());
}

LanguageContext Runtime::GetLanguageContext(const BaseClass &cls)
{
    return GetLanguageContext(cls.GetSourceLang());
}

LanguageContext Runtime::GetLanguageContext(panda_file::ClassDataAccessor *cda)
{
    auto res = cda->GetSourceLang();
    if (res) {
        return GetLanguageContext(res.value());
    }

    return GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
}

LanguageContext Runtime::GetLanguageContext(const std::string &runtimeType)
{
    return GetLanguageContext(plugins::RuntimeTypeToLang(runtimeType));
}

/* static */
bool Runtime::CreateInstance(const RuntimeOptions &options, mem::InternalAllocatorPtr internalAllocator)
{
    Locks::Initialize();

    if (options.WasSetEventsOutput()) {
        Events::Create(options.GetEventsOutput(), options.GetEventsFile());
    }

    {
        os::memory::LockHolder lock(mutex_);

        if (instance_ != nullptr) {
            return false;
        }

        instance_ = new Runtime(options, internalAllocator);
    }

    return true;
}

inline bool CreateMemorySpaces(const RuntimeOptions &options)
{
    uint32_t minFreePercentage = options.GetMinHeapFreePercentage();
    uint32_t maxFreePercentage = options.GetMaxHeapFreePercentage();
    if (minFreePercentage > PERCENT_100_U32) {
        LOG(ERROR, RUNTIME) << "Incorrect minimum free heap size percentage (min-free-percentage=" << minFreePercentage
                            << "), 0 <= min-free-percentage <= 100";
        return false;
    }
    if (maxFreePercentage > PERCENT_100_U32) {
        LOG(ERROR, RUNTIME) << "Incorrect maximum free heap size percentage (max-free-percentage=" << maxFreePercentage
                            << "), 0 <= max-free-percentage <= 100";
        return false;
    }
    if (minFreePercentage > maxFreePercentage) {
        LOG(ERROR, RUNTIME) << "Minimum free heap size percentage(min-free-percentage=" << minFreePercentage
                            << ") must be <= maximum free heap size percentage (max-free-percentage="
                            << maxFreePercentage << ")";
        return false;
    }
    size_t initialObjectSize = options.GetInitHeapSizeLimit();
    size_t maxObjectSize = options.GetHeapSizeLimit();
    bool wasSetInitialObjectSize = options.WasSetInitHeapSizeLimit();
    bool wasSetMaxObjectSize = options.WasSetHeapSizeLimit();
    if (!wasSetInitialObjectSize && wasSetMaxObjectSize) {
        initialObjectSize = maxObjectSize;
    } else if (initialObjectSize > maxObjectSize) {
        if (wasSetInitialObjectSize && !wasSetMaxObjectSize) {
            // Initial object heap size was set more default maximum object heap size, so set maximum heap size as
            // initial heap size
            maxObjectSize = initialObjectSize;
        } else {  // In this case user set initial object heap size more maximum object heap size explicitly
            LOG(ERROR, RUNTIME) << "Initial heap size (" << initialObjectSize << ") must be <= max heap size ("
                                << maxObjectSize << ")";
            return false;
        }
    }
    initialObjectSize =
        std::max(AlignDown(initialObjectSize, PANDA_POOL_ALIGNMENT_IN_BYTES), PANDA_POOL_ALIGNMENT_IN_BYTES);
    maxObjectSize = std::max(AlignDown(maxObjectSize, PANDA_POOL_ALIGNMENT_IN_BYTES), PANDA_POOL_ALIGNMENT_IN_BYTES);
    // Initialize memory spaces sizes
    mem::MemConfig::Initialize(maxObjectSize, options.GetInternalMemorySizeLimit(),
                               options.GetCompilerMemorySizeLimit(), options.GetCodeCacheSizeLimit(),
                               options.GetFramesMemorySizeLimit(), options.GetCoroutinesStackMemLimit(),
                               initialObjectSize);
    PoolManager::Initialize();
    return true;
}

// Deprecated. Only for capability with js_runtime.
bool Runtime::Create(const RuntimeOptions &options, const std::vector<LanguageContextBase *> &ctxs)
{
    g_ctxsJsRuntime = ctxs.front();
    return Runtime::Create(options);
}

/* static */
bool Runtime::Create(const RuntimeOptions &options)
{
    if (instance_ != nullptr) {
        return false;
    }

    IntrusiveTestOption::SetTestId(options);

    const_cast<RuntimeOptions &>(options).InitializeRuntimeSpacesAndType();
    trace::ScopedTrace scopedTrace("Runtime::Create");

    os::CpuAffinityManager::Initialize();

    if (!CreateMemorySpaces(options)) {
        LOG(ERROR, RUNTIME) << "Failed to create memory spaces for runtime";
        return false;
    }

    mem::InternalAllocatorPtr internalAllocator =
        RuntimeInternalAllocator::Create(options.UseMallocForInternalAllocations());

    BlockSignals();

    CreateDfxController(options);

    CreateInstance(options, internalAllocator);

    if (instance_ == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to create runtime instance";
        return false;
    }

    if (!instance_->Initialize()) {
        LOG(ERROR, RUNTIME) << "Failed to initialize runtime";
        if (instance_->GetPandaVM() != nullptr) {
            instance_->GetPandaVM()->UninitializeThreads();
        }
        delete instance_;
        instance_ = nullptr;
        return false;
    }

    instance_->GetPandaVM()->StartGC();

    auto *thread = ManagedThread::GetCurrent();
    instance_->GetNotificationManager()->VmStartEvent();
    instance_->GetNotificationManager()->VmInitializationEvent(thread);
    instance_->GetNotificationManager()->ThreadStartEvent(thread);

    if (options.IsSamplingProfilerEnable()) {
        instance_->GetTools().CreateSamplingProfiler();
        instance_->GetTools().StartSamplingProfiler(options.GetSamplingProfilerOutputFile(),
                                                    options.GetSamplingProfilerInterval());
    }

    return true;
}

Runtime *Runtime::GetCurrent()
{
    return instance_;
}

/* static */
bool Runtime::DestroyUnderLockHolder()
{
    os::memory::LockHolder lock(mutex_);

    if (instance_ == nullptr) {
        return false;
    }

    if (!instance_->Shutdown()) {
        LOG(ERROR, RUNTIME) << "Failed to shutdown runtime";
        return false;
    }
    if (GetOptions().WasSetEventsOutput()) {
        Events::Destroy();
    }

    /**
     * NOTE: Users threads can call log after destroying Runtime. We can't control these
     *       when they are in NATIVE_CODE mode because we don't destroy logger
     * Logger::Destroy();
     */

    DfxController::Destroy();
    ark::Logger::Sync();
    delete instance_;
    instance_ = nullptr;
    ark::mem::MemConfig::Finalize();

    return true;
}

/* static */
bool Runtime::Destroy()
{
    if (instance_ == nullptr) {
        return false;
    }

    trace::ScopedTrace scopedTrace("Runtime shutdown");

    if (instance_->SaveProfileInfo()) {
        ProfilingSaver profileSaver;
        auto isAotVerifyAbsPath = instance_->GetOptions().IsAotVerifyAbsPath();
        auto classCtxStr = instance_->GetClassLinker()->GetClassContextForAot(isAotVerifyAbsPath);
        auto &profiledMethods = instance_->GetClassLinker()->GetAotManager()->GetProfiledMethods();
        auto savingPath = PandaString(instance_->GetOptions().GetProfileOutput());
        auto profiledPandaFiles = instance_->GetClassLinker()->GetAotManager()->GetProfiledPandaFiles();
        profileSaver.SaveProfile(savingPath, classCtxStr, profiledMethods, profiledPandaFiles);
    }

    if (GetOptions().ShouldLoadBootPandaFiles()) {
        // Performing some actions before Runtime destroy.
        // For example, traversing FinalizableWeakRefList
        // in order to free internal data, which was not handled by GC
        instance_->GetPandaVM()->BeforeShutdown();
    }

    if (instance_->GetOptions().IsSamplingProfilerEnable()) {
        instance_->GetTools().StopSamplingProfiler();
    }

    // when signal start, but no signal stop tracing, should stop it
    if (Trace::isTracing_) {
        Trace::StopTracing();
    }

    instance_->GetNotificationManager()->VmDeathEvent();

    // Stop compiler first to make sure compile memleak doesn't occur
    auto compiler = instance_->GetPandaVM()->GetCompiler();
    if (compiler != nullptr) {
        // ecmascript doesn't have compiler
        // Here we just join JITthread
        // And destruction will be performed after thread uninitialization
        compiler->JoinWorker();
    }

    // Stop debugger first to correctly remove it as listener.
    instance_->UnloadDebugger();

    // Note JIT thread (compiler) may access to thread data,
    // so, it should be stopped before thread destroy
    /* @sync 1
     * @description Before starting to unitialize threads
     * */
    instance_->GetPandaVM()->UninitializeThreads();

    /* @sync 2
     * @description After uninitialization of threads all deamon threads should have gone into the termination loop and
     * all other threads should have finished.
     * */
    // Stop GC after UninitializeThreads because
    // UninitializeThreads may execute managed code which
    // uses barriers
    instance_->GetPandaVM()->StopGC();

    if (taskScheduler_ != nullptr) {
        taskScheduler_->Finalize();
    }

    if (IsEnabled(options_.GetVerificationMode())) {
        verifier::DestroyService(instance_->verifierService_, options_.IsVerificationUpdateCache());
    }

    DestroyUnderLockHolder();
    RuntimeInternalAllocator::Destroy();

    os::CpuAffinityManager::Finalize();
    if (taskScheduler_ != nullptr) {
        taskmanager::TaskScheduler::Destroy();
        taskScheduler_ = nullptr;
    }

    return true;
}

void Runtime::InitializeVerifierRuntime()
{
    auto mode = options_.GetVerificationMode();
    if (IsEnabled(mode)) {
        std::string const &cacheFile = options_.GetVerificationCacheFile();
        verifierService_ = ark::verifier::CreateService(verifierConfig_, internalAllocator_, classLinker_, cacheFile);
    }
}

/* static */
void Runtime::Halt(int32_t status)
{
    Runtime *runtime = Runtime::GetCurrent();
    if (runtime != nullptr && runtime->exit_ != nullptr) {
        runtime->exit_(status);
    }

    // _exit is safer to call because it guarantees a safe
    // completion in case of multi-threading as static destructors aren't called
    _exit(status);
}

/* static */
void Runtime::Abort(const char *message /* = nullptr */)
{
    Runtime *runtime = Runtime::GetCurrent();
    if (runtime != nullptr && runtime->abort_ != nullptr) {
        runtime->abort_();
    }

    std::cerr << "Runtime::Abort: " << ((message != nullptr) ? message : "") << std::endl;
    std::abort();
}

Runtime::Runtime(const RuntimeOptions &options, mem::InternalAllocatorPtr internalAllocator)
    : internalAllocator_(internalAllocator),
      notificationManager_(new RuntimeNotificationManager(internalAllocator_)),
      cha_(new ClassHierarchyAnalysis)
{
    Runtime::runtimeType_ = options.GetRuntimeType();
    /* @sync 1
     * @description Right before setting runtime options in Runtime constructor
     */
    Runtime::options_ = options;
    /* @sync 3
     * @description Right after setting runtime options in Runtime constructor
     */

    auto spaces = GetOptions().GetBootClassSpaces();

    std::vector<std::unique_ptr<ClassLinkerExtension>> extensions;
    extensions.reserve(spaces.size());

    for (const auto &space : spaces) {
        extensions.push_back(GetLanguageContext(space).CreateClassLinkerExtension());
    }

    classLinker_ = new ClassLinker(internalAllocator_, std::move(extensions));
#ifndef PANDA_TARGET_WINDOWS
    signalManager_ = new SignalManager(internalAllocator_);
#endif

    if (IsEnableMemoryHooks()) {
        ark::os::mem_hooks::PandaHooks::Initialize();
        ark::os::mem_hooks::PandaHooks::Enable();
    }

    saveProfilingInfo_ = options_.IsCompilerEnableJit() && options_.IsProfilesaverEnabled();

#ifdef PANDA_COMPILER_ENABLE
    // NOTE(maksenov): Enable JIT for debug mode
    isJitEnabled_ = !this->IsDebugMode() && Runtime::GetOptions().IsCompilerEnableJit();
#else
    isJitEnabled_ = false;
#endif

    verifierConfig_ = ark::verifier::NewConfig();
    InitializeVerifierRuntime();

    isZygote_ = options_.IsStartAsZygote();
    /* @sync 2
     * @description At the very end of the Runtime's constructor when all initialization actions have completed.
     * */
}

Runtime::~Runtime()
{
    ark::verifier::DestroyConfig(verifierConfig_);

    if (IsEnableMemoryHooks()) {
        ark::os::mem_hooks::PandaHooks::Disable();
    }
    trace::ScopedTrace scopedTrace("Delete state");

#ifndef PANDA_TARGET_WINDOWS
    signalManager_->DeleteHandlersArray();
    delete signalManager_;
#endif
    delete cha_;
    delete classLinker_;
    if (dprofiler_ != nullptr) {
        internalAllocator_->Delete(dprofiler_);
    }
    delete notificationManager_;

    if (pandaVm_ != nullptr) {
        internalAllocator_->Delete(pandaVm_);
        /* @sync 1
         * @description: This point is right after runtime deastroys panda VM.
         * */
    }

    /* @sync 1
     * @description Right after runtime's destructor has deleted panda virtual machine
     * */

    // crossing map is shared by different VMs.
    mem::CrossingMapSingleton::Destroy();

    RuntimeInternalAllocator::Finalize();
    PoolManager::Finalize();
}

bool Runtime::IsEnableMemoryHooks() const
{
    auto logLevel = Logger::IsInitialized() ? Logger::GetLevel() : Logger::Level::DEBUG;
    return options_.IsLimitStandardAlloc() && (logLevel == Logger::Level::FATAL || logLevel == Logger::Level::ERROR) &&
           (!options_.UseMallocForInternalAllocations());
}

static PandaVector<PandaString> GetPandaFilesList(const std::vector<std::string> &stdvec)
{
    PandaVector<PandaString> res;
    for (const auto &i : stdvec) {
        // NOLINTNEXTLINE(readability-redundant-string-cstr)
        res.push_back(i.c_str());
    }

    return res;
}

PandaVector<PandaString> Runtime::GetBootPandaFiles()
{
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    const auto &bootPandaFiles = GetPandaFilesList(options_.GetBootPandaFiles());
    return bootPandaFiles;
}

PandaVector<PandaString> Runtime::GetPandaFiles()
{
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    const auto &appPandaFiles = GetPandaFilesList(options_.GetPandaFiles());
    return appPandaFiles;
}

bool Runtime::LoadBootPandaFiles(panda_file::File::OpenMode openMode)
{
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    const auto &bootPandaFiles = options_.GetBootPandaFiles();
    for (const auto &name : bootPandaFiles) {
        if (!FileManager::LoadAbcFile(name, openMode)) {
#ifdef PANDA_PRODUCT_BUILD
            LOG(FATAL, RUNTIME) << "Load boot panda file failed: " << name;
#else
            LOG(ERROR, RUNTIME) << "Load boot panda file failed: " << name;
#endif  // PANDA_PRODUCT_BUILD
            return false;
        }
    }

    return true;
}

void Runtime::CheckBootPandaFiles()
{
    auto skipLast = static_cast<size_t>(options_.GetPandaFiles().empty() && !options_.IsStartAsZygote());
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    const auto &bootPandaFiles = options_.GetBootPandaFiles();
    for (size_t i = 0; i + skipLast < bootPandaFiles.size(); ++i) {
        auto &name = bootPandaFiles[i];
        if (classLinker_->GetAotManager()->FindPandaFile(name) == nullptr) {
            LOG(FATAL, RUNTIME) << "AOT file wasn't loaded for panda file: " << name;
        }
    }
}

void Runtime::SetDaemonMemoryLeakThreshold(uint32_t daemonMemoryLeakThreshold)
{
    RuntimeInternalAllocator::SetDaemonMemoryLeakThreshold(daemonMemoryLeakThreshold);
}

void Runtime::SetDaemonThreadsCount(uint32_t daemonThreadsCnt)
{
    RuntimeInternalAllocator::SetDaemonThreadsCount(daemonThreadsCnt);
}

mem::GCType Runtime::GetGCType(const RuntimeOptions &options, panda_file::SourceLang lang)
{
    auto gcType = ark::mem::GCTypeFromString(options.GetGcType(plugins::LangToRuntimeType(lang)));
    if (options.IsNoAsyncJit()) {
        // With no-async-jit we can force compilation inside of c2i bridge (we have DecrementHotnessCounter there)
        // and it can trigger GC which can move objects which are arguments for the method
        // because StackWalker ignores c2i frame
        return (gcType != ark::mem::GCType::EPSILON_GC) ? (ark::mem::GCType::STW_GC) : gcType;
    }
    return gcType;
}

bool Runtime::LoadVerificationConfig()
{
    return !IsEnabled(options_.GetVerificationMode()) ||
           verifier::LoadConfigFile(verifierConfig_, options_.GetVerificationConfigFile());
}

bool Runtime::CreatePandaVM(std::string_view runtimeType)
{
    ManagedThread::Initialize();

    pandaVm_ = PandaVM::Create(this, options_, runtimeType);
    if (pandaVm_ == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to create panda vm";
        return false;
    }

    if (taskScheduler_ != nullptr) {
        taskScheduler_->Initialize();
    }

    panda_file::File::OpenMode openMode = GetLanguageContext(GetRuntimeType()).GetBootPandaFilesOpenMode();
    bool loadBootPandaFilesIsFailed = options_.ShouldLoadBootPandaFiles() && !LoadBootPandaFiles(openMode);
    if (loadBootPandaFilesIsFailed) {
        LOG(ERROR, RUNTIME) << "Failed to load boot panda files";
        return false;
    }

    auto aotBootCtx = classLinker_->GetClassContextForAot(options_.IsAotVerifyAbsPath());
    if (options_.GetPandaFiles().empty() && !options_.IsStartAsZygote()) {
        // Main from panda.cpp puts application file into boot panda files as the last element.
        // During AOT compilation of boot files no application panda files were used.
        auto idx = aotBootCtx.find_last_of(compiler::AotClassContextCollector::DELIMETER);
        if (idx == std::string::npos) {
            // Only application file is in aot_boot_ctx
            classLinker_->GetAotManager()->SetAppClassContext(aotBootCtx);
            aotBootCtx = "";
        } else {
            // Last file is an application
            classLinker_->GetAotManager()->SetAppClassContext(aotBootCtx.substr(idx + 1));
            aotBootCtx = aotBootCtx.substr(0, idx);
        }
    }
    classLinker_->GetAotManager()->SetBootClassContext(aotBootCtx);
    if (pandaVm_->GetLanguageContext().IsEnabledCHA()) {
        classLinker_->GetAotManager()->VerifyClassHierarchy();
    }
#ifndef PANDA_PRODUCT_BUILD
    if (Runtime::GetOptions().IsEnableAnForce() && !Runtime::GetOptions().IsArkAot()) {
        CheckBootPandaFiles();
    }
#endif  // PANDA_PRODUCT_BUILD
    notificationManager_->SetRendezvous(pandaVm_->GetRendezvous());

    return true;
}

bool Runtime::InitializePandaVM()
{
    // temporary solution, see #7225
    if (!options_.IsRuntimeCompressedStringsEnabled()) {
        LOG(FATAL, RUNTIME) << "Non compressed strings is not supported";
    }

    if (!classLinker_->Initialize(options_.IsRuntimeCompressedStringsEnabled())) {
        LOG(ERROR, RUNTIME) << "Failed to initialize class loader";
        return false;
    }

    if (pandaVm_->ShouldEnableDebug()) {
        SetDebugMode(true);
        StartDebugSession();
    }

    if (!pandaVm_->Initialize()) {
        LOG(ERROR, RUNTIME) << "Failed to initialize panda vm";
        return false;
    }

    return true;
}

bool Runtime::HandleAotOptions()
{
    auto aotFiles = options_.GetAotFiles();
    const auto &name = options_.GetAotFile();
    if (!name.empty()) {
        aotFiles.push_back(name);
    }
    if (!aotFiles.empty()) {
        for (auto &fname : aotFiles) {
            auto res = FileManager::LoadAnFile(fname, true);
#ifdef PANDA_TARGET_OHOS
            if (!res) {
                LOG(ERROR, AOT) << "Failed to load AoT file: " << res.Error();
                continue;
            }
            if (!res.Value()) {
                LOG(ERROR, AOT) << "Failed to load '" << fname << "' with unknown reason";
            }
#else
            if (!res) {
                LOG(FATAL, AOT) << "Failed to load AoT file: " << res.Error();
            }
            if (!res.Value()) {
                LOG(FATAL, AOT) << "Failed to load '" << fname << "' with unknown reason";
            }
#endif
        }
    }

    return true;
}

template <typename Handler>
static void RegisterSignalHandler(SignalManager *manager, bool inCompiledCode)
{
    auto *handler = manager->GetAllocator()->New<Handler>();
    manager->AddHandler(handler, inCompiledCode);
}

void Runtime::HandleJitOptions()
{
#ifndef PANDA_TARGET_WINDOWS
    auto signalManagerFlag = DfxController::GetOptionValue(DfxOptionHandler::SIGNAL_HANDLER);
    if (signalManagerFlag == 1) {
        signalManager_->InitSignals();
    } else {
        LOG(ERROR, DFX) << "signal handler disabled, setprop ark.dfx.options to restart";
    }
#endif

    bool enableNpHandler = options_.IsCompilerEnableJit() && ark::compiler::g_options.IsCompilerImplicitNullCheck();
    if (GetClassLinker()->GetAotManager()->HasAotFiles()) {
        ASSERT(GetPandaVM()->GetCompiler()->IsNoAsyncJit() == options_.IsNoAsyncJit());
        if (GetPandaVM()->GetCompiler()->IsNoAsyncJit()) {
            LOG(FATAL, AOT) << "We can't use the option --no-async-jit=true with AOT";
        }
        enableNpHandler = true;
    }

#ifndef PANDA_TARGET_WINDOWS
    RegisterSignalHandler<SamplingProfilerHandler>(signalManager_, true);
    if (signalManager_->IsInitialized() && enableNpHandler) {
        RegisterSignalHandler<NullPointerHandler>(signalManager_, true);
    }
    RegisterSignalHandler<StackOverflowHandler>(signalManager_, true);
    RegisterSignalHandler<CrashFallbackDumpHandler>(signalManager_, false);
#endif
}

bool Runtime::CheckOptionsConsistency()
{
    {
        auto value = options_.GetResolveStringAotThreshold();
        auto limit = RuntimeInterface::RESOLVE_STRING_AOT_COUNTER_LIMIT;
        if (value >= limit) {
            LOG(ERROR, RUNTIME) << "--resolve-string-aot-threshold value (" << value << ") is "
                                << ((value == limit) ? "equal to " : "greater than ") << limit
                                << ", ResolveString optimization won't be applied to AOT-compiled code. "
                                << "Consider value lower than " << limit << " to enable the optimization.";
        }
    }
    return true;
}

void Runtime::SetPandaPath()
{
    PandaVector<PandaString> appPandaFiles = GetPandaFiles();
    for (size_t i = 0; i < appPandaFiles.size(); ++i) {
        pandaPathString_ += PandaStringToStd(appPandaFiles[i]);
        if (i != appPandaFiles.size() - 1) {
            pandaPathString_ += ":";
        }
    }
}

void Runtime::SetThreadClassPointers()
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    classLinker_->InitializeRoots(thread);
    auto ext = GetClassLinker()->GetExtension(GetLanguageContext(GetRuntimeType()));
    if (ext != nullptr) {
        thread->SetStringClassPtr(ext->GetClassRoot(ClassRoot::STRING));
        thread->SetArrayU16ClassPtr(ext->GetClassRoot(ClassRoot::ARRAY_U16));
        thread->SetArrayU8ClassPtr(ext->GetClassRoot(ClassRoot::ARRAY_U8));
    }
}

bool Runtime::Initialize()
{
    trace::ScopedTrace scopedTrace("Runtime::Initialize");

    if (!CheckOptionsConsistency()) {
        return false;
    }

    if (!LoadVerificationConfig()) {
        return false;
    }

    CheckOptionsFromOs();

    if (!CreatePandaVM(GetRuntimeType())) {
        return false;
    }

#if defined(PANDA_COMPILER_DEBUG_INFO) && !defined(NDEBUG)
    if (!compiler::g_options.WasSetCompilerEmitDebugInfo()) {
        compiler::g_options.SetCompilerEmitDebugInfo(true);
    }
#endif

    // We must load AOT file before InitializePandaVM, because during initialization, code execution may be called.
    if (!HandleAotOptions()) {
        return false;
    }

    if (!InitializePandaVM()) {
        return false;
    }

    SetThreadClassPointers();

    fingerPrint_ = ConvertToString(options_.GetFingerprint());

    HandleJitOptions();

    SetPandaPath();

    if (!pandaVm_->InitializeFinish()) {
        LOG(ERROR, RUNTIME) << "Failed to finish panda vm initialization";
        return false;
    }

    if (IsDebugMode()) {
        pandaVm_->LoadDebuggerAgent();
    }

    if (options_.WasSetMemAllocDumpExec()) {
        StartMemAllocDumper(ConvertToString(options_.GetMemAllocDumpFile()));
    }

#ifdef PANDA_TARGET_MOBILE
    mem::GcHung::InitPreFork(true);
#else
    mem::GcHung::InitPreFork(false);
#endif  // PANDA_TARGET_MOBILE

    isInitialized_ = true;
    return true;
}

int Runtime::StartMemAllocDumper(const PandaString &dumpFile)
{
    ASSERT(memAllocDumper_ == nullptr);

    memAllocDumper_ = internalAllocator_->New<tooling::MemoryAllocationDumper>(dumpFile, Runtime::GetCurrent());
    return 0;
}

static bool GetClassAndMethod(std::string_view entryPoint, PandaString *className, PandaString *methodName)
{
    size_t pos = entryPoint.find_last_of("::");
    if (pos == std::string_view::npos) {
        return false;
    }

    *className = PandaString(entryPoint.substr(0, pos - 1));
    *methodName = PandaString(entryPoint.substr(pos + 1));

    return true;
}

static const uint8_t *GetStringArrayDescriptor(const LanguageContext &ctx, PandaString *out)
{
    *out = "[";
    *out += utf::Mutf8AsCString(ctx.GetStringClassDescriptor());

    return utf::CStringAsMutf8(out->c_str());
}

Expected<Method *, Runtime::Error> Runtime::ResolveEntryPoint(std::string_view entryPoint)
{
    PandaString className;
    PandaString methodName;

    if (!GetClassAndMethod(entryPoint, &className, &methodName)) {
        LOG(ERROR, RUNTIME) << "Invalid entry point: " << entryPoint;
        return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
    }

    PandaString descriptor;
    auto classNameBytes = ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor);
    auto methodNameBytes = utf::CStringAsMutf8(methodName.c_str());

    Class *cls = nullptr;
    ClassLinkerContext *context = appContext_.ctx;
    if (context == nullptr) {
        context = classLinker_->GetExtension(GetLanguageContext(GetRuntimeType()))->GetBootContext();
    }

    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread sa(thread);
    cls = classLinker_->GetClass(classNameBytes, true, context);
    if (cls == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot find class '" << className << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_FOUND);
    }

    LanguageContext ctx = GetLanguageContext(*cls);
    PandaString stringArrayDescriptor;
    GetStringArrayDescriptor(ctx, &stringArrayDescriptor);

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {stringArrayDescriptor});

    auto method = cls->GetDirectMethod(methodNameBytes, proto);
    if (method == nullptr) {
        method = cls->GetDirectMethod(methodNameBytes);
        if (method == nullptr) {
            LOG(ERROR, RUNTIME) << "Entry point '" << entryPoint << "' not found";
            return Unexpected(Runtime::Error::METHOD_NOT_FOUND);
        }
    }

    return method;
}

PandaString Runtime::GetMemoryStatistics()
{
    return pandaVm_->GetMemStats()->GetStatistics();
}

PandaString Runtime::GetFinalStatistics()
{
    return pandaVm_->GetGCStats()->GetFinalStatistics(pandaVm_->GetHeapManager());
}

void Runtime::NotifyAboutLoadedModules()
{
    PandaVector<const panda_file::File *> pfs;

    classLinker_->EnumerateBootPandaFiles([&pfs](const panda_file::File &pf) {
        pfs.push_back(&pf);
        return true;
    });

    for (const auto *pf : pfs) {
        GetNotificationManager()->LoadModuleEvent(pf->GetFilename());
    }
}

Expected<LanguageContext, Runtime::Error> Runtime::ExtractLanguageContext(const panda_file::File *pf,
                                                                          std::string_view entryPoint)
{
    PandaString className;
    PandaString methodName;
    if (!GetClassAndMethod(entryPoint, &className, &methodName)) {
        LOG(ERROR, RUNTIME) << "Invalid entry point: " << entryPoint;
        return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
    }

    PandaString descriptor;
    auto classNameBytes = ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor);
    auto methodNameBytes = utf::CStringAsMutf8(methodName.c_str());

    auto classId = pf->GetClassId(classNameBytes);
    if (!classId.IsValid() || pf->IsExternal(classId)) {
        LOG(ERROR, RUNTIME) << "Cannot find class '" << className << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_FOUND);
    }

    panda_file::ClassDataAccessor cda(*pf, classId);
    LanguageContext ctx = GetLanguageContext(&cda);
    bool found = false;
    cda.EnumerateMethods([this, &pf, methodNameBytes, &found, &ctx](panda_file::MethodDataAccessor &mda) {
        if (!found && utf::IsEqual(pf->GetStringData(mda.GetNameId()).data, methodNameBytes)) {
            found = true;
            auto val = mda.GetSourceLang();
            if (val) {
                ctx = GetLanguageContext(val.value());
            }
        }
    });

    if (!found) {
        LOG(ERROR, RUNTIME) << "Cannot find method '" << entryPoint << "'";
        return Unexpected(Runtime::Error::METHOD_NOT_FOUND);
    }

    return ctx;
}

std::optional<Runtime::Error> Runtime::CreateApplicationClassLinkerContext(std::string_view filename,
                                                                           std::string_view entryPoint)
{
    bool isLoaded = false;
    classLinker_->EnumerateBootPandaFiles([&isLoaded, filename](const panda_file::File &pf) {
        if (pf.GetFilename() == filename) {
            isLoaded = true;
            return false;
        }
        return true;
    });

    if (isLoaded) {
        return {};
    }

    auto pf = panda_file::OpenPandaFileOrZip(filename);
    if (pf == nullptr) {
        return Runtime::Error::PANDA_FILE_LOAD_ERROR;
    }

    auto res = ExtractLanguageContext(pf.get(), entryPoint);
    if (!res) {
        return res.Error();
    }

    if (!classLinker_->HasExtension(res.Value())) {
        LOG(ERROR, RUNTIME) << "class linker hasn't " << res.Value() << " language extension";
        return Runtime::Error::CLASS_LINKER_EXTENSION_NOT_FOUND;
    }

    auto *ext = classLinker_->GetExtension(res.Value());
    appContext_.lang = ext->GetLanguage();
    appContext_.ctx = classLinker_->GetAppContext(filename);
    if (appContext_.ctx == nullptr) {
        auto appFiles = GetPandaFiles();
        auto foundIter =
            std::find_if(appFiles.begin(), appFiles.end(), [&](auto &appFileName) { return appFileName == filename; });
        if (foundIter == appFiles.end()) {
            PandaString path(filename);
            appFiles.push_back(path);
        }
        appContext_.ctx = ext->CreateApplicationClassLinkerContext(appFiles);
        ASSERT(appContext_.ctx != nullptr);
    }

    PandaString aotCtx;
    {
        ScopedManagedCodeThread smct(ManagedThread::GetCurrent());
        appContext_.ctx->EnumeratePandaFiles(
            compiler::AotClassContextCollector(&aotCtx, options_.IsAotVerifyAbsPath()));
    }
    classLinker_->GetAotManager()->SetAppClassContext(aotCtx);

    tooling::DebugInf::AddCodeMetaInfo(pf.get());
    return {};
}

Expected<int, Runtime::Error> Runtime::ExecutePandaFile(std::string_view filename, std::string_view entryPoint,
                                                        const std::vector<std::string> &args)
{
    if (options_.IsDistributedProfiling()) {
        // Create app name from path to executable file.
        std::string_view appName = [](std::string_view path) -> std::string_view {
            auto pos = path.find_last_of('/');
            return path.substr((pos == std::string_view::npos) ? 0 : (pos + 1));
        }(filename);
        StartDProfiler(appName);
    }

    auto ctxErr = CreateApplicationClassLinkerContext(filename, entryPoint);
    if (ctxErr) {
        return Unexpected(ctxErr.value());
    }

    if (pandaVm_->GetLanguageContext().IsEnabledCHA()) {
        classLinker_->GetAotManager()->VerifyClassHierarchy();
    }

    // Check if all input files are either quickened or not
    uint32_t quickenedFiles = 0;
    uint32_t pandaFiles = 0;
    classLinker_->EnumeratePandaFiles([&quickenedFiles, &pandaFiles](const panda_file::File &pf) {
        if (pf.GetHeader()->quickenedFlag != 0) {
            ++quickenedFiles;
        }
        pandaFiles++;
        return true;
    });
    if (quickenedFiles != 0 && quickenedFiles != pandaFiles) {
        LOG(ERROR, RUNTIME) << "All input files should be either quickened or not. Got " << quickenedFiles
                            << " quickened files out of " << pandaFiles << " input files.";
        classLinker_->EnumeratePandaFiles([](const panda_file::File &pf) {
            if (pf.GetHeader()->quickenedFlag != 0) {
                LOG(ERROR, RUNTIME) << "File " << pf.GetFilename() << " is quickened";
            } else {
                LOG(ERROR, RUNTIME) << "File " << pf.GetFilename() << " is not quickened";
            }
            return true;
        });
        return false;
    }

    return Execute(entryPoint, args);
}

Expected<int, Runtime::Error> Runtime::Execute(std::string_view entryPoint, const std::vector<std::string> &args)
{
    auto resolveRes = ResolveEntryPoint(entryPoint);
    if (!resolveRes) {
        return Unexpected(resolveRes.Error());
    }

    NotifyAboutLoadedModules();

    Method *method = resolveRes.Value();

    return pandaVm_->InvokeEntrypoint(method, args);
}

int Runtime::StartDProfiler(std::string_view appName)
{
    if (dprofiler_ != nullptr) {
        LOG(ERROR, RUNTIME) << "DProfiller already started";
        return -1;
    }

    dprofiler_ = internalAllocator_->New<DProfiler>(appName, Runtime::GetCurrent());
    return 0;
}

Runtime::DebugSessionHandle Runtime::StartDebugSession()
{
    os::memory::LockHolder lock(debugSessionCreationMutex_);

    auto session = debugSession_;
    if (session) {
        return session;
    }

    session = MakePandaShared<DebugSession>(*this);

    debugSession_ = session;

    return session;
}

void Runtime::UnloadDebugger()
{
    GetPandaVM()->UnloadDebuggerAgent();
    debugSession_.reset();
}

bool Runtime::Shutdown()
{
    if (memAllocDumper_ != nullptr) {
        internalAllocator_->Delete(memAllocDumper_);
    }
    ManagedThread::Shutdown();
    return true;
}

coretypes::String *Runtime::ResolveString(PandaVM *vm, const Method &caller, panda_file::File::EntityId id)
{
    auto *pf = caller.GetPandaFile();
    return vm->ResolveString(*pf, id);
}

coretypes::String *Runtime::ResolveStringFromCompiledCode(PandaVM *vm, const Method &caller,
                                                          panda_file::File::EntityId id)
{
    auto *pf = caller.GetPandaFile();
    return vm->ResolveStringFromCompiledCode(*pf, id);
}

coretypes::String *Runtime::ResolveString(PandaVM *vm, const panda_file::File &pf, panda_file::File::EntityId id,
                                          const LanguageContext &ctx)
{
    coretypes::String *str = vm->GetStringTable()->GetInternalStringFast(pf, id);
    if (str != nullptr) {
        return str;
    }
    str = vm->GetStringTable()->GetOrInternInternalString(pf, id, ctx);
    return str;
}

coretypes::String *Runtime::ResolveString(PandaVM *vm, const uint8_t *mutf8, uint32_t length,
                                          const LanguageContext &ctx)
{
    return vm->GetStringTable()->GetOrInternString(mutf8, length, ctx);
}

coretypes::Array *Runtime::ResolveLiteralArray(PandaVM *vm, const Method &caller, uint32_t id)
{
    auto *pf = caller.GetPandaFile();
    id = pf->GetLiteralArrays()[id];
    LanguageContext ctx = GetLanguageContext(caller);
    return ResolveLiteralArray(vm, *pf, id, ctx);
}

Class *Runtime::GetClassRootForLiteralTag(const ClassLinkerExtension &ext, panda_file::LiteralTag tag) const
{
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1:
            return ext.GetClassRoot(ClassRoot::ARRAY_U1);
        case panda_file::LiteralTag::ARRAY_U8:
            return ext.GetClassRoot(ClassRoot::ARRAY_U8);
        case panda_file::LiteralTag::ARRAY_I8:
            return ext.GetClassRoot(ClassRoot::ARRAY_I8);
        case panda_file::LiteralTag::ARRAY_U16:
            return ext.GetClassRoot(ClassRoot::ARRAY_U16);
        case panda_file::LiteralTag::ARRAY_I16:
            return ext.GetClassRoot(ClassRoot::ARRAY_I16);
        case panda_file::LiteralTag::ARRAY_U32:
            return ext.GetClassRoot(ClassRoot::ARRAY_U32);
        case panda_file::LiteralTag::ARRAY_I32:
            return ext.GetClassRoot(ClassRoot::ARRAY_I32);
        case panda_file::LiteralTag::ARRAY_U64:
            return ext.GetClassRoot(ClassRoot::ARRAY_U64);
        case panda_file::LiteralTag::ARRAY_I64:
            return ext.GetClassRoot(ClassRoot::ARRAY_I64);
        case panda_file::LiteralTag::ARRAY_F32:
            return ext.GetClassRoot(ClassRoot::ARRAY_F32);
        case panda_file::LiteralTag::ARRAY_F64:
            return ext.GetClassRoot(ClassRoot::ARRAY_F64);
        case panda_file::LiteralTag::ARRAY_STRING:
            return ext.GetClassRoot(ClassRoot::ARRAY_STRING);
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::BOOL:
        case panda_file::LiteralTag::INTEGER:
        case panda_file::LiteralTag::FLOAT:
        case panda_file::LiteralTag::DOUBLE:
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCMETHOD:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE: {
            break;
        }
        default: {
            break;
        }
    }
    UNREACHABLE();
    return nullptr;
}

/* static */
bool Runtime::GetLiteralTagAndValue(const panda_file::File &pf, uint32_t id, panda_file::LiteralTag *tag,
                                    panda_file::LiteralDataAccessor::LiteralValue *value)
{
    panda_file::File::EntityId literalArraysId = pf.GetLiteralArraysId();
    panda_file::LiteralDataAccessor literalDataAccessor(pf, literalArraysId);
    bool result = false;
    literalDataAccessor.EnumerateLiteralVals(
        panda_file::File::EntityId(id), [tag, value, &result](const panda_file::LiteralDataAccessor::LiteralValue &val,
                                                              const panda_file::LiteralTag &tg) {
            *tag = tg;
            *value = val;
            result = true;
        });
    return result;
}

uintptr_t Runtime::GetPointerToConstArrayData(const panda_file::File &pf, uint32_t id) const
{
    panda_file::LiteralTag tag;
    panda_file::LiteralDataAccessor::LiteralValue value;
    if (!GetLiteralTagAndValue(pf, id, &tag, &value)) {
        UNREACHABLE();
        return 0;
    }

    auto sp = pf.GetSpanFromId(panda_file::File::EntityId(std::get<uint32_t>(value)));
    // first element in the sp is array size, panda_file::helpers::Read move sp pointer to next element
    [[maybe_unused]] auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    return reinterpret_cast<uintptr_t>(sp.data());
}

coretypes::Array *Runtime::ResolveLiteralArray(PandaVM *vm, const panda_file::File &pf, uint32_t id,
                                               const LanguageContext &ctx)
{
    panda_file::LiteralTag tag;
    panda_file::LiteralDataAccessor::LiteralValue value;
    if (!GetLiteralTagAndValue(pf, id, &tag, &value)) {
        return nullptr;
    }

    auto sp = pf.GetSpanFromId(panda_file::File::EntityId(std::get<uint32_t>(value)));

    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    auto ext = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx);

    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        return coretypes::Array::Create(GetClassRootForLiteralTag(*ext, tag), sp.data(), len);
    }

    // special handling of arrays of strings
    auto array = coretypes::Array::Create(GetClassRootForLiteralTag(*ext, tag), len);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(ManagedThread::GetCurrent());
    VMHandle<coretypes::Array> obj(ManagedThread::GetCurrent(), array);
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (size_t i = 0; i < len; i++) {
        auto strId = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
        auto str = Runtime::GetCurrent()->ResolveString(vm, pf, panda_file::File::EntityId(strId), ctx);
        obj->Set<ObjectHeader *>(i, str);
    }
    return obj.GetPtr();
}

void Runtime::UpdateProcessState([[maybe_unused]] int state)
{
    LOG(INFO, RUNTIME) << __func__ << " is an empty implementation now.";
}

void Runtime::RegisterSensitiveThread() const
{
    LOG(INFO, RUNTIME) << __func__ << " is an empty implementation now.";
}

void Runtime::CreateDfxController(const RuntimeOptions &options)
{
    DfxController::Initialize();
    DfxController::SetOptionValue(DfxOptionHandler::COMPILER_NULLCHECK, options.GetCompilerNullcheck());
    DfxController::SetOptionValue(DfxOptionHandler::REFERENCE_DUMP, options.GetReferenceDump());
    DfxController::SetOptionValue(DfxOptionHandler::SIGNAL_CATCHER, options.GetSignalCatcher());
    DfxController::SetOptionValue(DfxOptionHandler::SIGNAL_HANDLER, options.GetSignalHandler());
    DfxController::SetOptionValue(DfxOptionHandler::ARK_SIGQUIT, options.GetSigquitFlag());
    DfxController::SetOptionValue(DfxOptionHandler::ARK_SIGUSR1, options.GetSigusr1Flag());
    DfxController::SetOptionValue(DfxOptionHandler::ARK_SIGUSR2, options.GetSigusr2Flag());
    DfxController::SetOptionValue(DfxOptionHandler::MOBILE_LOG, options.GetMobileLogFlag());
    DfxController::SetOptionValue(DfxOptionHandler::DFXLOG, options.GetDfxLog());

    auto compilerNullcheckFlag = DfxController::GetOptionValue(DfxOptionHandler::COMPILER_NULLCHECK);
    if (compilerNullcheckFlag == 0) {
        ark::compiler::g_options.SetCompilerImplicitNullCheck(false);
    }
}

void Runtime::BlockSignals()
{
    sigset_t set;
    if (sigemptyset(&set) == -1) {
        LOG(ERROR, RUNTIME) << "sigemptyset failed";
        return;
    }
    int rc = 0;
    rc += sigaddset(&set, SIGPIPE);
#ifdef PANDA_TARGET_MOBILE
    rc += sigaddset(&set, SIGQUIT);
    rc += sigaddset(&set, SIGUSR1);
    rc += sigaddset(&set, SIGUSR2);
#endif  // PANDA_TARGET_MOBILE
    if (rc < 0) {
        LOG(ERROR, RUNTIME) << "sigaddset failed";
        return;
    }

    if (os::native_stack::g_PandaThreadSigmask(SIG_BLOCK, &set, nullptr) != 0) {
        LOG(ERROR, RUNTIME) << "g_PandaThreadSigmask failed";
    }
}

void Runtime::DumpForSigQuit(std::ostream &os)
{
    os << "\n";
    os << "-> Dump class loaders\n";
    classLinker_->EnumerateContextsForDump(
        [](ClassLinkerContext *ctx, std::ostream &stream, ClassLinkerContext *parent) {
            ctx->Dump(stream);
            return ctx->FindClassLoaderParent(parent);
        },
        os);
    os << "\n";

    // dump GC
    os << "-> Dump GC\n";
    os << GetFinalStatistics();
    os << "\n";

    // dump memory management
    os << "-> Dump memory management\n";
    os << GetMemoryStatistics();
    os << "\n";

    // dump PandaVM
    os << "-> Dump Ark VM\n";
    pandaVm_->DumpForSigQuit(os);
    os << "\n";
}

void Runtime::InitNonZygoteOrPostFork(bool isSystemServer, [[maybe_unused]] const char *isa,
                                      const std::function<void()> &initHook, [[maybe_unused]] bool profileSystemServer)
{
    isZygote_ = false;

    // NOTE(00510180): wait NativeBridge ready

    // NOTE(00510180): wait profile ready

    // NOTE(00510180): wait ThreadPool ready

    // NOTE(00510180): wait ResetGcPerformanceInfo() ready

    pandaVm_->PreStartup();

    initHook();

    mem::GcHung::InitPostFork(isSystemServer);
}

void Runtime::PreZygoteFork()
{
    pandaVm_->PreZygoteFork();
}

void Runtime::PostZygoteFork()
{
    pandaVm_->PostZygoteFork();
}

// Returns true if profile saving is enabled. GetJit() will be not null in this case.
bool Runtime::SaveProfileInfo() const
{
    return saveProfilingInfo_;
}

void Runtime::CheckOptionsFromOs() const
{
#if !defined(PANDA_QEMU_BUILD) && !defined(PANDA_TARGET_MOBILE)
    // for qemu-aarch64 we will get 32 from GetCacheLineSize()
    // for native arm and qemu-arm we will get 0 from GetCacheLineSize()
    ASSERT(ark::CACHE_LINE_SIZE == os::mem::GetCacheLineSize());
#endif
}

}  // namespace ark
