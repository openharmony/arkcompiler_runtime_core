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
#ifndef PANDA_LIBPANDABASE_MEM_STWGC_H
#define PANDA_LIBPANDABASE_MEM_STWGC_H

#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/gc_marker.h"
#include "runtime/mem/gc/gc_root.h"

namespace panda::mem {

template <LangTypeT LANG_TYPE, bool HAS_VALUE_OBJECT_TYPES>
class StwGCMarker
    : public DefaultGCMarker<StwGCMarker<LANG_TYPE, HAS_VALUE_OBJECT_TYPES>, LANG_TYPE, HAS_VALUE_OBJECT_TYPES> {
    using Base = DefaultGCMarker<StwGCMarker<LANG_TYPE, HAS_VALUE_OBJECT_TYPES>, LANG_TYPE, HAS_VALUE_OBJECT_TYPES>;

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

/**
 * \brief Stop the world, non-concurrent GC
 */
template <class LanguageConfig>
class StwGC final : public GCLang<LanguageConfig> {
public:
    explicit StwGC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(StwGC);
    NO_MOVE_SEMANTIC(StwGC);
    ~StwGC() override = default;

    void WaitForGC(GCTask task) override;

    void InitGCBits(panda::ObjectHeader *object) override;

    void InitGCBitsForAllocationInTLAB(panda::ObjectHeader *obj_header) override;

    void Trigger() override;

    void WorkerTaskProcessing(GCWorkersTask *task, void *worker_data) override;

private:
    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void Mark(const GCTask &task);
    void MarkStack(GCMarkingStackType *stack, const GC::MarkPredicate &markPredicate);
    void SweepStringTable();
    void Sweep();

    void MarkObject(ObjectHeader *object) override;
    bool IsMarked(const ObjectHeader *object) const override;
    void UnMarkObject(ObjectHeader *object_header);
    void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) override;

    StwGCMarker<LanguageConfig::LANG_TYPE, LanguageConfig::HAS_VALUE_OBJECT_TYPES> marker_;
};

}  // namespace panda::mem

#endif  // PANDA_LIBPANDABASE_MEM_STWGC_H
