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

#ifndef PANDA_MEM_GC_G1_REM_SET_INL_H
#define PANDA_MEM_GC_G1_REM_SET_INL_H

#include "runtime/mem/rem_set.h"
#include "runtime/mem/region_space-inl.h"
#include "runtime/mem/region_allocator.h"

namespace panda::mem {

template <typename LockConfigT>
RemSet<LockConfigT>::~RemSet()
{
    Clear();
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRef(const ObjectHeader *from_obj_addr, size_t offset)
{
    ASSERT(from_obj_addr != nullptr);
    auto ref = ToUintPtr(from_obj_addr) + offset;
    auto bitmap_begin_addr = ref & ~DEFAULT_REGION_MASK;
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(rem_set_lock_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    bitmaps_[bitmap_begin_addr].Set(GetIdxInBitmap(ref, bitmap_begin_addr));
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Clear()
{
    os::memory::LockHolder lock(rem_set_lock_);
    bitmaps_.clear();
    ref_regions_.clear();
}

/* static */
template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::InvalidateRegion(Region *invalid_region)
{
    RemSet<> *invalid_remset = invalid_region->GetRemSet();
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(invalid_remset->rem_set_lock_);

    for (Region *ref_reg : invalid_remset->ref_regions_) {
        ref_reg->GetRemSet()->RemoveFromRegion<NEED_LOCK>(invalid_region);
    }

    for (auto entry : invalid_remset->bitmaps_) {
        auto bitmap_begin_addr = entry.first;
        auto *from_region = AddrToRegion(ToVoidPtr(bitmap_begin_addr));
        from_region->GetRemSet()->RemoveRefRegion<NEED_LOCK>(invalid_region);
    }
}

/* static */
template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::InvalidateRefsFromRegion(Region *invalid_region)
{
    RemSet<> *invalid_remset = invalid_region->GetRemSet();
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(invalid_remset->rem_set_lock_);
    for (Region *ref_reg : invalid_remset->ref_regions_) {
        ref_reg->GetRemSet()->RemoveFromRegion<NEED_LOCK>(invalid_region);
    }
    invalid_remset->ref_regions_.clear();
}

/* static */
template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRefWithAddr(const ObjectHeader *obj_addr, size_t offset, const ObjectHeader *value_addr)
{
    auto *from_region = ObjectToRegion(obj_addr);
    auto *to_region = ObjectToRegion(value_addr);
    // TSAN thinks that we can have a data race here when we get region or getRemSet from region, because we don't have
    // synchronization between these events. In reality it's impossible, because if we get write from/to region it
    // should be created already by allocator in mutator thread, and only then writes happens.
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    ASSERT(from_region != nullptr);
    ASSERT(from_region->GetRemSet() != nullptr);
    ASSERT(to_region != nullptr);
    ASSERT_PRINT(to_region->GetRemSet() != nullptr,
                 "region " << to_region << ", obj " << obj_addr << ", value " << value_addr);

    to_region->GetRemSet()->AddRef<NEED_LOCK>(obj_addr, offset);
    from_region->GetRemSet()->AddRefRegion<NEED_LOCK>(to_region);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
}

template <typename LockConfigT>
template <typename RegionPred, typename MemVisitor>
inline void RemSet<LockConfigT>::Iterate(const RegionPred &region_pred, const MemVisitor &visitor)
{
    for (auto &[bitmap_begin_addr, bitmap] : bitmaps_) {
        auto *region = AddrToRegion(ToVoidPtr(bitmap_begin_addr));
        if (region_pred(region)) {
            MemRange bitmap_range(bitmap_begin_addr, bitmap_begin_addr + DEFAULT_REGION_SIZE);
            bitmap.Iterate(bitmap_range, [region, visitor](const MemRange &range) { visitor(region, range); });
        }
    }
}

template <typename LockConfigT>
template <typename Visitor>
inline void RemSet<LockConfigT>::IterateOverObjects(const Visitor &visitor)
{
    auto region_pred = []([[maybe_unused]] Region *region) { return true; };
    Iterate(region_pred, [visitor](Region *region, const MemRange &range) {
        region->GetLiveBitmap()->IterateOverMarkedChunkInRange(
            ToVoidPtr(range.GetStartAddress()), ToVoidPtr(range.GetEndAddress()),
            [visitor](void *mem) { visitor(static_cast<ObjectHeader *>(mem)); });
    });
}

template <typename LockConfigT>
template <bool NEED_LOCK>
PandaUnorderedSet<Region *> *RemSet<LockConfigT>::GetRefRegions()
{
    os::memory::LockHolder lock(rem_set_lock_);
    return &ref_regions_;
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRefRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(rem_set_lock_);
    ref_regions_.insert(region);
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::RemoveFromRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(rem_set_lock_);
    for (auto bitmap_begin_addr = ToUintPtr(region); bitmap_begin_addr < region->End();
         bitmap_begin_addr += DEFAULT_REGION_SIZE) {
        bitmaps_.erase(bitmap_begin_addr);
    }
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::RemoveRefRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(rem_set_lock_);
    ref_regions_.erase(region);
}

template <typename LockConfigT>
size_t RemSet<LockConfigT>::GetIdxInBitmap(uintptr_t addr, uintptr_t bitmap_begin_addr)
{
    size_t mem_size = DEFAULT_REGION_SIZE / Bitmap::GetNumBits();
    ASSERT(bitmap_begin_addr <= addr && addr < bitmap_begin_addr + DEFAULT_REGION_SIZE);
    return (addr - bitmap_begin_addr) / mem_size;
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Dump(std::ostream &out)
{
    os::memory::LockHolder lock(rem_set_lock_);
    auto pred = []([[maybe_unused]] Region *region) { return true; };
    Iterate(pred, [&out](Region *region, const MemRange &range) {
        if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
            out << " H";
        } else if (region->HasFlag(RegionFlag::IS_NONMOVABLE)) {
            out << " NM";
        } else if (region->HasFlag(RegionFlag::IS_OLD)) {
            out << " T";
        } else {
            out << " Y";
        }
        out << "[" << ToVoidPtr(range.GetStartAddress()) << "-" << ToVoidPtr(range.GetEndAddress()) << "]";
    });
    out << " To:";
    for (auto reg : ref_regions_) {
        out << " " << *reg;
    }
    out << std::dec;
}

}  // namespace panda::mem

#endif  // PANDA_MEM_GC_G1_REM_SET_INL_H
