/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark {
class ManagedThread;
}  // namespace ark
namespace ark::mem {

template <MTModeT MT_MODE>
class AllocConfig<GCType::GEN_GC, MT_MODE> {
public:
    using ObjectAllocatorType = ObjectAllocatorGen<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

/// @brief Generational GC
template <class LanguageConfig>
class GenGC : public GenerationalGC<LanguageConfig> {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    explicit GenGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(GenGC);
    NO_MOVE_SEMANTIC(GenGC);
    ~GenGC() override = default;

    void InitGCBits(ark::ObjectHeader *objHeader) override;

    void InitGCBitsForAllocationInTLAB(ark::ObjectHeader *objHeader) override;

    bool IsPinningSupported() const final
    {
        // Gen GC can not pin objects in young space, so Gen GC does not support pinning
        return false;
    }

    void MarkReferences(GCMarkingStackType *references, GCPhase gcPhase) override;

    void MarkObject(ObjectHeader *object) override;

    void UnMarkObject(ObjectHeader *objectHeader);

    bool InGCSweepRange(const ObjectHeader *obj) const override;

    bool IsPostponeGCSupported() const override;

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

    void MarkYoungStack(GCMarkingStackType *objectsStack);

    /**
     * Mark roots and add them to the stack
     * @param objects_stack
     * @param visit_class_roots
     * @param visit_card_table_roots
     */
    void MarkRoots(GCMarkingStackType *objectsStack, CardTableVisitFlag visitCardTableRoots,
                   const ReferenceCheckPredicateT &pred, VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL);

    /**
     * Initial marks roots and fill in 1st level from roots into stack.
     * STW
     * @param objects_stack
     */
    void InitialMark(GCMarkingStackType *objectsStack);

    /**
     * Concurrently marking all objects
     * @param objects_stack
     */
    NO_THREAD_SAFETY_ANALYSIS void ConcurrentMark(GCMarkingStackType *objectsStack);

    /**
     * ReMarks objects after Concurrent marking
     * @param objects_stack
     * @param task gc task for current GC
     */
    void ReMark(GCMarkingStackType *objectsStack, const GCTask &task);

    /**
     * Mark objects for the whole heap on pause
     * @param task gc task for current GC
     */
    void FullMark(const GCTask &task);

    /// Collect dead objects in young generation and move survivors
    void CollectYoungAndMove();

    /**
     * Collect verification info for CollectAndMove phase
     * @param young_mem_range young memory region
     * @return instance of verifier to be used to verify for updated references
     */
    [[nodiscard]] HeapVerifierIntoGC<LanguageConfig> CollectVerificationInfo(const MemRange &youngMemRange);

    /**
     * Verify updted references
     * @param collect_verifier instance of the verifier that was obtained before references were updated
     *
     * @see CollectVerificationInfo
     * @see UpdateRefsToMovedObjects
     */
    void VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&youngVerifier);

    /// Update all refs to moved objects
    void UpdateRefsToMovedObjects(PandaVector<ObjectHeader *> *movedObjects);

    /// Sweep dead objects in tenured space
    template <bool CONCURRENT>
    void Sweep();

    /// Sweep dead objects in young space
    void SweepYoungVmRefs();

    bool IsMarked(const ObjectHeader *object) const override;

    bool ShouldRunTenuredGC(const GCTask &task) override;

    /**
     * @param task gc task for current GC
     * @param have_enough_space_for_young boolean value that determines whether we have enough memory to move from
     * young space to tenured space
     * @return true if need run full gc or flase - otherwise
     */
    bool ShouldRunFullGC(const GCTask &task, bool haveEnoughSpaceForYoung) const;

    bool HaveEnoughSpaceToMove() const;

    DefaultGCMarkerImpl<LanguageConfig> marker_;
    /// Flag indicates if we currently in concurrent marking phase
    bool concurrentMarkingFlag_ {false};
    PandaUniquePtr<CardTable> cardTable_ {nullptr};
    bool isExplicitConcurrentGcEnabled_ {false};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GEN_GC_GEN_GC_H
