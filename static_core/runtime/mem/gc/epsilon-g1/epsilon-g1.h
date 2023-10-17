/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_MEM_GC_EPSILON_G1_EPSILON_G1_H
#define PANDA_RUNTIME_MEM_GC_EPSILON_G1_EPSILON_G1_H

#include "runtime/mem/gc/g1/g1-gc.h"

namespace panda::mem {

template <class LanguageConfig>
class EpsilonG1GC final : public G1GC<LanguageConfig> {
public:
    explicit EpsilonG1GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(EpsilonG1GC);
    NO_MOVE_SEMANTIC(EpsilonG1GC);
    ~EpsilonG1GC() override = default;

    void StartGC() override;
    void StopGC() override;

    void RunPhases(GCTask &task);
    bool WaitForGC(GCTask task) override;
    void InitGCBits(panda::ObjectHeader *obj_header) override;
    bool Trigger(PandaUniquePtr<GCTask> task) override;
    void OnThreadTerminate(ManagedThread *thread, mem::BuffersKeepingFlag keep_buffers) override;

private:
    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) override;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_EPSILON_G1_EPSILON_G1_H