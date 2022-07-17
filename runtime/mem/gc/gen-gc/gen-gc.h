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
#ifndef PANDA_RUNTIME_MEM_GC_GEN_GC_GEN_GC_H
#define PANDA_RUNTIME_MEM_GC_GEN_GC_GEN_GC_H

#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/mem/gc/gc_marker.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/mem/heap_verifier.h"

namespace panda {
class ManagedThread;
}  // namespace panda
namespace panda::mem {

/**
 * \brief Generational GC
 */
template <class LanguageConfig>
class GenGC : public GenerationalGC<LanguageConfig> {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    explicit GenGC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(GenGC);
    NO_MOVE_SEMANTIC(GenGC);
    ~GenGC() override = default;

    void InitGCBits(panda::ObjectHeader *obj_header) override;

    void InitGCBitsForAllocationInTLAB(panda::ObjectHeader *obj_header) override;

    void Trigger() override;

    void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) override;

    void MarkObject(ObjectHeader *object) override;

    void UnMarkObject(ObjectHeader *object_header);

    bool InGCSweepRange(const ObjectHeader *obj) const override;

private:
    void InitializeImpl() override;

    void RunPhasesImpl(GCTask &task) override;

    void PreStartupImp() override;

    /**
     * GC for young generation. Runs with STW.
     * @param task gc task for current GC
     */
    void RunYoungGC(GCTask &task);

    /**
     * GC for tenured generation.
     * @param task gc task for current GC
     */
    void RunTenuredGC(GCTask &task);

    /**
     * Full GC for both generations (tenured and young)
     * @param task gc task for current GC
     */
    void RunFullGC(GCTask &task);

    /**
     * Marks objects in young generation
     * @param task gc task for current GC
     */
    void MarkYoung(const GCTask &task);

    void MarkYoungStack(GCMarkingStackType *objects_stack);

    /**
     * Mark roots and add them to the stack
     * @param objects_stack
     * @param visit_class_roots
     * @param visit_card_table_roots
     */
    void MarkRoots(GCMarkingStackType *objects_stack, CardTableVisitFlag visit_card_table_roots,
                   const ReferenceCheckPredicateT &pred, VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL);

    /**
     * Initial marks roots and fill in 1st level from roots into stack.
     * STW
     * @param objects_stack
     */
    void InitialMark(GCMarkingStackType *objects_stack);

    /**
     * ReMarks objects after Concurrent marking
     * @param objects_stack
     * @param task gc task for current GC
     */
    void ReMark(GCMarkingStackType *objects_stack, const GCTask &task);

    /**
     * Mark objects for the whole heap on pause
     * @param task gc task for current GC
     */
    void FullMark(const GCTask &task);

    /**
     * Collect dead objects in young generation and move survivors
     */
    void CollectYoungAndMove();

    /**
     * Collect verification info for CollectAndMove phase
     * @param young_mem_range young memory region
     * @return instance of verifier to be used to verify for updated references
     */
    [[nodiscard]] HeapVerifierIntoGC<LanguageConfig> CollectVerificationInfo(const MemRange &young_mem_range);

    /**
     * Verify updted references
     * @param collect_verifier instance of the verifier that was obtained before references were updated
     *
     * @see CollectVerificationInfo
     * @see UpdateRefsToMovedObjects
     */
    void VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&young_verifier);

    /**
     * Remove dead strings from string table in tenured space
     */
    void SweepStringTable();

    /**
     * Update all refs to moved objects
     */
    void UpdateRefsToMovedObjects(PandaVector<ObjectHeader *> *moved_objects);

    /**
     * Sweep dead objects in tenured space on pause
     */
    void Sweep();

    /**
     * Concurrent sweep dead objects in tenured space
     */
    void ConcurrentSweep();

    bool IsMarked(const ObjectHeader *object) const override;

    bool ShouldRunTenuredGC(const GCTask &task) override;

    /**
     * @param task gc task for current GC
     * @param have_enough_space_for_young boolean value that determines whether we have enough memory to move from
     * young space to tenured space
     * @return true if need run full gc or flase - otherwise
     */
    bool ShouldRunFullGC(const GCTask &task, bool have_enough_space_for_young) const;

    bool HaveEnoughSpaceToMove() const;

    DefaultGCMarkerImpl<LanguageConfig::LANG_TYPE, LanguageConfig::HAS_VALUE_OBJECT_TYPES> marker_;
    bool concurrent_marking_flag_ {false};  //! flag indicates if we currently in concurrent marking phase
    PandaUniquePtr<CardTable> card_table_ {nullptr};
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GEN_GC_GEN_GC_H
