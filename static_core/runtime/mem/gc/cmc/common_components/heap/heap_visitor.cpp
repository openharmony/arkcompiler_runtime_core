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
 **/

#include "common_components/heap/heap.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/thread/mutator.h"
#include "common_interfaces/vm_interface.h"

namespace ark::common_vm {

void VisitRoots(const RefFieldVisitor &visitor)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) { vm->VisitAllRoots(visitor); });
}

void VisitSTWRoots(const RefFieldVisitor &visitor)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) {
        vm->VisitGlobalRoots(visitor);
        vm->VisitConcurrentRoots(visitor);
    });
}

void VisitConcurrentRoots(const RefFieldVisitor &visitor)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) { vm->VisitConcurrentRoots(visitor); });
}

void VisitWeakRoots(const WeakRefFieldVisitor &visitor)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) {
        vm->VisitAllRoots(visitor);
        vm->UpdateAndSweep(visitor);
    });
}

void VisitGlobalRoots(const RefFieldVisitor &visitor)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) { vm->VisitGlobalRoots(visitor); });
}

void VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor, bool isYoung)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) {
        vm->VisitGlobalRoots(visitor);
        vm->UpdateAndSweep(visitor);
    });
}

void VisitPreforwardRoots(const RefFieldVisitor &visitor)
{
    Heap::GetHeap().GetCollector().ForEachVM([&visitor](VMInterface *vm) { vm->VisitPreforwardRoots(visitor); });
}

void VisitMutatorPreforwardRoot(const RefFieldVisitor &visitor, Mutator &mutator) {}

void UnmarkAllXRefs() {}

void SweepUnmarkedXRefs() {}

void AddXRefToRoots() {}

void RemoveXRefFromRoots() {}
}  // namespace ark::common_vm
