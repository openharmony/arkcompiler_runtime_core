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

#include "common_components/heap/allocator/regional_heap.h"

#include "common_components/heap/collector/collector.h"
#include "common_components/heap/collector/collector_resources.h"
#include "common_components/platform/os.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "common_components/common/scoped_object_access.h"
#include "common_components/heap/heap.h"

namespace common_vm {
template <AllocBufferType type>
RegionDesc *RegionalHeap::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    if constexpr (type == AllocBufferType::TO) {
        return toSpace_.AllocateThreadLocalRegion(expectPhysicalMem);
    } else if constexpr (type == AllocBufferType::YOUNG) {
        return youngSpace_.AllocateThreadLocalRegion(expectPhysicalMem);
    } else if constexpr (type == AllocBufferType::OLD) {
        return oldSpace_.AllocateThreadLocalRegion(expectPhysicalMem);
    }
}

// used to dump a brief summary of all regions.
void RegionalHeap::DumpAllRegionSummary(const char *msg) const
{
    auto from = fromSpace_.GetAllocatedSize();
    auto exempt = fromSpace_.GetSurvivedSize();
    auto to = toSpace_.GetAllocatedSize();
    auto young = youngSpace_.GetAllocatedSize();
    auto old = oldSpace_.GetAllocatedSize();
    auto other = nonMovableSpace_.GetAllocatedSize() + largeSpace_.GetAllocatedSize();

    std::ostringstream oss;
    oss << msg << "Current allocated: " << Pretty(from + to + young + old + other) << ". (from: " << Pretty(from)
        << "(exempt: " << Pretty(exempt) << "), to: " << Pretty(to) << ", young: " << Pretty(young)
        << ", old: " << Pretty(old) << ", other: " << Pretty(other) << ")";
    LOG(DEBUG, GC) << oss.str();
}

// used to dump a detailed information of all regions.
void RegionalHeap::DumpAllRegionStats(const char *msg) const
{
    LOG(DEBUG, GC) << msg;
    youngSpace_.DumpRegionStats();
    oldSpace_.DumpRegionStats();
    fromSpace_.DumpRegionStats();
    toSpace_.DumpRegionStats();
    nonMovableSpace_.DumpRegionStats();
    largeSpace_.DumpRegionStats();

    regionManager_.DumpRegionStats();

    size_t usedUnits = GetUsedUnitCount();
    LOG(DEBUG, GC) << "\tused units: " << usedUnits << " (" << usedUnits * RegionDesc::UNIT_SIZE << " B)";
}

HeapAddress RegionalHeap::TryAllocateOnce(size_t allocSize, AllocType allocType)
{
    if (UNLIKELY(allocSize >= RegionDesc::LARGE_OBJECT_DEFAULT_THRESHOLD)) {
        return largeSpace_.Alloc(allocSize);
    }
    if (UNLIKELY(allocType == AllocType::NONMOVABLE_OBJECT)) {
        return nonMovableSpace_.Alloc(allocSize);
    }
    AllocationBuffer *allocBuffer = AllocationBuffer::GetOrCreateAllocBuffer();
    return allocBuffer->Allocate(allocSize, allocType);
}

bool RegionalHeap::ShouldRetryAllocation(size_t &tryTimes) const
{
    {
        // check safepoint
        ScopedEnterSaferegion enterSaferegion(true);
    }

    if (!IsRuntimeThread() && tryTimes <= static_cast<size_t>(TryAllocationThreshold::RESCHEDULE)) {
        return true;
    } else if (tryTimes < static_cast<size_t>(TryAllocationThreshold::TRIGGER_OOM)) {
        if (Heap::GetHeap().IsGcStarted()) {
            ScopedEnterSaferegion enterSaferegion(false);
            Heap::GetHeap().GetCollectorResources().WaitForGCFinish();
        } else {
            Heap::GetHeap().GetCollector().RequestGC(GC_REASON_HEU, false, GC_TYPE_FULL);
        }
        return true;
    } else if (tryTimes == static_cast<size_t>(TryAllocationThreshold::TRIGGER_OOM)) {
        if (!Heap::GetHeap().IsGcStarted()) {
            LOG(INFO, GC) << "gc is triggered for OOM";
            Heap::GetHeap().GetCollector().RequestGC(GC_REASON_OOM, false, GC_TYPE_FULL);
        } else {
            ScopedEnterSaferegion enterSaferegion(false);
            Heap::GetHeap().GetCollectorResources().WaitForGCFinish();
            tryTimes--;
        }
        return true;
    } else {  // LCOV_EXCL_BR_LINE
        Heap::throwOOM();
        return false;
    }
}

