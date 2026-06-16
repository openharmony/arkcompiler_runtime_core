/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/include/oom_stats.h"

#include "libarkbase/mem/space.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"

#ifdef PANDA_ENABLE_HISYSEVENT
// OHOS HiSysEvent / DFX headers use GNU-style macros; strict -Werror builds need local suppression.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
#include "dfx_signal_handler.h"
#include "hisysevent.h"
#endif

#include <string>
#include <unistd.h>

namespace ark::oom_stats {

namespace {
const std::string MAIN_THREAD_STR = "main thread";
const std::string CHILD_THREAD_STR = "child thread";
const std::string STATIC_STR = "static";
}  // namespace

void OomNotifier::NotifyBeforeManagedOom(size_t heapLimitBytes, size_t activeMemoryBytes, size_t failedAllocBytes,
                                         [[maybe_unused]] const std::string &processName,
                                         [[maybe_unused]] SpaceType spaceType)
{
#ifdef PANDA_ENABLE_HISYSEVENT
    pid_t pid = getprocpid();
    ark::os::thread::ThreadId tid = ark::os::thread::GetCurrentThreadId();
    const std::string threadType = (pid == static_cast<pid_t>(tid)) ? MAIN_THREAD_STR : CHILD_THREAD_STR;
    const char *spaceTypeStr = SpaceTypeToString(spaceType);

    int32_t ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "ARK_STATS_DUMP",
                                  OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, "PID", pid, "TID", tid, "PROCESS_NAME",
                                  processName.c_str(), "LIMITSIZE", heapLimitBytes, "ACTIVE_MEMORY", activeMemoryBytes,
                                  "TYPE", "OOMDump", "EVENT_CONFIG", "", "APP_RUNNING_UNIQUE_ID",
                                  &DFX_GetAppRunningUniqueId == nullptr ? "" : DFX_GetAppRunningUniqueId(),
                                  "ARKTS_TYPE", STATIC_STR, "THREAD_TYPE", threadType, "SPACE_TYPE", spaceTypeStr,
                                  "LAST_ALLOCATE_OBJECT_SIZE", failedAllocBytes, "HEAP_TYPE", "");
    if (ret != 0) {
        LOG(ERROR, RUNTIME) << "OomNotifier::NotifyBeforeManagedOom HiSysEventWrite failed, ret=" << ret;
    }
#else
    LOG(INFO, RUNTIME) << "OOM notify: heapLimit=" << heapLimitBytes << ", activeMemory=" << activeMemoryBytes
                       << ", failedAlloc=" << failedAllocBytes;
#endif
}

}  // namespace ark::oom_stats
