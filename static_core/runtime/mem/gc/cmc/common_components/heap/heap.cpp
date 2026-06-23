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

#include "common_components/heap/heap.h"

#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/collector/collector_resources.h"
#include "common_components/common/scoped_object_access.h"

#if defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#undef ERROR
#endif
#if defined(__APPLE__)
#include <mach/mach.h>
#endif
namespace ark::common_vm {
static_assert(Heap::NORMAL_UNIT_SIZE == RegionDesc::UNIT_SIZE);
static_assert(Heap::NORMAL_UNIT_HEADER_SIZE == RegionDesc::UNIT_HEADER_SIZE);
static_assert(Heap::NORMAL_UNIT_AVAILABLE_SIZE == RegionDesc::UNIT_AVAILABLE_SIZE);

HeapAddress Heap::heapStartAddr_ = 0;
HeapAddress Heap::heapCurrentEnd_ = 0;

class HeapImpl : public Heap {
public:
    HeapImpl() : theSpace_(Allocator::CreateAllocator())
    {
        RunType::InitRunTypeMap();
    }

    ~HeapImpl() override = default;
    void Init(const RuntimeParam &param) override;
    void Fini() override;
    void StartRuntimeThreads() override;
    void StopRuntimeThreads() override;

    bool IsSurvivedObject(const BaseObject *obj) const override
    {
        return RegionalHeap::IsMarkedObject(obj) || RegionalHeap::IsResurrectedObject(obj);
    }

    bool IsGcStarted() const override
    {
        return collectorResources_.IsGcStarted();
    }

    void WaitForGCFinish() override
    {
        return collectorResources_.WaitForGCFinish();
    }

    void WaitForGCCompletionCount(uint64_t targetCount) override
    {
        collectorResources_.WaitForGCCompletionCount(targetCount);
    }

    uint64_t GetGcCompletedCount() const override
    {
        return collectorResources_.GetGcCompletedCount();
    }

    void MarkGCStart() override
    {
        return collectorResources_.MarkGCStart();
    }

    void MarkGCFinish() override
    {
        return collectorResources_.MarkGCFinish();
    }

    bool IsGCEnabled() const override
    {
        // Atomic with seq_cst order reason: data race with isGCEnabled_ with requirement for sequentially consistent
        // order where threads observe all modifications in the same order
        return isGCEnabled_.load(std::memory_order_seq_cst);
    }

    void EnableGC(bool val) override
    {
        // Atomic with seq_cst order reason: data race with isGCEnabled_ with requirement for sequentially consistent
        // order where threads observe all modifications in the same order
        return isGCEnabled_.store(val, std::memory_order_seq_cst);
    }

    GCTaskCause GetGCReason() override
    {
        return gcReason_;
    }

    void SetGCReason(GCTaskCause reason) override
    {
        gcReason_ = reason;
    }

    bool InRecentSpace(const void *addr) override
    {
        RegionDesc *region = RegionDesc::GetRegionDescAt(reinterpret_cast<HeapAddress>(addr));
        return region->IsInRecentSpace();
    }
    bool GetForceThrowOOM() const override
    {
        return isForceThrowOOM_;
    };
    void SetForceThrowOOM(bool val) override
    {
        isForceThrowOOM_ = val;
    };
    void SetCollector(Collector *collector) override
    {
        collectorResources_.SetCollector(collector);
        collector_ = collector;

        collector_->Init();
        collectorResources_.Init();

        Heap::GetHeap().EnableGC(runtimeParam_.gcParam.enableGC);
    }

    const HeapParam &GetHeapParam() const override
    {
        return runtimeParam_.heapParam;
    }

    GCParam &GetGCParam() override
    {
        return runtimeParam_.gcParam;
    }