TLAB *RegionalHeap::CreateTLAB()
{
    AllocationBuffer *allocBuffer = AllocationBuffer::GetOrCreateAllocBuffer();
    RegionDesc *currRegion = allocBuffer->GetRegion();
    if (currRegion != RegionDesc::NullRegion()) {
        currRegion->SetRegionAllocPtr(allocBuffer->GetTLAB().allocPtr_);
        currRegion->DetachTLAB();
        this->HandleFullThreadLocalRegion<AllocBufferType::YOUNG>(currRegion);
    }
    RegionDesc *region = this->AllocateThreadLocalRegion<AllocBufferType::YOUNG>();
    if (UNLIKELY(region == nullptr)) {
        allocBuffer->SetRegion(RegionDesc::NullRegion());
        return nullptr;
    }

    region->AttachTLAB(&allocBuffer->GetTLAB());
    allocBuffer->SetRegion(region);

    return &allocBuffer->GetTLAB();
}

uintptr_t RegionalHeap::AllocOldRegion()
{
    return oldSpace_.AllocFullRegion();
}

uintptr_t RegionalHeap::AllocateNonMovableRegion()
{
    return nonMovableSpace_.AllocFullRegion();
}

uintptr_t RegionalHeap::AllocLargeRegion(size_t size)
{
    return largeSpace_.Alloc(size, false);
}

uintptr_t RegionalHeap::AllocJitFortRegion(size_t size)
{
    uintptr_t addr = largeSpace_.Alloc(size, false);
    os::PrctlSetVMA(reinterpret_cast<void *>(addr), size, "ArkTS Code");
    MarkJitFortMemAwaitingInstall(reinterpret_cast<BaseObject *>(addr));
    return addr;
}

HeapAddress RegionalHeap::Allocate(size_t size, AllocType allocType)
{
    size_t tryTimes = 0;
    uintptr_t internalAddr = 0;
    size_t allocSize = ToAllocatedSize(size);
    do {
        tryTimes++;
        internalAddr = TryAllocateOnce(allocSize, allocType);
        if (LIKELY(internalAddr != 0)) {
            break;
        }
        if (IsGcThread()) {
            return 0;  // it means gc doesn't have enough space to move this object.
        }
        if (!ShouldRetryAllocation(tryTimes)) {
            break;
        }
        (void)sched_yield();
    } while (true);
    if (internalAddr == 0) {
        return 0;
    }
    // Increments value of footprintBytes_ counter of heap usage on each non-TL allocation.
    // TL allocations don't call OnAllocate, so RecalculateFootprint is used for that
    // (see MarkingCollector::RunGarbageCollection, call Heap::GetHeap().GetAllocator().RecalculateFootprint)
    OnAllocate(allocSize);
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanAllocObject(reinterpret_cast<void *>(internalAddr), allocSize);
#endif
    return internalAddr + HEADER_SIZE;
}

// Only used for serialization in which allocType and target memory should keep consistency.
HeapAddress RegionalHeap::AllocateNoGC(size_t size, AllocType allocType)
{
    bool allowGC = false;
    uintptr_t internalAddr = 0;
    size_t allocSize = ToAllocatedSize(size);
    if (UNLIKELY(allocType == AllocType::NONMOVABLE_OBJECT)) {
        internalAddr = nonMovableSpace_.Alloc(allocSize, allowGC);
    } else if (LIKELY(allocType == AllocType::MOVEABLE_OBJECT || allocType == AllocType::MOVEABLE_OLD_OBJECT)) {
        AllocationBuffer *allocBuffer = AllocationBuffer::GetOrCreateAllocBuffer();
        internalAddr = allocBuffer->Allocate(allocSize, allocType);
    } else {  // LCOV_EXCL_BR_LINE
        // Unreachable for serialization
        UNREACHABLE();
    }
    if (internalAddr == 0) {
        return 0;
    }
    OnAllocate(allocSize);  // The same usage as in RegionalHeap::Allocate
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanAllocObject(reinterpret_cast<void *>(internalAddr), allocSize);
#endif
    return internalAddr + HEADER_SIZE;
}

