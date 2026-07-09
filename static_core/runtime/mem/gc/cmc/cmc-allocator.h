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

#ifndef RUNTIME_MEM_GC_CMCGCADAPTER_CMC_ALLOCATOR_ADAPTER_H
#define RUNTIME_MEM_GC_CMCGCADAPTER_CMC_ALLOCATOR_ADAPTER_H

#include "runtime/include/mem/allocator.h"

namespace ark::common_vm {
class HeapManager;
}

namespace ark::mem {
class ObjectAllocConfigWithCrossingMap;
class ObjectAllocConfig;
class TLAB;

class CMCObjectAllocator final : public ObjectAllocatorNoGen {
public:
    NO_MOVE_SEMANTIC(CMCObjectAllocator);
    NO_COPY_SEMANTIC(CMCObjectAllocator);

    explicit CMCObjectAllocator(MemStatsType *memStats, bool createPygoteSpaceAllocator);

#if defined(ARK_USE_COMMON_RUNTIME)
    ~CMCObjectAllocator();
#endif

    [[nodiscard]] void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                 ObjectAllocatorBase::ObjMemInitPolicy objInit, bool pinned) override;

    [[nodiscard]] void *AllocateNonMovable(size_t size, Alignment align, ark::ManagedThread *thread,
                                           ObjectAllocatorBase::ObjMemInitPolicy objInit) override;

    void IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor) override;

    bool IsNonMovable([[maybe_unused]] const ObjectHeader *obj) override
    {
        return false;
    }

    TLAB *CreateNewTLAB(size_t tlabSize) override;

    bool IsTLABSupported() override
    {
        return true;
    }

    size_t GetTLABMaxAllocSize() override;

    double GetCollectionRate() const
    {
        return collectionRate_;
    }

    void SetCollectionRate(double rate)
    {
        collectionRate_ = rate;
    }

    size_t GetHeapThreshold() const
    {
        return heapThreshold_;
    }

    void SetHeapThreshold(size_t threshold)
    {
        heapThreshold_ = threshold;
    }

    size_t GetLiveBytesAfterGC() const
    {
        return liveBytesAfterGC_;
    }

    void SetLiveBytesAfterGC(size_t bytes)
    {
        liveBytesAfterGC_ = bytes;
    }

    bool ShouldRequestYoung() const
    {
        return shouldRequestYoung_;
    }

    void SetShouldRequestYoung(bool value)
    {
        shouldRequestYoung_ = value;
    }

    bool NeedToTrackFreedObjects() const
    {
        return cmcTrackFreedObjects_;
    }

    void SetNeedToTrackFreedObjects(bool value)
    {
        cmcTrackFreedObjects_ = value;
    }

private:
    [[maybe_unused]] common_vm::HeapManager *heapManager_;

    double collectionRate_ {0.0};
    size_t heapThreshold_ {0};
    size_t liveBytesAfterGC_ {0};
    bool shouldRequestYoung_ {false};
    bool cmcTrackFreedObjects_ {false};
};

}  // namespace ark::mem
#endif  // RUNTIME_MEM_GC_CMCGCADAPTER_CMC_ALLOCATOR_ADAPTER_H
