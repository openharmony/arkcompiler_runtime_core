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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_ARKCOLLECTOR_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_ARKCOLLECTOR_H

#include <unordered_map>

#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/collector/copy_data_manager.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_interfaces/base_runtime.h"

namespace common_vm {

enum class GCMode: uint8_t {
    CMC = 0,
    CONCURRENT_MARK = 1,
    STW = 2
};

class ArkCollector : public MarkingCollector {
public:
    explicit ArkCollector(Allocator& allocator, CollectorResources& resources)
        : MarkingCollector(allocator, resources)
    {
        collectorType_ = CollectorType::SMOOTH_COLLECTOR;
    }

    ~ArkCollector() override = default;

    void Init(const RuntimeParam& param) override
    {
        HeapBitmapManager::GetHeapBitmapManager().InitializeHeapBitmap();
        DCHECK_CC(GetGCPhase() == GC_PHASE_IDLE);
        Heap::GetHeap().InstallBarrier(GC_PHASE_IDLE);
#ifdef PANDA_TARGET_32
        // cmc is not adapted for 32-bit systems
        gcMode_ = GCMode::STW;
#else
        if (param.gcParam.enableStwGC) {
            gcMode_ = GCMode::STW;
        } else {
            gcMode_ = GCMode::CMC;
        }
#endif

        // force STW for RB DFX
#ifdef ENABLE_CMC_RB_DFX
        gcMode_ = GCMode::STW;
#endif
    }

    void Fini() override
    {
        HeapBitmapManager::GetHeapBitmapManager().DestroyHeapBitmap();
    }

    bool ShouldIgnoreRequest(GCRequest& request) override;
    bool MarkObject(BaseObject* obj) const override;

    MarkingRefFieldVisitor CreateMarkingObjectRefFieldsVisitor(ParallelLocalMarkStack &workStack,
                                                               WeakStack &weakStack) override;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    void MarkingXRef(RefField<> &ref, ParallelLocalMarkStack &workStack) const;
    void MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack) override;
#endif

    void FixObjectRefFields(BaseObject* obj) const override;
    void FixRefField(BaseObject* obj, RefField<>& field) const;

    BaseObject* ForwardObject(BaseObject* fromVersion) override;

    bool IsOldPointer(RefField<>& ref) const override { return false; }

    bool IsCurrentPointer(RefField<>& ref) const override { return false; }

    bool IsFromObject(BaseObject* obj) const override
    {
        // filter const string object.
        if (Heap::IsHeapAddress(obj)) {
            RegionDesc::InlinedRegionMetaData *objMetaRegion =
                RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(obj));
            return objMetaRegion->IsFromRegion();
        }

        return false;
    }

    bool IsUnmovableFromObject(BaseObject* obj) const override;

    // this is called when caller assures from-object/from-region still exists.
    BaseObject* GetForwardingPointer(BaseObject* fromObj) { return fromObj->GetForwardingPointer(); }

    BaseObject* FindToVersion(BaseObject* obj) const override
    {
        return obj->GetForwardingPointer();
    }

    void SetGCThreadQosPriority(common_vm::PriorityMode mode);

    BaseObject* CopyObjectImpl(BaseObject* obj);
    BaseObject* CopyObjectAfterExclusive(BaseObject* obj) override;

    BaseObject* TryForwardObject(BaseObject* fromVersion);

    bool TryUpdateRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const override;
    bool TryForwardRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const override;

    void ProcessEvacuationStack(GlobalEvacuationStack &globalStack);
    void ProcessEvacuationStack(ParallelLocalEvacuationStack &markStack);

    void RemarkYoungCollectionSpace(GlobalEvacuationStack &globalStack);
    void MarkEvacuationStack(GlobalEvacuationStack &globalStack);
    void MarkEvacuationStack(ParallelLocalEvacuationStack &markStack);

protected:
    void CollectLargeGarbage()
    {
        COMMON_PHASE_TIMER("Collect large garbage");
        RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
        GCStats& stats = GetGCStats();
        stats.largeSpaceSize = space.LargeObjectSize();
        stats.largeGarbageSize = space.CollectLargeGarbage();
        stats.collectedBytes += stats.largeGarbageSize;
    }

    void CollectNonMovableGarbage()
    {
        RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
        GCStats& stats = GetGCStats();
        stats.nonMovableSpaceSize = space.NonMovableSpaceSize();
        stats.collectedBytes += stats.nonMovableGarbageSize;
    }

    void CollectSmallSpace();
    void ClearAllGCInfo();

    void DoGarbageCollection() override;

    void ProcessFinalizers() override;

