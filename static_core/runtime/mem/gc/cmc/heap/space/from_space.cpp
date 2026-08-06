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

#include "runtime/mem/gc/cmc/heap/allocator/regional_heap.h"
#include "runtime/mem/gc/cmc/heap/space/from_space.h"
#include "runtime/mem/gc/cmc/heap/space/old_space.h"
#include "runtime/mem/gc/cmc/heap/collector/collector_resources.h"
#include "common_components/taskpool/taskpool.h"
#include "mem/gc/workers/gc_workers_tasks.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace ark::common_vm {

void FromSpace::DumpRegionStats() const
{
    size_t fromRegions = fromRegionList_.GetRegionCount();
    size_t fromUnits = fromRegionList_.GetUnitCount();
    size_t fromSize = fromUnits * RegionDesc::UNIT_SIZE;
    size_t allocFromSize = fromRegionList_.GetAllocatedSize();

    size_t exemptedFromRegions = exemptedFromRegionList_.GetRegionCount();
    size_t exemptedFromUnits = exemptedFromRegionList_.GetUnitCount();
    size_t exemptedFromSize = exemptedFromUnits * RegionDesc::UNIT_SIZE;
    size_t allocExemptedFromSize = exemptedFromRegionList_.GetAllocatedSize();
    size_t units = fromUnits + exemptedFromUnits;

    LOG(DEBUG, GC) << "\tfrom space units: " << units << " (" << units * RegionDesc::UNIT_SIZE << " B)";
    LOG(DEBUG, GC) << "\t  from-regions " << fromRegions << ": " << fromUnits << " units (" << fromSize << " B, alloc "
                   << allocFromSize << ")";
    LOG(DEBUG, GC) << "\t  exempted from-regions " << exemptedFromRegions << ": " << exemptedFromUnits << " units ("
                   << exemptedFromSize << " B, alloc " << allocExemptedFromSize << ")";
}

// forward only regions whose garbage bytes is greater than or equal to exemptedRegionThreshold.
void FromSpace::ExemptFromRegions()
{
    size_t forwardBytes = 0;
    size_t floatingGarbage = 0;
    size_t oldFromBytes = fromRegionList_.GetUnitCount() * RegionDesc::UNIT_SIZE;
    RegionDesc *fromRegion = fromRegionList_.GetHeadRegion();
    while (fromRegion != nullptr) {
        size_t threshold = static_cast<size_t>(exemptedRegionThreshold_ * fromRegion->GetRegionSize());
        size_t liveBytes = fromRegion->GetLiveByteCount();
        if (liveBytes > threshold) {  // ignore this region
            RegionDesc *del = fromRegion;
            LOG(DEBUG, GC) << "region " << del << " @0x" << std::hex << del->GetRegionStart() << std::dec << "+"
                           << del->GetRegionAllocatedSize() << " exempted by forwarding: " << del->GetUnitCount()
                           << " units, " << del->GetLiveByteCount() << " live bytes";
            fromRegion = fromRegion->GetNextRegion();
            if (fromRegionList_.TryDeleteRegion(del, RegionDesc::RegionType::FROM_REGION,
                                                RegionDesc::RegionType::EXEMPTED_FROM_REGION)) {
                ExemptFromRegion(del);
            }
            floatingGarbage += (del->GetRegionSize() - del->GetLiveByteCount());
        } else {
            forwardBytes += fromRegion->GetLiveByteCount();
            fromRegion = fromRegion->GetNextRegion();
        }
    }

    size_t newFromBytes = fromRegionList_.GetUnitCount() * RegionDesc::UNIT_SIZE;
    size_t exemptedFromBytes = exemptedFromRegionList_.GetUnitCount() * RegionDesc::UNIT_SIZE;
    LOG(DEBUG, GC) << "exempt from-space: " << oldFromBytes << " B - " << exemptedFromBytes << " B -> " << newFromBytes
                   << " B, " << floatingGarbage << " B floating garbage, " << forwardBytes << " B to forward";
}

void FromSpace::ParallelCopyFromRegions(RegionDesc *startRegion, size_t regionCnt)
{
    RegionDesc *currentRegion = startRegion;
    for (size_t count = 0; (count < regionCnt) && currentRegion != nullptr; ++count) {
        RegionDesc *region = currentRegion;
        currentRegion = currentRegion->GetNextRegion();
        heap_.CopyRegion(region);
    }

    AllocationBuffer *allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();  // clear thread local region for gc threads.
    }
}

void FromSpace::CopyFromRegionsOnSingleThread()
{
    // iterate each region in fromRegionList
    RegionDesc *fromRegion = fromRegionList_.GetHeadRegion();
    while (fromRegion != nullptr) {
        ASSERT_PRINT(fromRegion->IsValidRegion(), "region is not head when get head region of from region list");
        RegionDesc *region = fromRegion;
        fromRegion = fromRegion->GetNextRegion();
        heap_.CopyRegion(region);
    }

    LOG(DEBUG, GC) << "forward " << fromRegionList_.GetUnitCount() << " from-region units";

    AllocationBuffer *allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();  // clear region for next GC
    }
}

void FromSpace::CopyFromRegionsOnGCWorkerTaskPool(mem::GCWorkersTaskPool *pool)
{
    // We won't change fromRegionList during gc, so we can use it without lock.
    const size_t totalRegionCount = fromRegionList_.GetRegionCount();
    if (UNLIKELY(totalRegionCount == 0)) {
        return;
    }

    RegionDesc *region = fromRegionList_.GetHeadRegion();
    PandaVector<mem::GCConcurrentCopyTask::TaskInfo> taskInfoVector;
    const size_t taskCount = (totalRegionCount + mem::GCConcurrentCopyTask::MAX_REGION_COUNT - 1) /
                             mem::GCConcurrentCopyTask::MAX_REGION_COUNT;
    taskInfoVector.reserve(taskCount);

    ScopedGcThreadType scopedGcThreadType;

    while (region != nullptr) {
        RegionDesc *startRegion = region;
        size_t regionCount = 0;
        while ((regionCount < mem::GCConcurrentCopyTask::MAX_REGION_COUNT) && (region != nullptr)) {
            region = region->GetNextRegion();
            ++regionCount;
        }
        taskInfoVector.emplace_back(this, startRegion, regionCount);
        [[maybe_unused]] bool added = pool->AddTask(mem::GCConcurrentCopyTask(&taskInfoVector.back()));
        ASSERT_PRINT(added, "failed to add concurrent copy task");
    }

    pool->WaitUntilTasksEnd();
}

void FromSpace::CopyFromRegions(mem::GCWorkersTaskPool *pool)
{
    if (pool != nullptr) {
        CopyFromRegionsOnGCWorkerTaskPool(pool);
    } else {
        CopyFromRegionsOnSingleThread();
    }
}

void FromSpace::GetPromotedTo(OldSpace &mspace)
{
    mspace.PromoteRegionList(exemptedFromRegionList_);
}
}  // namespace ark::common_vm
