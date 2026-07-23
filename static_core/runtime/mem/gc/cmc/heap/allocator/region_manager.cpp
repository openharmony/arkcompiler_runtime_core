/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/mem/gc/cmc/heap/allocator/region_manager.h"

#include "runtime/mem/gc/cmc/heap/region_desc.h"
#include "runtime/mem/gc/cmc/heap/allocator/region_list.h"
#include "runtime/mem/gc/cmc/heap/allocator/regional_heap.h"
#include "common_components/base/c_string.h"
#include "runtime/mem/gc/cmc/heap/collector/collector.h"
#include "common_components/base/time_utils.h"
#include "runtime/mem/gc/cmc/heap/heap.h"
#include "runtime/mem/gc/cmc/heap/allocator/fix_heap.h"

#include "common_interfaces/objects/base_object.h"
#include "runtime/include/thread_scopes.h"

#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "common_components/taskpool/taskpool.h"

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"

#if defined(_WIN64)
#include <sysinfoapi.h>
#endif

#include "runtime/mem/gc/cmc/cmc-gc.h"

#include <algorithm>

#include <cmath>
#include <cstdint>
#include <unistd.h>

namespace ark::mem {
uintptr_t RegionDesc::UnitInfo::totalUnitCount = 0;
uintptr_t RegionDesc::UnitInfo::heapStartAddress = 0;
}  // namespace ark::mem

namespace ark::common_vm {

static size_t GetPageSize() noexcept
{
#if defined(_WIN64)
    SYSTEM_INFO systeminfo;
    GetSystemInfo(&systeminfo);
    if (systeminfo.dwPageSize != 0) {
        return systeminfo.dwPageSize;
    } else {
        // default page size is 4KB if get system page size failed.
        return 4 * KB;
    }
#elif defined(__APPLE__)
    // default page size is 4KB in MacOS
    return 4 * KB;
#else
    return getpagesize();
#endif
}

// System default page size
const size_t COMMON_PAGE_SIZE = GetPageSize();
const size_t AllocatorUtils::ALLOC_PAGE_SIZE = COMMON_PAGE_SIZE;
// size of huge page is 2048KB.
constexpr size_t HUGE_PAGE_UNIT_NUM = (2048 * KB) / RegionDesc::UNIT_SIZE;

}  // namespace ark::common_vm

namespace ark::mem {
using ::ark::common_vm::RegionalHeap;

void RegionDesc::VisitAllObjects(const std::function<void(BaseObject *)> &&func)
{
    VisitAllObjectsBefore(std::move(func), GetRegionAllocPtr());
}

void RegionDesc::VisitAllObjectsBefore(const std::function<void(BaseObject *)> &&func, uintptr_t end)
{
    uintptr_t position = GetRegionStart();

    if (IsMonoSizeNonMovableRegion()) {
        size_t size = static_cast<size_t>(GetRegionCellCount() + 1) * sizeof(uint64_t);
        while (position < end) {
            BaseObject *obj = reinterpret_cast<BaseObject *>(position);
            position += size;
            if (position > end) {
                break;
            }
            if (IsFreeNonMovableObject(obj)) {
                continue;
            }
            func(obj);
        }
        return;
    } else if (IsLargeRegion() && (position < end)) {
        func(reinterpret_cast<BaseObject *>(GetRegionStart()));
    } else if (IsSmallRegion()) {
        while (position < end) {
            // GetAllocSize should before call func, because object maybe destroy in compact gc.
            func(reinterpret_cast<BaseObject *>(position));
            size_t size = RegionalHeap::GetAllocSize(*reinterpret_cast<BaseObject *>(position));
            position += size;
        }
    }
}

void RegionDesc::VisitAllObjectsBeforeCopy(const std::function<void(BaseObject *)> &&func)
{
    uintptr_t allocPtr = GetRegionAllocPtr();
    uintptr_t phaseLine = GetCopyLine();
    uintptr_t end = std::min(phaseLine, allocPtr);
    VisitAllObjectsBefore(std::move(func), end);
}

bool RegionDesc::VisitLiveObjectsUntilFalse(const std::function<bool(BaseObject *)> &&func)
{
    // no need to visit this region.
    if (GetLiveByteCount() == 0) {
        return true;
    }

    if (IsLargeRegion()) {
        return func(reinterpret_cast<BaseObject *>(GetRegionStart()));
    }
    if (IsSmallRegion()) {
        uintptr_t position = GetRegionStart();
        uintptr_t allocPtr = GetRegionAllocPtr();
        while (position < allocPtr) {
            BaseObject *obj = reinterpret_cast<BaseObject *>(position);
            if (RegionalHeap::IsSurvivedObject(obj) && !func(obj)) {
                return false;
            }
            position += RegionalHeap::GetAllocSize(*obj);
        }
    }
    return true;
}

void RegionDesc::VisitRememberSetBeforeMarking(const std::function<void(BaseObject *)> &func)
{
    uintptr_t end = std::min(GetMarkingLine(), GetRegionAllocPtr());
    GetRSet()->VisitAllMarkedCardBefore(func, GetRegionBaseFast(), end);
}

void RegionDesc::VisitRememberSetBeforeCopy(const std::function<void(BaseObject *)> &func)
{
    uintptr_t end = std::min(GetCopyLine(), GetRegionAllocPtr());
    GetRSet()->VisitAllMarkedCardBefore(func, GetRegionBaseFast(), end);
}

void RegionDesc::VisitRememberSet(const std::function<void(BaseObject *)> &func)
{
    GetRSet()->VisitAllMarkedCardBefore(func, GetRegionBaseFast(), GetRegionAllocPtr());
}
}  // namespace ark::mem

