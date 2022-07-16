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

#include <mem/gc/hybrid-gc/hybrid_object_allocator.h>

#include "libpandabase/mem/mem.h"
#include "runtime/include/class.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/humongous_obj_allocator-inl.h"
#include "runtime/mem/region_allocator-inl.h"

namespace panda::mem {
HybridObjectAllocator::HybridObjectAllocator(mem::MemStatsType *mem_stats, bool create_pygote_space_allocator)
    : ObjectAllocatorBase(mem_stats, GCCollectMode::GC_ALL, create_pygote_space_allocator)
{
    // TODO(ipetrov): Where do we use HybridObjectAllocator? Maybe remove?
    const auto &options = Runtime::GetOptions();
    heap_space_.Initialize(options.GetInitYoungSpaceSize(), options.WasSetInitYoungSpaceSize(),
                           options.GetYoungSpaceSize(), options.WasSetYoungSpaceSize(),
                           MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                           options.GetMinHeapFreePercentage(), options.GetMaxHeapFreePercentage());
    if (create_pygote_space_allocator) {
        ASSERT(pygote_space_allocator_ != nullptr);
        pygote_space_allocator_->SetHeapSpace(&heap_space_);
    }
    object_allocator_ = new (std::nothrow) ObjectAllocator(mem_stats, &heap_space_);
    large_object_allocator_ = new (std::nothrow) LargeObjectAllocator(mem_stats);
    humongous_object_allocator_ = new (std::nothrow) HumongousObjectAllocator(mem_stats);
}

void *HybridObjectAllocator::Allocate(size_t size, Alignment align, [[maybe_unused]] panda::ManagedThread *thread)
{
    void *mem = nullptr;
    size_t aligned_size = AlignUp(size, GetAlignmentInBytes(align));
    mem = object_allocator_->Alloc(aligned_size, align);
    return mem;
}

void *HybridObjectAllocator::AllocateInLargeAllocator(size_t size, Alignment align, BaseClass *base_cls)
{
    if (base_cls->IsDynamicClass()) {
        return nullptr;
    }
    auto cls = static_cast<Class *>(base_cls);
    size_t aligned_size = AlignUp(size, GetAlignmentInBytes(align));
    void *mem = nullptr;
    if ((aligned_size >= GetLargeThreshold()) &&
        (cls->IsStringClass() || (cls->IsArrayClass() && cls->GetComponentType()->IsPrimitive()))) {
        if (aligned_size <= LargeObjectAllocator::GetMaxSize()) {
            mem = large_object_allocator_->Alloc(size, align);
            if (UNLIKELY(mem == nullptr)) {
                size_t pool_size = std::max(PANDA_DEFAULT_POOL_SIZE, LargeObjectAllocator::GetMinPoolSize());
                auto pool = heap_space_.TryAllocPool(pool_size, SpaceType::SPACE_TYPE_OBJECT,
                                                     LargeObjectAllocator::GetAllocatorType(), large_object_allocator_);
                if (pool.GetMem() == nullptr ||
                    !large_object_allocator_->AddMemoryPool(pool.GetMem(), pool.GetSize())) {
                    LOG(FATAL, ALLOC) << "HybridObjectAllocator: couldn't add memory pool to large object allocator";
                }
                mem = large_object_allocator_->Alloc(size, align);
            }
        } else {
            mem = humongous_object_allocator_->Alloc(size, align);
            if (UNLIKELY(mem == nullptr)) {
                size_t pool_size;
                pool_size = std::max(PANDA_DEFAULT_POOL_SIZE, HumongousObjectAllocator::GetMinPoolSize(size));
                auto pool =
                    heap_space_.TryAllocPool(pool_size, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                                             HumongousObjectAllocator::GetAllocatorType(), humongous_object_allocator_);
                if (pool.GetMem() == nullptr ||
                    !humongous_object_allocator_->AddMemoryPool(pool.GetMem(), pool.GetSize())) {
                    LOG(FATAL, ALLOC)
                        << "HybridObjectAllocator: couldn't add memory pool to humongous object allocator";
                }
                mem = humongous_object_allocator_->Alloc(size, align);
            }
        }
    }
    return mem;
}

bool HybridObjectAllocator::ContainObject(const ObjectHeader *obj) const
{
    if (object_allocator_->ContainObject(obj)) {
        return true;
    }
    if (large_object_allocator_->ContainObject(obj)) {
        return true;
    }
    if (humongous_object_allocator_->ContainObject(obj)) {
        return true;
    }
    return false;
}

bool HybridObjectAllocator::IsLive(const ObjectHeader *obj)
{
    if (object_allocator_->ContainObject(obj)) {
        return object_allocator_->IsLive(obj);
    }
    if (large_object_allocator_->ContainObject(obj)) {
        return large_object_allocator_->IsLive(obj);
    }
    if (humongous_object_allocator_->ContainObject(obj)) {
        return humongous_object_allocator_->IsLive(obj);
    }
    return false;
}

size_t HybridObjectAllocator::VerifyAllocatorStatus()
{
    // TODO: implement it.
    return 0;
}

TLAB *HybridObjectAllocator::CreateNewTLAB(ManagedThread *thread)
{
    return object_allocator_->CreateNewTLAB(thread);
}

size_t HybridObjectAllocator::GetTLABMaxAllocSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

HybridObjectAllocator::~HybridObjectAllocator()
{
    delete object_allocator_;
    delete large_object_allocator_;
    delete humongous_object_allocator_;
}

}  // namespace panda::mem
