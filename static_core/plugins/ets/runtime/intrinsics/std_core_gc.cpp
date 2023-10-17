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

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/lambda_utils.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_void.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/thread.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/refstorage/reference.h"

namespace panda::ets::intrinsics {

/**
 * Class tracks GC tasks already processed by GC.
 * Also the class tracks concurrent mark GC phase and calls
 * the callback if it specified.
 */
class GCTaskTracker : public mem::GCListener {
public:
    void InitIfNeeded(mem::GC *gc);
    bool IsInitialized();
    void AddTaskId(uint64_t id);
    bool HasId(uint64_t id);
    void SetCallbackForTask(uint32_t task_id, mem::Reference *callback_ref);
    void GCStarted(const GCTask &task, size_t heap_size) override;
    void GCPhaseStarted(mem::GCPhase phase) override;
    void GCFinished(const GCTask &task, size_t heap_size_before_gc, size_t heap_size) override;
    void RemoveId(uint64_t id);

private:
    bool initialized_ = false;
    std::vector<uint64_t> task_ids_ GUARDED_BY(lock_);
    uint32_t current_task_id_ = 0;
    uint32_t callback_task_id_ = 0;
    mem::Reference *callback_ref_ = nullptr;
    os::memory::Mutex lock_;
};

void GCTaskTracker::InitIfNeeded(mem::GC *gc)
{
    if (initialized_) {
        return;
    }
    gc->AddListener(this);
    initialized_ = true;
}

bool GCTaskTracker::IsInitialized()
{
    return initialized_;
}

void GCTaskTracker::AddTaskId(uint64_t id)
{
    os::memory::LockHolder lock(lock_);
    task_ids_.push_back(id);
}

bool GCTaskTracker::HasId(uint64_t id)
{
    os::memory::LockHolder lock(lock_);
    return std::find(task_ids_.begin(), task_ids_.end(), id) != task_ids_.end();
}

void GCTaskTracker::SetCallbackForTask(uint32_t task_id, mem::Reference *callback_ref)
{
    callback_task_id_ = task_id;
    callback_ref_ = callback_ref;
}

void GCTaskTracker::GCStarted(const GCTask &task, [[maybe_unused]] size_t heap_size)
{
    current_task_id_ = task.GetId();
}

void GCTaskTracker::GCPhaseStarted(mem::GCPhase phase)
{
    if (phase != mem::GCPhase::GC_PHASE_MARK || callback_ref_ == nullptr || current_task_id_ != callback_task_id_) {
        return;
    }
    auto *coroutine = EtsCoroutine::GetCurrent();
    auto *obj = reinterpret_cast<EtsObject *>(coroutine->GetPandaVM()->GetGlobalObjectStorage()->Get(callback_ref_));
    Value arg(obj->GetCoreType());
    os::memory::ReadLockHolder lock(*coroutine->GetPandaVM()->GetRendezvous()->GetMutatorLock());
    LambdaUtils::InvokeVoid(coroutine, obj);
}

void GCTaskTracker::GCFinished(const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                               [[maybe_unused]] size_t heap_size)
{
    RemoveId(task.GetId());
}

void GCTaskTracker::RemoveId(uint64_t id)
{
    current_task_id_ = 0;
    if (id == callback_task_id_ && callback_ref_ != nullptr) {
        EtsCoroutine::GetCurrent()->GetPandaVM()->GetGlobalObjectStorage()->Remove(callback_ref_);
        callback_ref_ = nullptr;
    }
    if (id != 0) {
        os::memory::LockHolder lock(lock_);
        auto it = std::find(task_ids_.begin(), task_ids_.end(), id);
        // There may be no such id if the corresponding GC has been triggered not by startGC
        if (it != task_ids_.end()) {
            task_ids_.erase(it);
        }
    }
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
GCTaskTracker G_GCTASK_TRACKER;

static inline size_t ClampToSizeT(EtsLong n)
{
    if constexpr (sizeof(EtsLong) > sizeof(size_t)) {
        if (UNLIKELY(n > static_cast<EtsLong>(std::numeric_limits<size_t>::max()))) {
            return std::numeric_limits<size_t>::max();
        }
    }
    return n;
}

static GCTaskCause GCCauseFromInt(EtsInt cause)
{
    if (cause == 0) {
        return GCTaskCause::YOUNG_GC_CAUSE;
    }
    if (cause == 1) {
        return GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
    }
    if (cause == 2) {
        return GCTaskCause::MIXED;
    }
    if (cause == 3) {
        return GCTaskCause::OOM_CAUSE;
    }
    return GCTaskCause::INVALID_CAUSE;
}

/**
 * The function triggers specific GC.
 * @param cause - integer denotes type of GC. Possible values are: YOUNG_CAUSE = 0, THRESHOLD_CAUSE = 1,
 *                MIXED_CAUSE = 2, FULL_CAUSE = 3
 * @return gc id. The id should be passed to waitForFinishGC to ensure the GC is finished.
 *  - The function may return 0 in case the GC is executed in-place. It means there is no need to wait such GC.
 *  - The function may return -1 in case the task is canceled.
 */
extern "C" EtsLong StdGCStartGC(EtsInt cause, EtsObject *callback)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    bool run_gc_in_place = Runtime::GetOptions().IsRunGcInPlace("ets");

    GCTaskCause reason = GCCauseFromInt(cause);
    if (reason == GCTaskCause::INVALID_CAUSE) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_EXCEPTION,
                          "Invalid GC cause");
        return -1;
    }
    auto *gc = coroutine->GetVM()->GetGC();
    if (!gc->CheckGCCause(reason)) {
        PandaStringStream e_msg;
        e_msg << mem::GCStringFromType(gc->GetType()) << " does not support " << reason << " cause";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_EXCEPTION, e_msg.str());
        return -1;
    }
    G_GCTASK_TRACKER.InitIfNeeded(gc);
    auto task = MakePandaUnique<GCTask>(reason);
    uint32_t id = task->GetId();
    if (callback != nullptr) {
        auto *callback_ref = coroutine->GetPandaVM()->GetGlobalObjectStorage()->Add(callback->GetCoreType(),
                                                                                    mem::Reference::ObjectType::GLOBAL);
        G_GCTASK_TRACKER.SetCallbackForTask(id, callback_ref);
        // Run GC in place, because need to run callback in managed part
        run_gc_in_place = true;
    }

    // Young GC runs in place
    if (reason == GCTaskCause::YOUNG_GC_CAUSE) {
        run_gc_in_place = true;
    }
    if (run_gc_in_place) {
        return gc->WaitForGCInManaged(*task) ? 0 : -1;
    }
    // Run GC in GC-thread
    if ((reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE) && gc->IsPostponeEnabled()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_STATE_EXCEPTION,
                          "Calling GC threshold not in place after calling postponeGCStart");
        return -1;
    }
    G_GCTASK_TRACKER.AddTaskId(id);
    if (!gc->Trigger(std::move(task))) {
        G_GCTASK_TRACKER.RemoveId(id);
        return -1;
    }
    return static_cast<EtsLong>(id);
}

