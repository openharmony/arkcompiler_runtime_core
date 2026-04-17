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
#include "common_components/heap/ark_collector/idle_barrier.h"
#include "common_components/heap/heap.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/log/log.h"

#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "securec.h"

#include "common_interfaces/thread/mutator.h"

namespace common_vm {
BaseObject* IdleBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> oldField(field);
    return (BaseObject*)(oldField.GetFieldValue());
}

BaseObject* IdleBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

BaseObject* IdleBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> oldField(field.GetFieldValue(order));
    BaseObject* target = (BaseObject*)(oldField.GetFieldValue());
    DLOG(BARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

void IdleBarrier::UpdateRememberSet(BaseObject* object, BaseObject* ref) const
{
    DCHECK_CC(object != nullptr);
    RegionDesc::InlinedRegionMetaData *objMetaRegion = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
        reinterpret_cast<uintptr_t>(object));
    RegionDesc::InlinedRegionMetaData *refMetaRegion = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
        reinterpret_cast<uintptr_t>(ref));
    if (!objMetaRegion->IsInYoungSpaceForWB() && refMetaRegion->IsInYoungSpaceForWB()) {
        if (objMetaRegion->MarkRSetCardTable(object)) {
            DLOG(BARRIER, "update point-out remember set of region %p, obj %p, ref: %p<%p>",
                 objMetaRegion->GetRegionDesc(), object, ref, ref->GetTypeInfo());
        }
    }
}

void IdleBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject((HeapAddress)ref)) {
        return;
    }
    UpdateRememberSet(obj, ref);
    DLOG(BARRIER, "write obj %p ref@%p: %p => %p", obj, &field, field.GetTargetObject(), ref);
}
} // namespace common_vm
