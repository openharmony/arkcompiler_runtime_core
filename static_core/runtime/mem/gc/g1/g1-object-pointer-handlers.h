/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_OBJECT_POINTER_HANDLER_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_OBJECT_POINTER_HANDLER_H

#include "libpandabase/mem/mem.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_accessor.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/g1/object_ref.h"

namespace ark::mem {

template <typename LanguageConfig>
class G1GC;

template <typename LanguageConfig>
class G1EvacuateRegionsWorkerState;

class RemsetObjectPointerHandler {
public:
    RemsetObjectPointerHandler(Region *fromRegion, size_t regionSizeBits, const std::atomic_bool &deferCards)
        : fromRemset_(fromRegion->GetRemSet()), regionSizeBits_(regionSizeBits), deferCards_(deferCards)
    {
    }

    template <typename T>
    bool ProcessObjectPointer(T *ref) const
    {
        ProcessObjectPointerInternal(ref);
        // Atomic with relaxed order reason: memory order is not required
        return !deferCards_.load(std::memory_order_relaxed);
    }

private:
    template <typename T>
    void ProcessObjectPointerInternal(T *ref) const
    {
        // fromRemset_ is not changed while handling one card
        ASSERT(AddrToRegion(ref)->GetRemSet() == fromRemset_);

        auto o = ObjectAccessor::LoadAtomic(ref);
        if (!ObjectAccessor::IsHeapObject(o)) {
            return;
        }

        auto *obj = ObjectAccessor::DecodeNotNull(o);
        if (ark::mem::IsSameRegion(ref, obj, regionSizeBits_)) {
            return;
        }

        ASSERT_PRINT(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(obj)),
                     "Not suitable space for to_obj: " << obj);

        // don't need lock because only one thread changes remsets
        RemSet<>::AddRefWithAddr<false>(fromRemset_, ref, obj);
        LOG(DEBUG, GC) << "fill rem set " << ref << " -> " << obj;
    }
    RemSetT *fromRemset_;
    size_t regionSizeBits_;
    const std::atomic_bool &deferCards_;
};

template <class LanguageConfig>
class EvacuationObjectPointerHandler {
public:
    using Ref = typename ObjectReference<LanguageConfig::LANG_TYPE>::Type;

    explicit EvacuationObjectPointerHandler(G1EvacuateRegionsWorkerState<LanguageConfig> *workerState)
        : gc_(workerState->GetGC()), workerState_(workerState)
    {
    }

    bool ProcessObjectPointer(Ref ref) const
    {
        ProcessObjectPointerHelper(ref);
        return true;
    }

private:
    void ProcessObjectPointerHelper(Ref ref) const
    {
        auto o = ObjectAccessor::Load(ref);
        if (!ObjectAccessor::IsHeapObject(o)) {
            return;
        }
        auto *obj = ObjectAccessor::DecodeNotNull(o);
        if (gc_->InGCSweepRange(obj)) {
            workerState_->PushToQueue(ref);
        } else if (!workerState_->IsSameRegion(ref, obj)) {
            workerState_->EnqueueCard(ref);
        }
    }

    G1GC<LanguageConfig> *gc_;
    G1EvacuateRegionsWorkerState<LanguageConfig> *workerState_;
};
}  // namespace ark::mem

#endif
