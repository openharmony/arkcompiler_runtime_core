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

#include "common_interfaces/base_runtime.h"

#include "common_components/common_runtime/base_runtime_param.h"
#include "common_components/log/log.h"
#include "common_components/common/page_pool.h"
#include "common_components/common/scoped_object_access.h"
#include "common_components/heap/collector/heuristic_gc_policy.h"
#include "common_components/heap/heap.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/mutator/mutator_manager.h"

#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/logger.h"

#include <iomanip>

namespace ark::common_vm {

ark::os::memory::Mutex BaseRuntime::vmCreationLock_;
BaseRuntime *BaseRuntime::baseRuntimeInstance_ = nullptr;
bool BaseRuntime::initialized_ = false;

BaseRuntime *BaseRuntime::GetInstance()
{
    if (UNLIKELY(baseRuntimeInstance_ == nullptr)) {
        ark::os::memory::LockHolder lock(vmCreationLock_);
        if (baseRuntimeInstance_ == nullptr) {
            baseRuntimeInstance_ = new BaseRuntime();
        }
    }
    return baseRuntimeInstance_;
}

void BaseRuntime::DestroyInstance()
{
    ark::os::memory::LockHolder lock(vmCreationLock_);
    delete baseRuntimeInstance_;
    baseRuntimeInstance_ = nullptr;
}

template <typename T>
inline T *NewAndInit()
{
    T *temp = new (std::nothrow) T();
    LOG_IF(UNLIKELY(temp == nullptr), FATAL, RUNTIME) << "NewAndInit failed";
    temp->Init();
    return temp;
}

template <typename T, typename A>
inline T *NewAndInit(A arg)
{
    T *temp = new (std::nothrow) T();
    LOG_IF(UNLIKELY(temp == nullptr), FATAL, RUNTIME) << "NewAndInit failed";
    temp->Init(arg);
    return temp;
}

template <typename T>
inline void CheckAndFini(T *&module)
{
    if (module != nullptr) {
        module->Fini();
    }

    delete module;
    module = nullptr;
}

bool BaseRuntime::HasBeenInitialized()
{
    ark::os::memory::LockHolder lock(vmCreationLock_);
    return initialized_;
}

RuntimeParam BaseRuntime::GetDefaultParam()
{
    return BaseRuntimeParam::DefaultRuntimeParam();
}

void BaseRuntime::Init()
{
    Init(BaseRuntimeParam::DefaultRuntimeParam());
}

void BaseRuntime::Init(const RuntimeParam &param)
{
    ark::os::memory::LockHolder lock(vmCreationLock_);
    if (initialized_) {
        LOG(FATAL, COMMON) << "BaseRuntime has been initialized and don't need init again.";
        return;
    }

    param_ = param;
    size_t pagePoolSize = param_.heapParam.heapSize;
#if defined(PANDA_TARGET_32)
    pagePoolSize = pagePoolSize / 128;  // 128 means divided.
#endif
    PagePool::Instance().Init(pagePoolSize * KB / COMMON_PAGE_SIZE);
    mutatorManager_ = NewAndInit<MutatorManager>();
    heapManager_ = NewAndInit<HeapManager>(param_);

    [[maybe_unused]] constexpr int logFloatingPointPrecision = 2;
    LOG(DEBUG, GC) << "Arkcommon runtime started.";
    // Record runtime parameter to report. heap growth value needs to plus 1.
    LOG(DEBUG, GC) << "Runtime parameter:\n\tHeap size: " << pagePoolSize
                   << "(KB)\n\tRegion size: " << param_.heapParam.regionSize
                   << "(KB)\n\tExemption threshold: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param_.heapParam.exemptionThreshold << "\n\t"
                   << "Heap utilization: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param_.heapParam.heapUtilization << "\n\tHeap growth: " << std::fixed
                   << std::setprecision(logFloatingPointPrecision) << 1 + param_.heapParam.heapGrowth
                   << "\n\tAllocation rate: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param_.heapParam.allocationRate << "(MB/s)\n\t"
                   << "Alloction wait time: " << param_.heapParam.allocationWaitTime << "ns\n\t"
                   << "GC Threshold: " << param_.gcParam.gcThreshold / KB << "(KB)\n\tGarbage threshold: " << std::fixed
                   << std::setprecision(logFloatingPointPrecision) << param_.gcParam.garbageThreshold
                   << "\n\tGC interval: " << param_.gcParam.gcInterval / MILLI_SECOND_TO_NANO_SECOND
                   << "ms\n\tBackup GC interval: " << param_.gcParam.backupGCInterval / SECOND_TO_NANO_SECOND << "s\n\t"
                   << "Log level: " << 0 << "\n\tThread stack size: " << 0 << "(KB)\n\tArkcommon stack size: " << 0
                   << "(KB)\n\t"
                   << "Processor number: " << 0;

    initialized_ = true;
}

void BaseRuntime::Fini()
{
    ark::os::memory::LockHolder lock(vmCreationLock_);
    if (!initialized_) {
        LOG(FATAL, COMMON) << "BaseRuntime has been initialized and don't need init again.";
        return;
    }

    {
        // since there might be failure during initialization,
        // here we need to check and call fini.
        CheckAndFini<HeapManager>(heapManager_);
        CheckAndFini<MutatorManager>(mutatorManager_);
        PagePool::Instance().Fini();
    }

    LOG(DEBUG, GC) << "Arkcommon runtime shutdown.";
    initialized_ = false;
}

void BaseRuntime::SetMutatorLock(ark::MutatorLock *l)
{
    mutatorManager_->SetMutatorLock(l);
}

bool BaseRuntime::RegisterVM(VMInterface *vm)
{
    ark::os::memory::WriteLockHolder vmIfacesWriteLock(vmIfacesLock_);
    auto res = vmIfaces_.insert(vm);
    return res.second;
}

bool BaseRuntime::UnregisterVM(VMInterface *vm)
{
    ark::os::memory::WriteLockHolder vmIfacesWriteLock(vmIfacesLock_);
    auto erased = vmIfaces_.erase(vm);
    return erased != 0;
}

void BaseRuntime::ForEachVM(std::function<void(VMInterface *)> action)
{
    ark::os::memory::ReadLockHolder vmIfacesReadLock(vmIfacesLock_);
    std::for_each(vmIfaces_.begin(), vmIfaces_.end(), action);
}

void BaseRuntime::AddGCListener(GCListener *listener)
{
    Heap::GetHeap().GetCollector().AddGCListener(listener);
}

void BaseRuntime::RemoveGCListener(GCListener *listener)
{
    Heap::GetHeap().GetCollector().RemoveGCListener(listener);
}

void BaseRuntime::PreFork(Mutator *mutator)
{
    RequestGC(GC_REASON_USER, false, GC_TYPE_FULL);
    {
        common_vm::ScopedEnterSaferegion enterSaferegion(true);
        HeapManager::StopRuntimeThreads();
    }
}

void BaseRuntime::PostFork([[maybe_unused]] bool enableWarmStartup)
{
    HeapManager::StartRuntimeThreads();
#ifdef ENABLE_COLD_STARTUP_GC_POLICY
    if (!enableWarmStartup) {
        StartupStatusManager::OnAppStartup();
    }
#endif
}

void BaseRuntime::NotifyWarmStart()
{
    if (!Heap::GetHeap().IsGcStarted() && !Heap::GetHeap().OnStartupEvent()) {
        StartupStatusManager::OnAppStartup();
    }
}

void BaseRuntime::WriteBarrier(void *obj, void *field, void *ref, Mutator *mutator)
{
    DCHECK(field != nullptr);
    Heap::GetBarrier().WriteBarrier(mutator, reinterpret_cast<BaseObject *>(obj),
                                    *reinterpret_cast<RefField<> *>(field), reinterpret_cast<BaseObject *>(ref));
}

void BaseRuntime::PreWriteBarrier(void *preVal, Mutator *mutator)
{
    Heap::GetBarrier().PreWriteBarrier(mutator, reinterpret_cast<BaseObject *>(preVal));
}

void *BaseRuntime::ReadBarrier(void *obj, void *field)
{
    return reinterpret_cast<void *>(Heap::GetBarrier().ReadRefField(reinterpret_cast<BaseObject *>(obj),
                                                                    *reinterpret_cast<RefField<false> *>(field)));
}

void *BaseRuntime::ReadBarrier(void **field)
{
    return reinterpret_cast<void *>(Heap::GetBarrier().ReadStaticRef(reinterpret_cast<RefField<false> &>(*field)));
}

void *BaseRuntime::AtomicReadBarrier(void *obj, void *field, std::memory_order order)
{
    return reinterpret_cast<void *>(Heap::GetBarrier().AtomicReadRefField(
        reinterpret_cast<BaseObject *>(obj), *reinterpret_cast<RefField<true> *>(field), order));
}

void BaseRuntime::RequestGC(GCReason reason, bool async, GCType gcType, bool explicitRequest)
{
    if (reason < GC_REASON_BEGIN || reason > GC_REASON_END || gcType < GC_TYPE_BEGIN || gcType > GC_TYPE_END) {
        LOG(ERROR, GC) << "Invalid gc reason or gc type, gc reason: " << GCReasonToString(reason)
                       << ", gc type: " << GCTypeToString(gcType);
        return;
    }
    HeapManager::RequestGC(reason, async, gcType, explicitRequest);
}

void BaseRuntime::WaitForGCFinish()
{
    Heap::GetHeap().WaitForGCFinish();
}

void BaseRuntime::WaitForGCCompletionCount(uint64_t targetCount)
{
    Heap::GetHeap().WaitForGCCompletionCount(targetCount);
}

void BaseRuntime::StopGCWork()
{
    Heap::GetHeap().StopGCWork();
}

uint64_t BaseRuntime::GetGcCompletedCount()
{
    return Heap::GetHeap().GetGcCompletedCount();
}

size_t BaseRuntime::GetFreeHeapSize()
{
    return Heap::GetHeap().GetFreeHeapSize();
}

size_t BaseRuntime::GetUsedHeapSize()
{
    return Heap::GetHeap().GetUsedHeapSize();
}

size_t BaseRuntime::GetReservedHeapSize()
{
    return Heap::GetHeap().GetReservedHeapSize();
}

bool BaseRuntime::IsGcStarted()
{
    return Heap::GetHeap().IsGcStarted();
}

void BaseRuntime::EnterGCCriticalSection()
{
    return Heap::GetHeap().MarkGCStart();
}
void BaseRuntime::ExitGCCriticalSection()
{
    return Heap::GetHeap().MarkGCFinish();
}

bool BaseRuntime::ForEachObj(HeapVisitor &visitor, bool safe)
{
    return Heap::GetHeap().ForEachObject(visitor, safe);
}

void BaseRuntime::NotifyNativeAllocation(size_t bytes)
{
    Heap::GetHeap().NotifyNativeAllocation(bytes);
}

void BaseRuntime::NotifyNativeFree(size_t bytes)
{
    Heap::GetHeap().NotifyNativeFree(bytes);
}

void BaseRuntime::NotifyNativeReset(size_t oldBytes, size_t newBytes)
{
    Heap::GetHeap().NotifyNativeReset(oldBytes, newBytes);
}

size_t BaseRuntime::GetNotifiedNativeSize()
{
    return Heap::GetHeap().GetNotifiedNativeSize();
}

void BaseRuntime::ChangeGCParams(bool isBackground)
{
    return Heap::GetHeap().ChangeGCParams(isBackground);
}

bool BaseRuntime::CheckAndTriggerHintGC(MemoryReduceDegree degree)
{
    return Heap::GetHeap().CheckAndTriggerHintGC(degree);
}

void BaseRuntime::NotifyHighSensitive(bool isStart)
{
    Heap::GetHeap().NotifyHighSensitive(isStart);
}

}  // namespace ark::common_vm
