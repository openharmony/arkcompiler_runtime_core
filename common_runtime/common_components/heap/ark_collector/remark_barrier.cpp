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
#include "common_components/heap/ark_collector/remark_barrier.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_interfaces/thread/mutator.h"
#if defined(ARKCOMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common_vm {
// Because gc thread will also have impact on tagged pointer in enum and marking phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject* RemarkBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    return reinterpret_cast<BaseObject*>(tmpField.GetFieldValue());
}

BaseObject* RemarkBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

#ifdef ARK_USE_SATB_BARRIER
void RemarkBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    RefField<> tmpField(field);
    BaseObject* rememberedObject = nullptr;
    rememberedObject = tmpField.GetTargetObject();
    if (!Heap::IsTaggedObject(field.GetFieldValue())) {
        return;
    }
    UpdateRememberSet(obj, ref);
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    if (rememberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(rememberedObject);
    }
    if (ref != nullptr) {
        if (!Heap::IsTaggedObject((HeapAddress)ref)) {
            return;
        }
        ref = reinterpret_cast<BaseObject*>(reinterpret_cast<uintptr_t>(ref) & ~TAG_WEAK);
        mutator->RememberObjectInSatbBuffer(ref);
    }

    DLOG(BARRIER, "write obj %p ref-field@%p: %#zx -> %p", obj, &field, rememberedObject, ref);
}
#else
void RemarkBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject((HeapAddress)ref)) {
        return;
    }
    UpdateRememberSet(obj, ref);
    ref = reinterpret_cast<BaseObject*>(reinterpret_cast<uintptr_t>(ref) & ~TAG_WEAK);
    ASSERT_LOGF(mutator != nullptr, "Mutator is nullptr");
    mutator->RememberObjectInSatbBuffer(ref);
    DLOG(BARRIER, "write obj %p ref-field@%p: -> %p", obj, &field, ref);
}
#endif

BaseObject* RemarkBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    target =  reinterpret_cast<BaseObject*>((oldField.GetFieldValue()));
    DLOG(TBARRIER, "katomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

} // namespace panda

