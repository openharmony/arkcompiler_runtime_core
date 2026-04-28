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

#if defined(COMMON_TSAN_SUPPORT)
#include "common_interfaces/thread/mutator.h"
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

#include "libarkbase/utils/logger.h"

namespace common_vm {
// Because gc thread will also have impact on tagged pointer in enum and marking phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject *EnumBarrier::ReadRefField(BaseObject *obj, RefField<false> &field) const
{
    RefField<> tmpField(field);
    return (BaseObject *)tmpField.GetFieldValue();
}

BaseObject *EnumBarrier::ReadStaticRef(RefField<false> &field) const
{
    return ReadRefField(nullptr, field);
}

void EnumBarrier::PreWriteBarrier(Mutator *mutator, BaseObject *rememberedObject) const
{
    if (rememberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(rememberedObject);
        LOG(DEBUG, GC) << "pre-write barrier rememberedObject: " << rememberedObject;
    }
}

void EnumBarrier::WriteBarrier(Mutator *mutator, BaseObject *obj, RefField<false> &field, BaseObject *ref) const
{
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
        LOG(DEBUG, GC) << "write obj " << obj << " ref-field@" << &field << ": -> " << ref;
    }
}

BaseObject *EnumBarrier::AtomicReadRefField(BaseObject *obj, RefField<true> &field, MemoryOrder order) const
{
    BaseObject *target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    target = ReadRefField(obj, oldField);
    LOG(DEBUG, GC) << "atomic read obj " << obj << " ref@" << &field << ": 0x" << std::hex << oldField.GetFieldValue()
                   << std::dec << " -> " << target;
    return target;
}

}  // namespace common_vm
