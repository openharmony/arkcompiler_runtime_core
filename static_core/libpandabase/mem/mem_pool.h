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

#ifndef LIBPANDABASE_MEM_MEM_POOL_H
#define LIBPANDABASE_MEM_MEM_POOL_H

#include <cstddef>
#include "macros.h"
#include "mem.h"
#include "pool_map.h"

namespace panda {
class Arena;

enum class OSPagesPolicy : bool {
    NO_RETURN,
    IMMEDIATE_RETURN,
};

enum class OSPagesAllocPolicy : bool {
    NO_POLICY,
    ZEROED_MEMORY,
};

class Pool {
public:
    explicit constexpr Pool(size_t size, void *mem) : size_(size), mem_(mem) {}
    explicit Pool(std::pair<size_t, void *> pool) : size_(pool.first), mem_(pool.second) {}

    size_t GetSize() const
    {
        return size_;
    }

    void *GetMem()
    {
        return mem_;
    }

    bool operator==(const Pool &other) const
    {
        return (this->size_ == other.size_) && (this->mem_ == other.mem_);
    }

    bool operator!=(const Pool &other) const
    {
        return !(*this == other);
    }

    ~Pool() = default;

    DEFAULT_COPY_SEMANTIC(Pool);
    DEFAULT_MOVE_SEMANTIC(Pool);

private:
    size_t size_;
    void *mem_;
};

constexpr Pool NULLPOOL {0, nullptr};

template <class MemPoolImplT>
class MemPool {
public:
    virtual ~MemPool() = default;
    explicit MemPool(std::string pool_name) : name_(std::move(pool_name)) {}
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(MemPool);
    DEFAULT_COPY_SEMANTIC(MemPool);

    /**
     * Allocates arena with size bytes
     * @tparam ArenaT - type of Arena
     * @tparam OS_ALLOC_POLICY - allocated memory policy
     * @param size - size of buffer in arena in bytes
     * @param space_type - type of the space which arena allocated for
     * @param allocator_type - type of the allocator which arena allocated for
     * @param allocator_addr - address of the allocator header.
     * @return pointer to allocated arena
     */
    // TODO(aemelenko): We must always define allocator_addr for AllocArena
    // because we set up arena at the first bytes of the pool
    template <class ArenaT = Arena, OSPagesAllocPolicy OS_ALLOC_POLICY = OSPagesAllocPolicy::NO_POLICY>
    inline ArenaT *AllocArena(size_t size, SpaceType space_type, AllocatorType allocator_type,
                              const void *allocator_addr = nullptr)
    {
        auto *allocated_arena = static_cast<MemPoolImplT *>(this)->template AllocArenaImpl<ArenaT, OS_ALLOC_POLICY>(
            size, space_type, allocator_type, allocator_addr);
        if constexpr (IS_ZERO_CHECK_ENABLED && OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY) {
            if (allocated_arena != nullptr) {
                CheckZeroedMemory(allocated_arena->GetMem(), allocated_arena->GetSize());
            }
        }
        return allocated_arena;
    }

    /**
     * Frees allocated arena
     * @tparam ArenaT - arena type
     * @tparam OS_PAGES_POLICY - OS return pages policy
     * @param arena - pointer to the arena
     */
    template <class ArenaT = Arena, OSPagesPolicy OS_PAGES_POLICY = OSPagesPolicy::IMMEDIATE_RETURN>
    inline void FreeArena(ArenaT *arena)
    {
        static_cast<MemPoolImplT *>(this)->template FreeArenaImpl<ArenaT, OS_PAGES_POLICY>(arena);
    }

    /**
     * Allocates pool with at least size bytes
     * @tparam OS_ALLOC_POLICY - allocated memory policy
     * @param size - minimal size of a pool in bytes
     * @param space_type - type of the space which pool allocated for
     * @param allocator_type - type of the allocator which arena allocated for
     * @param allocator_addr - address of the allocator header.
     *  If it is not defined, it means that allocator header will be located at the first bytes of the returned pool.
     * @return pool info with the size and a pointer
     */
    template <OSPagesAllocPolicy OS_ALLOC_POLICY = OSPagesAllocPolicy::NO_POLICY>
    Pool AllocPool(size_t size, SpaceType space_type, AllocatorType allocator_type,
                   const void *allocator_addr = nullptr)
    {
        Pool allocated_pool = static_cast<MemPoolImplT *>(this)->template AllocPoolImpl<OS_ALLOC_POLICY>(
            size, space_type, allocator_type, allocator_addr);
        if constexpr (IS_ZERO_CHECK_ENABLED && OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY) {
            if (allocated_pool.GetMem() != nullptr) {
                CheckZeroedMemory(allocated_pool.GetMem(), allocated_pool.GetSize());
            }
        }
        return allocated_pool;
    }

    /**
     * Frees allocated pool
     * @tparam OS_PAGES_POLICY - OS return pages policy
     * @param mem - pointer to an allocated pool
     * @param size - size of the allocated pool in bytes
     */
    template <OSPagesPolicy OS_PAGES_POLICY = OSPagesPolicy::IMMEDIATE_RETURN>
    void FreePool(void *mem, size_t size)
    {
        static_cast<MemPoolImplT *>(this)->template FreePoolImpl<OS_PAGES_POLICY>(mem, size);
    }

    /**
     * Get info about the allocator in which this address is used
     * @param addr
     * @return Allocator info with a type and pointer to the allocator header
     */
    AllocatorInfo GetAllocatorInfoForAddr(const void *addr) const
    {
        return static_cast<const MemPoolImplT *>(this)->GetAllocatorInfoForAddrImpl(addr);
    }

    /**
     * Get space type which this address used for
     * @param addr
     * @return space type
     */
    SpaceType GetSpaceTypeForAddr(const void *addr) const
    {
        return static_cast<const MemPoolImplT *>(this)->GetSpaceTypeForAddrImpl(addr);
    }

    /**
     * Get address of pool start for input address
     * @param addr address in pool
     * @return address of pool start
     */
    void *GetStartAddrPoolForAddr(const void *addr) const
    {
        return static_cast<const MemPoolImplT *>(this)->GetStartAddrPoolForAddrImpl(addr);
    }

private:
#ifndef NDEBUG
    static constexpr bool IS_ZERO_CHECK_ENABLED = true;
#else
    static constexpr bool IS_ZERO_CHECK_ENABLED = false;
#endif

    inline void CheckZeroedMemory(void *mem, size_t size)
    {
        // Check that the memory is zeroed:
        [[maybe_unused]] auto iterator64 = static_cast<uint64_t *>(mem);
        size_t it64_end = size / sizeof(uint64_t);
        [[maybe_unused]] uintptr_t end_memory = ToUintPtr(mem) + size;
        // check by 64 byte
        for (size_t i = 0; i < it64_end; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ASSERT(ToUintPtr(&iterator64[i]) < end_memory);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ASSERT(iterator64[i] == 0);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        [[maybe_unused]] auto iterator8 = reinterpret_cast<uint8_t *>(&iterator64[it64_end]);
        size_t it8_end = size % sizeof(uint64_t);
        // check by byte
        for (size_t i = 0; i < it8_end; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ASSERT(ToUintPtr(&iterator8[i]) < end_memory);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ASSERT(iterator8[i] == 0);
        }
    }

    std::string name_;
};

}  // namespace panda

#endif  // LIBPANDABASE_MEM_MEM_POOL_H