void RegionalHeap::CopyRegion(RegionDesc *region)
{
    LOG_IF(UNLIKELY(!region->IsFromRegion()), FATAL, MM_OBJECT_EVENTS)
        << "region type " << static_cast<size_t>(region->GetRegionType());
    LOG(DEBUG, GC) << "try forward region " << region << " @0x" << std::hex << region->GetRegionStart() << "+"
                   << std::dec << region->GetRegionAllocatedSize() << " type "
                   << static_cast<size_t>(region->GetRegionType()) << ", live bytes " << region->GetLiveByteCount();

    if (region->GetLiveByteCount() == 0) {
        return;
    }

    Collector &collector = Heap::GetHeap().GetCollector();
    bool forwarded =
        region->VisitLiveObjectsUntilFalse([&collector](BaseObject *obj) { return collector.ForwardObject(obj); });
    if (!forwarded) {
        LOG(DEBUG, GC) << "failure to forward region " << region << " @0x" << std::hex << region->GetRegionStart()
                       << "+" << std::dec << region->GetRegionAllocatedSize() << " units[" << region->GetUnitIdx()
                       << "+" << region->GetUnitCount() << ", " << region->GetUnitIdx() + region->GetUnitCount()
                       << ") type " << static_cast<size_t>(region->GetRegionType()) << ", "
                       << region->GetLiveByteCount() << " live bytes";
        fromSpace_.DeleteFromRegion(region);
        // since this region is possibly partially-forwarded, treat it as to-region.
        toSpace_.AddFullRegion(region);
        RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(region)->SetCollectionSetRegionFlag(false);
    }
}

void RegionalHeap::Init(const RuntimeParam &param)
{
    MemoryMap::Option opt = MemoryMap::DEFAULT_OPTIONS;
    opt.tag = "region_heap";
    size_t heapSize = param.heapParam.heapSize * KB;

#ifndef PANDA_TARGET_32
    static constexpr uint64_t MAX_SUPPORT_CAPACITY = 4ULL * GB;
    // 2: double heap size
    LOG_IF(UNLIKELY((heapSize / 2) > MAX_SUPPORT_CAPACITY), FATAL, MM_OBJECT_EVENTS) << "Max support capacity 4G";
#endif

    size_t totalSize = RegionManager::GetHeapMemorySize(heapSize);
    size_t regionNum = RegionManager::GetHeapUnitCount(heapSize);
#if defined(COMMON_ASAN_SUPPORT)
    // asan's memory alias technique needs a shareable page
    opt.flags &= ~MAP_PRIVATE;
    opt.flags |= MAP_SHARED;
    LOG(DEBUG, GC) << "mmap flags set to 0x" << std::hex << opt.flags << std::dec;
#endif
    // this must succeed otherwise it won't return
    map_ = MemoryMap::MapMemoryAlignInner4G(totalSize, totalSize, opt);

    size_t metadataSize = RegionManager::GetMetadataSize(regionNum);
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(map_->GetBaseAddr());
    os::PrctlSetVMA(reinterpret_cast<void *>(baseAddr), metadataSize, "ArkTS Heap CMCGC Metadata");
    os::PrctlSetVMA(reinterpret_cast<void *>(baseAddr + metadataSize), totalSize - metadataSize,
                    "ArkTS Heap CMCGC RegionHeap");

#if defined(COMMON_SANITIZER_SUPPORT)
    Sanitizer::OnHeapAllocated(map->GetBaseAddr(), map->GetMappedSize());
#endif

    HeapAddress metadata = reinterpret_cast<HeapAddress>(map_->GetBaseAddr());
    fromSpace_.SetExemptedRegionThreshold(param.heapParam.exemptionThreshold);
    regionManager_.Initialize(regionNum, metadata);
    reservedStart_ = regionManager_.GetRegionHeapStart();
    reservedEnd_ = reinterpret_cast<HeapAddress>(map_->GetMappedEndAddr());
#if defined(COMMON_DUMP_ADDRESS)
    LOG(DEBUG, GC) << "region metadata@0x" << std::hex << metadata << std::dec << ", heap @[0x" << std::hex
                   << reservedStart << "+" << std::dec << (reservedEnd - reservedStart) << ", 0x" << std::hex
                   << reservedEnd << std::dec << ")";
#endif
    Heap::OnHeapCreated(reservedStart_);
    Heap::OnHeapExtended(reservedEnd_);
}

