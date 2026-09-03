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

#include "debugger_arkapi.h"
#include <fstream>
#include <string_view>
#include "libarkbase/os/mutex.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread.h"
#include "runtime/tooling/sampler/samples_record.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "types/profile_result.h"
#include "libarkbase/utils/json_builder.h"

namespace ark {
constexpr std::string_view ARK_DEBUGGER_LIB_PATH = "libark_inspector.z.so";
void *ArkDebugNativeAPI::gHybridDebuggerHandle_ = nullptr;
#ifdef PANDA_TARGET_WINDOWS
static void *Load(std::string_view libraryName)
{
    HMODULE module = LoadLibrary(libraryName.data());
    void *handle = reinterpret_cast<void *>(module);
    return handle;
}

static void *ResolveSymbol(void *handle, std::string_view symbol)
{
    HMODULE module = reinterpret_cast<HMODULE>(handle);
    void *addr = reinterpret_cast<void *>(GetProcAddress(module, symbol.data()));
    return addr;
}

static void Close(void *handle)
{
    FreeLibrary(reinterpret_cast<HMODULE>(handle));
}
#else  // UNIX_PLATFORM
static void *Load(std::string_view libraryName)
{
    void *handle = dlopen(libraryName.data(), RTLD_LAZY);
    return handle;
}

static void *ResolveSymbol(void *handle, std::string_view symbol)
{
    void *addr = dlsym(handle, symbol.data());
    return addr;
}

static void Close(void *handle)
{
    dlclose(handle);
}
#endif

bool ArkDebugNativeAPI::NotifyDebugMode([[maybe_unused]] int tid, [[maybe_unused]] int32_t instanceId,
                                        [[maybe_unused]] bool isStartWithDebug, [[maybe_unused]] void *vm,
                                        [[maybe_unused]] const DebuggerPostTask &debuggerPostTask,
                                        [[maybe_unused]] bool isDebugApp)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, tid = " << tid
                        << ", isStartWithDebug = " << isStartWithDebug << ", instanceId = " << instanceId
                        << ", isDebugApp = " << isDebugApp;

    if (!isDebugApp) {
        LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, not debug app, return directly.";
        return true;
    }

    if (!debuggerPostTask) {
        LOG(ERROR, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, debuggerPostTask is nullptr";
        return false;
    }

    bool reusedHandle = gHybridDebuggerHandle_ != nullptr;
    void *handle = reusedHandle ? gHybridDebuggerHandle_ : Load(ARK_DEBUGGER_LIB_PATH);
    if (handle == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] gHybridDebuggerHandle_ load fail";
        return false;
    }
    Runtime::GetCurrent()->SetDebugMode(isStartWithDebug);
    Runtime::SetDebuggerLaunchOption(Runtime::GetOptions());

    // store debugger postTask in inspector.
    using StoreDebuggerInfo = void (*)(int, void *, const DebuggerPostTask &);
    auto symOfStoreDebuggerInfo = reinterpret_cast<StoreDebuggerInfo>(ResolveSymbol(handle, "StoreDebuggerInfo"));
    if (symOfStoreDebuggerInfo == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] Resolve StoreDebuggerInfo symbol fail: " << dlerror();
        if (!reusedHandle) {
            Close(handle);
        }
        Runtime::GetCurrent()->SetDebugMode(false);
        Runtime::ResetDebuggerLaunchOption(Runtime::GetOptions());
        return false;
    }
    symOfStoreDebuggerInfo(tid, vm, debuggerPostTask);

    // Initialize debugger
    using InitializeDebuggerForSocketpair = bool (*)(void *, bool);
    auto sym =
        reinterpret_cast<InitializeDebuggerForSocketpair>(ResolveSymbol(handle, "InitializeDebuggerForSocketpair"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] InitializeDebuggerForSocketpair symbol fail: " << dlerror();
        if (!reusedHandle) {
            Close(handle);
        }
        Runtime::GetCurrent()->SetDebugMode(false);
        Runtime::ResetDebuggerLaunchOption(Runtime::GetOptions());
        return false;
    }
    if (!sym(vm, true)) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] InitializeDebuggerForSocketpair fail";
        if (!reusedHandle) {
            Close(handle);
        }
        Runtime::GetCurrent()->SetDebugMode(false);
        Runtime::ResetDebuggerLaunchOption(Runtime::GetOptions());
        return false;
    }
    gHybridDebuggerHandle_ = handle;
    DebuggerLaunchSetup();
    if (isStartWithDebug) {
        using WaitForDebugger = void (*)(void *);
        auto symOfWaitForDebugger = reinterpret_cast<WaitForDebugger>(ResolveSymbol(handle, "WaitForDebugger"));
        if (symOfWaitForDebugger == nullptr) {
            LOG(ERROR, DEBUGGER) << "Resolve symbol WaitForDebugger fail: " << dlerror();
            Runtime::GetCurrent()->SetDebugMode(false);
            Runtime::ResetDebuggerLaunchOption(Runtime::GetOptions());
            return false;
        }
        symOfWaitForDebugger(vm);
    }

    return true;
}

