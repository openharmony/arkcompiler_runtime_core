/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::mem {

template <typename LockConfigT>
RemSet<LockConfigT>::~RemSet()
{
    Clear();
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRef(const ObjectHeader *fromObjAddr, size_t offset)
{
    ASSERT(fromObjAddr != nullptr);
    auto ref = ToUintPtr(fromObjAddr) + offset;
    auto bitmapBeginAddr = ref & ~DEFAULT_REGION_MASK;
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(remSetLock_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    bitmaps_[bitmapBeginAddr].Set(GetIdxInBitmap(ref, bitmapBeginAddr));
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRef(const void *fromAddr)
{
    ASSERT(fromAddr != nullptr);
    auto ref = ToUintPtr(fromAddr);
    auto bitmapBeginAddr = ref & ~DEFAULT_REGION_MASK;
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(remSetLock_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    bitmaps_[bitmapBeginAddr].Set(GetIdxInBitmap(ref, bitmapBeginAddr));
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Clear()
{
    os::memory::LockHolder lock(remSetLock_);
    bitmaps_.clear();
    refRegions_.clear();
}

/* static */
template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::InvalidateRegion(Region *invalidRegion)
{
    RemSet<> *invalidRemset = invalidRegion->GetRemSet();
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(invalidRemset->remSetLock_);

    for (Region *refReg : invalidRemset->refRegions_) {
        refReg->GetRemSet()->RemoveFromRegion<NEED_LOCK>(invalidRegion);
    }

    for (auto entry : invalidRemset->bitmaps_) {
        auto bitmapBeginAddr = entry.first;
        auto *fromRegion = AddrToRegion(ToVoidPtr(bitmapBeginAddr));
        fromRegion->GetRemSet()->RemoveRefRegion<NEED_LOCK>(invalidRegion);
    }
}

/* static */
template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::InvalidateRefsFromRegion(Region *invalidRegion)
{
    RemSet<> *invalidRemset = invalidRegion->GetRemSet();
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(invalidRemset->remSetLock_);
    for (Region *refReg : invalidRemset->refRegions_) {
        refReg->GetRemSet()->RemoveFromRegion<NEED_LOCK>(invalidRegion);
    }
    invalidRemset->refRegions_.clear();
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Merge(RemSet<> *other)
{
    for (auto &[bitmap_begin_addr, bitmap] : other->bitmaps_) {
        bitmaps_[bitmap_begin_addr].AddBits(bitmap);
    }
}

template <typename LockConfigT>
PandaUnorderedSet<Region *> RemSet<LockConfigT>::GetDirtyRegions()
{
    PandaUnorderedSet<Region *> regions;
    for (auto &[bitmap_begin_addr, _] : bitmaps_) {
        auto *region = AddrToRegion(ToVoidPtr(bitmap_begin_addr));
        regions.insert(region);
    }
    return regions;
}

/* static */
template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRefWithAddr(const ObjectHeader *objAddr, size_t offset, const ObjectHeader *valueAddr)
{
    auto *fromRegion = ObjectToRegion(objAddr);
    auto *toRegion = ObjectToRegion(valueAddr);
    // TSAN thinks that we can have a data race here when we get region or getRemSet from region, because we don't have
    // synchronization between these events. In reality it's impossible, because if we get write from/to region it
    // should be created already by allocator in mutator thread, and only then writes happens.
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    ASSERT(fromRegion != nullptr);
    ASSERT(fromRegion->GetRemSet() != nullptr);
    ASSERT(toRegion != nullptr);
    ASSERT_PRINT(toRegion->GetRemSet() != nullptr,
                 "region " << toRegion << ", obj " << objAddr << ", value " << valueAddr);

    toRegion->GetRemSet()->AddRef<NEED_LOCK>(objAddr, offset);
    fromRegion->GetRemSet()->AddRefRegion<NEED_LOCK>(toRegion);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRefWithAddr(RemSet<> *fromRemset, const void *fromAddr, const ObjectHeader *valueAddr)
{
    ASSERT(AddrToRegion(fromAddr)->GetRemSet() == fromRemset);
    auto *toRegion = ObjectToRegion(valueAddr);
    // TSAN thinks that we can have a data race here when we get region or getRemSet from region, because we don't have
    // synchronization between these events. In reality it's impossible, because if we get write from/to region it
    // should be created already by allocator in mutator thread, and only then writes happens.
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    ASSERT(toRegion != nullptr);
    ASSERT_PRINT(toRegion->GetRemSet() != nullptr,
                 "region " << toRegion << ", from addr " << fromAddr << ", value " << valueAddr);

    toRegion->GetRemSet()->AddRef<NEED_LOCK>(fromAddr);
    fromRemset->AddRefRegion<NEED_LOCK>(toRegion);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
}

template <typename LockConfigT>
template <typename RegionPred, typename MemVisitor>
inline void RemSet<LockConfigT>::Iterate(const RegionPred &regionPred, const MemVisitor &visitor) const
{
    for (auto &[bitmapBeginAddr, bitmap] : bitmaps_) {
        auto *region = AddrToRegion(ToVoidPtr(bitmapBeginAddr));
        if (regionPred(region)) {
            MemRange bitmapRange(bitmapBeginAddr, bitmapBeginAddr + DEFAULT_REGION_SIZE);
            bitmap.Iterate(bitmapRange, [region, visitor](const MemRange &range) { visitor(region, range); });
        }
    }
}

template <typename LockConfigT>
template <typename Visitor>
inline void RemSet<LockConfigT>::IterateOverObjects(const Visitor &visitor) const
{
    auto regionPred = []([[maybe_unused]] Region *region) { return true; };
    Iterate(regionPred, [visitor](Region *region, const MemRange &range) {
        region->GetLiveBitmap()->IterateOverMarkedChunkInRange(
            ToVoidPtr(range.GetStartAddress()), ToVoidPtr(range.GetEndAddress()),
            [visitor](void *mem) { visitor(static_cast<ObjectHeader *>(mem)); });
    });
}

template <typename LockConfigT>
template <bool NEED_LOCK>
PandaUnorderedSet<Region *> *RemSet<LockConfigT>::GetRefRegions()
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(remSetLock_);
    return &refRegions_;
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::AddRefRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(remSetLock_);
    refRegions_.insert(region);
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::RemoveFromRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(remSetLock_);
    for (auto bitmapBeginAddr = ToUintPtr(region); bitmapBeginAddr < region->End();
         bitmapBeginAddr += DEFAULT_REGION_SIZE) {
        bitmaps_.erase(bitmapBeginAddr);
    }
}

template <typename LockConfigT>
template <bool NEED_LOCK>
void RemSet<LockConfigT>::RemoveRefRegion(Region *region)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(remSetLock_);
    refRegions_.erase(region);
}

template <typename LockConfigT>
size_t RemSet<LockConfigT>::GetIdxInBitmap(uintptr_t addr, uintptr_t bitmapBeginAddr)
{
    static constexpr size_t MEM_SIZE = DEFAULT_REGION_SIZE / Bitmap::GetNumBits();
    ASSERT(bitmapBeginAddr <= addr);
    ASSERT(addr < bitmapBeginAddr + DEFAULT_REGION_SIZE);
    return (addr - bitmapBeginAddr) / MEM_SIZE;
}

template <typename LockConfigT>
void RemSet<LockConfigT>::Dump(std::ostream &out)
{
    os::memory::LockHolder lock(remSetLock_);
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
    for (auto reg : refRegions_) {
        out << " " << *reg;
    }
    out << std::dec;
}

template <typename LockConfigT>
template <typename Visitor>
void RemSet<LockConfigT>::VisitBitmaps(const Visitor &visitor) const
{
    for (auto &[bitmapBeginAddr, bitmap] : bitmaps_) {
        visitor(bitmapBeginAddr, bitmap);
    }
}

template <typename RegionContainer, typename RegionPred, typename MemVisitor>
void GlobalRemSet::ProcessRemSets(const RegionContainer &cont, const RegionPred &regionPred, const MemVisitor &visitor)
{
    for (auto *region : cont) {
        FillBitmap(*region->GetRemSet(), regionPred);
    }
    IterateOverBits(visitor);
}

template <typename RegionPred>
void GlobalRemSet::FillBitmap(const RemSet<> &remSet, const RegionPred &regionPred)
{
    remSet.VisitBitmaps([this, &regionPred](uintptr_t beginAddr, const RemSet<>::Bitmap &bitmap) {
        auto *region = AddrToRegion(ToVoidPtr(beginAddr));
        if (regionPred(region)) {
            bitmaps_[beginAddr].AddBits(bitmap);
        }
    });
}

template <typename MemVisitor>
void GlobalRemSet::IterateOverBits(const MemVisitor &visitor) const
{
    for (auto &[bitmapBeginAddr, bitmap] : bitmaps_) {
        auto *region = AddrToRegion(ToVoidPtr(bitmapBeginAddr));
        MemRange bitmapRange(bitmapBeginAddr, bitmapBeginAddr + DEFAULT_REGION_SIZE);
        bitmap.Iterate(bitmapRange, [region, visitor](const MemRange &range) { visitor(region, range); });
    }
}

template <typename MemVisitor>
bool GlobalRemSet::IterateOverUniqueRange(Region *region, MemRange range, const MemVisitor &visitor)
{
    auto addr = range.GetStartAddress();
    auto bitmapBeginAddr = addr & ~DEFAULT_REGION_MASK;
    auto bitmapIt = bitmaps_.find(bitmapBeginAddr);
    if (bitmapIt == bitmaps_.cend()) {
        return visitor(region, range);
    }

    auto &bitmap = bitmapIt->second;
    auto endAddr = range.GetEndAddress() + 1U;
    static constexpr size_t MEM_SIZE = DEFAULT_REGION_SIZE / RemSet<>::Bitmap::GetNumBits();
    ASSERT(((endAddr - addr) % MEM_SIZE) == 0);
    bool allRefsProcessed = true;
    for (; addr != endAddr; addr += MEM_SIZE) {
        auto isMarked = bitmap.Check(RemSet<>::GetIdxInBitmap(addr, bitmapBeginAddr));
        if (isMarked) {
            allRefsProcessed = false;
            continue;
        }
        if (!visitor(region, MemRange(addr, addr + MEM_SIZE))) {
            allRefsProcessed = false;
        }
    }
    return allRefsProcessed;
}

}  // namespace ark::mem

#endif  // PANDA_MEM_GC_G1_REM_SET_INL_H
