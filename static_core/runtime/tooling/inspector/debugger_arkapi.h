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

#ifndef PANDA_DEBUGGER_ARKAPI_H
#define PANDA_DEBUGGER_ARKAPI_H

#include <functional>
#include <memory>
#include <string>
#include <dlfcn.h>
#include <cstdint>

#ifndef PANDA_TARGET_WINDOWS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDA_DEBUGGER_PUBLIC_API __attribute__((visibility("default")))
#else
#define PANDA_DEBUGGER_PUBLIC_API __declspec(dllexport)
#endif

using DebuggerPostTask = std::function<void(std::function<void()> &&)>;

namespace ark {
namespace tooling::sampler {  // NOLINT(misc-definitions-in-headers)
class SamplesRecord;
}  // namespace tooling::sampler

constexpr uint32_t DEFAULT_SAMPLE_INTERVAL_US = 500;  // default sampling interval in microseconds

enum class ProfilerType : uint8_t { CPU_PROFILER, HEAP_PROFILER };

struct ProfilerOption {
    ProfilerType profilerType = ProfilerType::CPU_PROFILER;
    uint32_t interval = DEFAULT_SAMPLE_INTERVAL_US;
    int tid = 0;
    int32_t instanceId = 0;
};

class PANDA_DEBUGGER_PUBLIC_API ArkDebugNativeAPI final {
public:
    using DebuggerPostTask = std::function<void(std::function<void()> &&)>;
    static bool StartDebuggerForSocketPair(int tid, int socketfd = -1);
    static bool NotifyDebugMode(int tid, int32_t instanceId, bool isStartWithDebug, void *vm,
                                const DebuggerPostTask &debuggerPostTask, bool isDebugApp);
    static bool StopDebugger(void *vm);
    static bool IsDebugModeEnabled();

    static bool StartProfiler(void *vm, const ProfilerOption &option, const DebuggerPostTask &debuggerPostTask,
                              bool isDebugApp);
    static bool StartProfiling(const std::string &filePath, uint32_t interval = DEFAULT_SAMPLE_INTERVAL_US);
    static bool StopProfiling();

    static bool StartProfilingSession(uint32_t interval);
    static std::shared_ptr<tooling::sampler::SamplesRecord> StopProfilingSession();
    static std::shared_ptr<tooling::sampler::SamplesRecord> GetProfileInfoBuffer();
    static bool IsProfilerRunning();
    static void ResetProfileInfoBuffer();

    ArkDebugNativeAPI() = delete;
    ~ArkDebugNativeAPI() = delete;

    ArkDebugNativeAPI(const ArkDebugNativeAPI &) = delete;
    void operator=(const ArkDebugNativeAPI &) = delete;
    ArkDebugNativeAPI(ArkDebugNativeAPI &&) = delete;
    ArkDebugNativeAPI &operator=(ArkDebugNativeAPI &&) = delete;

private:
    static void DebuggerLaunchSetup();
    static void *gHybridDebuggerHandle_;
};

}  // namespace ark

#endif  // PANDA_DEBUGGER_ARKAPI_H