namespace ark::common_vm {
void RegionList::MergeRegionListWithoutHead(RegionList &srcList, RegionDesc::RegionType regionType)
{
    RegionDesc *head = srcList.TakeHeadRegion();
    MergeRegionList(srcList, regionType);
    if (head) {
        srcList.PrependRegion(head, head->GetRegionType());
    }
}

void RegionList::MergeRegionList(RegionList &srcList, RegionDesc::RegionType regionType)
{
    RegionList regionList("region list cache");
    srcList.MoveTo(regionList);
    RegionDesc *head = regionList.GetHeadRegion();
    RegionDesc *tail = regionList.GetTailRegion();
    if (head == nullptr) {
        return;
    }
    ark::os::memory::LockHolder lock(listMutex_);
    regionList.SetElementType(regionType);
    IncCounts(regionList.GetRegionCount(), regionList.GetUnitCount());
    if (listHead_ == nullptr) {
        listHead_ = head;
        listTail_ = tail;
    } else {
        tail->SetNextRegion(listHead_);
        listHead_->SetPrevRegion(tail);
        listHead_ = head;
    }
}

static const char *RegionDescRegionTypeToString(RegionDesc::RegionType type)
{
    switch (type) {
        case RegionDesc::RegionType::FREE_REGION:
            return "FREE_REGION";
        case RegionDesc::RegionType::GARBAGE_REGION:
            return "GARBAGE_REGION";
        case RegionDesc::RegionType::THREAD_LOCAL_REGION:
            return "THREAD_LOCAL_REGION";
        case RegionDesc::RegionType::THREAD_LOCAL_OLD_REGION:
            return "THREAD_LOCAL_OLD_REGION";
        case RegionDesc::RegionType::RECENT_FULL_REGION:
            return "RECENT_FULL_REGION";
        case RegionDesc::RegionType::FROM_REGION:
            return "FROM_REGION";
        case RegionDesc::RegionType::LONE_FROM_REGION:
            return "LONE_FROM_REGION";
        case RegionDesc::RegionType::EXEMPTED_FROM_REGION:
            return "EXEMPTED_FROM_REGION";
        case RegionDesc::RegionType::TO_REGION:
            return "TO_REGION";
        case RegionDesc::RegionType::OLD_REGION:
            return "OLD_REGION";
        case RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION:
            return "RECENT_POLYSIZE_NONMOVABLE_REGION";
        case RegionDesc::RegionType::FULL_POLYSIZE_NONMOVABLE_REGION:
            return "FULL_POLYSIZE_NONMOVABLE_REGION";
        case RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION:
            return "MONOSIZE_NONMOVABLE_REGION";
        case RegionDesc::RegionType::FULL_MONOSIZE_NONMOVABLE_REGION:
            return "FULL_MONOSIZE_NONMOVABLE_REGION";
        case RegionDesc::RegionType::RECENT_LARGE_REGION:
            return "RECENT_LARGE_REGION";
        case RegionDesc::RegionType::LARGE_REGION:
            return "LARGE_REGION";
        case RegionDesc::RegionType::END_OF_REGION_TYPE:
            break;
    }
    ASSERT_PRINT(false, "Invalid region type");
    return "INVALID_REGION";
}

void RegionList::PrependRegion(RegionDesc *region, RegionDesc::RegionType type)
{
    ark::os::memory::LockHolder lock(listMutex_);
    PrependRegionLocked(region, type);
}

void RegionList::PrependRegionLocked(RegionDesc *region, RegionDesc::RegionType type)
{
    if (region == nullptr) {
        return;
    }

    LOG(DEBUG, GC) << listName_ << " (" << regionCount_ << ", " << unitCount_ << ")+(" << 1llu << ", "
                   << region->GetUnitCount() << ") prepend region " << region << "(base="
                   << "0x" << std::hex << region->GetRegionBase() << ")@0x" << std::hex << region->GetRegionStart()
                   << "+" << std::dec << region->GetRegionAllocatedSize() << " type "
                   << static_cast<size_t>(region->GetRegionType()) << "->" << static_cast<size_t>(type);
    region->SetRegionType(type);

    size_t totalRegionSize = region->GetRegionEnd() - region->GetRegionBase();
    os::mem::TagAnonymousMemory(reinterpret_cast<void *>(region->GetRegionBase()), totalRegionSize,
                                (std::string("ArkTS Heap CMCGC Region ") + RegionDescRegionTypeToString(type)).c_str());

    region->SetPrevRegion(nullptr);
    IncCounts(1, region->GetUnitCount());
    region->SetNextRegion(listHead_);
    if (listHead_ == nullptr) {
        ASSERT_PRINT(listTail_ == nullptr, "PrependRegion listTail is not null");
        listTail_ = region;
    } else {
        listHead_->SetPrevRegion(region);
    }
    listHead_ = region;
}

void RegionList::DeleteRegionLocked(RegionDesc *del)
{
    ASSERT_PRINT(listHead_ != nullptr && listTail_ != nullptr, "illegal region list");

    RegionDesc *pre = del->GetPrevRegion();
    RegionDesc *next = del->GetNextRegion();

    del->SetNextRegion(nullptr);
    del->SetPrevRegion(nullptr);

    LOG(DEBUG, GC) << listName_ << " (" << regionCount_ << ", " << unitCount_ << ")-(" << 1llu << ", "
                   << del->GetUnitCount() << ") delete region " << del << "(start=" << del->GetRegionBase() << "),@0x"
                   << std::hex << del->GetRegionStart() << "+" << std::dec << del->GetRegionAllocatedSize() << " type "
                   << static_cast<size_t>(del->GetRegionType());
    DecCounts(1, del->GetUnitCount());

    if (listHead_ == del) {  // delete head
        ASSERT_PRINT(pre == nullptr, "Delete Region pre is not null");
        listHead_ = next;
        if (listHead_ == nullptr) {  // now empty
            listTail_ = nullptr;
            return;
        }
    } else {
        pre->SetNextRegion(next);
    }

    if (listTail_ == del) {  // delete tail
        LOG_IF(UNLIKELY(!(next == nullptr)), FATAL, GC) << "Delete Region next is not null";
        listTail_ = pre;
        if (listTail_ == nullptr) {  // now empty
            listHead_ = nullptr;
            return;
        }
    } else {
        next->SetPrevRegion(pre);
    }
}

void RegionList::DumpRegionSummary() const
{
    LOG(DEBUG, GC) << "\t" << listName_ << " " << regionCount_ << ": " << unitCount_ << " units ("
                   << GetAllocatedSize(true) << " B, alloc " << GetAllocatedSize(false) << ")";
}

#ifndef NDEBUG
void RegionList::DumpRegionList(const char *msg)
{
    LOG(DEBUG, GC) << "dump region list " << msg;
    ark::os::memory::LockHolder lock(listMutex_);
    for (RegionDesc *region = listHead_; region != nullptr; region = region->GetNextRegion()) {
        LOG(DEBUG, GC) << "region " << region << " @0x" << std::hex << region->GetRegionBase() << "(start=0x"
                       << region->GetRegionStart() << ")+" << std::dec << region->GetRegionAllocatedSize() << " units ["
                       << region->GetUnitIdx() << "+" << region->GetUnitCount() << ", "
                       << region->GetUnitIdx() + region->GetUnitCount() << ") type "
                       << static_cast<size_t>(region->GetRegionType()) << " prev " << region->GetPrevRegion()
                       << " next " << region->GetNextRegion();
    }
}
#endif
inline void RegionManager::TagHugePage(RegionDesc *region, size_t num) const
{
#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
    (void)madvise(reinterpret_cast<void *>(region->GetRegionBase()), num * RegionDesc::UNIT_SIZE, MADV_HUGEPAGE);
#else
    (void)region;
    (void)num;
#endif
}

inline void RegionManager::UntagHugePage(RegionDesc *region, size_t num) const
{
#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
    (void)madvise(reinterpret_cast<void *>(region->GetRegionBase()), num * RegionDesc::UNIT_SIZE, MADV_NOHUGEPAGE);
#else
    (void)region;
    (void)num;
#endif
}

void RegionManager::Initialize(size_t nRegion, mem::HeapSpace *heapSpace)
{
    heapSpace_ = heapSpace;
    // propagate region heap layout
    RegionDesc::Initialize(nRegion, PoolManager::GetMmapMemPool()->GetMinObjectAddress());

    LOG(DEBUG, GC) << "region info heap [0x" << std::hex << PoolManager::GetMmapMemPool()->GetMinObjectAddress()
                   << ", 0x" << std::hex << PoolManager::GetMmapMemPool()->GetMaxObjectAddress() << "), unit count "
                   << std::dec << nRegion;
}

void RegionManager::Fini()
{
    os::memory::LockHolder h(regionsInUseLock_);
    IterateAllocatedUnits([this](RegionDesc *region) {
        if (region->IsValidRegion() && !region->IsFreeRegion()) {
#if defined(COMMON_SANITIZER_SUPPORT)
            Sanitizer::OnHeapDeallocated(region, region->GetRegionBaseSize());
#endif
            heapSpace_->FreePool(region, region->GetRegionBaseSize(), true);
        }
    });
    regionsInUse_.clear();
}

void RegionManager::ReclaimRegion(RegionDesc *region)
{
    size_t num = region->GetUnitCount();
    if (num >= HUGE_PAGE_UNIT_NUM) {
        UntagHugePage(region, num);
    }
    LOG(DEBUG, GC) << "reclaim region " << region << "(base="
                   << "0x" << std::hex << region->GetRegionBase() << ")@0x" << std::hex << region->GetRegionStart()
                   << "+" << std::dec << region->GetRegionAllocatedSize() << " type "
                   << static_cast<size_t>(region->GetRegionType());
    region->InitFreeUnits();
    heapSpace_->FreePool(region, num * RegionDesc::UNIT_SIZE, false);

    os::memory::LockHolder h(regionsInUseLock_);
    regionsInUse_.erase(region);
}

size_t RegionManager::ReleaseRegion(RegionDesc *region)
{
    size_t res = region->GetRegionSize();
    size_t num = region->GetUnitCount();
    if (num >= HUGE_PAGE_UNIT_NUM) {
        UntagHugePage(region, num);
    }
    LOG(DEBUG, GC) << "release region " << region << " @0x" << std::hex << region->GetRegionStart() << "+" << std::dec
                   << region->GetRegionAllocatedSize() << " type " << static_cast<size_t>(region->GetRegionType());
    region->InitFreeUnits();
    heapSpace_->FreePool(region, num * RegionDesc::UNIT_SIZE, true);

    os::memory::LockHolder h(regionsInUseLock_);
    regionsInUse_.erase(region);
    return res;
}

void RegionManager::CountLiveObject(const BaseObject *obj)
{
    RegionDesc *region = RegionDesc::GetRegionDescAt(obj);
    region->AddLiveByteCount(obj->GetSize());
}

void RegionManager::ForEachObjectUnsafe(const std::function<void(BaseObject *)> &visitor) const
{
    os::memory::LockHolder h(regionsInUseLock_);
    IterateAllocatedUnits([&visitor](RegionDesc *region) {
        if (region->IsValidRegion() && !region->IsFreeRegion() && !region->IsGarbageRegion()) {
            region->VisitAllObjects([&visitor](BaseObject *object) { visitor(object); });
        }
    });
}

void RegionManager::ForEachObjectSafe(const std::function<void(BaseObject *)> &visitor) const
{
    ScopedNativeCodeThread scope(ManagedThread::GetCurrent());
    ark::ScopedStopTheWorld stw;
    ForEachObjectUnsafe(visitor);
}

RegionDesc *RegionManager::TakeRegion(size_t num, RegionDesc::UnitRole type, bool expectPhysicalMem, bool allowGC,
                                      bool isCopy)
{
    // check for allocation since we do not want gc threads and mutators do any harm to each other.
    size_t size = num * RegionDesc::UNIT_SIZE;
    RequestForRegion(size);

    RegionDesc *head = garbageRegionList_.TakeHeadRegion();
    if (head != nullptr) {
        LOG(DEBUG, GC) << "take garbage region " << head << "@0x" << std::hex << head->GetRegionStart() << "+"
                       << std::dec << head->GetRegionSize();
        if (head->GetUnitCount() == num) {
            auto idx = head->GetUnitIdx();
#ifdef USE_HWASAN
            const uintptr_t pAddr = RegionDesc::GetUnitAddress(idx);
            ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<const volatile void *>(pAddr), size);
            LOG(DEBUG, COMMON) << std::hex << "set [" << pAddr << std::hex << ", " << pAddr + size << ") unpoisoned\n";
#endif
            RegionDesc::ClearUnits(idx, num);
            LOG(DEBUG, GC) << "reuse garbage region " << head << "@0x" << std::hex << head->GetRegionStart() << "+"
                           << std::dec << head->GetRegionSize();
            return RegionDesc::ResetRegion(idx, num, type);
        } else {
            LOG(DEBUG, GC) << "reclaim garbage region " << head << "@0x" << std::hex << head->GetRegionStart() << "+"
                           << std::dec << head->GetRegionSize();
            ReclaimRegion(head);
        }
    }

