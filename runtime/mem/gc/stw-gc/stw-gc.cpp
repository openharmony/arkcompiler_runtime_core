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

#include "runtime/mem/gc/stw-gc/stw-gc.h"

#include "libpandabase/trace/trace.h"
#include "libpandabase/utils/logger.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/hclass.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/gc/gc_workers_thread_pool.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/gc/static/gc_marker_static-inl.h"
#include "runtime/mem/gc/dynamic/gc_marker_dynamic-inl.h"

namespace panda::mem {

template <class LanguageConfig>
StwGC<LanguageConfig>::StwGC(ObjectAllocatorBase *object_allocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(object_allocator, settings), marker_(this)
{
    this->SetType(GCType::STW_GC);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::InitializeImpl()
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrier_set = allocator->New<GCDummyBarrierSet>(allocator);
    ASSERT(barrier_set != nullptr);
    this->SetGCBarrierSet(barrier_set);
    if (this->IsWorkerThreadsExist()) {
        auto thread_pool = allocator->New<GCWorkersThreadPool>(allocator, this, this->GetSettings()->GCWorkersCount());
        ASSERT(thread_pool != nullptr);
        this->SetWorkersPool(thread_pool);
    }
    LOG_DEBUG_GC << "STW GC Initialized";
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::RunPhasesImpl(GCTask &task)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats(), this->GetStats());
    [[maybe_unused]] size_t bytes_in_heap_before_gc = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    marker_.BindBitmaps(true);
    Mark(task);
    SweepStringTable();
    Sweep();
    marker_.ReverseMark();
    [[maybe_unused]] size_t bytes_in_heap_after_gc = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    ASSERT(bytes_in_heap_after_gc <= bytes_in_heap_before_gc);
    task.collection_type_ = GCCollectionType::FULL;
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::Mark(const GCTask &task)
{
    GCScope<TRACE_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);

    // Iterate over roots and add other roots
    bool use_gc_workers = this->GetSettings()->ParallelMarkingEnabled();
    GCMarkingStackType objects_stack(this, use_gc_workers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                     use_gc_workers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                     GCWorkersTaskTypes::TASK_MARKING);

    this->VisitRoots(
        [&objects_stack, &use_gc_workers, this](const GCRoot &gc_root) {
            LOG_DEBUG_GC << "Handle root " << GetDebugInfoAboutObject(gc_root.GetObjectHeader());
            if (marker_.MarkIfNotMarked(gc_root.GetObjectHeader())) {
                objects_stack.PushToStack(gc_root.GetType(), gc_root.GetObjectHeader());
            }
            if (!use_gc_workers) {
                MarkStack(&objects_stack, []([[maybe_unused]] const ObjectHeader *obj) { return true; });
            }
        },
        VisitGCRootFlags::ACCESS_ROOT_ALL);

    this->GetPandaVm()->VisitStringTable(
        [this, &objects_stack](ObjectHeader *str) {
            if (this->marker_.MarkIfNotMarked(str)) {
                ASSERT(str != nullptr);
                objects_stack.PushToStack(RootType::STRING_TABLE, str);
            }
        },
        VisitGCRootFlags::ACCESS_ROOT_ALL);
    MarkStack(&objects_stack, []([[maybe_unused]] const ObjectHeader *obj) { return true; });
    ASSERT(objects_stack.Empty());
    if (use_gc_workers) {
        this->GetWorkersPool()->WaitUntilTasksEnd();
    }
    auto ref_clear_pred = [this]([[maybe_unused]] const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::MarkStack(GCMarkingStackType *stack, const GC::MarkPredicate &markPredicate)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    ASSERT(stack != nullptr);
    size_t objects_count = 0;
    auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    while (!stack->Empty()) {
        objects_count++;
        auto *object = this->PopObjectFromStack(stack);
        ValidateObject(nullptr, object);
        auto *base_class = object->template ClassAddr<BaseClass>();
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);
        if (markPredicate) {
            this->marker_.MarkInstance(stack, ref_pred, object, base_class);
        }
    }
    LOG_DEBUG_GC << "Iterated over " << objects_count << " objects in the stack";
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::SweepStringTable()
{
    GCScope<TRACE_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_SWEEP_STRING_TABLE);
    auto vm = this->GetPandaVm();

    if (!marker_.IsReverseMark()) {
        LOG_DEBUG_GC << "SweepStringTable with MarkChecker";
        vm->SweepStringTable([this](ObjectHeader *object) { return this->marker_.MarkChecker(object); });
    } else {
        LOG_DEBUG_GC << "SweepStringTable with ReverseMarkChecker";
        vm->SweepStringTable([this](ObjectHeader *object) { return this->marker_.template MarkChecker<true>(object); });
    }
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::Sweep()
{
    GCScope<TRACE_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_SWEEP);

    // TODO(dtrubenk): add other allocators when they will be used/implemented
    if (!marker_.IsReverseMark()) {
        LOG_DEBUG_GC << "Sweep with MarkChecker";
        this->GetObjectAllocator()->Collect(
            [this](ObjectHeader *object) {
                auto status = this->marker_.MarkChecker(object);
                if (status == ObjectStatus::DEAD_OBJECT) {
                    LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT: " << object;
                }
                return status;
            },
            GCCollectMode::GC_ALL);
        this->GetObjectAllocator()->VisitAndRemoveFreePools(
            [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });

    } else {
        LOG_DEBUG_GC << "Sweep with ReverseMarkChecker";
        this->GetObjectAllocator()->Collect(
            [this](ObjectHeader *object) {
                auto status = this->marker_.template MarkChecker<true>(object);
                if (status == ObjectStatus::DEAD_OBJECT) {
                    LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT: " << object;
                }
                return status;
            },
            GCCollectMode::GC_ALL);
        this->GetObjectAllocator()->VisitAndRemoveFreePools(
            [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    }
}

// NOLINTNEXTLINE(misc-unused-parameters)
template <class LanguageConfig>
void StwGC<LanguageConfig>::WaitForGC(GCTask task)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    Runtime::GetCurrent()->GetNotificationManager()->GarbageCollectorStartEvent();
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto old_counter = this->gc_counter_.load(std::memory_order_acquire);
    this->GetPandaVm()->GetRendezvous()->SafepointBegin();

    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto new_counter = this->gc_counter_.load(std::memory_order_acquire);
    // Atomic with acquire order reason: data race with last_cause_ with dependecies on reads after the load which
    // should become visible
    if (new_counter > old_counter && this->last_cause_.load(std::memory_order_acquire) >= task.reason_) {
        this->GetPandaVm()->GetRendezvous()->SafepointEnd();
        return;
    }
    auto mem_stats = this->GetPandaVm()->GetMemStats();
    mem_stats->RecordGCPauseStart();
    // Create a copy of the constant GCTask to be able to change its value
    this->RunPhases(task);
    mem_stats->RecordGCPauseEnd();
    this->GetPandaVm()->GetRendezvous()->SafepointEnd();
    Runtime::GetCurrent()->GetNotificationManager()->GarbageCollectorFinishEvent();
    this->GetPandaVm()->HandleGCFinished();
    this->GetPandaVm()->HandleEnqueueReferences();
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::InitGCBits(panda::ObjectHeader *object)
{
    if (!marker_.IsReverseMark()) {
        object->SetUnMarkedForGC();
        ASSERT(!object->IsMarkedForGC());
    } else {
        object->SetMarkedForGC();
        ASSERT(object->IsMarkedForGC());
    }
    LOG_DEBUG_GC << "Init gc bits for object: " << std::hex << object << " bit: " << object->IsMarkedForGC()
                 << " reversed_mark: " << marker_.IsReverseMark();
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] panda::ObjectHeader *obj_header)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::Trigger()
{
    auto task = MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos());
    this->AddGCTask(true, std::move(task), true);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::WorkerTaskProcessing(GCWorkersTask *task, [[maybe_unused]] void *worker_data)
{
    switch (task->GetType()) {
        case GCWorkersTaskTypes::TASK_MARKING: {
            auto stack = task->GetMarkingStack();
            MarkStack(stack, []([[maybe_unused]] const ObjectHeader *obj) { return true; });
            this->GetInternalAllocator()->Delete(stack);
            break;
        }
        default:
            LOG(FATAL, GC) << "Unimplemented for " << GCWorkersTaskTypesToString(task->GetType());
            UNREACHABLE();
    }
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    marker_.Mark(object);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::UnMarkObject([[maybe_unused]] ObjectHeader *object_header)
{
    LOG(FATAL, GC) << "UnMarkObject for STW GC shouldn't be called";
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::MarkReferences(GCMarkingStackType *references, [[maybe_unused]] GCPhase gc_phase)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    ASSERT(gc_phase == GCPhase::GC_PHASE_MARK);
    LOG_DEBUG_GC << "Start marking " << references->Size() << " references";
    MarkStack(references, []([[maybe_unused]] const ObjectHeader *obj) { return true; });
}

template <class LanguageConfig>
bool StwGC<LanguageConfig>::IsMarked(const ObjectHeader *object) const
{
    return marker_.IsMarked(object);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(StwGC);
template class StwGCMarker<LANG_TYPE_STATIC, false>;
template class StwGCMarker<LANG_TYPE_DYNAMIC, false>;

}  // namespace panda::mem
