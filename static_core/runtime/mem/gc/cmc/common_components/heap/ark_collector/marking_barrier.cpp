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
#include "common_components/heap/ark_collector/marking_barrier.h"
#include "common_components/heap/heap.h"

#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

#include "common_interfaces/thread/mutator.h"

namespace common_vm {
// Because gc thread will also have impact on tagged pointer in enum and marking phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject *MarkingBarrier::ReadRefField(BaseObject *obj, RefField<false> &field) const
{
    RefField<> tmpField(field);
    return (BaseObject *)tmpField.GetFieldValue();
}

BaseObject *MarkingBarrier::ReadStaticRef(RefField<false> &field) const
{
    return ReadRefField(nullptr, field);
}

void MarkingBarrier::PreWriteBarrier(Mutator *mutator, BaseObject *rememberedObject) const
{
    if (rememberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(rememberedObject);
        DLOG(BARRIER, "pre-write barrier rememberedObject: %p", rememberedObject);
    }
}

void MarkingBarrier::WriteBarrier(Mutator *mutator, BaseObject *obj, RefField<false> &field, BaseObject *ref) const
{
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
        DLOG(BARRIER, "write obj %p ref-field@%p: -> %p", obj, &field, ref);
    }
}

BaseObject *MarkingBarrier::AtomicReadRefField(BaseObject *obj, RefField<true> &field, MemoryOrder order) const
{
    BaseObject *target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    target = (BaseObject *)(oldField.GetFieldValue());
    DLOG(TBARRIER, "katomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

}  // namespace common_vm