    auto pool = heapSpace_->TryAllocPool(size,
                                         type == RegionDesc::UnitRole::SMALL_SIZED_UNITS
                                             ? SpaceType::SPACE_TYPE_OBJECT
                                             : SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                                         AllocatorType::REGION_ALLOCATOR, nullptr);
    if (pool.GetMem() == nullptr) {
        return nullptr;
    }

    uintptr_t addr = ToUintPtr(pool.GetMem());

#ifndef PANDA_TARGET_32
    size_t totalHeapSize = heapSpace_->GetMaxHeapSize();
    // 2: half space reserved for forward copy. throw oom when gc finish.
    if (heapSpace_->GetCurrentHeapSize() * 2 > totalHeapSize) {
        if (!isCopy) {
            heapSpace_->FreePool(pool.GetMem(), pool.GetSize(), false);
            return nullptr;
        } else {
            Heap::GetHeap().SetForceThrowOOM(true);
        }
    }
#endif

#if defined(COMMON_SANITIZER_SUPPORT)
    Sanitizer::OnHeapAllocated(pool.GetMem(), pool.GetSize());
#endif

    RegionDesc *region = RegionDesc::InitRegionAt(addr, num, type);
    size_t idx = region->GetUnitIdx();
    LOG(DEBUG, GC) << "take inactive units [" << idx << "+" << num << ", " << idx + num << ") at [0x" << std::hex
                   << RegionDesc::GetUnitAddress(idx) << ", 0x" << RegionDesc::GetUnitAddress(idx + num) << std::dec
                   << ")";
    if (num >= HUGE_PAGE_UNIT_NUM) {
        TagHugePage(region, num);
    }
    RegionDesc::ClearUnits(idx, num);