bool ArkDebugNativeAPI::StopDebugger([[maybe_unused]] void *vm)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StopDebugger";

    if (gHybridDebuggerHandle_ == nullptr) {
        LOG(ERROR, DEBUGGER) << "ArkDebugNativeAPI::StopDebugger, debugger library is not loaded";
        return false;
    }
    using StopDebug = void (*)(void *);
    auto sym = reinterpret_cast<StopDebug>(ResolveSymbol(gHybridDebuggerHandle_, "StopDebug"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "g_initializeInspectorForStatic load error" << dlerror();
        return false;
    }

    sym(vm);
    ark::Runtime::GetCurrent()->SetDebugMode(false);
    ark::Runtime::GetCurrent()->UnloadDebugger();
    ark::Runtime::ResetDebuggerLaunchOption(Runtime::GetOptions());
    return true;
}

void ArkDebugNativeAPI::DebuggerLaunchSetup()
{
    if (Runtime::GetCurrent()->IsDebuggerAttached()) {
        LOG(ERROR, DEBUGGER) << "ArkDebugNativeAPI::DebuggerLaunchSetup: debug session already running, skip";
        return;
    }
    Runtime::GetCurrent()->StartDebugSession();
    Runtime::GetCurrent()->GetPandaVM()->LoadDebuggerAgent();
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(ManagedThread::GetCurrent());
}

bool ArkDebugNativeAPI::StartDebuggerForSocketPair([[maybe_unused]] int tid, [[maybe_unused]] int socketfd)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StartDebugForSocketPair, tid = " << tid << " socketfd is " << socketfd;
    Runtime::GetCurrent()->SetDebugMode(true);
    using StartDebuggerForSocketpair = bool (*)(int, int, bool);
    auto sym =
        reinterpret_cast<StartDebuggerForSocketpair>(ResolveSymbol(gHybridDebuggerHandle_, "StartDebugForSocketpair"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "g_initializeInspectorForStatic load error:%{public}s" << dlerror();
        ark::Runtime::GetCurrent()->SetDebugMode(false);
        return false;
    }
    bool ret = sym(tid, socketfd, true);
    if (!ret) {
        // Reset the config
        ark::Runtime::GetCurrent()->SetDebugMode(false);
    }
    return ret;
}

bool ArkDebugNativeAPI::IsDebugModeEnabled()
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::IsDebugModeEnabled is " << ark::Runtime::GetCurrent()->IsDebugMode();

    return ark::Runtime::GetCurrent()->IsDebugMode();
}

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
static std::shared_ptr<tooling::sampler::SamplesRecord> g_profileInfoBuffer = nullptr;
static os::memory::Mutex g_profileMutex;
static std::string g_filePath;
// NOLINTEND(fuchsia-statically-constructed-objects)

bool ArkDebugNativeAPI::StartProfiling(const std::string &filePath, uint32_t interval)
{
    if (filePath.empty()) {
        LOG(DEBUG, PROFILER) << "File path is empty";
        return false;
    }
    os::memory::LockHolder lock(g_profileMutex);
    if (ark::Runtime::GetCurrent() == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        return false;
    }
    if (g_profileInfoBuffer) {
        LOG(WARNING, PROFILER) << "Repeatedly start cpu profiler, please call StopProfiling first.";
        return false;
    }
    g_profileInfoBuffer = std::make_shared<tooling::sampler::SamplesRecord>();
    g_profileInfoBuffer->SetThreadStartTime(tooling::sampler::Sampler::GetMicrosecondsTimeStamp());
    if (!Runtime::GetCurrent()->GetTools().IsSamplingProfilerCreate()) {
        Runtime::GetCurrent()->GetTools().CreateSamplingProfiler();
    }
    auto result = Runtime::GetCurrent()->GetTools().StartSamplingProfiler(
        std::make_unique<tooling::sampler::InspectorStreamWriter>(g_profileInfoBuffer), interval);
    if (!result) {
        g_profileInfoBuffer.reset();
        LOG(DEBUG, PROFILER) << "Fatal, profiler start failed";
        return false;
    }
    g_filePath = filePath;
    return true;
}

bool ArkDebugNativeAPI::StopProfiling()
{
    os::memory::LockHolder lock(g_profileMutex);
    if (Runtime::GetCurrent() == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        g_profileInfoBuffer.reset();
        return false;
    }

    if (!g_profileInfoBuffer) {
        LOG(DEBUG, PROFILER) << "Fatal, profiler inactive";
        g_profileInfoBuffer.reset();
        return false;
    }

    if (g_filePath.empty()) {
        LOG(WARNING, PROFILER) << "Fatal, profiler session is not file mode";
        return false;
    }

    Runtime::GetCurrent()->GetTools().StopSamplingProfiler();
    auto profileInfoPtr = g_profileInfoBuffer->GetAllThreadsProfileInfos();
    if (!profileInfoPtr) {
        g_profileInfoBuffer.reset();
        LOG(WARNING, PROFILER) << "The CPU profiler did not collect any data.";
        return true;
    }
    g_profileInfoBuffer.reset();
    tooling::inspector::Profile profile(std::move(profileInfoPtr));
    JsonObjectBuilder builder;
    profile.Serialize(builder);
    std::string jsonData = std::move(builder).Build();
    std::ofstream outputFile(g_filePath);
    if (!outputFile.is_open()) {
        LOG(ERROR, PROFILER) << "Failed to open output file: " << g_filePath;
        return false;
    }
    outputFile << jsonData;
    if (outputFile.fail()) {
        LOG(ERROR, PROFILER) << "Failed to write profiling data to file: " << g_filePath;
        return false;
    }
    outputFile.close();
    g_filePath.clear();
    return true;
}

