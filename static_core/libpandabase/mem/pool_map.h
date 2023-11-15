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

#ifndef LIBPANDABASE_MEM_POOL_MAP_H
#define LIBPANDABASE_MEM_POOL_MAP_H

#include <cstddef>
#include <array>
#include "macros.h"
#include "mem.h"
#include "space.h"

WEAK_FOR_LTO_START

namespace panda {

enum class AllocatorType : uint16_t {
    UNDEFINED,
    RUNSLOTS_ALLOCATOR,
    FREELIST_ALLOCATOR,
    HUMONGOUS_ALLOCATOR,
    ARENA_ALLOCATOR,
    TLAB_ALLOCATOR,
    BUMP_ALLOCATOR,
    REGION_ALLOCATOR,
    FRAME_ALLOCATOR,
    STACK_LIKE_ALLOCATOR,
    BUMP_ALLOCATOR_WITH_TLABS,
    NATIVE_STACKS_ALLOCATOR,
};

class AllocatorInfo {
public:
    explicit constexpr AllocatorInfo(AllocatorType type, const void *addr) : type_(type), header_addr_(addr)
    {
        // We can't create AllocatorInfo without correct pointer to the allocator header
        ASSERT(header_addr_ != nullptr);
    }

    AllocatorType GetType() const
    {
        return type_;
    }

    const void *GetAllocatorHeaderAddr() const
    {
        return header_addr_;
    }

    virtual ~AllocatorInfo() = default;

    DEFAULT_COPY_SEMANTIC(AllocatorInfo);
    DEFAULT_MOVE_SEMANTIC(AllocatorInfo);

private:
    AllocatorType type_;
    const void *header_addr_;
};

// PoolMap is used to manage all pools which has been given to the allocators.
// It can be used to find which allocator has been used to allocate an object.
class PoolMap {
public:
    PANDA_PUBLIC_API void AddPoolToMap(const void *pool_addr, size_t pool_size, SpaceType space_type,
                                       AllocatorType allocator_type, const void *allocator_addr);
    PANDA_PUBLIC_API void RemovePoolFromMap(const void *pool_addr, size_t pool_size);
    // Get Allocator info for the object allocated at this address.
    PANDA_PUBLIC_API AllocatorInfo GetAllocatorInfo(const void *addr) const;

    PANDA_PUBLIC_API void *GetFirstByteOfPoolForAddr(const void *addr) const;

    PANDA_PUBLIC_API SpaceType GetSpaceType(const void *addr) const;

    PANDA_PUBLIC_API bool IsEmpty() const;

private:
    static constexpr uint64_t POOL_MAP_COVERAGE = PANDA_MAX_HEAP_SIZE;
    static constexpr size_t POOL_MAP_GRANULARITY_SHIFT = 18;
    static constexpr size_t POOL_MAP_GRANULARITY_MASK = (1U << POOL_MAP_GRANULARITY_SHIFT) - 1U;
    static_assert(PANDA_POOL_ALIGNMENT_IN_BYTES == 1U << POOL_MAP_GRANULARITY_SHIFT);
    static constexpr size_t POOL_MAP_SIZE = POOL_MAP_COVERAGE >> POOL_MAP_GRANULARITY_SHIFT;

    static constexpr bool FIRST_BYTE_IN_SEGMENT_VALUE = true;

    using MapNumType = size_t;

    class PoolInfo {
    public:
        void Initialize(MapNumType segment_first_map_num, SpaceType space_type, AllocatorType allocator_type,
                        const void *allocator_addr)
        {
            ASSERT(allocator_type_ == AllocatorType::UNDEFINED);
            // Added a TSAN ignore here because TSAN thinks
            // that we can have a data race here - concurrent
            // initialization and reading.
            // However, we can't get an access for this fields
            // without initialization in the correct flow.
            TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
            segment_first_map_num_ = segment_first_map_num;
            allocator_addr_ = allocator_addr;
            space_type_ = space_type;
            allocator_type_ = allocator_type;
            TSAN_ANNOTATE_IGNORE_WRITES_END();
        }

        inline bool IsEmpty() const
        {
            return space_type_ == SpaceType::SPACE_TYPE_UNDEFINED;
        }

        void Destroy()
        {
            segment_first_map_num_ = 0;
            allocator_addr_ = nullptr;
            allocator_type_ = AllocatorType::UNDEFINED;
            space_type_ = SpaceType::SPACE_TYPE_UNDEFINED;
        }

        MapNumType GetSegmentFirstMapNum() const
        {
            ASSERT(space_type_ != SpaceType::SPACE_TYPE_UNDEFINED);
            return segment_first_map_num_;
        }

        AllocatorType GetAllocatorType() const
        {
            return allocator_type_;
        }

        const void *GetAllocatorAddr() const
        {
            return allocator_addr_;
        }

        SpaceType GetSpaceType() const
        {
            return space_type_;
        }

    private:
        AllocatorType allocator_type_ {AllocatorType::UNDEFINED};
        SpaceType space_type_ {SpaceType::SPACE_TYPE_UNDEFINED};
        MapNumType segment_first_map_num_ {0};
        const void *allocator_addr_ = nullptr;
    };

    static MapNumType AddrToMapNum(const void *addr)
    {
        MapNumType map_num = ToUintPtr(addr) >> POOL_MAP_GRANULARITY_SHIFT;
        ASSERT(map_num < POOL_MAP_SIZE);
        return map_num;
    }

    static void *MapNumToAddr(MapNumType map_num)
    {
        // Checking overflow
        ASSERT(static_cast<uint64_t>(map_num) << POOL_MAP_GRANULARITY_SHIFT == map_num << POOL_MAP_GRANULARITY_SHIFT);
        return ToVoidPtr(map_num << POOL_MAP_GRANULARITY_SHIFT);
    }

    std::array<PoolInfo, POOL_MAP_SIZE> pool_map_;

    friend class PoolMapTest;
};

}  // namespace panda

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_MEM_POOL_MAP_H
