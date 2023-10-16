/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_STW_GC_STW_GC_H
#define PANDA_RUNTIME_MEM_GC_STW_GC_STW_GC_H

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/gc_marker.h"
#include "runtime/mem/gc/gc_root.h"

namespace panda::mem {

template <class LanguageConfig>
class StwGCMarker : public DefaultGCMarker<StwGCMarker<LanguageConfig>, LanguageConfig> {
    using Base = DefaultGCMarker<StwGCMarker<LanguageConfig>, LanguageConfig>;

public:
    explicit StwGCMarker(GC *gc) : Base(gc) {}

    bool MarkIfNotMarked(ObjectHeader *object) const
    {
        if (!reverse_mark_) {
            return Base::template MarkIfNotMarked<false>(object);
        }
        return Base::template MarkIfNotMarked<true>(object);
    }

    void Mark(ObjectHeader *object) const
    {
        if (!reverse_mark_) {
            LOG(DEBUG, GC) << "Set mark for GC " << GetDebugInfoAboutObject(object);
            Base::template Mark<false>(object);
        } else {
            LOG(DEBUG, GC) << "Set unmark for GC " << GetDebugInfoAboutObject(object);
            Base::template Mark<true>(object);
        }
    }

    bool IsMarked(const ObjectHeader *object) const
    {
        if (!reverse_mark_) {
            LOG(DEBUG, GC) << "Get marked for GC " << GetDebugInfoAboutObject(object);
            return Base::template IsMarked<false>(object);
        }
        LOG(DEBUG, GC) << "Get unmarked for GC " << GetDebugInfoAboutObject(object);
        return Base::template IsMarked<true>(object);
    }

    void ReverseMark()
    {
        reverse_mark_ = !reverse_mark_;
    }

    bool IsReverseMark() const
    {
        return reverse_mark_;
    }

private:
    bool reverse_mark_ = false;
};

/// @brief Stop the world, non-concurrent GC
template <class LanguageConfig>
class StwGC final : public GCLang<LanguageConfig> {
public:
    explicit StwGC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(StwGC);
    NO_MOVE_SEMANTIC(StwGC);
    ~StwGC() override = default;

    void InitGCBits(panda::ObjectHeader *object) override;

    void InitGCBitsForAllocationInTLAB(panda::ObjectHeader *obj_header) override;

    bool IsPinningSupported() const final
    {
        // STW GC does not move object
        return true;
    }

    bool CheckGCCause(GCTaskCause cause) const override;

    bool Trigger(PandaUniquePtr<GCTask> task) override;

    void WorkerTaskProcessing(GCWorkersTask *task, void *worker_data) override;

    bool IsPostponeGCSupported() const override;

private:
    enum MarkType : bool { DIRECT_MARK = false, REVERSE_MARK = true };

    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void Mark(const GCTask &task);
    void MarkStack(GCMarkingStackType *stack);
    void Sweep();
    template <MarkType MARK_TYPE>
    void SweepImpl();

    void MarkObject(ObjectHeader *object) override;
    bool IsMarked(const ObjectHeader *object) const override;
    void UnMarkObject(ObjectHeader *object_header);
    void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) override;

    StwGCMarker<LanguageConfig> marker_;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_STW_GC_STW_GC_H