bool ArkDebugNativeAPI::StartProfiler(void *vm, const ProfilerOption &option, const DebuggerPostTask &debuggerPostTask,
                                      bool isDebugApp)
{
    LOG(INFO, PROFILER) << "ArkDebugNativeAPI::StartProfiler, type = " << static_cast<int>(option.profilerType)
                        << ", tid = " << option.tid << ", instanceId = " << option.instanceId
                        << ", isDebugApp = " << isDebugApp;

    if (ark::Runtime::GetCurrent() == nullptr) {
        LOG(DEBUG, PROFILER) << "ArkDebugNativeAPI::StartProfiler, runtime is not created";
        return false;
    }

    if (option.profilerType != ProfilerType::CPU_PROFILER) {
        LOG(ERROR, PROFILER) << "ArkDebugNativeAPI::StartProfiler, unsupported profiler type";
        return false;
    }

    if (isDebugApp && vm == nullptr) {
        LOG(WARNING, PROFILER) << "ArkDebugNativeAPI::StartProfiler, debug app without vm: hybrid debugger "
                                  "setup on the dynamic side will be skipped";
    }

    if (!NotifyDebugMode(option.tid, option.instanceId, false, vm, debuggerPostTask, isDebugApp)) {
        LOG(ERROR, PROFILER) << "ArkDebugNativeAPI::StartProfiler, NotifyDebugMode failed";
        return false;
    }

    if (!StartProfilingSession(option.interval)) {
        LOG(ERROR, PROFILER) << "ArkDebugNativeAPI::StartProfiler, profiler session start failed";
        if (isDebugApp && !StopDebugger(vm)) {
            LOG(WARNING, PROFILER) << "ArkDebugNativeAPI::StartProfiler, failed to roll back the debugger state";
        }
        return false;
    }

    LOG(INFO, PROFILER) << "ArkDebugNativeAPI::StartProfiler success";
    return true;
}

bool ArkDebugNativeAPI::StartProfilingSession(uint32_t interval)
{
    os::memory::LockHolder lock(g_profileMutex);
    if (g_profileInfoBuffer) {
        LOG(WARNING, PROFILER) << "StartProfilingSession: session already exists, stop it first";
        return false;
    }

    g_filePath.clear();
    g_profileInfoBuffer = std::make_shared<tooling::sampler::SamplesRecord>();
    g_profileInfoBuffer->SetThreadStartTime(tooling::sampler::Sampler::GetMicrosecondsTimeStamp());
    if (!Runtime::GetCurrent()->GetTools().IsSamplingProfilerCreate()) {
        Runtime::GetCurrent()->GetTools().CreateSamplingProfiler();
    }
    auto result = Runtime::GetCurrent()->GetTools().StartSamplingProfiler(
        std::make_unique<tooling::sampler::InspectorStreamWriter>(g_profileInfoBuffer), interval);
    if (!result) {
        LOG(ERROR, PROFILER) << "StartProfilingSession: failed to start the sampling profiler";
        g_profileInfoBuffer.reset();
        return false;
    }
    return true;
}

std::shared_ptr<tooling::sampler::SamplesRecord> ArkDebugNativeAPI::StopProfilingSession()
{
    os::memory::LockHolder lock(g_profileMutex);
    if (g_profileInfoBuffer == nullptr) {
        return nullptr;
    }

    if (!g_filePath.empty()) {
        LOG(WARNING, PROFILER) << "StopProfilingSession: session is file mode";
        return nullptr;
    }
    auto buffer = g_profileInfoBuffer;
    g_profileInfoBuffer.reset();

    if (Runtime::GetCurrent() != nullptr && Runtime::GetCurrent()->GetTools().IsSamplingProfilerCreate()) {
        Runtime::GetCurrent()->GetTools().StopSamplingProfiler();
    }
    return buffer;
}

std::shared_ptr<tooling::sampler::SamplesRecord> ArkDebugNativeAPI::GetProfileInfoBuffer()
{
    os::memory::LockHolder lock(g_profileMutex);
    return g_profileInfoBuffer;
}

bool ArkDebugNativeAPI::IsProfilerRunning()
{
    os::memory::LockHolder lock(g_profileMutex);
    return g_profileInfoBuffer != nullptr;
}

void ArkDebugNativeAPI::ResetProfileInfoBuffer()
{
    os::memory::LockHolder lock(g_profileMutex);
    g_profileInfoBuffer.reset();
}
}  // namespace ark
