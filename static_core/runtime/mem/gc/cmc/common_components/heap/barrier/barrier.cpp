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

#include "common_components/heap/barrier/barrier.h"

#include "common_components/heap/collector/collector.h"
#include "common_components/heap/heap.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

#include "libarkbase/utils/logger.h"

#include "securec.h"

namespace common_vm {

void Barrier::PreWriteBarrier(Mutator *mutator, BaseObject *rememberedObject) const
{
    LOG(DEBUG, GC) << "pre-write barrier rememberedObject: " << rememberedObject;
}

void Barrier::WriteBarrier(Mutator *mutator, BaseObject *obj, RefField<false> &field, BaseObject *ref) const
{
    LOG(DEBUG, GC) << "write obj " << obj << " ref-field@" << &field << ": " << field.GetTargetObject() << " => "
                   << ref;
}

BaseObject *Barrier::ReadRefField(BaseObject *obj, RefField<false> &field) const
{
    HeapAddress target = field.GetFieldValue();
    return reinterpret_cast<BaseObject *>(target);
}

BaseObject *Barrier::ReadStaticRef(RefField<false> &field) const
{
    HeapAddress target = field.GetFieldValue();
    return reinterpret_cast<BaseObject *>(target);
}

BaseObject *Barrier::AtomicReadRefField(BaseObject *obj, RefField<true> &field, MemoryOrder order) const
{
    RefField<false> tmpField(field.GetFieldValue(order));
    HeapAddress target = tmpField.GetFieldValue();
    LOG(DEBUG, GC) << "atomic read obj " << obj << " ref@" << &field << ": 0x" << std::hex << tmpField.GetFieldValue()
                   << std::dec << " -> " << target;
    return (BaseObject *)target;
}

}  // namespace common_vm