AllocationBuffer *AllocationBuffer::GetOrCreateAllocBuffer()
{
    auto *buffer = AllocationBuffer::GetAllocBuffer();
    if (buffer == nullptr) {
        buffer = new (std::nothrow) AllocationBuffer();
        LOG_IF(UNLIKELY(buffer == nullptr), FATAL, MM_OBJECT_EVENTS) << "new region alloc buffer fail";
        buffer->Init();
        ThreadLocal::SetAllocBuffer(buffer);
    }
    return buffer;
}
void AllocationBuffer::ClearThreadLocalRegion()
{
    if (LIKELY(tlRegion_ != RegionDesc::NullRegion())) {
        RegionalHeap &heap = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
        tlRegion_->SetRegionAllocPtr(GetTLAB().allocPtr_);
        tlRegion_->DetachTLAB();
        heap.HandleFullThreadLocalRegion<AllocBufferType::YOUNG>(tlRegion_);
        tlRegion_ = RegionDesc::NullRegion();
    }
    if (LIKELY(tlOldRegion_ != RegionDesc::NullRegion())) {
        RegionalHeap &heap = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
        heap.HandleFullThreadLocalRegion<AllocBufferType::OLD>(tlOldRegion_);
        tlOldRegion_ = RegionDesc::NullRegion();
    }
    if (LIKELY(tlToRegion_ != RegionDesc::NullRegion())) {
        RegionalHeap &heap = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
        heap.HandleFullThreadLocalRegion<AllocBufferType::TO>(tlToRegion_);
        tlToRegion_ = RegionDesc::NullRegion();
    }
}

void AllocationBuffer::Unregister()
{
    // Null the pointer, the region stays in ToSpace::tlToRegionList_ and will be promoted by GC thread
    // This is needed to prevent race between mutator and GC threads
    tlToRegion_ = RegionDesc::NullRegion();

    Heap::GetHeap().UnregisterAllocBuffer(*this);
}

AllocationBuffer *AllocationBuffer::GetAllocBuffer()
{
    return ThreadLocal::GetAllocBuffer();
}

AllocationBuffer::~AllocationBuffer()
{
    ClearThreadLocalRegion();
}

void AllocationBuffer::Init()
{
    static_assert(MEMBER_OFFSET(AllocationBuffer, tlRegion_) == 0,
                  "need to modify the offset of this value in llvm-project at the same time");
    tlRegion_ = RegionDesc::NullRegion();
    tlOldRegion_ = RegionDesc::NullRegion();
    Heap::GetHeap().RegisterAllocBuffer(*this);
}

HeapAddress AllocationBuffer::ToSpaceAllocate(size_t totalSize)
{
    HeapAddress addr = 0;
    if (LIKELY(tlToRegion_ != RegionDesc::NullRegion())) {
        addr = tlToRegion_->Alloc(totalSize);
    }

    if (UNLIKELY(addr == 0)) {
        RegionalHeap &heapSpace = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());

        heapSpace.HandleFullThreadLocalRegion<AllocBufferType::TO>(tlToRegion_);
        tlToRegion_ = RegionDesc::NullRegion();

        RegionDesc *r = heapSpace.AllocateThreadLocalRegion<AllocBufferType::TO>(false);
        if (UNLIKELY(r == nullptr)) {
            return 0;
        }

        tlToRegion_ = r;
        addr = tlToRegion_->Alloc(totalSize);
    }

    LOG(DEBUG, GC) << "alloc to 0x" << std::hex << addr << std::dec << "(" << totalSize << ")";
    return addr;
}

HeapAddress AllocationBuffer::Allocate(size_t totalSize, AllocType allocType)
{
    // a hoisted specific fast path which can be inlined
    HeapAddress addr = 0;

    ASSERT_PRINT(allocType == AllocType::MOVEABLE_OBJECT || allocType == AllocType::MOVEABLE_OLD_OBJECT,
                 "unexpected alloc type");

    if (allocType == AllocType::MOVEABLE_OBJECT) {
        if (LIKELY(tlRegion_ != RegionDesc::NullRegion())) {
            addr = tlab_.Alloc(totalSize);
        }
    } else if (allocType == AllocType::MOVEABLE_OLD_OBJECT) {
        if (LIKELY(tlOldRegion_ != RegionDesc::NullRegion())) {
            addr = tlOldRegion_->Alloc(totalSize);
        }
    }

    if (UNLIKELY(addr == 0)) {
        addr = AllocateImpl(totalSize, allocType);
    }

    LOG(DEBUG, GC) << "alloc 0x" << std::hex << addr << std::dec << "(" << totalSize << ")";
    return addr;
}

