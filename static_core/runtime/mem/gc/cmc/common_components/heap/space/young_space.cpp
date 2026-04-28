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

#include "common_components/heap/space/young_space.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace ark::common_vm {
void YoungSpace::DumpRegionStats() const
{
    size_t tlRegions = tlRegionList_.GetRegionCount();
    size_t tlUnits = tlRegionList_.GetUnitCount();
    size_t tlSize = tlUnits * RegionDesc::UNIT_SIZE;
    size_t allocTLSize = tlRegionList_.GetAllocatedSize();

    size_t recentFullRegions = recentFullRegionList_.GetRegionCount();
    size_t recentFullUnits = recentFullRegionList_.GetUnitCount();
    size_t recentFullSize = recentFullUnits * RegionDesc::UNIT_SIZE;
    size_t allocRecentFullSize = recentFullRegionList_.GetAllocatedSize();

    size_t units = tlUnits + recentFullUnits;
    LOG(DEBUG, GC) << "\tyoung space units: " << units << " (" << units * RegionDesc::UNIT_SIZE << " B)";
    LOG(DEBUG, GC) << "\t  tl-regions " << tlRegions << ": " << tlUnits << " units (" << tlSize << " B, alloc "
                   << allocTLSize << ")";
    LOG(DEBUG, GC) << "\t  recent-full regions " << recentFullRegions << ": " << recentFullUnits << " units ("
                   << recentFullSize << " B, alloc " << allocRecentFullSize << ")";
}

RegionDesc *YoungSpace::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    RegionDesc *region = regionManager_.TakeRegion(expectPhysicalMem, true);
    ASSERT_PRINT(!IsGcThread(), "GC thread cannot take tlRegion");
    if (region != nullptr) {
        LOG(DEBUG, GC) << "alloc thread local young region @0x" << std::hex << region->GetRegionStart() << "+"
                       << std::dec << region->GetRegionAllocatedSize() << " type "
                       << static_cast<size_t>(region->GetRegionType());
        InitRegionPhaseLine(region);
        tlRegionList_.PrependRegion(region, RegionDesc::RegionType::THREAD_LOCAL_REGION);
    }
    return region;
}
}  // namespace ark::common_vm
