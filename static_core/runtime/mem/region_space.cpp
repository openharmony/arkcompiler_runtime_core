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
#include "mem/mem_pool.h"
#include "runtime/mem/region_space-inl.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace panda::mem {

uint32_t Region::GetAllocatedBytes() const
{
    if (!IsTLAB()) {
        return top_ - begin_;
    }
    uint32_t allocated_bytes = 0;
    ASSERT(tlab_vector_ != nullptr);
    for (auto i : *tlab_vector_) {
        allocated_bytes += i->GetOccupiedSize();
    }
    return allocated_bytes;
}

double Region::GetFragmentation() const
{
    ASSERT(Size() >= GetAllocatedBytes());
    return static_cast<double>(Size() - GetAllocatedBytes()) / Size();
}

InternalAllocatorPtr Region::GetInternalAllocator()
{
    return space_->GetPool()->GetInternalAllocator();
}

void Region::CreateRemSet()
{
    ASSERT(rem_set_ == nullptr);
    rem_set_ = GetInternalAllocator()->New<RemSetT>();
}

void Region::SetupAtomics()
{
    live_bytes_ = GetInternalAllocator()->New<std::atomic<uint32_t>>();
    pinned_objects_ = GetInternalAllocator()->New<std::atomic<uint32_t>>();
}

void Region::CreateTLABSupport()
{
    ASSERT(tlab_vector_ == nullptr);
    tlab_vector_ = GetInternalAllocator()->New<PandaVector<TLAB *>>(GetInternalAllocator()->Adapter());
}

size_t Region::GetRemainingSizeForTLABs() const
{
    ASSERT(IsTLAB());
    // TLABs are stored one by one.
    uintptr_t last_tlab_end_byte = tlab_vector_->empty() ? Top() : ToUintPtr(tlab_vector_->back()->GetEndAddr());
    ASSERT((last_tlab_end_byte <= End()) && (last_tlab_end_byte >= Top()));
    return End() - last_tlab_end_byte;
}

TLAB *Region::CreateTLAB(size_t size)
{
    ASSERT(IsTLAB());
    ASSERT(Begin() != 0);
    ASSERT(Top() == Begin());
    size_t remaining_size = GetRemainingSizeForTLABs();
    if (remaining_size < size) {
        return nullptr;
    }
    ASSERT(End() > remaining_size);
    TLAB *tlab = GetInternalAllocator()->New<TLAB>(ToVoidPtr(End() - remaining_size), size);
    tlab_vector_->push_back(tlab);
    return tlab;
}

MarkBitmap *Region::CreateMarkBitmap()
{
    ASSERT(mark_bitmap_ == nullptr);
    auto allocator = GetInternalAllocator();
    auto bitmap_data = allocator->Alloc(MarkBitmap::GetBitMapSizeInByte(Size()));
    ASSERT(bitmap_data != nullptr);
    mark_bitmap_ = allocator->New<MarkBitmap>(this, Size(), bitmap_data);
    ASSERT(mark_bitmap_ != nullptr);
    mark_bitmap_->ClearAllBits();
    return mark_bitmap_;
}

MarkBitmap *Region::CreateLiveBitmap()
{
    ASSERT(live_bitmap_ == nullptr);
    auto allocator = GetInternalAllocator();
    auto bitmap_data = allocator->Alloc(MarkBitmap::GetBitMapSizeInByte(Size()));
    ASSERT(bitmap_data != nullptr);
    live_bitmap_ = allocator->New<MarkBitmap>(this, Size(), bitmap_data);
    ASSERT(live_bitmap_ != nullptr);
    live_bitmap_->ClearAllBits();
    return live_bitmap_;
}

void Region::SetMarkBit(ObjectHeader *object)
{
    ASSERT(IsInRange(object));
    mark_bitmap_->Set(object);
}

uint32_t Region::CalcLiveBytes() const
{
    ASSERT(live_bitmap_ != nullptr);
    uint32_t live_bytes = 0;
    live_bitmap_->IterateOverMarkedChunks<true>(
        [&live_bytes](const void *object) { live_bytes += GetAlignedObjectSize(GetObjectSize(object)); });
    return live_bytes;
}

uint32_t Region::CalcMarkBytes() const
{
    ASSERT(mark_bitmap_ != nullptr);
    uint32_t live_bytes = 0;
    mark_bitmap_->IterateOverMarkedChunks(
        [&live_bytes](const void *object) { live_bytes += GetAlignedObjectSize(GetObjectSize(object)); });
    return live_bytes;
}

void Region::Destroy()
{
    auto allocator = GetInternalAllocator();
    if (rem_set_ != nullptr) {
        allocator->Delete(rem_set_);
        rem_set_ = nullptr;
    }
    if (live_bytes_ != nullptr) {
        allocator->Delete(live_bytes_);
        live_bytes_ = nullptr;
    }
    if (pinned_objects_ != nullptr) {
        allocator->Delete(pinned_objects_);
        pinned_objects_ = nullptr;
    }
    if (tlab_vector_ != nullptr) {
        for (auto i : *tlab_vector_) {
            allocator->Delete(i);
        }
        allocator->Delete(tlab_vector_);
        tlab_vector_ = nullptr;
    }
    if (live_bitmap_ != nullptr) {
        allocator->Delete(live_bitmap_->GetBitMap().data());
        allocator->Delete(live_bitmap_);
        live_bitmap_ = nullptr;
    }
    if (mark_bitmap_ != nullptr) {
        allocator->Delete(mark_bitmap_->GetBitMap().data());
        allocator->Delete(mark_bitmap_);
        mark_bitmap_ = nullptr;
    }
}