// try an allocation but do not handle failure
HeapAddress AllocationBuffer::AllocateImpl(size_t totalSize, AllocType allocType)
{
    RegionalHeap &heapSpace = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());

    // allocate new thread local region and try alloc
    if (allocType == AllocType::MOVEABLE_OBJECT) {
        if (tlRegion_->IsTLABAttached()) {
            tlRegion_->SetRegionAllocPtr(tlab_.allocPtr_);
            tlRegion_->DetachTLAB();
        }
        heapSpace.HandleFullThreadLocalRegion<AllocBufferType::YOUNG>(tlRegion_);
        tlRegion_ = RegionDesc::NullRegion();

        RegionDesc *r = heapSpace.AllocateThreadLocalRegion<AllocBufferType::YOUNG>(false);
        if (UNLIKELY(r == nullptr)) {
            return 0;
        }

        tlRegion_ = r;
        tlRegion_->AttachTLAB(&tlab_);
        return tlab_.Alloc(totalSize);
    } else if (allocType == AllocType::MOVEABLE_OLD_OBJECT) {
        heapSpace.HandleFullThreadLocalRegion<AllocBufferType::OLD>(tlOldRegion_);
        tlOldRegion_ = RegionDesc::NullRegion();

        RegionDesc *r = heapSpace.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
        if (UNLIKELY(r == nullptr)) {
            return 0;
        }

        tlOldRegion_ = r;
        return tlOldRegion_->Alloc(totalSize);
    }
    UNREACHABLE();
}

#ifndef NDEBUG
bool RegionalHeap::IsHeapObject(HeapAddress addr) const
{
    if (!IsHeapAddress(addr)) {
        return false;
    }
    return true;
}
#endif
void RegionalHeap::FeedHungryBuffers()
{
    ScopedObjectAccess soa;
    AllocBufferManager::HungryBuffers hungryBuffers;
    allocBufferManager_->SwapHungryBuffers(hungryBuffers);
    RegionDesc *region = nullptr;
    for (auto *buffer : hungryBuffers) {
        if (buffer->GetPreparedRegion() != nullptr) {
            continue;
        }
        if (region == nullptr) {
            region = AllocateThreadLocalRegion<AllocBufferType::YOUNG>(true);
            if (region == nullptr) {
                return;
            }
        }
        if (buffer->SetPreparedRegion(region)) {
            region = nullptr;
        }
    }
    if (region != nullptr) {
        regionManager_.CollectRegion(region);
    }
}

void RegionalHeap::MarkRememberSet(const std::function<void(BaseObject *)> &func)
{
    oldSpace_.MarkRememberSet(func);
    nonMovableSpace_.MarkRememberSet(func);
    largeSpace_.MarkRememberSet(func);
}

void RegionalHeap::ForEachAwaitingJitFortUnsafe(const std::function<void(BaseObject *)> &visitor) const
{
    DCHECK(BaseRuntime::GetInstance()->GetMutatorManager().WorldStopped());
    for (const auto jitFort : awaitingJitFort_) {
        visitor(jitFort);
    }
}

void RegionalHeap::MarkJitFortMemInstalled(void *thread, BaseObject *obj)
{
    ark::os::memory::LockHolder guard(awaitingJitFortMutex_);
    // GC is running, we should mark JitFort installled after GC finish
    if (Heap::GetHeap().GetGCPhase() != GCPhase::GC_PHASE_IDLE) {
        jitFortPostGCInstallTask_.emplace(nullptr, obj);
    } else {
        // a threadlocal JitFort mem
        if (!thread) {
            RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj))->SetJitFortAwaitInstallFlag(false);
        }
        awaitingJitFort_.erase(obj);
    }
}

void RegionalHeap::MarkJitFortMemAwaitingInstall(BaseObject *obj)
{
    ark::os::memory::LockHolder guard(awaitingJitFortMutex_);
    RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj))->SetJitFortAwaitInstallFlag(true);
    awaitingJitFort_.insert(obj);
}

void RegionalHeap::HandlePostGCJitFortInstallTask()
{
    DCHECK(Heap::GetHeap().GetGCPhase() == GCPhase::GC_PHASE_IDLE);
    while (!jitFortPostGCInstallTask_.empty()) {
        auto [thread, machineCode] = jitFortPostGCInstallTask_.top();
        MarkJitFortMemInstalled(thread, machineCode);
        jitFortPostGCInstallTask_.pop();
    }
}

}  // namespace common_vm
