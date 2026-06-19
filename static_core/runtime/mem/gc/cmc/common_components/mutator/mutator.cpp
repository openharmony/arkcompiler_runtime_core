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

#include <cstdint>
#include <unistd.h>

#include "common_components/common/type_def.h"
#include "mutator/satb_buffer.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/common/scoped_object_access.h"

namespace ark::common_vm {

ThreadLocalData *GetThreadLocalData()
{
    uintptr_t tlDataAddr = reinterpret_cast<uintptr_t>(ThreadLocal::GetThreadLocalData());
#if defined(__aarch64__)
    if (Heap::GetHeap().IsGcStarted()) {
        // Since the TBI(top bit ignore) feature in Aarch64,
        // set gc phase to high 8-bit of ThreadLocalData Address for gc barrier fast path.
        // 56: make gcphase value shift left 56 bit to set the high 8-bit
        tlDataAddr = tlDataAddr | (static_cast<uint64_t>(Heap::GetHeap().GetGCPhase()) << 56);
    }
#endif
    return reinterpret_cast<ThreadLocalData *>(tlDataAddr);
}

void Mutator::HandleGCCallback()
{
    Heap::GetHeap().GetCollector().ForEachVM([](VMInterface *vm) { vm->ProcessFinalizationRegistryCleanup(); });
}

static SatbBuffer::TreapNode *&CastSatbNode(void *&satbNode)
{
    return reinterpret_cast<SatbBuffer::TreapNode *&>(satbNode);
}

Mutator::~Mutator()
{
    if (satbNode_ != nullptr) {
        SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
        satbNode_ = nullptr;
    }
}

void Mutator::RememberObjectImpl(const BaseObject *obj)
{
    if (LIKELY(Heap::IsHeapAddress(obj))) {
        if (SatbBuffer::ShouldEnqueue(obj)) {
            SatbBuffer::Instance().EnsureGoodNode(CastSatbNode(satbNode_));
            CastSatbNode(satbNode_)->Push(obj);
        }
    }
}

void Mutator::ResetMutator()
{
    if (satbNode_ != nullptr) {
        SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
        satbNode_ = nullptr;
    }
    static_cast<ark::Mutator *>(this)->ClearReferencesCleanupRequest();
}

const void *Mutator::GetSatbBufferNode() const
{
    return satbNode_;
}

void Mutator::ClearSatbBufferNode()
{
    if (satbNode_ == nullptr) {
        return;
    }
    CastSatbNode(satbNode_)->Clear();
}

void Mutator::VisitMutatorRoots(const RefFieldVisitor &visitor) {}

void Mutator::GcPhaseEnum(GCPhase newPhase) {}

// comment all
void Mutator::GCPhasePreForward(GCPhase newPhase) {}

void Mutator::HandleGCPhase(GCPhase newPhase)
{
    if (newPhase == GCPhase::GC_PHASE_POST_MARK) {
        if (satbNode_ != nullptr) {
            DCHECK(CastSatbNode(satbNode_)->IsEmpty());
            SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
            satbNode_ = nullptr;
        }
    } else if (newPhase == GCPhase::GC_PHASE_ENUM) {
        GcPhaseEnum(newPhase);
    } else if (newPhase == GCPhase::GC_PHASE_PRECOPY) {
        GCPhasePreForward(newPhase);
    } else if (newPhase == GCPhase::GC_PHASE_REMARK_SATB || newPhase == GCPhase::GC_PHASE_FINAL_MARK) {
        if (satbNode_ != nullptr) {
            SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
            satbNode_ = nullptr;
        }
    }
}

void Mutator::ReleaseAllocBuffer()
{
    allocBuffer_ = nullptr;
}

bool IsRuntimeThread()
{
    if (static_cast<int>(ThreadLocal::GetThreadType()) >= static_cast<int>(ThreadType::GC_THREAD)) {
        return true;
    }
    return false;
}

bool IsGcThread()
{
    if (static_cast<int>(ThreadLocal::GetThreadType()) == static_cast<int>(ThreadType::GC_THREAD)) {
        return true;
    }
    return false;
}

}  // namespace ark::common_vm
