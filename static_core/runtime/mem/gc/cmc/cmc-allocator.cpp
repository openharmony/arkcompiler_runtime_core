/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/runtime.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/gc/cmc/cmc-allocator.h"
#include "runtime/mem/runslots_allocator-inl.h"
#if defined(ARK_USE_COMMON_RUNTIME)
#include "common_interfaces/heap/heap_allocator.h"
#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/base_state_word.h"
#include "common_components/heap/heap.h"
#include "common_components/common/page_pool.h"
#include "common_components/common_runtime/base_runtime_param.h"
#include "common_components/heap/heap_manager.h"
#endif

namespace ark::mem {

namespace cvm = common_vm;

template <MTModeT MT_MODE>
CMCObjectAllocator<MT_MODE>::CMCObjectAllocator(MemStatsType *memStats, bool createPygoteSpaceAllocator)
    : ObjectAllocatorNoGen<MT_MODE>(memStats, createPygoteSpaceAllocator)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    auto param = cvm::BaseRuntimeParam::DefaultRuntimeParam();
    // Single pass compaction should be enabled explicitly
    auto &runtimeOptions = Runtime::GetCurrent()->GetOptions();
    param.gcParam.singlePassCompactionEnabled =
        runtimeOptions.WasSetG1SinglePassCompactionEnabled() && runtimeOptions.IsG1SinglePassCompactionEnabled();

    size_t pagePoolSize = param.heapParam.heapSize;
#if defined(PANDA_TARGET_32)
    pagePoolSize = pagePoolSize / 128;  // 128 means divided.
#endif
    cvm::PagePool::Instance().Init(pagePoolSize * cvm::KB / cvm::COMMON_PAGE_SIZE);

    heapManager_ = new (std::nothrow) cvm::HeapManager();
    LOG_IF(UNLIKELY(heapManager_ == nullptr), FATAL, RUNTIME) << "HeapManager instance creation failed.";
    heapManager_->Init(param);

    constexpr int logFloatingPointPrecision = 2;
    LOG(DEBUG, GC) << "Arkcommon runtime started.";
    // Record runtime parameter to report. heap growth value needs to plus 1.
    LOG(DEBUG, GC) << "Runtime parameter:\n\tHeap size: " << pagePoolSize
                   << "(KB)\n\tRegion size: " << param.heapParam.regionSize
                   << "(KB)\n\tExemption threshold: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param.heapParam.exemptionThreshold << "\n\t"
                   << "Heap utilization: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param.heapParam.heapUtilization << "\n\tHeap growth: " << std::fixed
                   << std::setprecision(logFloatingPointPrecision) << 1 + param.heapParam.heapGrowth
                   << "\n\tAllocation rate: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param.heapParam.allocationRate << "(MB/s)\n\t"
                   << "Alloction wait time: " << param.heapParam.allocationWaitTime << "ns\n\t"
                   << "GC Threshold: " << param.gcParam.gcThreshold / cvm::KB
                   << "(KB)\n\tGarbage threshold: " << std::fixed << std::setprecision(logFloatingPointPrecision)
                   << param.gcParam.garbageThreshold
                   << "\n\tGC interval: " << param.gcParam.gcInterval / cvm::MILLI_SECOND_TO_NANO_SECOND
                   << "ms\n\tBackup GC interval: " << param.gcParam.backupGCInterval / cvm::SECOND_TO_NANO_SECOND
                   << "s\n\t"
                   << "Log level: " << 0 << "\n\tThread stack size: " << 0 << "(KB)\n\tArkcommon stack size: " << 0
                   << "(KB)\n\t"
                   << "Processor number: " << 0;
#endif
}

#if defined(ARK_USE_COMMON_RUNTIME)
template <MTModeT MT_MODE>
CMCObjectAllocator<MT_MODE>::~CMCObjectAllocator()
{
    // since there might be failure during initialization,
    // here we need to check and call fini.
    if (heapManager_ != nullptr) {
        heapManager_->Fini();
    }
    delete heapManager_;
    heapManager_ = nullptr;

    cvm::PagePool::Instance().Fini();
}
#endif

template <MTModeT MT_MODE>
void *CMCObjectAllocator<MT_MODE>::Allocate([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align,
                                            [[maybe_unused]] ark::ManagedThread *thread,
                                            [[maybe_unused]] ObjectAllocatorBase::ObjMemInitPolicy objInit,
                                            [[maybe_unused]] bool pinned)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    return reinterpret_cast<void *>(ark::common_vm::HeapAllocator::AllocateInYoungOrHuge(size));
#else
    return nullptr;
#endif
}

template <MTModeT MT_MODE>
void *CMCObjectAllocator<MT_MODE>::AllocateNonMovable([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align,
                                                      [[maybe_unused]] ark::ManagedThread *thread,
                                                      [[maybe_unused]] ObjectAllocatorBase::ObjMemInitPolicy objInit)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    return reinterpret_cast<ObjectHeader *>(ark::common_vm::HeapAllocator::AllocateInNonmoveOrHuge(size));
#else
    return nullptr;
#endif
}

template <MTModeT MT_MODE>
void CMCObjectAllocator<MT_MODE>::IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    auto visitor = [&](ark::common_vm::BaseObject *obj) { objectVisitor(reinterpret_cast<ObjectHeader *>(obj)); };
    ark::common_vm::Heap::GetHeap().ForEachObject(visitor, true);
#endif  // ARK_USE_COMMON_RUNTIME
}

template <MTModeT MT_MODE>
TLAB *CMCObjectAllocator<MT_MODE>::CreateNewTLAB([[maybe_unused]] size_t size)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    return reinterpret_cast<TLAB *>(ark::common_vm::HeapAllocator::CreateTLAB());
#else
    return nullptr;
#endif
}

template <MTModeT MT_MODE>
size_t CMCObjectAllocator<MT_MODE>::GetTLABMaxAllocSize()
{
#if defined(ARK_USE_COMMON_RUNTIME)
    return ark::common_vm::HeapAllocator::GetTLABMaxAllocSize();
#else
    return 0;
#endif
}

template class CMCObjectAllocator<MT_MODE_SINGLE>;
template class CMCObjectAllocator<MT_MODE_MULTI>;
template class CMCObjectAllocator<MT_MODE_TASK>;

}  // namespace ark::mem
