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
#include "common_components/heap/ark_collector/post_marking_barrier.h"

#include "common_components/heap/allocator/regional_heap.h"
#include "common_interfaces/thread/mutator.h"
#include "heap/space/young_space.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common_vm {
BaseObject* PostMarkingBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    return (BaseObject*)tmpField.GetFieldValue();
}

BaseObject* PostMarkingBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

void PostMarkingBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
    }
}

BaseObject* PostMarkingBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));

    target = ReadRefField(nullptr, oldField);
    DLOG(TBARRIER, "katomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}
} // namespace common_vm