void RegionBlock::Init(uintptr_t regions_begin, uintptr_t regions_end)
{
    os::memory::LockHolder lock(lock_);
    ASSERT(occupied_.Empty());
    ASSERT(Region::IsAlignment(regions_begin, region_size_));
    ASSERT((regions_end - regions_begin) % region_size_ == 0);
    size_t num_regions = (regions_end - regions_begin) / region_size_;
    if (num_regions > 0) {
        size_t size = num_regions * sizeof(Region *);
        auto data = reinterpret_cast<Region **>(allocator_->Alloc(size));
        memset_s(data, size, 0, size);
        occupied_ = Span<Region *>(data, num_regions);
        regions_begin_ = regions_begin;
        regions_end_ = regions_end;
    }
}

Region *RegionBlock::AllocRegion()
{
    os::memory::LockHolder lock(lock_);
    // NOTE(yxr) : find a unused region, improve it
    for (size_t i = 0; i < occupied_.Size(); ++i) {
        if (occupied_[i] == nullptr) {
            auto *region = RegionAt(i);
            occupied_[i] = region;
            num_used_regions_++;
            return region;
        }
    }
    return nullptr;
}

Region *RegionBlock::AllocLargeRegion(size_t large_region_size)
{
    os::memory::LockHolder lock(lock_);
    // NOTE(yxr) : search continuous unused regions, improve it
    size_t alloc_region_num = large_region_size / region_size_;
    size_t left = 0;
    while (left + alloc_region_num <= occupied_.Size()) {
        bool found = true;
        size_t right = left;
        while (right < left + alloc_region_num) {
            if (occupied_[right] != nullptr) {
                found = false;
                break;
            }
            ++right;
        }
        if (found) {
            // mark those regions as 'used'
            auto *region = RegionAt(left);
            for (size_t i = 0; i < alloc_region_num; i++) {
                occupied_[left + i] = region;
            }
            num_used_regions_ += alloc_region_num;
            return region;
        }
        // next round
        left = right + 1;
    }
    return nullptr;
}

void RegionBlock::FreeRegion(Region *region, bool release_pages)
{
    os::memory::LockHolder lock(lock_);
    size_t region_idx = RegionIndex(region);
    size_t region_num = region->Size() / region_size_;
    ASSERT(region_idx + region_num <= occupied_.Size());
    for (size_t i = 0; i < region_num; i++) {
        ASSERT(occupied_[region_idx + i] == region);
        occupied_[region_idx + i] = nullptr;
    }
    num_used_regions_ -= region_num;
    if (release_pages) {
        os::mem::ReleasePages(ToUintPtr(region), region->End());
    }
}

Region *RegionPool::NewRegion(RegionSpace *space, SpaceType space_type, AllocatorType allocator_type,
                              size_t region_size, RegionFlag eden_or_old_or_nonmovable, RegionFlag properties,
                              OSPagesAllocPolicy alloc_policy)
{
    // check that the input region_size is aligned
    ASSERT(region_size % region_size_ == 0);
    ASSERT(IsYoungRegionFlag(eden_or_old_or_nonmovable) || eden_or_old_or_nonmovable == RegionFlag::IS_OLD ||
           eden_or_old_or_nonmovable == RegionFlag::IS_NONMOVABLE);

    // Ensure leaving enough space so there's always some free regions in heap which we can use for full gc
    if (eden_or_old_or_nonmovable == RegionFlag::IS_NONMOVABLE || region_size > region_size_) {
        if (!spaces_->CanAllocInSpace(false, region_size + region_size_)) {
            return nullptr;
        }
    }

    if (!spaces_->CanAllocInSpace(IsYoungRegionFlag(eden_or_old_or_nonmovable), region_size)) {
        return nullptr;
    }

    // 1.get region from pre-allocated region block(e.g. a big mmaped continuous space)
    void *region = nullptr;
    if (block_.GetFreeRegionsNum() > 0) {
        region = (region_size <= region_size_) ? block_.AllocRegion() : block_.AllocLargeRegion(region_size);
    }
    if (region != nullptr) {
        IsYoungRegionFlag(eden_or_old_or_nonmovable) ? spaces_->IncreaseYoungOccupiedInSharedPool(region_size)
                                                     : spaces_->IncreaseTenuredOccupiedInSharedPool(region_size);
    } else if (extend_) {  // 2.mmap region directly, this is more flexible for memory usage
        region =
            IsYoungRegionFlag(eden_or_old_or_nonmovable)
                ? spaces_->TryAllocPoolForYoung(region_size, space_type, allocator_type, this).GetMem()
                : spaces_->TryAllocPoolForTenured(region_size, space_type, allocator_type, this, alloc_policy).GetMem();
    }

    if (UNLIKELY(region == nullptr)) {
        return nullptr;
    }
    return NewRegion(region, space, region_size, eden_or_old_or_nonmovable, properties);
}

