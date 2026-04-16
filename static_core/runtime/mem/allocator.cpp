/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
// These includes to avoid linker error:

#include "runtime/arch/memory_helpers.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/allocator-inl.h"
#include "libarkbase/mem/mem_pool.h"
#include "libarkbase/mem/mem_config.h"
#include "libarkbase/mem/mem.h"
#include "runtime/include/mutator.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/object_header.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/bump-allocator-inl.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/tlab.h"

namespace ark::mem {

Allocator::~Allocator() = default;

ObjectAllocatorBase::ObjectAllocatorBase(MemStatsType *memStats, GCCollectMode gcCollectMode,
                                         bool createPygoteSpaceAllocator)
    : Allocator(memStats, AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT, gcCollectMode)
{
    if (createPygoteSpaceAllocator) {
        pygoteSpaceAllocator_ = new (std::nothrow) PygoteAllocator(memStats);
        pygoteAllocEnabled_ = true;
    }
}

ObjectAllocatorBase::~ObjectAllocatorBase()
{
    // NOLINTNEXTLINE(readability-delete-null-pointer)
    if (pygoteSpaceAllocator_ != nullptr) {
        delete pygoteSpaceAllocator_;
        pygoteSpaceAllocator_ = nullptr;
    }
}

bool ObjectAllocatorBase::HaveEnoughPoolsInObjectSpace(size_t poolsNum) const
{
    auto memPool = PoolManager::GetMmapMemPool();
    auto poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, PANDA_DEFAULT_ALLOCATOR_POOL_SIZE);
    return memPool->HaveEnoughPoolsInObjectSpace(poolsNum, poolSize);
}

void ObjectAllocatorBase::MemoryInitialize(void *mem, size_t size) const
{
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    memset_s(mem, size, 0, size);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // zero init should be visible from other threads even if pointer to object was fetched
    // without 'volatile' specifier so full memory barrier is required
    // required by some language-specs
    arch::FullMemoryBarrier();
}

void ObjectAllocatorBase::ObjectMemoryInit(void *mem, size_t size) const
{
    if (mem == nullptr) {
        return;
    }
    [[maybe_unused]] auto *object = static_cast<ObjectHeader *>(mem);
    ASSERT(object->AtomicGetMark().GetValue() == 0);
    ASSERT(object->ClassAddr<BaseClass *>() == nullptr);
    ASSERT(size >= ObjectHeader::ObjectHeaderSize());
    // zeroing according to newobj description in ISA
    size_t sizeToInit = size - ObjectHeader::ObjectHeaderSize();
    void *memToInit = ToVoidPtr(ToUintPtr(mem) + ObjectHeader::ObjectHeaderSize());
    MemoryInitialize(memToInit, sizeToInit);
}

void ObjectAllocatorBase::IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    ScopedChangeMutatorStatus ets(thread, MutatorStatus::RUNNING);
    ScopedSuspendAllThreadsRunning ssatr(thread->GetVM()->GetRendezvous());
    IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
ObjectAllocatorNoGen<MT_MODE>::ObjectAllocatorNoGen(MemStatsType *memStats, bool createPygoteSpaceAllocator)
    : ObjectAllocatorBase(memStats, GCCollectMode::GC_ALL, createPygoteSpaceAllocator)
{
    const auto &options = Runtime::GetOptions();
    heapSpace_.Initialize(MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                          options.GetMinHeapFreePercentage(), options.GetMaxHeapFreePercentage());
    if (createPygoteSpaceAllocator) {
        ASSERT(pygoteSpaceAllocator_ != nullptr);
        pygoteSpaceAllocator_->SetHeapSpace(&heapSpace_);
    }
    objectAllocator_ = new (std::nothrow) ObjectAllocator(memStats);
    ASSERT(objectAllocator_ != nullptr);
    largeObjectAllocator_ = new (std::nothrow) LargeObjectAllocator(memStats);
    ASSERT(largeObjectAllocator_ != nullptr);
    humongousObjectAllocator_ = new (std::nothrow) HumongousObjectAllocator(memStats);
    ASSERT(humongousObjectAllocator_ != nullptr);
}

