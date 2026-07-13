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

#include "common_components/heap/allocator/fix_heap.h"

#include "common_components/heap/collector/collector.h"
#include "common_interfaces/heap/region_desc.h"
#include "common_components/heap/allocator/region_list.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc/cmc-allocator.h"

namespace ark::common_vm {

void FixHeap::CollectFixHeapTasks(FixHeapTaskList &taskList, RegionList &list, FixRegionType type)
{
    list.VisitAllRegions([&taskList, type](RegionDesc *region) { taskList.emplace_back(region, type); });
}

std::stack<std::pair<RegionList *, RegionDesc *>> PostFixHeap::emptyRegionsToCollect;

void PostFixHeap::AddEmptyRegionToCollectDuringPostFix(RegionList *list, RegionDesc *region)
{
    emptyRegionsToCollect.emplace(list, region);
}

size_t PostFixHeap::CollectEmptyRegions()
{
    RegionalHeap &theAllocator = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
    RegionManager &regionManager = theAllocator.GetRegionManager();
    bool trackFreedObjects =
        static_cast<mem::CMCObjectAllocator *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator())
            ->NeedToTrackFreedObjects();
    size_t garbageSize = 0;
    size_t freedObjectBytes = 0;
    size_t freedObjectCount = 0;

    while (!emptyRegionsToCollect.empty()) {
        auto [list, del] = emptyRegionsToCollect.top();
        emptyRegionsToCollect.pop();

        if (trackFreedObjects) {
            del->VisitAllObjects([&freedObjectCount](BaseObject *obj) {
                LOG_DEBUG_OBJECT_EVENTS << "DELETE NONMOVABLE object " << obj;
                ++freedObjectCount;
            });
        }

        list->DeleteRegion(del);
        freedObjectBytes += del->GetRegionAllocatedSize();
        garbageSize += regionManager.CollectRegion(del);
    }

    if (freedObjectBytes > 0) {
        Runtime::GetCurrent()->GetPandaVM()->GetMemStats()->RecordFreeObjects(
            freedObjectCount, freedObjectBytes, ark::SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    }
    return garbageSize;
}

};  // namespace ark::common_vm