/**
 * The function returns when the specified GC gets finished.
 * @param gc_id - id of the GC which is returned by startGc.
 * If gc_id is 0 or -1 the function returns immediately.
 */
extern "C" EtsVoid *StdGCWaitForFinishGC(EtsLong gc_id)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    if (gc_id <= 0) {
        return EtsVoid::GetInstance();
    }
    auto id = static_cast<uint64_t>(gc_id);
    ASSERT(G_GCTASK_TRACKER.IsInitialized());
    ScopedNativeCodeThread s(thread);
    while (G_GCTASK_TRACKER.HasId(id)) {
        constexpr uint64_t WAIT_TIME_MS = 10;
        os::thread::NativeSleep(WAIT_TIME_MS);
    }
    return EtsVoid::GetInstance();
}

extern "C" EtsBoolean StdGCIsScheduledGCTriggered()
{
    auto thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    auto *vm = thread->GetVM();
    auto *trigger = vm->GetGCTrigger();

    if (trigger->GetType() != mem::GCTriggerType::ON_NTH_ALLOC) {
        return ToEtsBoolean(false);
    }
    auto sched_trigger = reinterpret_cast<mem::SchedGCOnNthAllocTrigger *>(vm->GetGCTrigger());
    return ToEtsBoolean(sched_trigger->IsTriggered());
}

