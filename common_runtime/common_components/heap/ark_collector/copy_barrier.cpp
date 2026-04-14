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
#include "common_components/heap/ark_collector/copy_barrier.h"
#include "common_components/base/sys_call.h"
#include "common_components/common/scoped_object_lock.h"
#include "common_interfaces/thread/mutator.h"
#include "heap/collector/collector_proxy.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common_vm {
BaseObject* CopyBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    do {
        RefField<> tmpField(field);
        BaseObject* oldRef = reinterpret_cast<BaseObject *>(tmpField.GetAddress());
        if (LIKELY_CC(!static_cast<CollectorProxy *>(&theCollector)->IsFromObject(oldRef))) {
            return oldRef;
        }

        auto weakMask = reinterpret_cast<MAddress>(oldRef) & TAG_WEAK;
        oldRef = reinterpret_cast<BaseObject *>(reinterpret_cast<MAddress>(oldRef) & (~TAG_WEAK));
        BaseObject* toObj = nullptr;
        if (static_cast<CollectorProxy *>(&theCollector)->TryForwardRefField(obj, field, toObj)) {
            return (BaseObject*)((uintptr_t)toObj | weakMask);
        }
    } while (true);
    // unreachable path.
    return nullptr;
}

BaseObject* CopyBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

BaseObject* CopyBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> tmpField(field.GetFieldValue(order));
    BaseObject* target = ReadRefField(nullptr, tmpField);
    DLOG(FBARRIER, "atomic read obj %p ref-field@%p: %#zx -> %p", obj, &field, tmpField.GetFieldValue(), target);
    return target;
}

void YoungCopyBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject(reinterpret_cast<HeapAddress>(ref))) {
        return;
    }
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
    }
    ref = (BaseObject*)((uintptr_t)ref & ~(TAG_WEAK));
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    mutator->RememberObjectInSatbBuffer(ref);
    DLOG(BARRIER, "write obj %p ref-field@%p: -> %p", obj, &field, ref);
}
}  // namespace common_vm
