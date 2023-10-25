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

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/mem/gc/gc_settings.h"
#include "libpandabase/globals.h"

namespace panda::mem {

GCSettings::GCSettings(const RuntimeOptions &options, panda_file::SourceLang lang)
{
    auto runtime_lang = plugins::LangToRuntimeType(lang);
    is_dump_heap_ = options.IsGcDumpHeap(runtime_lang);
    is_concurrency_enabled_ = options.IsConcurrentGcEnabled(runtime_lang);
    is_gc_enable_tracing_ = options.IsGcEnableTracing(runtime_lang);
    is_explicit_concurrent_gc_enabled_ = options.IsExplicitConcurrentGcEnabled();
    run_gc_in_place_ = options.IsRunGcInPlace(runtime_lang);
    full_gc_bombing_frequency_ = options.GetFullGcBombingFrequency(runtime_lang);
    native_gc_trigger_type_ = NativeGcTriggerTypeFromString(options.GetNativeGcTriggerType(runtime_lang));
    enable_fast_heap_verifier_ = options.IsEnableFastHeapVerifier(runtime_lang);
    auto hv_params = options.GetHeapVerifier(runtime_lang);
    pre_gc_heap_verification_ = std::find(hv_params.begin(), hv_params.end(), "pre") != hv_params.end();
    into_gc_heap_verification_ = std::find(hv_params.begin(), hv_params.end(), "into") != hv_params.end();
    post_gc_heap_verification_ = std::find(hv_params.begin(), hv_params.end(), "post") != hv_params.end();
    before_g1_concurrent_heap_verification_ =
        std::find(hv_params.begin(), hv_params.end(), "before_g1_concurrent") != hv_params.end();
    fail_on_heap_verification_ =
        std::find(hv_params.begin(), hv_params.end(), "fail_on_verification") != hv_params.end();
    run_gc_every_safepoint_ = options.IsRunGcEverySafepoint();
    g1_region_garbage_rate_threshold_ = options.GetG1RegionGarbageRateThreshold() / PERCENT_100_D;
    g1_number_of_tenured_regions_at_mixed_collection_ = options.GetG1NumberOfTenuredRegionsAtMixedCollection();
    g1_promotion_region_alive_rate_ = options.GetG1PromotionRegionAliveRate();
    g1_full_gc_region_fragmentation_rate_ = options.GetG1FullGcRegionFragmentationRate() / PERCENT_100_D;
    g1_track_freed_objects_ = options.IsG1TrackFreedObjects();
    gc_workers_count_ = options.GetGcWorkersCount();
    manage_gc_threads_affinity_ = options.IsManageGcThreadsAffinity();
    use_weak_cpu_for_gc_concurrent_ = options.IsUseWeakCpuForGcConcurrent();
    use_thread_pool_for_gc_workers_ = Runtime::GetTaskScheduler() == nullptr;
    gc_marking_stack_new_tasks_frequency_ = options.GetGcMarkingStackNewTasksFrequency();
    gc_root_marking_stack_max_size_ = options.GetGcRootMarkingStackMaxSize();
    gc_workers_marking_stack_max_size_ = options.GetGcWorkersMarkingStackMaxSize();
    young_space_size_ = options.GetYoungSpaceSize();
    log_detailed_gc_info_enabled_ = options.IsLogDetailedGcInfoEnabled();
    log_detailed_gc_compaction_info_enabled_ = options.IsLogDetailedGcCompactionInfoEnabled();
    parallel_marking_enabled_ = options.IsGcParallelMarkingEnabled() && (options.GetGcWorkersCount() != 0);
    parallel_compacting_enabled_ = options.IsGcParallelCompactingEnabled() && (options.GetGcWorkersCount() != 0);
    parallel_ref_updating_enabled_ = options.IsGcParallelRefUpdatingEnabled() && (options.GetGcWorkersCount() != 0);
    g1_enable_concurrent_update_remset_ = options.IsG1EnableConcurrentUpdateRemset();
    g1_min_concurrent_cards_to_process_ = options.GetG1MinConcurrentCardsToProcess();
    g1_enable_pause_time_goal_ = options.IsG1PauseTimeGoal();
    g1_max_gc_pause_ms_ = options.GetG1PauseTimeGoalMaxGcPause();
    g1_gc_pause_interval_ms_ = options.WasSetG1PauseTimeGoalGcPauseInterval()
                                   ? options.GetG1PauseTimeGoalGcPauseInterval()
                                   : g1_max_gc_pause_ms_ + 1;
    LOG_IF(FullGCBombingFrequency() && RunGCInPlace(), FATAL, GC)
        << "full-gc-bombimg-frequency and run-gc-in-place options can't be used together";
}

bool GCSettings::IsGcEnableTracing() const
{
    return is_gc_enable_tracing_;
}

NativeGcTriggerType GCSettings::GetNativeGcTriggerType() const
{
    return native_gc_trigger_type_;
}

bool GCSettings::IsDumpHeap() const
{
    return is_dump_heap_;
}

bool GCSettings::IsConcurrencyEnabled() const
{
    return is_concurrency_enabled_;
}

bool GCSettings::IsExplicitConcurrentGcEnabled() const
{
    return is_explicit_concurrent_gc_enabled_;
}

bool GCSettings::RunGCInPlace() const
{
    return run_gc_in_place_;
}

bool GCSettings::EnableFastHeapVerifier() const
{
    return enable_fast_heap_verifier_;
}

bool GCSettings::PreGCHeapVerification() const
{
    return pre_gc_heap_verification_;
}

bool GCSettings::IntoGCHeapVerification() const
{
    return into_gc_heap_verification_;
}

bool GCSettings::PostGCHeapVerification() const
{
    return post_gc_heap_verification_;
}

bool GCSettings::BeforeG1ConcurrentHeapVerification() const
{
    return before_g1_concurrent_heap_verification_;
}

bool GCSettings::FailOnHeapVerification() const
{
    return fail_on_heap_verification_;
}

bool GCSettings::RunGCEverySafepoint() const
{
    return run_gc_every_safepoint_;
}

double GCSettings::G1RegionGarbageRateThreshold() const
{
    return g1_region_garbage_rate_threshold_;
}

uint32_t GCSettings::FullGCBombingFrequency() const
{
    return full_gc_bombing_frequency_;
}

uint32_t GCSettings::GetG1NumberOfTenuredRegionsAtMixedCollection() const
{
    return g1_number_of_tenured_regions_at_mixed_collection_;
}

double GCSettings::G1PromotionRegionAliveRate() const
{
    return g1_promotion_region_alive_rate_;
}

double GCSettings::G1FullGCRegionFragmentationRate() const
{
    return g1_full_gc_region_fragmentation_rate_;
}

bool GCSettings::G1TrackFreedObjects() const
{
    return g1_track_freed_objects_;
}

size_t GCSettings::GCWorkersCount() const
{
    return gc_workers_count_;
}

bool GCSettings::ManageGcThreadsAffinity() const
{
    return manage_gc_threads_affinity_;
}

bool GCSettings::UseWeakCpuForGcConcurrent() const
{
    return use_weak_cpu_for_gc_concurrent_;
}

void GCSettings::SetGCWorkersCount(size_t value)
{
    gc_workers_count_ = value;
}

bool GCSettings::UseThreadPoolForGC() const
{
    return use_thread_pool_for_gc_workers_;
}

bool GCSettings::UseTaskManagerForGC() const
{
    return !use_thread_pool_for_gc_workers_;
}

size_t GCSettings::GCMarkingStackNewTasksFrequency() const
{
    return gc_marking_stack_new_tasks_frequency_;
}

size_t GCSettings::GCRootMarkingStackMaxSize() const
{
    return gc_root_marking_stack_max_size_;
}

size_t GCSettings::GCWorkersMarkingStackMaxSize() const
{
    return gc_workers_marking_stack_max_size_;
}

uint64_t GCSettings::YoungSpaceSize() const
{
    return young_space_size_;
}

bool GCSettings::LogDetailedGCInfoEnabled() const
{
    return log_detailed_gc_info_enabled_;
}

bool GCSettings::LogDetailedGCCompactionInfoEnabled() const
{
    return log_detailed_gc_compaction_info_enabled_;
}

bool GCSettings::ParallelMarkingEnabled() const
{
    return parallel_marking_enabled_;
}

void GCSettings::SetParallelMarkingEnabled(bool value)
{
    parallel_marking_enabled_ = value;
}

bool GCSettings::ParallelCompactingEnabled() const
{
    return parallel_compacting_enabled_;
}

void GCSettings::SetParallelCompactingEnabled(bool value)
{
    parallel_compacting_enabled_ = value;
}

bool GCSettings::ParallelRefUpdatingEnabled() const
{
    return parallel_ref_updating_enabled_;
}

void GCSettings::SetParallelRefUpdatingEnabled(bool value)
{
    parallel_ref_updating_enabled_ = value;
}

bool GCSettings::G1EnableConcurrentUpdateRemset() const
{
    return g1_enable_concurrent_update_remset_;
}

size_t GCSettings::G1MinConcurrentCardsToProcess() const
{
    return g1_min_concurrent_cards_to_process_;
}

bool GCSettings::G1EnablePauseTimeGoal() const
{
    return g1_enable_pause_time_goal_;
}

uint32_t GCSettings::GetG1MaxGcPauseInMillis() const
{
    return g1_max_gc_pause_ms_;
}

uint32_t GCSettings::GetG1GcPauseIntervalInMillis() const
{
    return g1_gc_pause_interval_ms_;
}

}  // namespace panda::mem
