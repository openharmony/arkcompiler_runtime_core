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

#include "pool_map.h"

namespace panda {

void PoolMap::AddPoolToMap(const void *pool_addr, size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                           const void *allocator_addr)
{
    ASSERT((ToUintPtr(pool_addr) & POOL_MAP_GRANULARITY_MASK) == 0);
    ASSERT((pool_size & POOL_MAP_GRANULARITY_MASK) == 0);
    ASSERT(allocator_addr != nullptr);
    MapNumType first_map_num = AddrToMapNum(pool_addr);
    MapNumType last_map_num = AddrToMapNum(ToVoidPtr(ToUintPtr(pool_addr) + pool_size - 1U));
    pool_map_[first_map_num].Initialize(first_map_num, space_type, allocator_type, allocator_addr);
    for (MapNumType i = first_map_num + 1U; i <= last_map_num; i++) {
        pool_map_[i].Initialize(first_map_num, space_type, allocator_type, allocator_addr);
    }
}

void PoolMap::RemovePoolFromMap(const void *pool_addr, size_t pool_size)
{
    ASSERT((ToUintPtr(pool_addr) & POOL_MAP_GRANULARITY_MASK) == 0);
    ASSERT((pool_size & POOL_MAP_GRANULARITY_MASK) == 0);
    MapNumType first_map_num = AddrToMapNum(pool_addr);
    MapNumType last_map_num = AddrToMapNum(ToVoidPtr(ToUintPtr(pool_addr) + pool_size - 1U));
    for (MapNumType i = first_map_num; i <= last_map_num; i++) {
        pool_map_[i].Destroy();
    }
}

AllocatorInfo PoolMap::GetAllocatorInfo(const void *addr) const
{
    MapNumType map_num = AddrToMapNum(addr);
    AllocatorType allocator_type = pool_map_[map_num].GetAllocatorType();
    const void *allocator_addr = pool_map_[map_num].GetAllocatorAddr();
    // We can't get allocator info for not properly initialized pools
    ASSERT(allocator_type != AllocatorType::UNDEFINED);
    ASSERT(allocator_addr != nullptr);
    return AllocatorInfo(allocator_type, allocator_addr);
}

SpaceType PoolMap::GetSpaceType(const void *addr) const
{
    if (ToUintPtr(addr) > (POOL_MAP_COVERAGE - 1U)) {
        return SpaceType::SPACE_TYPE_UNDEFINED;
    }
    MapNumType map_num = AddrToMapNum(addr);
    SpaceType space_type = pool_map_[map_num].GetSpaceType();
    // We can't get space type for not properly initialized pools
    ASSERT(space_type != SpaceType::SPACE_TYPE_UNDEFINED);
    return space_type;
}

void *PoolMap::GetFirstByteOfPoolForAddr(const void *addr) const
{
    MapNumType segment_first_map_num = pool_map_[AddrToMapNum(addr)].GetSegmentFirstMapNum();
    return MapNumToAddr(segment_first_map_num);
}

bool PoolMap::IsEmpty() const
{
    for (auto i : pool_map_) {
        if (!i.IsEmpty()) {
            return false;
        }
    }
    return true;
}

}  // namespace panda
