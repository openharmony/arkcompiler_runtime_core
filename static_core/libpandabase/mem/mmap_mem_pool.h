/**
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
    explicit MmapPool(Pool pool, FreePoolsIter free_pools_iter, bool returned_to_os = true)
        : pool_(pool), returned_to_os_(returned_to_os), free_pools_iter_(free_pools_iter)
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
        return returned_to_os_;
    }

    void SetReturnedToOS(bool value)
    {
        returned_to_os_ = value;
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
    bool IsUsed(FreePoolsIter end_iter)
    {
        return free_pools_iter_ == end_iter;
    }

    FreePoolsIter GetFreePoolsIter()
    {
        return free_pools_iter_;
    }

    void SetFreePoolsIter(FreePoolsIter free_pools_iter)
    {
        free_pools_iter_ = free_pools_iter;
    }

private:
    Pool pool_;
    bool returned_to_os_;
    // record the iterator of the pool in the multimap
    FreePoolsIter free_pools_iter_;
};

class MmapPoolMap {
public:
    MmapPoolMap() = default;

    ~MmapPoolMap()
    {
        for (auto &pool : pool_map_) {
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
    bool HaveEnoughFreePools(size_t pools_num, size_t pool_size) const;

private:
    std::map<void *, MmapPool *> pool_map_;
    std::multimap<size_t, MmapPool *> free_pools_;
};

class MmapMemPool : public MemPool<MmapMemPool> {
public:
    NO_COPY_SEMANTIC(MmapMemPool);
    NO_MOVE_SEMANTIC(MmapMemPool);
    ~MmapMemPool() override;

    /**
     * Get min address in pool
     * @return min address in pool
     */
    uintptr_t GetMinObjectAddress() const
    {
        return min_object_memory_addr_;
    }

    /**
     * Get max address in pool
     * @return max address in pool
     */
    uintptr_t GetMaxObjectAddress() const
    {
        return min_object_memory_addr_ + mmaped_object_memory_size_;
    }

    size_t GetTotalObjectSize() const
    {
        return mmaped_object_memory_size_;
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
    bool HaveEnoughPoolsInObjectSpace(size_t pools_num, size_t pool_size) const;

    /// Release pages in all cached free pools
    void ReleasePagesInFreePools();

    /// @return used bytes count in object space (so exclude bytes in free pools)
    size_t GetObjectUsedBytes() const;

private:
    template <class ArenaT = Arena, OSPagesAllocPolicy OS_ALLOC_POLICY>
    ArenaT *AllocArenaImpl(size_t size, SpaceType space_type, AllocatorType allocator_type, const void *allocator_addr);
    template <class ArenaT = Arena, OSPagesPolicy OS_PAGES_POLICY>
    void FreeArenaImpl(ArenaT *arena);

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    void *AllocRawMemImpl(size_t size, SpaceType type);
    void *AllocRawMemNonObjectImpl(size_t size, SpaceType type);
    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    void *AllocRawMemObjectImpl(size_t size, SpaceType type);
    void FreeRawMemImpl(void *mem, size_t size);

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Pool AllocPoolImpl(size_t size, SpaceType space_type, AllocatorType allocator_type, const void *allocator_addr);
    template <OSPagesPolicy OS_PAGES_POLICY>
    void FreePoolImpl(void *mem, size_t size);

    PANDA_PUBLIC_API AllocatorInfo GetAllocatorInfoForAddrImpl(const void *addr) const;
    PANDA_PUBLIC_API SpaceType GetSpaceTypeForAddrImpl(const void *addr) const;
    PANDA_PUBLIC_API void *GetStartAddrPoolForAddrImpl(const void *addr) const;

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Pool AllocPoolUnsafe(size_t size, SpaceType space_type, AllocatorType allocator_type, const void *allocator_addr);
    template <OSPagesPolicy OS_PAGES_POLICY>
    void FreePoolUnsafe(void *mem, size_t size);

    void AddToNonObjectPoolsMap(std::tuple<Pool, AllocatorInfo, SpaceType> pool_info);
    void RemoveFromNonObjectPoolsMap(void *pool_addr);
    std::tuple<Pool, AllocatorInfo, SpaceType> FindAddrInNonObjectPoolsMap(const void *addr) const;

    MmapMemPool();

    // A super class for raw memory allocation for spaces.
    class SpaceMemory {
    public:
        void Initialize(uintptr_t min_addr, size_t max_size)
        {
            min_address_ = min_addr;
            max_size_ = max_size;
            cur_alloc_offset_ = 0U;
            unreturned_to_os_size_ = 0U;
        }

        uintptr_t GetMinAddress() const
        {
            return min_address_;
        }

        size_t GetMaxSize() const
        {
            return max_size_;
        }

        size_t GetOccupiedMemorySize() const
        {
            return cur_alloc_offset_;
        }

        inline size_t GetFreeSpace() const
        {
            ASSERT(max_size_ >= cur_alloc_offset_);
            return max_size_ - cur_alloc_offset_;
        }

        template <OSPagesAllocPolicy OS_ALLOC_POLICY>
        void *AllocRawMem(size_t size, MmapPoolMap *pool_map)
        {
            if (UNLIKELY(GetFreeSpace() < size)) {
                return nullptr;
            }
            void *mem = ToVoidPtr(min_address_ + cur_alloc_offset_);
            cur_alloc_offset_ += size;
            size_t memory_to_clear = 0;
            if (unreturned_to_os_size_ >= size) {
                unreturned_to_os_size_ -= size;
                memory_to_clear = size;
            } else {
                memory_to_clear = unreturned_to_os_size_;
                unreturned_to_os_size_ = 0;
            }
            if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY && memory_to_clear != 0) {
                uintptr_t pool_start = ToUintPtr(mem);
                os::mem::ReleasePages(pool_start, pool_start + memory_to_clear);
            }
            pool_map->AddNewPool(Pool(size, mem));
            return mem;
        }

        Pool GetAndClearUnreturnedToOSMemory()
        {
            void *mem = ToVoidPtr(min_address_ + cur_alloc_offset_);
            size_t size = unreturned_to_os_size_;
            unreturned_to_os_size_ = 0;
            return Pool(size, mem);
        }

        void FreeMem(size_t size, OSPagesPolicy pages_policy)
        {
            ASSERT(cur_alloc_offset_ >= size);
            cur_alloc_offset_ -= size;
            if ((pages_policy == OSPagesPolicy::NO_RETURN) || (unreturned_to_os_size_ != 0)) {
                unreturned_to_os_size_ += size;
                ASSERT(unreturned_to_os_size_ <= max_size_);
            }
        }

    private:
        uintptr_t min_address_ {0U};         ///< Min address for the space
        size_t max_size_ {0U};               ///< Max size in bytes for the space
        size_t cur_alloc_offset_ {0U};       ///< A value of occupied memory from the min_address_
        size_t unreturned_to_os_size_ {0U};  ///< A value of unreturned memory from the min_address_ + cur_alloc_offset_
    };

    uintptr_t min_object_memory_addr_ {0U};  ///< Minimal address of the mmaped object memory
    size_t mmaped_object_memory_size_ {0U};  ///< Size of whole the mmaped object memory

    SpaceMemory common_space_;

    /// Pool map for object pools with all required information for quick search
    PoolMap pool_map_;

    MmapPoolMap common_space_pools_;

    std::array<size_t, SPACE_TYPE_SIZE> non_object_spaces_current_size_;

    std::array<size_t, SPACE_TYPE_SIZE> non_object_spaces_max_size_;

    /// Map for non object pools allocated via mmap
    std::map<const void *, std::tuple<Pool, AllocatorInfo, SpaceType>> non_object_mmaped_pools_;
    // AllocRawMem is called both from alloc and externally
    mutable os::memory::RecursiveMutex lock_;

    friend class PoolManager;
    friend class MemPool<MmapMemPool>;
    friend class MMapMemPoolTest;
    friend class mem::test::InternalAllocatorTest;
};

}  // namespace panda

#endif  // LIBPANDABASE_MEM_MMAP_MEM_POOL_H
