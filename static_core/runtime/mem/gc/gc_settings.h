/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H
#define PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H

#include <cstddef>
#include <string_view>
#include <cstdint>

namespace panda {
class RuntimeOptions;
}  // namespace panda

namespace panda::mem {

enum class NativeGcTriggerType { INVALID_NATIVE_GC_TRIGGER, NO_NATIVE_GC_TRIGGER, SIMPLE_STRATEGY };
inline NativeGcTriggerType NativeGcTriggerTypeFromString(std::string_view native_gc_trigger_type_str)
{
    if (native_gc_trigger_type_str == "no-native-gc-trigger") {
        return NativeGcTriggerType::NO_NATIVE_GC_TRIGGER;
    }
    if (native_gc_trigger_type_str == "simple-strategy") {
        return NativeGcTriggerType::SIMPLE_STRATEGY;
    }
    return NativeGcTriggerType::INVALID_NATIVE_GC_TRIGGER;
}

class GCSettings {
public:
    GCSettings() = default;
    explicit GCSettings(const RuntimeOptions &options, panda_file::SourceLang lang);

    /// @brief if true then enable tracing
    bool IsGcEnableTracing() const;

    /// @brief type of native trigger
    NativeGcTriggerType GetNativeGcTriggerType() const;

    /// @brief dump heap at the beginning and the end of GC
    bool IsDumpHeap() const;

    /// @brief true if concurrency enabled
    bool IsConcurrencyEnabled() const;

    /// @brief true if explicit GC should be running in concurrent
    bool IsExplicitConcurrentGcEnabled() const;

    /// @brief true if GC should be running in place
    bool RunGCInPlace() const;

    /// @brief use FastHeapVerifier if true
    bool EnableFastHeapVerifier() const;

    /// @brief true if heap verification before GC enabled
    bool PreGCHeapVerification() const;

    /// @brief true if heap verification during GC enabled
    bool IntoGCHeapVerification() const;

    /// @brief true if heap verification after GC enabled
    bool PostGCHeapVerification() const;

    /// true if heap verification before G1-GC concurrent phase enabled
    bool BeforeG1ConcurrentHeapVerification() const;

    /// @brief if true then fail execution if heap verifier found heap corruption
    bool FailOnHeapVerification() const;

    /// @brief if true then run gc every savepoint
    bool RunGCEverySafepoint() const;

    /// @brief garbage rate threshold of a tenured region to be included into a mixed collection
    double G1RegionGarbageRateThreshold() const;

    /// @brief runs full collection one of N times in GC thread
    uint32_t FullGCBombingFrequency() const;

    /// @brief specify a max number of tenured regions which can be collected at mixed collection in G1GC.
    uint32_t GetG1NumberOfTenuredRegionsAtMixedCollection() const;

    /// @brief minimum percentage of alive bytes in young region to promote it into tenured without moving for G1GC
    double G1PromotionRegionAliveRate() const;

    /**
     * @brief minimum percentage of not used bytes in tenured region to collect it on full even if we have no garbage
     * inside
     */
    double G1FullGCRegionFragmentationRate() const;

    /**
     * @brief Specify if we need to track removing objects
     * (i.e. update objects count in memstats and log removed objects) during the G1GC collection or not.
     */
    bool G1TrackFreedObjects() const;

    /// @brief if not zero and gc supports workers, create a gc workers and try to use them
    size_t GCWorkersCount() const;

    void SetGCWorkersCount(size_t value);

    bool ManageGcThreadsAffinity() const;

    bool UseWeakCpuForGcConcurrent() const;

    /// @return true if thread pool is used, false - if task manager is used
    bool UseThreadPoolForGC() const;

    /// @return true if task manager is used, false - if thread pool is used
    bool UseTaskManagerForGC() const;

    /**
     * @brief Limit the creation rate of tasks during marking in nanoseconds.
     * if average task creation is less than this value - it increases the stack size limit to create tasks less
     * frequently.
     */
    size_t GCMarkingStackNewTasksFrequency() const;

    /**
     * @brief Max stack size for marking in main thread, if it exceeds we will send a new task to workers,
     * 0 means unlimited.
     */
    size_t GCRootMarkingStackMaxSize() const;

    /**
     * @brief Max stack size for marking in a gc worker, if it exceeds we will send a new task to workers,
     * 0 means unlimited.
     */
    size_t GCWorkersMarkingStackMaxSize() const;

    /// @brief size of young-space
    uint64_t YoungSpaceSize() const;

    /// @brief true if print INFO log to get more detailed information in GC.
    bool LogDetailedGCInfoEnabled() const;

    /// @brief true if print INFO log to get more detailed compaction/promotion information per region in GC.
    bool LogDetailedGCCompactionInfoEnabled() const;

    /// @brief true if we want to do marking phase in multithreading mode.
    bool ParallelMarkingEnabled() const;

    void SetParallelMarkingEnabled(bool value);

