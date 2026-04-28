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

#include "common_components/heap/space/to_space.h"
#include "common_components/heap/space/old_space.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common_vm {
void ToSpace::DumpRegionStats() const
{
    size_t tlToRegions = tlToRegionList_.GetRegionCount();
    size_t tlToUnits = tlToRegionList_.GetUnitCount();
    size_t tlToSize = tlToUnits * RegionDesc::UNIT_SIZE;
    size_t allocTLToSize = tlToRegionList_.GetAllocatedSize(false);

    size_t fullToRegions = fullToRegionList_.GetRegionCount();
    size_t fullToUnits = fullToRegionList_.GetUnitCount();
    size_t fullToSize = fullToUnits * RegionDesc::UNIT_SIZE;
    size_t allocfullToSize = fullToRegionList_.GetAllocatedSize();

    size_t units = tlToUnits + fullToUnits;
    LOG(DEBUG, GC) << "\tto space units: " << units << " (" << units * RegionDesc::UNIT_SIZE << " B)";
    LOG(DEBUG, GC) << "\t  thread-local to-regions " << tlToRegions << ": " << tlToUnits << " units (" << tlToSize
                   << " B, alloc " << allocTLToSize << ")";
    LOG(DEBUG, GC) << "\t  full to-regions " << fullToRegions << ": " << fullToUnits << " units (" << fullToSize
                   << " B, alloc " << allocfullToSize << ")";
}

void ToSpace::GetPromotedTo(OldSpace &mspace)
{
    // release thread-local to-space regions as they will promote to old-space
    AllocBufferVisitor visitor = [](AllocationBuffer &tlab) { tlab.ClearRegion<AllocBufferType::TO>(); };
    Heap::GetHeap().GetAllocator().VisitAllocBuffers(visitor);

    mspace.PromoteRegionList(fullToRegionList_);
    mspace.PromoteRegionList(tlToRegionList_);
}

RegionDesc *ToSpace::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    RegionDesc *region = regionManager_.TakeRegion(expectPhysicalMem, false, true);
    if (region != nullptr) {
        LOG(DEBUG, GC) << "alloc thread local to region @0x" << std::hex << region->GetRegionStart() << "+" << std::dec
                       << region->GetRegionAllocatedSize() << " type " << static_cast<size_t>(region->GetRegionType());
        tlToRegionList_.PrependRegion(region, RegionDesc::RegionType::TO_REGION);
    }
    return region;
}

}  // namespace common_vm
