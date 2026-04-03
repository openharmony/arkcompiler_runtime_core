/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "common_components/heap/collector/gc_stats.h"

#include "common_interfaces/base_runtime.h"
#include "common_components/base/time_utils.h"
#include "common_components/heap/heap.h"
#include "common_components/log/log.h"

#include <cmath>
#include <iomanip>

namespace common_vm {
size_t g_gcCount = 0;
uint64_t g_gcTotalTimeUs = 0;
size_t g_gcCollectedTotalBytes = 0;

size_t g_fullGCCount = 0;
double g_fullGCMeanRate = 0.0;

uint64_t GCStats::prevGcStartTime = TimeUtil::NanoSeconds() - LONG_MIN_HEU_GC_INTERVAL_NS;
uint64_t GCStats::prevGcFinishTime = TimeUtil::NanoSeconds() - LONG_MIN_HEU_GC_INTERVAL_NS;

void GCStats::Init()
{
    isConcurrentMark = false;
    async = false;
    gcStartTime = TimeUtil::NanoSeconds();
    gcEndTime = TimeUtil::NanoSeconds();

    totalSTWTime = 0;
    maxSTWTime = 0;

    collectedObjects = 0;
    collectedBytes = 0;

    fromSpaceSize = 0;
    smallGarbageSize = 0;

    nonMovableSpaceSize = 0;
    nonMovableGarbageSize = 0;

    largeSpaceSize = 0;
    largeGarbageSize = 0;

    liveBytesBeforeGC = 0;
    liveBytesAfterGC = 0;

    garbageRatio = 0.0;
    collectionRate = 0.0;

    // 20 MB:set 20 MB as intial value
    heapThreshold = std::min(BaseRuntime::GetInstance()->GetGCParam().gcThreshold, 20 * MB);
    // 0.2:set 20% heap size as intial value
    heapThreshold = std::min(static_cast<size_t>(Heap::GetHeap().GetMaxCapacity() * 0.2), heapThreshold);

    targetFootprint = heapThreshold;
    shouldRequestYoung = false;
}

static std::string FormatTimeMs(uint64_t ns)
{
    constexpr double nsPerMs = 1e6;
    std::ostringstream s;
    s << std::fixed << std::setprecision(3U) << (static_cast<double>(ns) / nsPerMs) << "ms";
    return s.str();
}

static std::string FormatMemory(size_t bytes)
{
    constexpr size_t kb = 1024;
    constexpr size_t mb = 1024 * 1024;
    constexpr size_t gb = 1024 * 1024 * 1024;
    std::ostringstream s;
    if (bytes >= gb) {
        s << (bytes / gb) << "GB";
    } else if (bytes >= mb) {
        s << (bytes / mb) << "MB";
    } else if (bytes >= kb) {
        s << (bytes / kb) << "KB";
    } else {
        s << bytes << "B";
    }
    return s.str();
}

static std::string FormatTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm tm;
    localtime_r(&time, &tm);
    constexpr int bufSize = 32U;
    char buf[bufSize];
    std::strftime(buf, sizeof(buf), "%b %d %T", &tm);
    std::ostringstream s;
    s << buf << "." << std::setfill('0') << std::setw(3U) << ms.count();
    return s.str();
}

void GCStats::Dump() const
{
    size_t allocatedNow = Heap::GetHeap().GetAllocatedSize();
    size_t totalHeap = Heap::GetHeap().GetMaxCapacity();
    constexpr uint16_t MAX_PERCENT = 100;
    uint16_t freePercent = (totalHeap == 0) ? 0 :
        static_cast<uint16_t>(round((1.0 - static_cast<double>(allocatedNow) / totalHeap) * MAX_PERCENT));
    uint64_t totalTimeNs = gcEndTime - gcStartTime;

    std::ostringstream oss;
    oss << "[" << g_gcCount << "] "
        << "[" << GCTypeToString(gcType) << " (" << g_gcRequests[reason].name << ")] "
        << FormatTimestamp() << " CMC GC freed "
        << FormatMemory(collectedBytes) << ", "
        << FormatMemory(largeGarbageSize) << " LOS objects, "
        << freePercent << "% free, "
        << FormatMemory(allocatedNow) << "/" << FormatMemory(totalHeap) << ", "
        << "phase: STW_TOTAL paused: " << FormatTimeMs(TotalSTWTime())
        << " total: " << FormatTimeMs(totalTimeNs);
    LOG_GC(INFO) << oss.str();

    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::GCStatsDump", (
                    "collectedObjects:" + std::to_string(collectedObjects) +
                    ";collectedBytes:" + std::to_string(collectedBytes) +
                    ";liveSize:" + std::to_string(allocatedNow) +
                    ";heapSize:" + std::to_string(totalHeap) +
                    ";total pause:" + FormatTimeMs(TotalSTWTime()) +
                    ";total GC time:" + FormatTimeMs(totalTimeNs)
                ).c_str());
}
} // namespace common_vm