extern "C" EtsVoid *StdGCPostponeGCStart()
{
    auto coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *gc = coroutine->GetVM()->GetGC();
    if (!gc->IsPostponeGCSupported()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                          "GC postpone is not supported for this GC type");
        return EtsVoid::GetInstance();
    }
    if (gc->IsPostponeEnabled()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_STATE_EXCEPTION,
                          "Calling postponeGCStart without calling postponeGCEnd");
        return EtsVoid::GetInstance();
    }
    gc->PostponeGCStart();
    return EtsVoid::GetInstance();
}

extern "C" EtsVoid *StdGCPostponeGCEnd()
{
    auto coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *gc = coroutine->GetVM()->GetGC();
    if (!gc->IsPostponeGCSupported()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                          "GC postpone is not supported for this GC type");
        return EtsVoid::GetInstance();
    }
    if (!gc->IsPostponeEnabled()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_STATE_EXCEPTION,
                          "Calling postponeGCEnd without calling postponeGCStart");
        return EtsVoid::GetInstance();
    }
    gc->PostponeGCEnd();
    return EtsVoid::GetInstance();
}

template <class ResArrayType>
[[nodiscard]] static ResArrayType *StdGCAllocatePinnedPrimitiveTypeArray(EtsLong length)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    if (length < 0) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NEGATIVE_ARRAY_SIZE_EXCEPTION,
                          "The value must be non negative");
        return nullptr;
    }
    auto *vm = coroutine->GetVM();
    if (!vm->GetGC()->IsPinningSupported()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                          "Object pinning does not support with current GC");
        return nullptr;
    }
    auto *array = ResArrayType::Create(length);

    if (array == nullptr) {
        PandaStringStream ss;
        ss << "Could not allocate array of " << length << " elements";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR, ss.str());
        return nullptr;
    }
    auto *obj_allocator = vm->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
    obj_allocator->PinObject(array->GetCoreType());

    return array;
}

extern "C" EtsBooleanArray *StdGCAllocatePinnedBooleanArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsBooleanArray>(length);
}

extern "C" EtsByteArray *StdGCAllocatePinnedByteArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsByteArray>(length);
}

extern "C" EtsCharArray *StdGCAllocatePinnedCharArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsCharArray>(length);
}

extern "C" EtsShortArray *StdGCAllocatePinnedShortArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsShortArray>(length);
}

extern "C" EtsIntArray *StdGCAllocatePinnedIntArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsIntArray>(length);
}

extern "C" EtsLongArray *StdGCAllocatePinnedLongArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsLongArray>(length);
}

extern "C" EtsFloatArray *StdGCAllocatePinnedFloatArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsFloatArray>(length);
}

extern "C" EtsDoubleArray *StdGCAllocatePinnedDoubleArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsDoubleArray>(length);
}

extern "C" EtsInt StdGCGetObjectSpaceType(EtsObject *obj)
{
    if (obj == nullptr) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ASSERT(coroutine != nullptr);
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NULL_POINTER_EXCEPTION, "Non heap object");
        return SpaceTypeToIndex(SpaceType::SPACE_TYPE_UNDEFINED);
    }

    auto *vm = Thread::GetCurrent()->GetVM();
    SpaceType obj_space_type =
        PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(static_cast<void *>(obj->GetCoreType()));

    if (obj_space_type == SpaceType::SPACE_TYPE_OBJECT && vm->GetGC()->IsGenerational()) {
        auto *obj_allocator = vm->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
        if (obj_allocator->IsObjectInYoungSpace(obj->GetCoreType())) {
            const EtsInt young_space = 4;
            return young_space;
        }
        const EtsInt tenured_space = 5;
        return tenured_space;
    }
    return SpaceTypeToIndex(obj_space_type);
}

