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

    /**
     * \brief if true then enable tracing
     */
    bool IsGcEnableTracing() const;

    /**
     * \brief type of native trigger
     */
    NativeGcTriggerType GetNativeGcTriggerType() const;

    /**
     * \brief dump heap at the beginning and the end of GC
     */
    bool IsDumpHeap() const;

    /**
     * \brief true if concurrency enabled
     */
    bool IsConcurrencyEnabled() const;

    /**
     * \brief true if GC should be running in place
     */
    bool RunGCInPlace() const;

    /**
     * \brief use FastHeapVerifier if true
     */
    bool EnableFastHeapVerifier() const;

    /**
     * \brief true if heap verification before GC enabled
     */
    bool PreGCHeapVerification() const;

    /**
     * \brief true if heap verification during GC enabled
     */
    bool IntoGCHeapVerification() const;

    /**
     * \brief true if heap verification after GC enabled
     */
    bool PostGCHeapVerification() const;

    /**
     * \brief if true then fail execution if heap verifier found heap corruption
     */
    bool FailOnHeapVerification() const;

    /**
     * \brief garbage rate threshold of a tenured region to be included into a mixed collection
     */
    double G1RegionGarbageRateThreshold() const;

    /**
     * \brief minimum percentage of alive bytes in young region to promote it into tenured without moving for G1GC
     */
    double G1PromotionRegionAliveRate() const;

    /**
     * \brief Specify if we need to track removing objects
     * (i.e. update objects count in memstats and log removed objects) during the G1GC collection or not.
     */
    bool G1TrackFreedObjects() const;

    /**
     * \brief if not zero and gc supports workers, create a gc workers and try to use them
     */
    size_t GCWorkersCount() const;

    void SetGCWorkersCount(size_t value);

    /**
     * \brief Max stack size for marking in main thread, if it exceeds we will send a new task to workers,
     * 0 means unlimited.
     */
    size_t GCRootMarkingStackMaxSize() const;

    /**
     * \brief Max stack size for marking in a gc worker, if it exceeds we will send a new task to workers,
     * 0 means unlimited.
     */
    size_t GCWorkersMarkingStackMaxSize() const;

    /**
     * \brief size of young-space
     */
    uint64_t YoungSpaceSize() const;

    /**
     * \brief true if print INFO log to get more detailed information in GC.
     */
    bool LogDetailedGCInfoEnabled() const;

    /**
     * \brief true if we want to do marking phase in multithreading mode.
     */
    bool ParallelMarkingEnabled() const;

    void SetParallelMarkingEnabled(bool value);

    /**
     * \brief true if we want to do compacting phase in multithreading mode.
     */
    bool ParallelCompactingEnabled() const;

    void SetParallelCompactingEnabled(bool value);

    /**
     * \brief true if G1 should updates remsets concurrently
     */
    bool G1EnableConcurrentUpdateRemset() const;

    /**
     * \brief
     */
    size_t G1MinConcurrentCardsToProcess() const;

private:
    bool is_gc_enable_tracing_ = false;  /// if true then enable tracing
    NativeGcTriggerType native_gc_trigger_type_ = {
        NativeGcTriggerType::INVALID_NATIVE_GC_TRIGGER};  /// type of native trigger
    bool is_dump_heap_ = false;                           /// dump heap at the beginning and the end of GC
    bool is_concurrency_enabled_ = true;                  /// true if concurrency enabled
    bool run_gc_in_place_ = false;                        /// true if GC should be running in place
    bool enable_fast_heap_verifier_ = true;               /// use FastHeapVerifier if true
    bool pre_gc_heap_verification_ = false;               /// true if heap verification before GC enabled
    bool into_gc_heap_verification_ = false;              /// true if heap verification during GC enabled
    bool post_gc_heap_verification_ = false;              /// true if heap verification after GC enabled
    bool fail_on_heap_verification_ = false;  /// if true then fail execution if heap verifier found heap corruption
    double g1_region_garbage_rate_threshold_ =
        0;  /// garbage rate threshold of a tenured region to be included into a mixed collection
    uint32_t g1_promotion_region_alive_rate_ =
        0;  /// minimum percentage of alive bytes in young region to promote it into tenured without moving for G1GC
    size_t gc_workers_count_ = 0;  /// if not zero and gc supports workers, create a gc workers and try to use them
    /// Max stack size for marking in main thread, if it exceeds we will send a new task to workers, 0 means unlimited.
    size_t gc_root_marking_stack_max_size_ = 0;
    /// Max stack size for marking in a gc worker, if it exceeds we will send a new task to workers, 0 means unlimited.
    size_t gc_workers_marking_stack_max_size_ = 0;
    bool g1_track_freed_objects_ = false;  /// if we need to track removing objects during the G1GC collection or not
    uint64_t young_space_size_ = 0;        /// size of young-space
    bool log_detailed_gc_info_enabled_ = false;  /// true if print INFO log to get more detailed information in GC.
    bool parallel_marking_enabled_ = false;      /// true if we want to do marking phase in multithreading mode.
    bool parallel_compacting_enabled_ = false;   /// true if we want to do compacting phase in multithreading mode.
    bool g1_enable_concurrent_update_remset_ = false;  /// true if G1 should updates remsets concurrently
    size_t g1_min_concurrent_cards_to_process_ = 0;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H