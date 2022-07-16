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

#ifndef PANDA_MEM_MEMORY_MANAGER_H
#define PANDA_MEM_MEMORY_MANAGER_H

#include <macros.h>
#include <mem/mem.h>
#include <runtime/mem/gc/gc_types.h>
#include <runtime/mem/heap_manager.h>

namespace panda {
class RuntimeOptions;
}  // namespace panda
namespace panda::mem {

class GCStats;
class GCTrigger;
class GCSettings;
class GCTriggerConfig;
class Allocator;

/*
 * Relations between VMs, MemoryManager and Runtime:
 *
 * +-----------------------------------------------+
 * |                                               |
 * |                  Runtime                      |
 * |                                               |
 * |  +----------+  +----------+     +----------+  |
 * |  |          |  |          |     |          |  |
 * |  |   VM_0   |  |   VM_1   |     |   VM_N   |  |
 * |  |          |  |          |     |          |  |
 * |  |          |  |          | ... |          |  |
 * |  |  +----+  |  |  +----+  |     |  +----+  |  |
 * |  |  |MM_0|  |  |  |MM_1|  |     |  |MM_N|  |  |
 * |  |  +----+  |  |  +----+  |     |  +----+  |  |
 * |  +----------+  +----------+     +----------+  |
 * |         \           |            /            |
 * |          \          |           /             |
 * |           +--------------------+              |
 * |           | Internal Allocator |              |
 * |           +--------------------+              |
 * +-----------------------------------------------+
 */

/**
 * A class that encapsulates components for working with memory.
 * Each VM is allocated its own instance.
 */
class MemoryManager {
public:
    struct HeapOptions {
        HeapManager::IsObjectFinalizebleFunc is_object_finalizeble_func;
        HeapManager::RegisterFinalizeReferenceFunc register_finalize_reference_func;
        uint32_t max_global_ref_size;
        bool is_global_reference_size_check_enabled;
        bool is_single_thread;
        bool is_use_tlab_for_allocations;
        bool is_start_as_zygote;
    };

    static MemoryManager *Create(const LanguageContext &ctx, InternalAllocatorPtr internal_allocator, GCType gc_type,
                                 const GCSettings &gc_settings, const GCTriggerConfig &gc_trigger_config,
                                 const HeapOptions &heap_options);
    static void Destroy(MemoryManager *mm);

    NO_COPY_SEMANTIC(MemoryManager);
    NO_MOVE_SEMANTIC(MemoryManager);

    void PreStartup();
    void PreZygoteFork();
    void PostZygoteFork();
    void InitializeGC(PandaVM *vm);
    void StartGC();
    void StopGC();

    void Finalize();

    HeapManager *GetHeapManager()
    {
        ASSERT(heap_manager_ != nullptr);
        return heap_manager_;
    }

    GC *GetGC() const
    {
        ASSERT(gc_ != nullptr);
        return gc_;
    }

    GCTrigger *GetGCTrigger()
    {
        ASSERT(gc_trigger_ != nullptr);
        return gc_trigger_;
    }

    GCStats *GetGCStats()
    {
        ASSERT(gc_stats_ != nullptr);
        return gc_stats_;
    }

    GlobalObjectStorage *GetGlobalObjectStorage() const
    {
        ASSERT(global_object_storage_ != nullptr);
        return global_object_storage_;
    }

    MemStatsType *GetMemStats()
    {
        ASSERT(mem_stats_ != nullptr);
        return mem_stats_;
    }

private:
    explicit MemoryManager(InternalAllocatorPtr internal_allocator, HeapManager *heap_manager, GC *gc,
                           GCTrigger *gc_trigger, GCStats *gc_stats, MemStatsType *mem_stats,
                           GlobalObjectStorage *global_object_storage)
        : internal_allocator_(internal_allocator),
          heap_manager_(heap_manager),
          gc_(gc),
          gc_trigger_(gc_trigger),
          gc_stats_(gc_stats),
          global_object_storage_(global_object_storage),
          mem_stats_(mem_stats)
    {
    }
    ~MemoryManager();

    InternalAllocatorPtr internal_allocator_;
    HeapManager *heap_manager_;
    GC *gc_;
    GCTrigger *gc_trigger_;
    GCStats *gc_stats_;
    GlobalObjectStorage *global_object_storage_;
    MemStatsType *mem_stats_;

    friend class mem::Allocator;
};

}  // namespace panda::mem

#endif  // PANDA_MEM_MEMORY_MANAGER_H