private:
    friend class RemarkAndPreforwardVisitor;
    template<bool copy>
    bool TryUpdateRefFieldImpl(BaseObject* obj, RefField<>& ref, BaseObject*& oldRef, BaseObject*& newRef) const;

    enum class EnumRootsPolicy {
        NO_STW_AND_NO_FLIP_MUTATOR,
        STW_AND_NO_FLIP_MUTATOR,
        STW_AND_FLIP_MUTATOR,
    };

    template <EnumRootsPolicy policy>
    CArrayList<BaseObject *> EnumRoots();

    template <void (&rootsVisitFunc)(const common_vm::RefFieldVisitor &)>
    void EnumRootsImpl(const common_vm::RefFieldVisitor &visitor)
    {
        // assemble garbage candidates.
        reinterpret_cast<RegionalHeap &>(theAllocator_).AssembleGarbageCandidates();
        reinterpret_cast<RegionalHeap &>(theAllocator_).PrepareMarking();

        COMMON_PHASE_TIMER("enum roots & update old pointers within");
        TransitionToGCPhase(GCPhase::GC_PHASE_ENUM, true);

        rootsVisitFunc(visitor);
    }
    CArrayList<CArrayList<BaseObject *>> EnumRootsFlip(STWParam& param, const common_vm::RefFieldVisitor &visitor);

    void MarkingHeap(const CArrayList<BaseObject *> &collectedRoots);
    void PostMarking();
    void RemarkAndPreforwardStaticRoots(GlobalMarkStack &globalMarkStack) override;
    void ParallelRemarkAndPreforward(GlobalMarkStack &globalMarkStack);
    void Preforward();
    void ConcurrentPreforward();

    void PreforwardConcurrentRoots();
    void PreforwardStaticWeakRoots();
    void PreforwardConcurrencyModelRoots();

    void PrepareFix();
    void ParallelFixHeap();
    void FixHeap(); // roots and ref-fields
    WeakRefFieldVisitor GetWeakRefFieldVisitor();
    RefFieldVisitor GetPrefowardRefFieldVisitor();
    void PreforwardFlip();

    void CollectGarbageWithXRef();

    template <EnumRootsPolicy policy>
    void PreforwardNonHeapRoots(GlobalEvacuationStack &globalStack);
    template <void (&rootsVisitFunc)(const common_vm::RefFieldVisitor &)>
    void PreforwardNonHeapRootsImpl(CArrayList<BaseObject *> &forwardedRoots);
    void PreforwardNonHeapRoot(RefField<> &root, CArrayList<BaseObject *> &forwardedRoots);
    void PreforwardNonHeapRootsFlip(CArrayList<BaseObject *> &forwardedRoots);
    void RemarkNonHeapRoots(GlobalEvacuationStack &globalStack);
    void EnqueueRememberedSetRefs(GlobalEvacuationStack &globalStack);
    void MarkSatbBuffer(GlobalEvacuationStack &globalStack);
    template <typename Stack>
    void EnqueueRefsToYoungCollectionSpace(BaseObject *obj, Stack &stack);
    template <typename Stack>
    size_t EnqueueRefsToYoungCollectionSpaceAndGetSize(BaseObject *obj, Stack &stack);
    bool InYoungCollectionSpace(const RegionDesc::InlinedRegionMetaData *region) const;
    bool InYoungCollectionSpace(const RegionDesc *region) const;
    bool InYoungCollectionSpace(const BaseObject *obj) const;
    bool InYoungCollectionSpace(RegionDesc::RegionType type) const;
    void DoGarbageCollectionWithoutConcurrentMarking();

    template<typename Stack>
    void RemarkNonHeapRoot(RefField<> &ref, Stack &stack);

    template<typename Stack>
    void ProcessRef(RefField<> &ref, Stack &stack);

    template<typename Stack>
    void MarkRef(RefField<> &ref, Stack &stack);

    void ProcessReferencesAfterCopy();

    GCMode gcMode_ = GCMode::CMC;
};
} // namespace common_vm

#endif // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_ARKCOLLECTOR_H
