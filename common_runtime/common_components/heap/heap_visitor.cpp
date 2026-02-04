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

#include "common_interfaces/heap/heap_visitor.h"
#include "common_components/common_runtime/hooks.h"
#include "common_interfaces/thread/mutator.h"

namespace common {

void VisitRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicGlobalRoots(visitor);
    VisitDynamicLocalRoots(visitor);
    VisitDynamicConcurrentRoots(visitor);
    VisitAllStaticRoots(visitor);
    VisitBaseRoots(visitor);
}

void VisitSTWRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicGlobalRoots(visitor);
    VisitDynamicLocalRoots(visitor);
    VisitStaticGlobalRoots(visitor);
    VisitStaticConcurrentRoots(visitor);
    VisitBaseRoots(visitor);
}

void VisitConcurrentRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicConcurrentRoots(visitor);
    VisitStaticConcurrentRoots(visitor);
}

void VisitWeakRoots(const WeakRefFieldVisitor &visitor)
{
    VisitDynamicWeakGlobalRoots(visitor);
    VisitDynamicWeakGlobalRootsOld(visitor);
    VisitDynamicWeakLocalRoots(visitor);
    VisitAllStaticRoots(visitor);
    UpdateAndSweepStaticRefs(visitor);
}

void VisitGlobalRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicGlobalRoots(visitor);
    VisitStaticGlobalRoots(visitor);
    VisitBaseRoots(visitor);
}

void VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor, bool isYoung)
{
    VisitDynamicWeakGlobalRoots(visitor);
    if (!isYoung) {
        VisitDynamicWeakGlobalRootsOld(visitor);
    }
    VisitStaticGlobalRoots(visitor);
    UpdateAndSweepStaticRefs(visitor);
}

void VisitPreforwardRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicPreforwardRoots(visitor);
}

// Visit specific mutator roots.
void VisitMutatorRoots(const RefFieldVisitor &visitor, Mutator &mutator)
{
    if (mutator.GetEcmaVMPtr()) {
        VisitDynamicThreadRoot(visitor, mutator.GetEcmaVMPtr());
    }
    void *thread = mutator.GetArkthreadPtr();
    if (thread != nullptr) {
        VisitStaticMutatorRoots(visitor, thread);
    }
}

void VisitWeakMutatorRoot(const WeakRefFieldVisitor &visitor, Mutator &mutator)
{
    if (mutator.GetEcmaVMPtr()) {
        VisitDynamicWeakThreadRoot(visitor, mutator.GetEcmaVMPtr());
    }
    void *thread = mutator.GetArkthreadPtr();
    if (thread != nullptr) {
        VisitStaticMutatorRoots(visitor, thread);
    }
}

void VisitMutatorPreforwardRoot(const RefFieldVisitor &visitor, Mutator &mutator)
{
    if (mutator.GetEcmaVMPtr()) {
        VisitDynamicThreadPreforwardRoot(visitor, mutator.GetEcmaVMPtr());
    }
}

void UnmarkAllXRefs()
{
}

void SweepUnmarkedXRefs()
{
}

void AddXRefToRoots()
{
}

void RemoveXRefFromRoots()
{
}
}  // namespace common
