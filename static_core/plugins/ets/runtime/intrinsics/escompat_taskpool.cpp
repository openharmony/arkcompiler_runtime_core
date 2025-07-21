/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <atomic>
#include <thread>
#include "libpandabase/os/time.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "runtime/include/runtime.h"

namespace ark::ets::intrinsics::taskpool {

static std::atomic<EtsInt> g_taskId = 1;
static std::atomic<EtsInt> g_taskGroupId = 1;
static std::atomic<EtsInt> g_seqRunnerId = 1;
static constexpr const char *LAUNCH = "launch";

extern "C" EtsInt GenerateTaskId()
{
    return g_taskId++;
}

extern "C" EtsInt GenerateGroupId()
{
    return g_taskGroupId++;
}

extern "C" EtsInt GenerateSeqRunnerId()
{
    return g_seqRunnerId++;
}

extern "C" EtsBoolean IsUsingLaunchMode()
{
    const auto &taskpoolMode =
        Runtime::GetOptions().GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));

    bool res = (taskpoolMode == LAUNCH);
    return ark::ets::ToEtsBoolean(res);
}

extern "C" EtsBoolean IsSupportingInterop()
{
    bool res = false;
#ifdef PANDA_ETS_INTEROP_JS
    res = Runtime::GetOptions().IsTaskpoolSupportInterop(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));
#endif /* PANDA_ETS_INTEROP_JS */
    return ark::ets::ToEtsBoolean(res);
}

extern "C" EtsInt GetTaskPoolWorkersLimit()
{
    int32_t cpuCount = std::thread::hardware_concurrency();
    return cpuCount > 1 ? cpuCount - 1 : 1;  // 1: default number
}

}  // namespace ark::ets::intrinsics::taskpool