    HeapAddress Allocate(size_t size, AllocType allocType, bool allowGC = true) override;
    Collector &GetCollector() override;
    Allocator &GetAllocator() override;
    HeuristicGCPolicy &GetHeuristicGCPolicy() override;
    size_t GetMaxCapacity() const override;
    size_t GetCurrentCapacity() const override;
    size_t GetUsedPageSize() const override;
    size_t GetAllocatedSize() const override;
    size_t GetFootprintBytes() const override;
    size_t GetSurvivedSize() const override;
    size_t GetRemainHeapSize() const override;
    size_t GetAccumulatedAllocateSize() const override;
    size_t GetAccumulatedFreeSize() const override;
    HeapAddress GetStartAddress() const override;
    HeapAddress GetSpaceEndAddress() const override;
    bool ForEachObject(const std::function<void(BaseObject *)> &, bool) override;
    CollectorResources &GetCollectorResources() override;
    void RegisterAllocBuffer(AllocationBuffer &buffer) override;
    void UnregisterAllocBuffer(AllocationBuffer &buffer) override;
    void StopGCWork() override;
    void TryHeuristicGC() override;
    void NotifyNativeAllocation(size_t bytes) override;
    void NotifyNativeFree(size_t bytes) override;
    void NotifyNativeReset(size_t oldBytes, size_t newBytes) override;
    size_t GetNotifiedNativeSize() const override;
    void SetNativeHeapThreshold(size_t newThreshold) override;
    size_t GetNativeHeapThreshold() const override;
    void ChangeGCParams(bool isBackground) override;
    void RecordAliveSizeAfterLastGC(size_t aliveBytes) override;
    void NotifyHighSensitive(bool isStart) override;
    void SetRecordHeapObjectSizeBeforeSensitive(size_t objSize) override;
    AppSensitiveStatus GetSensitiveStatus() override;
    StartupStatus GetStartupStatus() override;
    bool OnStartupEvent() const override;

private:
    // allocator is actually a subspace in heap
    Allocator *theSpace_;

    CollectorResources collectorResources_;

    // collector is closely related to barrier. but we do not put barrier inside collector because even without
    // collector (i.e. no-gc), allocator and barrier (interface to access heap) is still needed.
    Collector *collector_;

    HeuristicGCPolicy heuristicGCPolicy_;

    std::atomic<bool> isGCEnabled_ = {true};

