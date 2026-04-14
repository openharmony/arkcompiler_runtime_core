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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_IDLE_BARRIER_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_IDLE_BARRIER_H

#include "common_components/heap/barrier/barrier.h"

namespace common_vm {
// IdleBarrier is the barrier for concurrent enum phase
class IdleBarrier : public Barrier {
public:
    explicit IdleBarrier(Collector& collector) : Barrier(collector) {}

    BaseObject* ReadRefField(BaseObject* obj, RefField<false>& field) const override;
    BaseObject* ReadStaticRef(RefField<false>& field) const override;

    void WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const override;

    BaseObject* AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const override;
    
    void UpdateRememberSet(BaseObject* object, BaseObject* ref) const;
};
} // namespace common_vm

#endif // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_IDLE_BARRIER_H
