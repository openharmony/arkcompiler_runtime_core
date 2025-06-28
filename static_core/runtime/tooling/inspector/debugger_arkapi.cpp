/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "os/mutex.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/tooling/sampler/samples_record.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "types/profile_result.h"
#include "utils/json_builder.h"

namespace ark {
bool ArkDebugNativeAPI::NotifyDebugMode([[maybe_unused]] int tid, [[maybe_unused]] int32_t instanceId,
                                        [[maybe_unused]] bool debugApp)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, tid = " << tid << ", debugApp = " << debugApp
                        << ", instanceId = " << instanceId;
    if ((!debugApp) || (!Runtime::GetOptions().IsDebuggerEnable())) {
        return true;
    }

    auto loadRes = os::library_loader::Load(Runtime::GetOptions().GetDebuggerLibraryPath());
    if (!loadRes) {
        LOG(ERROR, DEBUGGER) << "Load library fail: " << Runtime::GetOptions().GetDebuggerLibraryPath() << " " << errno;
        return false;
    }
    os::library_loader::LibraryHandle handle = std::move(loadRes.Value());

    Runtime::GetCurrent()->SetDebugMode(true);

    using WaitForDebugger = void (*)();
    auto symOfWaitForDebugger = os::library_loader::ResolveSymbol(handle, "WaitForDebugger");
    if (!symOfWaitForDebugger) {
        LOG(ERROR, DEBUGGER) << "Resolve symbol WaitForDebugger fail: " << symOfWaitForDebugger.Error().ToString();
        return false;
    }
    reinterpret_cast<WaitForDebugger>(symOfWaitForDebugger.Value())();

    return true;
}

bool ArkDebugNativeAPI::StopDebugger()
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StopDebugger";

    ark::Runtime::GetCurrent()->SetDebugMode(false);
    return true;
}

bool ArkDebugNativeAPI::StartDebuggerForSocketPair([[maybe_unused]] int tid, [[maybe_unused]] int socketfd)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StartDebuggerForSocketPair, tid = " << tid;

    auto loadRes = os::library_loader::Load(Runtime::GetOptions().GetDebuggerLibraryPath());
    if (!loadRes) {
        LOG(ERROR, DEBUGGER) << "Load library fail: " << Runtime::GetOptions().GetDebuggerLibraryPath() << " " << errno;
        return false;
    }
    os::library_loader::LibraryHandle handle = std::move(loadRes.Value());

    using StartDebuggerForSocketpair = bool (*)(int, bool);
    auto sym = os::library_loader::ResolveSymbol(handle, "StartDebuggerForSocketpair");
    if (!sym) {
        LOG(ERROR, DEBUGGER) << "[StartDebuggerForSocketPair] Resolve symbol fail: " << sym.Error().ToString();
        return false;
    }
    bool breakOnStart = Runtime::GetOptions().IsDebuggerBreakOnStart();
    bool ret = reinterpret_cast<StartDebuggerForSocketpair>(sym.Value())(socketfd, breakOnStart);
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
}  // namespace ark