    GCTaskCause gcReason_ = GCTaskCause::INVALID_CAUSE;
    bool isForceThrowOOM_ = {false};
    RuntimeParam runtimeParam_;
};  // end class HeapImpl

static ImmortalWrapper<HeapImpl> g_heapInstance;

HeapAddress HeapImpl::Allocate(size_t size, AllocType allocType, bool allowGC)
{
    if (allowGC) {
        return theSpace_->Allocate(size, allocType);
    } else {
        return theSpace_->AllocateNoGC(size, allocType);
    }
}

bool HeapImpl::ForEachObject(const std::function<void(BaseObject *)> &visitor, bool safe)
{
    {
        ScopedEnterSaferegion enterSaferegion(false);
        this->GetCollectorResources().WaitForGCFinish();
    }
    // Expect no gc in ForEachObj, dfx tools and oom gc should be considered.
    return theSpace_->ForEachObject(visitor, safe);
}

void HeapImpl::Init(const RuntimeParam &param)
{
    if (theSpace_ == nullptr) {
        // Hack impl, since HeapImpl is Immortal, this may happen in multi UT case
        new (this) HeapImpl();
    }
    runtimeParam_ = param;
    theSpace_->Init(param);
    heuristicGCPolicy_.Init();
}

void HeapImpl::Fini()
{
    collectorResources_.Fini();
    if (theSpace_ != nullptr) {
        delete theSpace_;
        theSpace_ = nullptr;
    }
}

void HeapImpl::StartRuntimeThreads()
{
    collectorResources_.StartRuntimeThreads();
}

void HeapImpl::StopRuntimeThreads()
{
    collectorResources_.StopRuntimeThreads();
}

HeuristicGCPolicy &HeapImpl::GetHeuristicGCPolicy()
{
    return heuristicGCPolicy_;
}

void HeapImpl::TryHeuristicGC()
{
    heuristicGCPolicy_.TryHeuristicGC();
}

void HeapImpl::NotifyNativeAllocation(size_t bytes)
{
    heuristicGCPolicy_.NotifyNativeAllocation(bytes);
}

void HeapImpl::NotifyNativeFree(size_t bytes)
{
    heuristicGCPolicy_.NotifyNativeFree(bytes);
}

void HeapImpl::NotifyNativeReset(size_t oldBytes, size_t newBytes)
{
    heuristicGCPolicy_.NotifyNativeFree(oldBytes);
    heuristicGCPolicy_.NotifyNativeAllocation(newBytes);
}

size_t HeapImpl::GetNotifiedNativeSize() const
{
    return heuristicGCPolicy_.GetNotifiedNativeSize();
}

void HeapImpl::SetNativeHeapThreshold(size_t newThreshold)
{
    heuristicGCPolicy_.SetNativeHeapThreshold(newThreshold);
}

size_t HeapImpl::GetNativeHeapThreshold() const
{
    return heuristicGCPolicy_.GetNativeHeapThreshold();
}

void HeapImpl::ChangeGCParams(bool isBackground)
{
    heuristicGCPolicy_.ChangeGCParams(isBackground);
}

void HeapImpl::RecordAliveSizeAfterLastGC(size_t aliveBytes)
{
    heuristicGCPolicy_.RecordAliveSizeAfterLastGC(aliveBytes);
}

void HeapImpl::NotifyHighSensitive(bool isStart)
{
    heuristicGCPolicy_.NotifyHighSensitive(isStart);
}

void HeapImpl::SetRecordHeapObjectSizeBeforeSensitive(size_t objSize)
{
    if (heuristicGCPolicy_.InSensitiveStatus()) {
        heuristicGCPolicy_.SetRecordHeapObjectSizeBeforeSensitive(objSize);
    }
}

AppSensitiveStatus HeapImpl::GetSensitiveStatus()
{
    return heuristicGCPolicy_.GetSensitiveStatus();
}

StartupStatus HeapImpl::GetStartupStatus()
{
    return heuristicGCPolicy_.GetStartupStatus();
}

bool HeapImpl::OnStartupEvent() const
{
    return heuristicGCPolicy_.OnStartupEvent();
}

Collector &HeapImpl::GetCollector()
{
    return *collector_;
}

Allocator &HeapImpl::GetAllocator()
{
    return *theSpace_;
}

size_t HeapImpl::GetMaxCapacity() const
{
    return theSpace_->GetMaxCapacity();
}

size_t HeapImpl::GetCurrentCapacity() const
{
    return theSpace_->GetCurrentCapacity();
}

size_t HeapImpl::GetUsedPageSize() const
{
    return theSpace_->GetUsedPageSize();
}

size_t HeapImpl::GetAllocatedSize() const
{
    return theSpace_->GetAllocatedBytes();
}

size_t HeapImpl::GetFootprintBytes() const
{
    return theSpace_->GetFootprintBytes();
}

size_t HeapImpl::GetRemainHeapSize() const
{
    return theSpace_->GetMaxCapacity() - theSpace_->GetAllocatedBytes();
}

size_t HeapImpl::GetSurvivedSize() const
{
    return theSpace_->GetSurvivedSize();
}

size_t HeapImpl::GetAccumulatedAllocateSize() const
{
    return collectorResources_.GetGCStats().GetAccumulatedFreeSize() + theSpace_->GetAllocatedBytes();
}

size_t HeapImpl::GetAccumulatedFreeSize() const
{
    return collectorResources_.GetGCStats().GetAccumulatedFreeSize();
}

HeapAddress HeapImpl::GetStartAddress() const
{
    return theSpace_->GetSpaceStartAddress();
}

HeapAddress HeapImpl::GetSpaceEndAddress() const
{
    return theSpace_->GetSpaceEndAddress();
}

Heap &Heap::GetHeap()
{
    return *g_heapInstance;
}

CollectorResources &HeapImpl::GetCollectorResources()
{
    return collectorResources_;
}

void HeapImpl::StopGCWork()
{
    collectorResources_.StopGCWork();
}

void HeapImpl::RegisterAllocBuffer(AllocationBuffer &buffer)
{
    GetAllocator().RegisterAllocBuffer(buffer);
}
void HeapImpl::UnregisterAllocBuffer(AllocationBuffer &buffer)
{
    GetAllocator().UnregisterAllocBuffer(buffer);
}
}  // namespace ark::common_vm