    os::memory::LockHolder h(regionsInUseLock_);
    regionsInUse_.insert(region);
    return region;
}

void RegionManager::DumpRegionStats() const
{
    size_t totalSize = heapSpace_->GetMaxHeapSize();
    size_t totalUnits = totalSize / RegionDesc::UNIT_SIZE;
    size_t activeSize = heapSpace_->GetCurrentHeapSize();
    size_t activeUnits = activeSize / RegionDesc::UNIT_SIZE;
    LOG(DEBUG, GC) << "\ttotal units: " << totalUnits << " (" << totalSize << " B)";
    LOG(DEBUG, GC) << "\tactive units: " << activeUnits << " (" << activeSize << " B)";

    garbageRegionList_.DumpRegionSummary();
}

void RegionManager::RequestForRegion(size_t size)
{
    if (IsGcThread()) {
        // gc thread is always permitted for allocation.
        return;
    }

    Heap &heap = Heap::GetHeap();
    auto *cmcAllocator =
        static_cast<mem::CMCObjectAllocator *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    size_t liveBytesAfterGC = cmcAllocator->GetLiveBytesAfterGC();
    size_t allocatedBytes = heap.GetAllocatedSize() - liveBytesAfterGC;
    constexpr double pi = 3.14;
    size_t availableBytesAfterGC = heap.GetMaxCapacity() - liveBytesAfterGC;
    double heuAllocRate =
        std::cos((pi / 2.0) * allocatedBytes / availableBytesAfterGC) * cmcAllocator->GetCollectionRate();
    // for maximum performance, choose the larger one.
    double allocRate = std::max(
        static_cast<double>(Heap::GetHeap().GetHeapParam().allocationRate) * MB / SECOND_TO_NANO_SECOND, heuAllocRate);
    ASSERT_PRINT(allocRate > 0.00001, "allocRate is zero");  // If it is less than 0.00001, it is considered as 0
    size_t waitTime = static_cast<size_t>(size / allocRate);
    uint64_t now = TimeUtil::NanoSeconds();
    // Atomic with relaxed order reason: data race with prevRegionAllocTime_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (prevRegionAllocTime_.load(std::memory_order_relaxed) + waitTime <= now) {
        // Atomic with relaxed order reason: data race with prevRegionAllocTime_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        prevRegionAllocTime_.store(TimeUtil::NanoSeconds(), std::memory_order_relaxed);
        return;
    }

    // Atomic with relaxed order reason: data race with prevRegionAllocTime_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    uint64_t sleepTime = std::min<uint64_t>(Heap::GetHeap().GetHeapParam().allocationWaitTime,
                                            prevRegionAllocTime_.load(std::memory_order_relaxed) + waitTime - now);
    LOG(DEBUG, GC) << "wait " << sleepTime << " ns to alloc " << size << "(B)";
    ark::os::thread::NativeSleepUS(std::chrono::microseconds(sleepTime / NS_PER_US));
    // Atomic with relaxed order reason: data race with prevRegionAllocTime_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    prevRegionAllocTime_.store(TimeUtil::NanoSeconds(), std::memory_order_relaxed);
}

size_t RegionManager::GetCurrentHeapSize() const
{
    return heapSpace_->GetCurrentHeapSize();
}

size_t RegionManager::GetMaxHeapSize() const
{
    return heapSpace_->GetMaxHeapSize();
}
}  // namespace ark::common_vm
