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
#ifndef PANDA_RUNTIME_MEM_REGION_SPACE_INL_H
#define PANDA_RUNTIME_MEM_REGION_SPACE_INL_H

#include "runtime/mem/region_space.h"
#include "libpandabase/utils/asan_interface.h"

namespace panda::mem {

class RegionAllocCheck {
public:
    explicit RegionAllocCheck(Region *region) : region_(region)
    {
        ASSERT(region_->SetAllocating(true));
    }
    ~RegionAllocCheck()
    {
        ASSERT(region_->SetAllocating(false));
    }
    NO_COPY_SEMANTIC(RegionAllocCheck);
    NO_MOVE_SEMANTIC(RegionAllocCheck);

private:
    Region *region_ FIELD_UNUSED;
};

class RegionIterateCheck {
public:
    explicit RegionIterateCheck(Region *region) : region_(region)
    {
        ASSERT(region_->SetIterating(true));
    }
    ~RegionIterateCheck()
    {
        ASSERT(region_->SetIterating(false));
    }
    NO_COPY_SEMANTIC(RegionIterateCheck);
    NO_MOVE_SEMANTIC(RegionIterateCheck);

private:
    Region *region_ FIELD_UNUSED;
};

template <bool ATOMIC>
void *Region::Alloc(size_t aligned_size)
{
    RegionAllocCheck alloc(this);
    ASSERT(AlignUp(aligned_size, DEFAULT_ALIGNMENT_IN_BYTES) == aligned_size);
    ASSERT(!IsTLAB());
    uintptr_t old_top;
    uintptr_t new_top;
    if (ATOMIC) {
        auto atomic_top = reinterpret_cast<std::atomic<uintptr_t> *>(&top_);
        // Atomic with relaxed order reason: data race with top_ with no synchronization or ordering constraints imposed
        // on other reads or writes
        old_top = atomic_top->load(std::memory_order_relaxed);
        do {
            new_top = old_top + aligned_size;
            if (UNLIKELY(new_top > end_)) {
                return nullptr;
            }
        } while (!atomic_top->compare_exchange_weak(old_top, new_top, std::memory_order_relaxed));
        ASAN_UNPOISON_MEMORY_REGION(ToVoidPtr(old_top), aligned_size);
        return ToVoidPtr(old_top);
    }
    new_top = top_ + aligned_size;
    if (UNLIKELY(new_top > end_)) {
        return nullptr;
    }
    old_top = top_;
    top_ = new_top;

    ASAN_UNPOISON_MEMORY_REGION(ToVoidPtr(old_top), aligned_size);
    return ToVoidPtr(old_top);
}

template <typename ObjectVisitor>
void Region::IterateOverObjects(const ObjectVisitor &visitor)
{
    // This method doesn't work for nonmovable regions
    ASSERT(!HasFlag(RegionFlag::IS_NONMOVABLE));
    // currently just for gc stw phase, so check it is not in allocating state
    RegionIterateCheck iterate(this);
    if (!IsTLAB()) {
        auto cur_ptr = Begin();
        auto end_ptr = Top();
        while (cur_ptr < end_ptr) {
            auto object_header = reinterpret_cast<ObjectHeader *>(cur_ptr);
            size_t object_size = GetObjectSize(object_header);
            visitor(object_header);
            cur_ptr = AlignUp(cur_ptr + object_size, DEFAULT_ALIGNMENT_IN_BYTES);
        }
    } else {
        for (auto i : *tlab_vector_) {
            i->IterateOverObjects(visitor);
        }
    }
}

template <OSPagesPolicy OS_PAGES_POLICY>
void RegionPool::FreeRegion(Region *region)
{
    bool release_pages = OS_PAGES_POLICY == OSPagesPolicy::IMMEDIATE_RETURN;
    if (block_.IsAddrInRange(region)) {
        region->IsYoung() ? spaces_->ReduceYoungOccupiedInSharedPool(region->Size())
                          : spaces_->ReduceTenuredOccupiedInSharedPool(region->Size());
        block_.FreeRegion(region, release_pages);
    } else {
        region->IsYoung() ? spaces_->FreeYoungPool(region, region->Size(), release_pages)
                          : spaces_->FreeTenuredPool(region, region->Size(), release_pages);
    }
}

template <RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY, OSPagesPolicy OS_PAGES_POLICY>
void RegionSpace::FreeRegion(Region *region)
{
    ASSERT(region->GetSpace() == this);
    ASAN_POISON_MEMORY_REGION(ToVoidPtr(region->Begin()), region->End() - region->Begin());
    regions_.erase(region->AsListNode());
    if (region->IsYoung()) {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        ASSERT(young_regions_in_use_.fetch_sub(1, std::memory_order_relaxed) > 0);
    }
    region->Destroy();
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (REGIONS_RELEASE_POLICY == ReleaseRegionsPolicy::NoRelease) {
        if (region->IsYoung()) {
            // unlimited
            empty_young_regions_.push_back(region->AsListNode());
            return;
        }
        if (region->HasFlag(RegionFlag::IS_OLD) && (empty_tenured_regions_.size() < empty_tenured_regions_max_count_)) {
            empty_tenured_regions_.push_back(region->AsListNode());
            return;
        }
    }
    region_pool_->FreeRegion<OS_PAGES_POLICY>(region);
}

template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
void RegionSpace::ReleaseEmptyRegions()
{
    auto visitor = [this](Region *region) { region_pool_->FreeRegion<OS_PAGES_POLICY>(region); };
    if (IsYoungRegionFlag(REGION_TYPE)) {
        IterateRegionsList(empty_young_regions_, visitor);
        empty_young_regions_.clear();
    } else {
        IterateRegionsList(empty_tenured_regions_, visitor);
        empty_tenured_regions_.clear();
    }
}

template <typename RegionVisitor>
void RegionSpace::IterateRegions(RegionVisitor visitor)
{
    IterateRegionsList(regions_, visitor);
}

template <typename RegionVisitor>
void RegionSpace::IterateRegionsList(DList &regions_list, RegionVisitor visitor)
{
    auto it = regions_list.begin();
    while (it != regions_list.end()) {
        auto *region = Region::AsRegion(&(*it));
        ++it;  // increase before visitor which may remove region
        visitor(region);
    }
}

template <bool CROSS_REGION>
bool RegionSpace::ContainObject(const ObjectHeader *object) const
{
    return GetRegion<CROSS_REGION>(object) != nullptr;
}

template <bool CROSS_REGION>
bool RegionSpace::IsLive(const ObjectHeader *object) const
{
    auto *region = GetRegion<CROSS_REGION>(object);

    // check if the object is live in the range
    return region != nullptr && region->IsInAllocRange(object);
}

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_REGION_SPACE_INL_H