Region *RegionPool::NewRegion(void *region, RegionSpace *space, size_t region_size,
                              RegionFlag eden_or_old_or_nonmovable, RegionFlag properties)
{
    ASSERT(Region::IsAlignment(ToUintPtr(region), region_size_));

    ASAN_UNPOISON_MEMORY_REGION(region, Region::HeadSize());
    auto *ret = new (region) Region(space, ToUintPtr(region) + Region::HeadSize(), ToUintPtr(region) + region_size);
    // NOTE(dtrubenkov): remove this fast fixup
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    ret->AddFlag(eden_or_old_or_nonmovable);
    ret->AddFlag(properties);
    ret->CreateRemSet();
    ret->SetupAtomics();
    ret->CreateMarkBitmap();
    if (!IsYoungRegionFlag(eden_or_old_or_nonmovable)) {
        ret->CreateLiveBitmap();
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    return ret;
}

void RegionPool::PromoteYoungRegion(Region *region)
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    if (block_.IsAddrInRange(region)) {
        spaces_->ReduceYoungOccupiedInSharedPool(region->Size());
        spaces_->IncreaseTenuredOccupiedInSharedPool(region->Size());
    } else {
        spaces_->PromoteYoungPool(region->Size());
    }
    // Change region type
    region->AddFlag(RegionFlag::IS_PROMOTED);
    region->RmvFlag(RegionFlag::IS_EDEN);
    region->AddFlag(RegionFlag::IS_OLD);
}

bool RegionPool::HaveTenuredSize(size_t size) const
{
    return spaces_->CanAllocInSpace(GenerationalSpaces::IS_TENURED_SPACE, size);
}

bool RegionPool::HaveFreeRegions(size_t num_regions, size_t region_size) const
{
    if (block_.GetFreeRegionsNum() >= num_regions) {
        return true;
    }
    num_regions -= block_.GetFreeRegionsNum();
    return PoolManager::GetMmapMemPool()->HaveEnoughPoolsInObjectSpace(num_regions, region_size);
}

Region *RegionSpace::NewRegion(size_t region_size, RegionFlag eden_or_old_or_nonmovable, RegionFlag properties,
                               OSPagesAllocPolicy alloc_policy)
{
    Region *region = nullptr;
    auto young_region_flag = IsYoungRegionFlag(eden_or_old_or_nonmovable);
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    if (young_region_flag && young_regions_in_use_.load(std::memory_order_relaxed) > desired_eden_length_) {
        return nullptr;
    }
    if (young_region_flag && (!empty_young_regions_.empty())) {
        region = GetRegionFromEmptyList(empty_young_regions_);
        ASAN_UNPOISON_MEMORY_REGION(region, Region::HeadSize());
        ASSERT(region_size == region->Size());
        region_pool_->NewRegion(region, this, region_size, eden_or_old_or_nonmovable, properties);
    } else if (!young_region_flag && (!empty_tenured_regions_.empty())) {
        region = GetRegionFromEmptyList(empty_tenured_regions_);
        ASAN_UNPOISON_MEMORY_REGION(region, Region::HeadSize());
        ASSERT(region_size == region->Size());
        region_pool_->NewRegion(region, this, region_size, eden_or_old_or_nonmovable, properties);
    } else {
        region = region_pool_->NewRegion(this, space_type_, allocator_type_, region_size, eden_or_old_or_nonmovable,
                                         properties, alloc_policy);
    }
    if (UNLIKELY(region == nullptr)) {
        return nullptr;
    }
    ASAN_POISON_MEMORY_REGION(ToVoidPtr(region->Begin()), region->End() - region->Begin());
    regions_.push_back(region->AsListNode());
    if (young_region_flag) {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        young_regions_in_use_.fetch_add(1, std::memory_order_relaxed);
    }
    return region;
}

void RegionSpace::PromoteYoungRegion(Region *region)
{
    ASSERT(region->GetSpace() == this);
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    region_pool_->PromoteYoungRegion(region);
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    [[maybe_unused]] auto previous_regions_in_use = young_regions_in_use_.fetch_sub(1, std::memory_order_relaxed);
    ASSERT(previous_regions_in_use > 0);
}

void RegionSpace::FreeAllRegions()
{
    // delete all regions
    IterateRegions([this](Region *region) { FreeRegion(region); });
    ReleaseEmptyRegions<RegionFlag::IS_EDEN, OSPagesPolicy::IMMEDIATE_RETURN>();
    ReleaseEmptyRegions<RegionFlag::IS_OLD, OSPagesPolicy::IMMEDIATE_RETURN>();
}

Region *RegionSpace::GetRegionFromEmptyList(DList &region_list)
{
    Region *region = Region::AsRegion(&(*region_list.begin()));
    region_list.erase(region_list.begin());
    ASSERT(region != nullptr);
    return region;
}

}  // namespace panda::mem
