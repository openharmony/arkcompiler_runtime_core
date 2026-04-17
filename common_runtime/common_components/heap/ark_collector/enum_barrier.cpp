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
#include "common_components/heap/ark_collector/enum_barrier.h"
#include "common_components/heap/heap.h"
#include "common_components/log/log.h"

#if defined(COMMON_TSAN_SUPPORT)
#include "common_interfaces/thread/mutator.h"
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common_vm {
// Because gc thread will also have impact on tagged pointer in enum and marking phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject* EnumBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    return (BaseObject*)tmpField.GetFieldValue();
}

BaseObject* EnumBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

#ifdef ARK_USE_SATB_BARRIER
void EnumBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    RefField<> tmpField(field);
    BaseObject* remeberedObject = nullptr;
    //Because it is possible to read a Double,
    // and the lower 48 bits happen to be a HeapAddress, we need to avoid this situation
    if (!Heap::IsTaggedObject(field.GetFieldValue())) {
        return;
    }
    UpdateRememberSet(obj, ref);
    remeberedObject = tmpField.GetTargetObject();
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    if (remeberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(remeberedObject);
    }
    if (ref != nullptr) {
        // Wait Conccurent Enum Fix
        // ref maybe not valid
        // mutator->RememberObjectInSatbBuffer(ref)
    }
    DLOG(BARRIER, "write obj %p ref@%p: 0x%zx -> %p", obj, &field, remeberedObject, ref);
}
#else
void EnumBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject((HeapAddress)ref)) {
        return;
    }
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
    }
    ref = (BaseObject*)((uintptr_t)ref & ~(TAG_WEAK));
    ASSERT_LOGF(mutator != nullptr, "Mutator is nullptr");
    mutator->RememberObjectInSatbBuffer(ref);
    DLOG(BARRIER, "write obj %p ref-field@%p: -> %p", obj, &field, ref);
}
#endif

BaseObject* EnumBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    target = ReadRefField(obj, oldField);
    DLOG(EBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

} // namespace common_vm
