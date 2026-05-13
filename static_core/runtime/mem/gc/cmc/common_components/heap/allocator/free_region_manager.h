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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_FREE_REGION_MANAGER_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_FREE_REGION_MANAGER_H

#include "common_components/heap/allocator/treap.h"
#include "common_interfaces/heap/region_desc.h"
#include "common_components/common/scoped_object_access.h"

namespace common_vm {
class RegionManager;

// This class is and should be accessed only for region allocation. we do not rely on it to check region status.
class FreeRegionManager {
public:
    explicit FreeRegionManager(RegionManager &manager) : regionManager_(manager) {}

    virtual ~FreeRegionManager()
    {
        dirtyUnitTree_.Fini();
        releasedUnitTree_.Fini();
    }

    void Initialize(uint32_t regionCnt)
    {
        releasedUnitTree_.Init(regionCnt);
        dirtyUnitTree_.Init(regionCnt);
    }

    RegionDesc *TakeRegion(size_t num, RegionDesc::UnitRole uclass, bool expectPhysicalMem)
    {
        uint32_t idx = 0;
        bool tryDirtyTree = true;
        bool tryReleasedTree = true;

        // try as hard as we can to take free regions for allocation.
        while (tryDirtyTree || tryReleasedTree) {
            // first try to get a dirty region.
            if (tryDirtyTree && dirtyUnitTreeMutex_.TryLock()) {
                if (dirtyUnitTree_.TakeUnits(num, idx)) {
                    LOG(DEBUG, GC) << "c-tree " << &dirtyUnitTree_ << " alloc dirty units[" << idx << "+" << num << ", "
                                   << idx + num << ") @[0x" << std::hex << RegionDesc::GetUnitAddress(idx) << ", 0x"
                                   << std::hex << RegionDesc::GetUnitAddress(idx + num) << "), " << std::dec
                                   << dirtyUnitTree_.GetTotalCount() << " dirty-units left";
                    // it makes sense to slow down allocation by clearing region memory.
                    RegionDesc::ClearUnits(idx, num);
                    RegionDesc *region = RegionDesc::InitRegion(idx, num, uclass);
                    dirtyUnitTreeMutex_.Unlock();
                    return region;
                }
                tryDirtyTree = false;  // once we fail to take units, stop trying.
                dirtyUnitTreeMutex_.Unlock();
            }

            // then try to get a released region.
            if (tryReleasedTree && releasedUnitTreeMutex_.TryLock()) {
                if (releasedUnitTree_.TakeUnits(num, idx)) {
#ifdef _WIN64
                    MemoryMap::CommitMemory(reinterpret_cast<void *>(RegionDesc::GetUnitAddress(idx)),
                                            num * RegionDesc::UNIT_SIZE);
#endif
                    LOG(DEBUG, GC) << "c-tree " << &releasedUnitTree_ << " alloc released units" << idx << "+" << num
                                   << " @0x" << std::hex << RegionDesc::GetUnitAddress(idx) << "+" << std::dec
                                   << num * RegionDesc::UNIT_SIZE << ", " << releasedUnitTree_.GetTotalCount()
                                   << " released-units left";
                    RegionDesc *region = RegionDesc::InitRegion(idx, num, uclass);
                    releasedUnitTreeMutex_.Unlock();
                    PrehandleReleasedUnit(expectPhysicalMem, idx, num);
                    return region;
                }
                tryReleasedTree = false;  // once we fail to take units, stop trying.
                releasedUnitTreeMutex_.Unlock();
            }
        }

        return nullptr;
    }

    // add units [idx, idx + num)
    void AddGarbageUnits(uint32_t idx, uint32_t num)
    {
        ark::os::memory::LockHolder lg(dirtyUnitTreeMutex_);
        if (UNLIKELY(!dirtyUnitTree_.MergeInsert(idx, num, true))) {
            LOG(FATAL, COMMON) << "tid " << GetTid() << ": failed to add dirty units [" << idx << "+" << num << ", "
                               << (idx + num) << ")";
        }
    }

    void AddReleaseUnits(uint32_t idx, uint32_t num)
    {
        ark::os::memory::LockHolder lg(releasedUnitTreeMutex_);
        if (UNLIKELY(!releasedUnitTree_.MergeInsert(idx, num, true))) {
            LOG(FATAL, COMMON) << "tid " << GetTid() << ": failed to add release units [" << idx << "+" << num << ", "
                               << (idx + num) << ")";
        }
    }

    uint32_t GetDirtyUnitCount() const
    {
        return dirtyUnitTree_.GetTotalCount();
    }
    uint32_t GetReleasedUnitCount() const
    {
        return releasedUnitTree_.GetTotalCount();
    }

#ifndef NDEBUG
    void DumpReleasedUnitTree() const
    {
        releasedUnitTree_.DumpTree("released-unit tree");
    }
    void DumpDirtyUnitTree() const
    {
        dirtyUnitTree_.DumpTree("dirty-unit tree");
    }
#endif

    size_t CalculateBytesToRelease() const;
    size_t ReleaseGarbageRegions(size_t targetCachedSize);

private:
    inline void PrehandleReleasedUnit(bool expectPhysicalMem, size_t idx, size_t num) const
    {
        if (expectPhysicalMem) {
            RegionDesc::ClearUnits(idx, num);
        }
    }
    RegionManager &regionManager_;

    // physical pages of released units are probably released and they are prepared for allocation.
    ark::os::memory::Mutex releasedUnitTreeMutex_;
    Treap releasedUnitTree_;

    // dirty units are neither cleared nor released, thus must be zeroed explicitly for allocation.
    ark::os::memory::Mutex dirtyUnitTreeMutex_;
    Treap dirtyUnitTree_;
};
}  // namespace common_vm
#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_FREE_REGION_MANAGER_H
