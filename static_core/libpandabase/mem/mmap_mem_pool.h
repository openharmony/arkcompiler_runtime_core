/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_MEM_MMAP_MEM_POOL_H
#define LIBPANDABASE_MEM_MMAP_MEM_POOL_H

#include "libpandabase/mem/mem_pool.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/os/mem.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/mem/space.h"

#include <map>
#include <tuple>
#include <utility>

namespace panda {

class MMapMemPoolTest;
namespace mem::test {
class InternalAllocatorTest;
}  // namespace mem::test

class MmapPool {
public:
    using FreePoolsIter = std::multimap<size_t, MmapPool *>::iterator;
    explicit MmapPool(Pool pool, FreePoolsIter freePoolsIter, bool returnedToOs = true)
        : pool_(pool), returnedToOs_(returnedToOs), freePoolsIter_(freePoolsIter)
    {
    }

    ~MmapPool() = default;

    DEFAULT_COPY_SEMANTIC(MmapPool);
    DEFAULT_MOVE_SEMANTIC(MmapPool);

    size_t GetSize()
    {
        return pool_.GetSize();
    }

    bool IsReturnedToOS() const
    {
        return returnedToOs_;
    }

    void SetReturnedToOS(bool value)
    {
        returnedToOs_ = value;
    }

    void SetSize(size_t size)
    {
        pool_ = Pool(size, GetMem());
    }

    void *GetMem()
    {
        return pool_.GetMem();
    }

    // A free pool will be store in the free_pools_, and it's iterator will be recorded in the free_pools_iter_.
    // If the free_pools_iter_ is equal to the end of free_pools_, the pool is used.
    bool IsUsed(FreePoolsIter endIter)
    {
        return freePoolsIter_ == endIter;
    }

    FreePoolsIter GetFreePoolsIter()
    {
        return freePoolsIter_;
    }

    void SetFreePoolsIter(FreePoolsIter freePoolsIter)
    {
        freePoolsIter_ = freePoolsIter;
    }

private:
    Pool pool_;
    bool returnedToOs_;
    // record the iterator of the pool in the multimap
    FreePoolsIter freePoolsIter_;
};

class MmapPoolMap {
public:
    MmapPoolMap() = default;

    ~MmapPoolMap()
    {
        for (auto &pool : poolMap_) {
            delete pool.second;
        }
    }

    DEFAULT_COPY_SEMANTIC(MmapPoolMap);
    DEFAULT_MOVE_SEMANTIC(MmapPoolMap);

    // Find a free pool with enough size in the map. Split the pool, if the pool size is larger than required size.
    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Pool PopFreePool(size_t size);

    // Push the unused pool to the map.
    // If it is the last pool the method return its size and pages policy.
    // Else the method returns 0.
    template <OSPagesPolicy OS_PAGES_POLICY>
    [[nodiscard]] std::pair<size_t, OSPagesPolicy> PushFreePool(Pool pool);

    // Add a new pool to the map. This pool will be marked as used.
    void AddNewPool(Pool pool);

    // Get the sum of all free pools size.
    size_t GetAllSize() const;

    /**
     * Iterate over all free pools
     * @param visitor function for pool visit
     */
    void IterateOverFreePools(const std::function<void(size_t, MmapPool *)> &visitor);

    /**
     * To check if we can alloc enough pools from free pools
     * @param pools_num the number of pools we need
     * @param pool_size the size of the pool we need
     * @return true if we can make sure that we have enough space in free pools to alloc pools we need
     */
    bool HaveEnoughFreePools(size_t poolsNum, size_t poolSize) const;

private:
    std::map<void *, MmapPool *> poolMap_;
    std::multimap<size_t, MmapPool *> freePools_;
};

class MmapMemPool : public MemPool<MmapMemPool> {
public:
    NO_COPY_SEMANTIC(MmapMemPool);
    NO_MOVE_SEMANTIC(MmapMemPool);
    void ClearNonObjectMmapedPools();
    ~MmapMemPool() override;

    /**
     * Get min address in pool
     * @return min address in pool
     */
    uintptr_t GetMinObjectAddress() const
    {
        return minObjectMemoryAddr_;
    }

    /**
     * Get max address in pool
     * @return max address in pool
     */
    uintptr_t GetMaxObjectAddress() const
    {
        return minObjectMemoryAddr_ + mmapedObjectMemorySize_;
    }

    size_t GetTotalObjectSize() const
    {
        return mmapedObjectMemorySize_;
    }

    /**
     * Get start address of pool for input address in this pool
     * @param addr address in pool
     * @return start address of pool
     */
    void *GetStartAddrPoolForAddr(const void *addr) const
    {
        return GetStartAddrPoolForAddrImpl(addr);
    }

    size_t GetObjectSpaceFreeBytes() const;

    // To check if we can alloc enough pools in object space
    bool HaveEnoughPoolsInObjectSpace(size_t poolsNum, size_t poolSize) const;

    /// Release pages in all cached free pools
    void IterateOverFreePools();
    void ReleasePagesInFreePools();

