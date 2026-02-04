/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "common_components/common_runtime/hooks.h"

namespace common {

void (*g_visitAllStaticRootsCallback)(const RefFieldVisitor &) = nullptr;
void (*g_visitStaticMutatorRootsCallback)(const RefFieldVisitor &, void *mutator) = nullptr;
void (*g_visitStaticGlobalRootsCallback)(const RefFieldVisitor &) = nullptr;
void (*g_updateAndSweepStaticRefsCallback)(const WeakRefFieldVisitor &visitor) = nullptr;
void (*g_visitStaticConcurrentRootsCallback)(const RefFieldVisitor &) = nullptr;
void (*g_unmarkAllStaticXRefsCallback)() = nullptr;
void (*g_sweepUnmarkedStaticXRefsCallback)() = nullptr;
void (*g_addXRefToStaticRootsCallback)() = nullptr;
void (*g_removeXRefFromStaticRootsCallback)() = nullptr;

void VisitBaseRoots([[maybe_unused]] const RefFieldVisitor &visitor) {}

void VisitAllStaticRoots(const RefFieldVisitor &visitorFunc)
{
    if (g_visitAllStaticRootsCallback != nullptr) {
        g_visitAllStaticRootsCallback(visitorFunc);
    }
}

void SetVisitAllStaticRootsCallback(void (*callback)(const RefFieldVisitor &))
{
    g_visitAllStaticRootsCallback = callback;
}

void VisitStaticMutatorRoots(const RefFieldVisitor &visitorFunc, void *mutator)
{
    if (g_visitStaticMutatorRootsCallback != nullptr) {
        g_visitStaticMutatorRootsCallback(visitorFunc, mutator);
    }
}

void SetVisitStaticMutatorRootsCallback(void (*callback)(const RefFieldVisitor &visitorFunc, void *mutator))
{
    g_visitStaticMutatorRootsCallback = callback;
}

void VisitStaticGlobalRoots(const RefFieldVisitor &visitor)
{
    if (g_visitStaticGlobalRootsCallback != nullptr) {
        g_visitStaticGlobalRootsCallback(visitor);
    }
}

void SetVisitStaticGlobalRootsCallback(void (*callback)(const RefFieldVisitor &))
{
    g_visitStaticGlobalRootsCallback = callback;
}

void UpdateAndSweepStaticRefs(const WeakRefFieldVisitor &visitor)
{
    if (g_updateAndSweepStaticRefsCallback != nullptr) {
        g_updateAndSweepStaticRefsCallback(visitor);
    }
}

void SetUpdateAndSweepStaticRefsCallback(void (*callback)(const WeakRefFieldVisitor &visitor))
{
    g_updateAndSweepStaticRefsCallback = callback;
}

void VisitStaticConcurrentRoots(const RefFieldVisitor &visitor)
{
    if (g_visitStaticConcurrentRootsCallback != nullptr) {
        g_visitStaticConcurrentRootsCallback(visitor);
    }
}

void SetVisitStaticConcurrentRootsCallback(void (*callback)(const RefFieldVisitor &))
{
    g_visitStaticConcurrentRootsCallback = callback;
}

void UnmarkAllStaticXRefs()
{
    g_unmarkAllStaticXRefsCallback();
}

void SetUnmarkAllStaticXRefsCallback(void (*callback)())
{
    g_unmarkAllStaticXRefsCallback = callback;
}

void SweepUnmarkedStaticXRefs()
{
    g_sweepUnmarkedStaticXRefsCallback();
}

void SetSweepUnmarkedStaticXRefsCallback(void (*callback)())
{
    g_sweepUnmarkedStaticXRefsCallback = callback;
}

void AddXRefToStaticRoots()
{
    g_addXRefToStaticRootsCallback();
}

void SetAddXRefToStaticRootsCallback(void (*callback)())
{
    g_addXRefToStaticRootsCallback = callback;
}

void RemoveXRefFromStaticRoots()
{
    g_removeXRefFromStaticRootsCallback();
}

void SetRemoveXRefFromStaticRootsCallback(void (*callback)())
{
    g_removeXRefFromStaticRootsCallback = callback;
}

void SetBaseAddress([[maybe_unused]] uintptr_t base) {}

void SweepThreadLocalJitFort() {}

bool IsMachineCodeObject([[maybe_unused]] uintptr_t obj)
{
    return false;
}

void JitFortUnProt([[maybe_unused]] size_t size, [[maybe_unused]] void *base) {}

void MarkThreadLocalJitFortInstalled([[maybe_unused]] void *mutator, [[maybe_unused]] void *machineCode) {}

void InvokeSharedNativePointerCallbacks() {}

void SynchronizeGCPhaseToJSThread([[maybe_unused]] void *jsThread, [[maybe_unused]] GCPhase gcPhase) {}

void JSGCCallback([[maybe_unused]] void *ecmaVM) {}

void AddXRefToDynamicRoots() {}

void RemoveXRefFromDynamicRoots() {}

void VisitDynamicThreadRoot([[maybe_unused]] const RefFieldVisitor &visitorFunc, [[maybe_unused]] void *vm) {}

void VisitDynamicWeakThreadRoot([[maybe_unused]] const WeakRefFieldVisitor &visitorFunc, [[maybe_unused]] void *vm) {}

void VisitDynamicThreadPreforwardRoot([[maybe_unused]] const RefFieldVisitor &visitorFunc, [[maybe_unused]] void *vm) {}

void VisitDynamicGlobalRoots([[maybe_unused]] const RefFieldVisitor &visitor) {}

void VisitDynamicWeakGlobalRoots([[maybe_unused]] const WeakRefFieldVisitor &visitorFunc) {}

void VisitDynamicWeakGlobalRootsOld([[maybe_unused]] const WeakRefFieldVisitor &visitorFunc) {}

void VisitDynamicLocalRoots([[maybe_unused]] const RefFieldVisitor &visitor) {}

void VisitDynamicWeakLocalRoots([[maybe_unused]] const WeakRefFieldVisitor &visitorFunc) {}

void VisitDynamicPreforwardRoots([[maybe_unused]] const RefFieldVisitor &visitorFunc) {}

void VisitDynamicConcurrentRoots([[maybe_unused]] const RefFieldVisitor &visitorFunc) {}
}  // namespace common
