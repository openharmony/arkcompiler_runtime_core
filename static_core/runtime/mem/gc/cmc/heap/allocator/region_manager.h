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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_MANAGER_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_MANAGER_H

#include <list>
#include <map>
#include <set>
#include <thread>
#include <unordered_set>
#include <vector>

#include "libarkbase/os/mutex.h"
#include "common_components/common/run_type.h"
#include "runtime/mem/gc/cmc/heap/allocator/alloc_buffer.h"
#include "runtime/mem/gc/cmc/heap/allocator/allocator.h"
#include "runtime/mem/gc/cmc/heap/allocator/region_list.h"
#include "runtime/mem/gc/cmc/heap/allocator/fix_heap.h"
#include "runtime/mem/gc/cmc/heap/allocator/slot_list.h"

namespace ark::common_vm {
class CompactCollector;
class RegionManager;
class Taskpool;
// RegionManager needs to know header size and alignment in order to iterate objects linearly
// and thus its Alloc should be rewrite with AllocObj(objSize)
class RegionManager {
public:
    /* region memory layout:
        1. some paddings memory to aligned
        2. region info for each region, part of heap metadata
        3. region space for allocation, i.e., the heap  --- start address is aligend to `RegionDesc::UNIT_SIZE`
    */
    static size_t GetHeapMemorySize(size_t heapSize)
    {
        size_t totalSize = RoundUp<size_t>(heapSize, RegionDesc::UNIT_SIZE);
        return totalSize;
    }

    static size_t GetHeapUnitCount(size_t heapSize)
    {
        heapSize = RoundUp<size_t>(heapSize, RegionDesc::UNIT_SIZE);
        size_t regionNum = heapSize / RegionDesc::UNIT_SIZE;
        return regionNum;
    }

    void Initialize(size_t regionNum, mem::HeapSpace *heapSpace);

    RegionManager() : garbageRegionList_("garbage regions") {}

    RegionManager(const RegionManager &) = delete;

    RegionManager &operator=(const RegionManager &) = delete;

    void DumpRegionStats() const;

    ~RegionManager()
    {
        Fini();
    }

    // take a region with *num* units for allocation
    RegionDesc *TakeRegion(size_t num, RegionDesc::UnitRole, bool expectPhysicalMem = false, bool allowgc = true,
                           bool isCopy = false);

    RegionDesc *TakeRegion(bool expectPhysicalMem, bool allowgc, bool isCopy = false)
    {
        return TakeRegion(1, RegionDesc::UnitRole::SMALL_SIZED_UNITS, expectPhysicalMem, allowgc, isCopy);
    }

    void CountLiveObject(const BaseObject *obj);

    void CollectFromSpaceGarbage(RegionList &fromList)
    {
        garbageRegionList_.MergeRegionList(fromList, RegionDesc::RegionType::GARBAGE_REGION);
    }

    size_t CollectRegion(RegionDesc *region)
    {
        LOG(DEBUG, GC) << "collect region " << region << "@0x" << std::hex << region->GetRegionStart() << "+"
                       << std::dec << region->GetLiveByteCount() << " type "
                       << static_cast<size_t>(region->GetRegionType());

#ifdef USE_HWASAN
        ASAN_POISON_MEMORY_REGION(reinterpret_cast<const volatile void *>(region->GetRegionBase()),
                                  region->GetRegionBaseSize());
        const uintptr_t p_addr = region->GetRegionBase();
        const uintptr_t p_size = region->GetRegionBaseSize();
        LOG(DEBUG, COMMON) << std::hex << "set [" << p_addr << std::hex << ", " << p_addr + p_size << ") poisoned\n";
#endif
        garbageRegionList_.PrependRegion(region, RegionDesc::RegionType::GARBAGE_REGION);
        if (region->IsLargeRegion()) {
            return region->GetRegionSize();
        } else {
            return region->GetRegionSize() - region->GetLiveByteCount();
        }
    }

    void ReclaimRegion(RegionDesc *region);
    size_t ReleaseRegion(RegionDesc *region);

    void ReclaimGarbageRegions()
    {
        RegionDesc *garbage = garbageRegionList_.TakeHeadRegion();
        while (garbage != nullptr) {
            ReclaimRegion(garbage);
            garbage = garbageRegionList_.TakeHeadRegion();
        }
    }

    void ForEachObjectUnsafe(const std::function<void(BaseObject *)> &visitor) const;
    void ForEachObjectSafe(const std::function<void(BaseObject *)> &visitor) const;

    // this method checks whether allocation is permitted for now, otherwise, it is suspened
    // until allocation does no harm to gc.
    void RequestForRegion(size_t size);

    size_t GetCurrentHeapSize() const;

    size_t GetMaxHeapSize() const;

private:
    inline void TagHugePage(RegionDesc *region, size_t num) const;
    inline void UntagHugePage(RegionDesc *region, size_t num) const;

    template <typename V>
    void IterateAllocatedUnits(const V &visitor) const
    {
        for (auto *region : regionsInUse_) {
            visitor(region);
        }
    }

    void ClearGCInfo(RegionList &list)
    {
        RegionList tmp("temp region list");
        list.CopyListTo(tmp);
        tmp.VisitAllRegions([](RegionDesc *region) {
            region->ClearMarkingCopyLine();
            region->ClearLiveInfo();
            region->ResetMarkBit();
        });
    }

    void Fini();

    // cache for fromRegionList after forwarding.
    RegionList garbageRegionList_;
    mutable os::memory::Mutex regionsInUseLock_;
    std::unordered_set<RegionDesc *> regionsInUse_;

    // the time when previous region was allocated, which is assigned with returned value by timeutil::NanoSeconds().
    std::atomic<uint64_t> prevRegionAllocTime_ = {0};

    mem::HeapSpace *heapSpace_ {nullptr};

    friend class VerifyIterator;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_MANAGER_H
