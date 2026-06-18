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
#include "common_components/heap/collector/gc_request.h"

#include "common_components/base/time_utils.h"
#include "common_components/heap/collector/gc_stats.h"
namespace ark::common_vm {
namespace {
// Set a safe initial value so that the first GC is able to trigger.
uint64_t g_initHeuTriggerTimestamp = TimeUtil::NanoSeconds() - LONG_MIN_HEU_GC_INTERVAL_NS;
uint64_t g_initNativeTriggerTimestamp = TimeUtil::NanoSeconds() - MIN_ASYNC_GC_INTERVAL_NS;
}  // namespace

inline bool GCRequest::IsFrequentGC() const
{
    if (minIntervelNs == 0) {
        return false;
    }
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    return (now - prevRequestTime < minIntervelNs);
}

inline bool GCRequest::IsFrequentAsyncGC() const
{
    uint64_t now = TimeUtil::NanoSeconds();
    return (now - GCStats::GetPrevGCFinishTime()) < minIntervelNs;
}

// heuristic gc is triggered by object allocation,
// the heap stats should take into consideration.
inline bool GCRequest::IsFrequentHeuristicGC() const
{
    return IsFrequentAsyncGC();
}

bool GCRequest::ShouldBeIgnored() const
{
    switch (reason) {
        case GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE:
        case GCTaskCause::MIXED:
        case GCTaskCause::YOUNG_GC_CAUSE:
        case GCTaskCause::STARTUP_COMPLETE_CAUSE:
            return IsFrequentHeuristicGC();
        case GCTaskCause::NATIVE_ALLOC_CAUSE:
            return IsFrequentAsyncGC();
        case GCTaskCause::OOM_CAUSE:
        case GCTaskCause::EXPLICIT_CAUSE:
            return IsFrequentGC();
        default:
            return false;
    }
}

ark::common_vm::GCRequest g_gcRequests[] = {
    {GCTaskCause::INVALID_CAUSE, "invalid", true, false, 0, 0},
    {GCTaskCause::YOUNG_GC_CAUSE, "young", false, true, LONG_MIN_HEU_GC_INTERVAL_NS, g_initHeuTriggerTimestamp},
    {GCTaskCause::PYGOTE_FORK_CAUSE, "pygote_fork", true, false, 0, 0},
    {GCTaskCause::STARTUP_COMPLETE_CAUSE, "startup_complete", false, true, 0, 0},
    {GCTaskCause::NATIVE_ALLOC_CAUSE, "native_alloc", false, true, MIN_ASYNC_GC_INTERVAL_NS,
     g_initNativeTriggerTimestamp},
    {GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, "heuristic", false, true, LONG_MIN_HEU_GC_INTERVAL_NS,
     g_initHeuTriggerTimestamp},
    {GCTaskCause::MIXED, "mixed", true, false, 0, 0},
    {GCTaskCause::EXPLICIT_CAUSE, "explicit", true, false, 0, 0},
    {GCTaskCause::OOM_CAUSE, "oom", true, false, 0, 0},
    {GCTaskCause::CROSSREF_CAUSE, "xref", true, false, 0, 0},
};
}  // namespace ark::common_vm