template <MTModeT MT_MODE>
ObjectAllocatorNoGen<MT_MODE>::~ObjectAllocatorNoGen()
{
    delete objectAllocator_;
    delete largeObjectAllocator_;
    delete humongousObjectAllocator_;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorNoGen<MT_MODE>::Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                              ObjMemInitPolicy objInit, [[maybe_unused]] bool pinned)
{
    void *mem = nullptr;
    size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
    if (alignedSize <= ObjectAllocator::GetMaxSize()) {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, ObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe(size, align, objectAllocator_, poolSize, SpaceType::SPACE_TYPE_OBJECT, &heapSpace_);
    } else if (alignedSize <= LargeObjectAllocator::GetMaxSize()) {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, LargeObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe(size, align, largeObjectAllocator_, poolSize, SpaceType::SPACE_TYPE_OBJECT, &heapSpace_);
    } else {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, HumongousObjectAllocator::GetMinPoolSize(size));
        mem = AllocateSafe(size, align, humongousObjectAllocator_, poolSize, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                           &heapSpace_);
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorNoGen<MT_MODE>::AllocateNonMovable(size_t size, Alignment align, ark::ManagedThread *thread,
                                                        ObjMemInitPolicy objInit)
{
    void *mem = nullptr;
    // before pygote fork, allocate small non-movable objects in pygote space
    if (UNLIKELY(IsPygoteAllocEnabled() && pygoteSpaceAllocator_->CanAllocNonMovable(size, align))) {
        mem = pygoteSpaceAllocator_->Alloc(size, align);
    } else {
        // Without generations - no compaction now, so all allocations are non-movable
        mem = Allocate(size, align, thread, objInit, false);
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
Alignment ObjectAllocatorNoGen<MT_MODE>::CalculateAllocatorAlignment(size_t align)
{
    ASSERT(GetPurpose() == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
    return GetAlignment(align);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveAllPools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveAllPools(memVisitor);
    largeObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveFreePools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveFreePools(memVisitor);
    largeObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    objectAllocator_->IterateOverObjects(objectVisitor);
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::IterateRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    objectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::FreeObjectsMovedToPygoteSpace()
{
    // clear because we have move all objects in it to pygote space
    objectAllocator_->VisitAndRemoveAllPools(
        [](void *mem, size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    delete objectAllocator_;
    objectAllocator_ = new (std::nothrow) ObjectAllocator(memStats_);
    ASSERT(objectAllocator_ != nullptr);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::Collect(const GCObjectVisitor &gcObjectVisitor,
                                            [[maybe_unused]] GCCollectMode collectMode)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->Collect(gcObjectVisitor);
    }
    objectAllocator_->Collect(gcObjectVisitor);
    largeObjectAllocator_->Collect(gcObjectVisitor);
    humongousObjectAllocator_->Collect(gcObjectVisitor);
}

// if there is a common base class for these allocators, we could split this func and return the pointer to the
// allocator containing the object
template <MTModeT MT_MODE>
bool ObjectAllocatorNoGen<MT_MODE>::ContainObject(const ObjectHeader *obj) const
{
    if (objectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (largeObjectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return true;
    }

    return false;
}

template <MTModeT MT_MODE>
bool ObjectAllocatorNoGen<MT_MODE>::IsLive(const ObjectHeader *obj)
{
    if (pygoteSpaceAllocator_ != nullptr && pygoteSpaceAllocator_->ContainObject(obj)) {
        return pygoteSpaceAllocator_->IsLive(obj);
    }
    if (objectAllocator_->ContainObject(obj)) {
        return objectAllocator_->IsLive(obj);
    }
    if (largeObjectAllocator_->ContainObject(obj)) {
        return largeObjectAllocator_->IsLive(obj);
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return humongousObjectAllocator_->IsLive(obj);
    }
    return false;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorNoGen<MT_MODE>::GetRegularObjectMaxSize()
{
    return ObjectAllocator::GetMaxSize();
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorNoGen<MT_MODE>::GetLargeObjectMaxSize()
{
    return LargeObjectAllocator::GetMaxSize();
}

template <MTModeT MT_MODE>
TLAB *ObjectAllocatorNoGen<MT_MODE>::CreateNewTLAB([[maybe_unused]] size_t tlabSize)
{
    LOG(FATAL, ALLOC) << "TLAB is not supported for this allocator";
    return nullptr;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorNoGen<MT_MODE>::GetTLABMaxAllocSize()
{
    // NOTE(aemelenko): TLAB usage is not supported for non-gen GCs.
    return 0;
}

ObjectAllocatorGenBase::ObjectAllocatorGenBase(MemStatsType *memStats, GCCollectMode gcCollectMode,
                                               bool createPygoteSpaceAllocator)
    : ObjectAllocatorBase(memStats, gcCollectMode, createPygoteSpaceAllocator)
{
    const auto &options = Runtime::GetOptions();
    heapSpaces_.Initialize(options.GetInitYoungSpaceSize(), options.WasSetInitYoungSpaceSize(),
                           options.GetYoungSpaceSize(), options.WasSetYoungSpaceSize(),
                           MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                           options.GetMinHeapFreePercentage(), options.GetMaxHeapFreePercentage());
    if (createPygoteSpaceAllocator) {
        ASSERT(pygoteSpaceAllocator_ != nullptr);
        pygoteSpaceAllocator_->SetHeapSpace(&heapSpaces_);
    }
}

void ObjectAllocatorGenBase::InvalidateSpaceData()
{
    ranges_.clear();
    youngBitmaps_.clear();
}

template class ObjectAllocatorNoGen<MT_MODE_SINGLE>;
template class ObjectAllocatorNoGen<MT_MODE_MULTI>;
template class ObjectAllocatorNoGen<MT_MODE_TASK>;

}  // namespace ark::mem