    /// @return used bytes count in object space (so exclude bytes in free pools)
    size_t GetObjectUsedBytes() const;

private:
    template <class ArenaT = Arena, OSPagesAllocPolicy OS_ALLOC_POLICY>
    ArenaT *AllocArenaImpl(size_t size, SpaceType spaceType, AllocatorType allocatorType, const void *allocatorAddr);
    template <class ArenaT = Arena, OSPagesPolicy OS_PAGES_POLICY>
    void FreeArenaImpl(ArenaT *arena);

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    void *AllocRawMemImpl(size_t size, SpaceType type);
    void *AllocRawMemNonObjectImpl(size_t size, SpaceType type);
    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    void *AllocRawMemObjectImpl(size_t size, SpaceType type);
    void FreeRawMemImpl(void *mem, size_t size);

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Pool AllocPoolImpl(size_t size, SpaceType spaceType, AllocatorType allocatorType, const void *allocatorAddr);
    template <OSPagesPolicy OS_PAGES_POLICY>
    void FreePoolImpl(void *mem, size_t size);

    PANDA_PUBLIC_API AllocatorInfo GetAllocatorInfoForAddrImpl(const void *addr) const;
    PANDA_PUBLIC_API SpaceType GetSpaceTypeForAddrImpl(const void *addr) const;
    PANDA_PUBLIC_API void *GetStartAddrPoolForAddrImpl(const void *addr) const;

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Pool AllocPoolUnsafe(size_t size, SpaceType spaceType, AllocatorType allocatorType, const void *allocatorAddr);
    template <OSPagesPolicy OS_PAGES_POLICY>
    void FreePoolUnsafe(void *mem, size_t size);

    void AddToNonObjectPoolsMap(std::tuple<Pool, AllocatorInfo, SpaceType> poolInfo);
    void RemoveFromNonObjectPoolsMap(void *poolAddr);
    std::tuple<Pool, AllocatorInfo, SpaceType> FindAddrInNonObjectPoolsMap(const void *addr) const;

    MmapMemPool();

    // A super class for raw memory allocation for spaces.
    class SpaceMemory {
    public:
        void Initialize(uintptr_t minAddr, size_t maxSize)
        {
            minAddress_ = minAddr;
            maxSize_ = maxSize;
            curAllocOffset_ = 0U;
            unreturnedToOsSize_ = 0U;
        }

        uintptr_t GetMinAddress() const
        {
            return minAddress_;
        }

        size_t GetMaxSize() const
        {
            return maxSize_;
        }

        size_t GetOccupiedMemorySize() const
        {
            return curAllocOffset_;
        }

        inline size_t GetFreeSpace() const
        {
            ASSERT(maxSize_ >= curAllocOffset_);
            return maxSize_ - curAllocOffset_;
        }

        template <OSPagesAllocPolicy OS_ALLOC_POLICY>
        void *AllocRawMem(size_t size, MmapPoolMap *poolMap)
        {
            if (UNLIKELY(GetFreeSpace() < size)) {
                return nullptr;
            }
            void *mem = ToVoidPtr(minAddress_ + curAllocOffset_);
            curAllocOffset_ += size;
            size_t memoryToClear = 0;
            if (unreturnedToOsSize_ >= size) {
                unreturnedToOsSize_ -= size;
                memoryToClear = size;
            } else {
                memoryToClear = unreturnedToOsSize_;
                unreturnedToOsSize_ = 0;
            }
            if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY && memoryToClear != 0) {
                uintptr_t poolStart = ToUintPtr(mem);
                os::mem::ReleasePages(poolStart, poolStart + memoryToClear);
            }
            poolMap->AddNewPool(Pool(size, mem));
            return mem;
        }

        Pool GetAndClearUnreturnedToOSMemory()
        {
            void *mem = ToVoidPtr(minAddress_ + curAllocOffset_);
            size_t size = unreturnedToOsSize_;
            unreturnedToOsSize_ = 0;
            return Pool(size, mem);
        }

        void FreeMem(size_t size, OSPagesPolicy pagesPolicy)
        {
            ASSERT(curAllocOffset_ >= size);
            curAllocOffset_ -= size;
            if ((pagesPolicy == OSPagesPolicy::NO_RETURN) || (unreturnedToOsSize_ != 0)) {
                unreturnedToOsSize_ += size;
                ASSERT(unreturnedToOsSize_ <= maxSize_);
            }
        }

    private:
        uintptr_t minAddress_ {0U};       ///< Min address for the space
        size_t maxSize_ {0U};             ///< Max size in bytes for the space
        size_t curAllocOffset_ {0U};      ///< A value of occupied memory from the min_address_
        size_t unreturnedToOsSize_ {0U};  ///< A value of unreturned memory from the min_address_ + cur_alloc_offset_
    };

    uintptr_t minObjectMemoryAddr_ {0U};  ///< Minimal address of the mmaped object memory
    size_t mmapedObjectMemorySize_ {0U};  ///< Size of whole the mmaped object memory

    SpaceMemory commonSpace_;

    /// Pool map for object pools with all required information for quick search
    PoolMap poolMap_;

    MmapPoolMap commonSpacePools_;

    std::array<size_t, SPACE_TYPE_SIZE> nonObjectSpacesCurrentSize_;

    std::array<size_t, SPACE_TYPE_SIZE> nonObjectSpacesMaxSize_;

    /// Map for non object pools allocated via mmap
    std::map<const void *, std::tuple<Pool, AllocatorInfo, SpaceType>> nonObjectMmapedPools_;
    // AllocRawMem is called both from alloc and externally
    mutable os::memory::RecursiveMutex lock_;

    friend class PoolManager;
    friend class MemPool<MmapMemPool>;
    friend class MMapMemPoolTest;
    friend class mem::test::InternalAllocatorTest;
};

}  // namespace panda

#endif  // LIBPANDABASE_MEM_MMAP_MEM_POOL_H