    /// @brief true if we want to do compacting phase in multithreading mode.
    bool ParallelCompactingEnabled() const;

    void SetParallelCompactingEnabled(bool value);

    /// @brief true if we want to do ref updating phase in multithreading mode
    bool ParallelRefUpdatingEnabled() const;

    void SetParallelRefUpdatingEnabled(bool value);

    /// @brief true if G1 should updates remsets concurrently
    bool G1EnableConcurrentUpdateRemset() const;

    /// @brief
    size_t G1MinConcurrentCardsToProcess() const;

    bool G1EnablePauseTimeGoal() const;

    uint32_t GetG1MaxGcPauseInMillis() const;

    uint32_t GetG1GcPauseIntervalInMillis() const;

private:
    // clang-tidy complains about excessive padding
    /// Garbage rate threshold of a tenured region to be included into a mixed collection
    double g1_region_garbage_rate_threshold_ = 0;
    /**
     * Minimum percentage of not used bytes in tenured region to
     * collect it on full even if we have no garbage inside
     */
    double g1_full_gc_region_fragmentation_rate_ = 0;
    /**
     * Limit the creation rate of tasks during marking in nanoseconds.
     * If average task creation is less than this value - it increases the stack size limit twice to create tasks less
     * frequently.
     */
    size_t gc_marking_stack_new_tasks_frequency_ = 0;
    /// Max stack size for marking in main thread, if it exceeds we will send a new task to workers, 0 means unlimited.
    size_t gc_root_marking_stack_max_size_ = 0;
    /// Max stack size for marking in a gc worker, if it exceeds we will send a new task to workers, 0 means unlimited.
    size_t gc_workers_marking_stack_max_size_ = 0;
    /// If not zero and gc supports workers, create a gc workers and try to use them
    size_t gc_workers_count_ = 0;
    /// Size of young-space
    uint64_t young_space_size_ = 0;
    size_t g1_min_concurrent_cards_to_process_ = 0;
    /// Type of native trigger
    NativeGcTriggerType native_gc_trigger_type_ = {NativeGcTriggerType::INVALID_NATIVE_GC_TRIGGER};
    /// Runs full collection one of N times in GC thread
    uint32_t full_gc_bombing_frequency_ = 0;
    /// Specify a max number of tenured regions which can be collected at mixed collection in G1GC.
    uint32_t g1_number_of_tenured_regions_at_mixed_collection_ = 0;
    /// Minimum percentage of alive bytes in young region to promote it into tenured without moving for G1GC
    uint32_t g1_promotion_region_alive_rate_ = 0;
    uint32_t g1_max_gc_pause_ms_ = 0;
    uint32_t g1_gc_pause_interval_ms_ = 0;
    /// If true then enable tracing
    bool is_gc_enable_tracing_ = false;
    /// Dump heap at the beginning and the end of GC
    bool is_dump_heap_ = false;
    /// True if concurrency enabled
    bool is_concurrency_enabled_ = true;
    /// True if explicit GC should be running in concurrent
    bool is_explicit_concurrent_gc_enabled_ = true;
    /// True if GC should be running in place
    bool run_gc_in_place_ = false;
    /// Use FastHeapVerifier if true
    bool enable_fast_heap_verifier_ = true;
    /// True if heap verification before GC enabled
    bool pre_gc_heap_verification_ = false;
    /// True if heap verification during GC enabled
    bool into_gc_heap_verification_ = false;
    /// True if heap verification after GC enabled
    bool post_gc_heap_verification_ = false;
    /// True if heap verification before G1-GC concurrent phase enabled
    bool before_g1_concurrent_heap_verification_ = false;
    /// If true then fail execution if heap verifier found heap corruption
    bool fail_on_heap_verification_ = false;
    /// If true then run gc every savepoint
    bool run_gc_every_safepoint_ = false;
    bool manage_gc_threads_affinity_ = true;
    bool use_weak_cpu_for_gc_concurrent_ = false;
    /// if true then thread pool is used, false - task manager is used
    bool use_thread_pool_for_gc_workers_ = true;
    /// If we need to track removing objects during the G1GC collection or not
    bool g1_track_freed_objects_ = false;
    /// True if print INFO log to get more detailed information in GC
    bool log_detailed_gc_info_enabled_ = false;
    /// True if print INFO log to get more detailed compaction/promotion information per region in GC
    bool log_detailed_gc_compaction_info_enabled_ = false;
    /// True if we want to do marking phase in multithreading mode
    bool parallel_marking_enabled_ = false;
    /// True if we want to do compacting phase in multithreading mode
    bool parallel_compacting_enabled_ = false;
    /// True if we want to do ref updating phase in multithreading mode
    bool parallel_ref_updating_enabled_ = false;
    /// True if G1 should updates remsets concurrently
    bool g1_enable_concurrent_update_remset_ = false;
    bool g1_enable_pause_time_goal_ {false};
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H