extern "C" EtsVoid *StdGCPinObject(EtsObject *obj)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    if (obj == nullptr) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NULL_POINTER_EXCEPTION,
                          "The value must be an object");
        return EtsVoid::GetInstance();
    }

    auto *vm = coroutine->GetVM();
    auto *obj_allocator = vm->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
    auto *gc = vm->GetGC();
    if (!gc->IsPinningSupported()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                          "Object pinning does not support with current gc");
        return EtsVoid::GetInstance();
    }
    obj_allocator->PinObject(obj->GetCoreType());
    return EtsVoid::GetInstance();
}

extern "C" void StdGCUnpinObject(EtsObject *obj)
{
    if (obj == nullptr) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ASSERT(coroutine != nullptr);
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NULL_POINTER_EXCEPTION,
                          "The value must be an object");
        return;
    }

    auto *vm = Thread::GetCurrent()->GetVM();
    vm->GetHeapManager()->GetObjectAllocator().AsObjectAllocator()->UnpinObject(obj->GetCoreType());
}

extern "C" EtsLong StdGCGetObjectAddress(EtsObject *obj)
{
    return obj == nullptr ? 0 : reinterpret_cast<EtsLong>(obj->GetCoreType());
}

// Function schedules GC before n-th allocation by setting counter to the specific GC trigger.
// Another call may reset the counter.  In this case the last counter will be used to trigger the GC.
extern "C" EtsVoid *StdGCScheduleGCAfterNthAlloc(EtsInt counter, EtsInt cause)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    if (counter < 0) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_EXCEPTION,
                          "counter for allocation is negative");
        return EtsVoid::GetInstance();
    }
    GCTaskCause reason = GCCauseFromInt(cause);
    if (reason == GCTaskCause::INVALID_CAUSE) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_EXCEPTION,
                          "Invalid GC cause");
        return EtsVoid::GetInstance();
    }

    auto *vm = coroutine->GetVM();
    auto *gc = vm->GetGC();
    if (!gc->CheckGCCause(reason)) {
        PandaStringStream e_msg;
        e_msg << mem::GCStringFromType(gc->GetType()) << " does not support " << reason << " cause";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_EXCEPTION, e_msg.str());
        return EtsVoid::GetInstance();
    }
    mem::GCTrigger *trigger = vm->GetGCTrigger();
    if (trigger->GetType() != mem::GCTriggerType::ON_NTH_ALLOC) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::RUNTIME_EXCEPTION,
                          "VM is running with unsupported GC trigger");
        return EtsVoid::GetInstance();
    }
    EtsVoid *void_instance = EtsVoid::GetInstance();
    auto sched_trigger = reinterpret_cast<mem::SchedGCOnNthAllocTrigger *>(trigger);
    sched_trigger->ScheduleGc(reason, counter);
    return void_instance;
}

extern "C" EtsLong StdGetFreeHeapSize()
{
    const auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return static_cast<EtsLong>(coroutine->GetPandaVM()->GetHeapManager()->GetFreeMemory());
}

extern "C" EtsLong StdGetUsedHeapSize()
{
    const auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return static_cast<EtsLong>(coroutine->GetPandaVM()->GetHeapManager()->GetTotalMemory());
}

extern "C" EtsLong StdGetReservedHeapSize()
{
    const auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return static_cast<EtsLong>(coroutine->GetPandaVM()->GetHeapManager()->GetMaxMemory());
}

extern "C" void StdGCRegisterNativeAllocation(EtsLong size)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    if (size < 0) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NEGATIVE_ARRAY_SIZE_EXCEPTION,
                          "The value must be non negative");
        return;
    }

    ScopedNativeCodeThread s(ManagedThread::GetCurrent());
    coroutine->GetVM()->GetGC()->RegisterNativeAllocation(ClampToSizeT(size));
}

extern "C" void StdGCRegisterNativeFree(EtsLong size)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    if (size < 0) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::NEGATIVE_ARRAY_SIZE_EXCEPTION,
                          "The value must be non negative");
        return;
    }

    ScopedNativeCodeThread s(ManagedThread::GetCurrent());
    coroutine->GetVM()->GetGC()->RegisterNativeFree(ClampToSizeT(size));
}

}  // namespace panda::ets::intrinsics
