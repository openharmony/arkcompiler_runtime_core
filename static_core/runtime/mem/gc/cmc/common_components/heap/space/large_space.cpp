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

#include "common_components/heap/space/large_space.h"
#include "heap/allocator/region_manager.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/heap/allocator/regional_heap.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace ark::common_vm {
void LargeSpace::AssembleGarbageCandidates()
{
    largeRegionList_.MergeRegionList(recentLargeRegionList_, RegionDesc::RegionType::LARGE_REGION);
}

void LargeSpace::CollectFixTasks(FixHeapTaskList &taskList)
{
    if (Heap::GetHeap().GetGCReason() == GCTaskCause::YOUNG_GC_CAUSE) {
        FixHeapWorker::CollectFixHeapTasks(taskList, largeRegionList_, FIX_OLD_REGION);
        FixHeapWorker::CollectFixHeapTasks(taskList, recentLargeRegionList_, FIX_RECENT_OLD_REGION);
    } else {
        FixHeapWorker::CollectFixHeapTasks(taskList, largeRegionList_, FIX_REGION);
        FixHeapWorker::CollectFixHeapTasks(taskList, recentLargeRegionList_, FIX_RECENT_REGION);
    }
}

size_t LargeSpace::CollectLargeGarbage()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectLargeGarbage", "");
    size_t garbageSize = 0;
    RegionDesc *region = largeRegionList_.GetHeadRegion();
    while (region != nullptr) {
        HeapAddress addr = region->GetRegionStart();
        BaseObject *obj = reinterpret_cast<BaseObject *>(addr);

        if (region->IsJitFortAwaitInstallFlag()) {
            region = region->GetNextRegion();
            continue;
        }
        if (!RegionalHeap::IsSurvivedObject(obj) && !region->IsNewObjectSinceMarking(obj)) {
            LOG(DEBUG, GC) << "reclaim large region " << region << "@0x" << std::hex << region->GetRegionStart() << "+"
                           << std::dec << region->GetRegionAllocatedSize() << " type "
                           << static_cast<size_t>(region->GetRegionType());
            RegionDesc *del = region;
            region = region->GetNextRegion();
            largeRegionList_.DeleteRegion(del);
            if (del->GetRegionSize() > RegionDesc::LARGE_OBJECT_RELEASE_THRESHOLD) {
                garbageSize += regionManager_.ReleaseRegion(del);
            } else {
                garbageSize += regionManager_.CollectRegion(del);
            }
        } else {
            LOG(DEBUG, GC) << "clear mark-bit for large region " << region << "@0x" << std::hex
                           << region->GetRegionStart() << "+" << std::dec << region->GetRegionAllocatedSize()
                           << " type " << static_cast<size_t>(region->GetRegionType());
            region = region->GetNextRegion();
        }
    }

    region = recentLargeRegionList_.GetHeadRegion();
    while (region != nullptr) {
        region = region->GetNextRegion();
    }

    return garbageSize;
}

uintptr_t LargeSpace::Alloc(size_t size, bool allowGC)
{
    size_t alignedSize = AlignUp<size_t>(size + RegionDesc::UNIT_HEADER_SIZE, RegionDesc::UNIT_SIZE);
    size_t regionCount = alignedSize / RegionDesc::UNIT_SIZE;
    RegionDesc *region =
        regionManager_.TakeRegion(regionCount, RegionDesc::UnitRole::LARGE_SIZED_UNITS, false, allowGC);
    if (region == nullptr) {
        return 0;
    }
    InitRegionPhaseLine(region);
    LOG(DEBUG, GC) << "alloc large region @0x" << std::hex << region->GetRegionStart() << "+" << std::dec
                   << region->GetRegionSize() << " type " << static_cast<size_t>(region->GetRegionType());
    uintptr_t addr = region->Alloc(size);
    DCHECK(addr > 0);
    recentLargeRegionList_.PrependRegion(region, RegionDesc::RegionType::RECENT_LARGE_REGION);
    return addr;
}

}  // namespace ark::common_vm
