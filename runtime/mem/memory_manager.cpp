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

#include "memory_manager.h"
#include "runtime/include/runtime_options.h"
#include "runtime/mem/refstorage/global_object_storage.h"

#include <runtime/mem/gc/gc.h>
#include <runtime/mem/gc/gc_trigger.h>
#include <runtime/mem/gc/gc_stats.h>
#include <runtime/mem/heap_manager.h>

namespace panda::mem {

static HeapManager *CreateHeapManager(InternalAllocatorPtr internal_allocator,
                                      const MemoryManager::HeapOptions &options, GCType gc_type,
                                      MemStatsType *mem_stats)
{
    auto *heap_manager = new HeapManager();
    if (heap_manager == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to allocate HeapManager";
        return nullptr;
    }

    if (!heap_manager->Initialize(gc_type, options.is_single_thread, options.is_use_tlab_for_allocations, mem_stats,
                                  internal_allocator, options.is_start_as_zygote)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize HeapManager";
        return nullptr;
    }
    heap_manager->SetIsFinalizableFunc(options.is_object_finalizeble_func);
    heap_manager->SetRegisterFinalizeReferenceFunc(options.register_finalize_reference_func);

    return heap_manager;
}

/* static */
MemoryManager *MemoryManager::Create(const LanguageContext &ctx, InternalAllocatorPtr internal_allocator,
                                     GCType gc_type, const GCSettings &gc_settings,
                                     const GCTriggerConfig &gc_trigger_config, const HeapOptions &heap_options)
{
    std::unique_ptr<MemStatsType> mem_stats = std::make_unique<MemStatsType>();

    HeapManager *heap_manager = CreateHeapManager(internal_allocator, heap_options, gc_type, mem_stats.get());
    if (heap_manager == nullptr) {
        return nullptr;
    }

    InternalAllocatorPtr allocator = heap_manager->GetInternalAllocator();
    GCStats *gc_stats = allocator->New<GCStats>(mem_stats.get(), gc_type, allocator);
    GC *gc = ctx.CreateGC(gc_type, heap_manager->GetObjectAllocator().AsObjectAllocator(), gc_settings);
    GCTrigger *gc_trigger =
        CreateGCTrigger(mem_stats.get(), heap_manager->GetObjectAllocator().AsObjectAllocator()->GetHeapSpace(),
                        gc_trigger_config, allocator);

    GlobalObjectStorage *global_object_storage = internal_allocator->New<GlobalObjectStorage>(
        internal_allocator, heap_options.max_global_ref_size, heap_options.is_global_reference_size_check_enabled);
    if (global_object_storage == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to allocate GlobalObjectStorage";
        return nullptr;
    }

    return new MemoryManager(internal_allocator, heap_manager, gc, gc_trigger, gc_stats, mem_stats.release(),
                             global_object_storage);
}

/* static */
void MemoryManager::Destroy(MemoryManager *mm)
{
    delete mm;
}

MemoryManager::~MemoryManager()
{
    heap_manager_->GetInternalAllocator()->Delete(gc_);
    heap_manager_->GetInternalAllocator()->Delete(gc_trigger_);
    heap_manager_->GetInternalAllocator()->Delete(gc_stats_);
    heap_manager_->GetInternalAllocator()->Delete(global_object_storage_);

    delete heap_manager_;

    // One more check that we don't have memory leak in internal allocator.
    ASSERT(mem_stats_->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL) == 0);
    delete mem_stats_;
}

void MemoryManager::Finalize()
{
    heap_manager_->Finalize();
}

void MemoryManager::InitializeGC(PandaVM *vm)
{
    heap_manager_->SetPandaVM(vm);
    gc_->Initialize(vm);
    gc_->AddListener(gc_trigger_);
}

void MemoryManager::PreStartup()
{
    gc_->PreStartup();
}

void MemoryManager::PreZygoteFork()
{
    gc_->PreZygoteFork();
    heap_manager_->PreZygoteFork();
}

void MemoryManager::PostZygoteFork()
{
    gc_->PostZygoteFork();
}

void MemoryManager::StartGC()
{
    gc_->StartGC();
}

void MemoryManager::StopGC()
{
    gc_->StopGC();
}

}  // namespace panda::mem
