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
RemSet<LockConfigT>::RemSet(Region *region, CardTable *card_table)
    : region_(region), allocator_(region->GetInternalAllocator()), card_table_(card_table)
{
}

template <typename LockConfigT>
RemSet<LockConfigT>::~RemSet()
{
    Clear();
}

template <typename LockConfigT>
template <bool need_lock>
void RemSet<LockConfigT>::AddRef(const ObjectHeader *from_obj_addr)
{
    ASSERT(from_obj_addr != nullptr);
    auto from_region = ObjectToRegion(from_obj_addr);
    ASSERT(from_region != nullptr);
    auto card_ptr = GetCardPtr(from_obj_addr);
    auto list = GetCardList<need_lock>(from_region);
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    if (list == nullptr) {
        list = allocator_->New<CardList>();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        regions_[from_region] = list;
    }
    if (find(list->begin(), list->end(), card_ptr) == list->end()) {
        list->push_back(card_ptr);
    }
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Clear()
{
    os::memory::LockHolder lock(rem_set_lock_);
    for (auto region_iter : regions_) {
        auto list = region_iter.second;
        allocator_->Delete(list);
    }
    regions_.clear();
    ref_regions_.clear();
}

template <typename LockConfigT>
template <bool need_lock>
CardList *RemSet<LockConfigT>::GetCardList(Region *region)
{
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    auto card_list = regions_.find(region);
    if (card_list == regions_.end()) {
        return nullptr;
    }
    return card_list->second;
}

template <typename LockConfigT>
CardPtr RemSet<LockConfigT>::GetCardPtr(const void *addr)
{
    return card_table_ != nullptr ? card_table_->GetCardPtr(ToUintPtr(addr)) : nullptr;
}

template <typename LockConfigT>
MemRange RemSet<LockConfigT>::GetMemoryRange(CardPtr card)
{
    return card_table_->GetMemoryRange(card);
}

/* static */
template <typename LockConfigT>
template <bool need_lock>
void RemSet<LockConfigT>::InvalidateRegion(Region *invalid_region)
{
    RemSet<> *invalid_remset = invalid_region->GetRemSet();
    os::memory::LockHolder<LockConfigT, need_lock> lock(invalid_remset->rem_set_lock_);
    for (Region *ref_reg : invalid_remset->ref_regions_) {
        ref_reg->GetRemSet()->RemoveFromRegion<need_lock>(invalid_region);
    }

    for (auto entry : invalid_remset->regions_) {
        Region *from_region = entry.first;
        from_region->GetRemSet()->RemoveRefRegion<need_lock>(invalid_region);
    }
}

/* static */
template <typename LockConfigT>
template <bool need_lock>
void RemSet<LockConfigT>::AddRefWithAddr(const ObjectHeader *obj_addr, const ObjectHeader *value_addr)
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
    ASSERT(to_region->GetRemSet() != nullptr);

    to_region->GetRemSet()->AddRef<need_lock>(obj_addr);
    from_region->GetRemSet()->AddRefRegion<need_lock>(to_region);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
}

/* static */
template <typename LockConfigT>
void RemSet<LockConfigT>::TraverseObjectToAddRef(const void *addr)
{
    // TODO(xucheng) : need a special thread to add ref.
    auto traverse_object_visitor = [](ObjectHeader *from_object, ObjectHeader *object_to_traverse) {
        AddRefWithAddr(from_object, object_to_traverse);
    };
    auto obj = static_cast<ObjectHeader *>(const_cast<void *>(addr));
    GCStaticObjectHelpers::TraverseAllObjects(obj, traverse_object_visitor);
}

template <typename LockConfigT>
template <bool need_lock, typename ObjectVisitor>
inline void RemSet<LockConfigT>::VisitMarkedCards(const ObjectVisitor &object_visitor)
{
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    auto visitor = [&object_visitor](void *mem) { object_visitor(static_cast<ObjectHeader *>(mem)); };
    for (auto region_iter : regions_) {
        auto *region = region_iter.first;
        auto *card_list = region_iter.second;
        if (region == nullptr) {
            // process humongous
            continue;
        } else {  // NOLINT(readability-else-after-return)
            for (auto card_ptr : *card_list) {
                // visit live objects in old region
                auto mem_range = GetMemoryRange(card_ptr);
                region->GetLiveBitmap()->IterateOverMarkedChunkInRange(ToVoidPtr(mem_range.GetStartAddress()),
                                                                       ToVoidPtr(mem_range.GetEndAddress()), visitor);
            }
        }
    }
}

template <typename LockConfigT>
template <bool need_lock, typename CardVisitor>
inline void RemSet<LockConfigT>::ProceedMarkedCards(const CardVisitor &card_visitor)
{
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    for (auto region_iter : regions_) {
        auto *region = region_iter.first;
        auto *card_list = region_iter.second;
        if (region == nullptr) {
            // process humongous
            continue;
        } else {  // NOLINT(readability-else-after-return)
            for (auto card_ptr : *card_list) {
                // visit live objects in old region
                card_visitor(card_ptr, region);
            }
        }
    }
}

template <typename LockConfigT>
template <bool need_lock>
PandaUnorderedSet<Region *> *RemSet<LockConfigT>::GetRefRegions()
{
    os::memory::LockHolder lock(rem_set_lock_);
    return &ref_regions_;
}

template <typename LockConfigT>
template <bool need_lock>
void RemSet<LockConfigT>::AddRefRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    ref_regions_.insert(region);
}

template <typename LockConfigT>
template <bool need_lock>
void RemSet<LockConfigT>::RemoveFromRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    auto it = regions_.find(region);
    if (it == regions_.end()) {
        return;
    }
    allocator_->Delete(it->second);
    regions_.erase(it);
}

template <typename LockConfigT>
template <bool need_lock>
void RemSet<LockConfigT>::RemoveRefRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, need_lock> lock(rem_set_lock_);
    ref_regions_.erase(region);
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Dump(std::ostream &out)
{
    os::memory::LockHolder lock(rem_set_lock_);
    out << *region_ << ": From";
    for (auto entry : regions_) {
        CardList *cards = entry.second;
        if (cards != nullptr) {
            for (CardPtr card : *cards) {
                if (entry.first->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
                    out << " H-";
                } else if (entry.first->HasFlag(RegionFlag::IS_NONMOVABLE)) {
                    out << " NM-";
                } else if (entry.first->HasFlag(RegionFlag::IS_OLD)) {
                    out << " T-";
                } else {
                    out << " Y-";
                }
                out << std::hex << "[" << card_table_->GetCardStartAddress(card) << "-"
                    << card_table_->GetCardEndAddress(card) << "]";
            }
        }
    }
    out << " To:";
    for (auto reg : ref_regions_) {
        out << " " << *reg;
    }
    out << std::dec;
}

}  // namespace panda::mem

#endif  // PANDA_MEM_GC_G1_REM_SET_INL_H
