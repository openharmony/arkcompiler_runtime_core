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

#include "runtime/mem/gc/gc_trigger.h"

#include <algorithm>
#include <atomic>

#include "libpandabase/macros.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/panda_vm.h"
#include "utils/logger.h"

namespace panda::mem {

static constexpr size_t PERCENT_100 = 100;

GCTriggerConfig::GCTriggerConfig(const RuntimeOptions &options, panda_file::SourceLang lang)
{
    auto runtime_lang = plugins::LangToRuntimeType(lang);
    gc_trigger_type_ = options.GetGcTriggerType(runtime_lang);
    debug_start_ = options.GetGcDebugTriggerStart(runtime_lang);
    percent_threshold_ = std::min(options.GetGcTriggerPercentThreshold(), PERCENT_100_U32);
    adaptive_multiplier_ = options.GetGcTriggerAdaptiveMultiplier();
    min_extra_heap_size_ = options.GetMinExtraHeapSize();
    max_extra_heap_size_ = options.GetMaxExtraHeapSize();
    max_trigger_percent_ = std::min(options.GetMaxTriggerPercent(), PERCENT_100_U32);
    skip_startup_gc_count_ = options.GetSkipStartupGcCount(runtime_lang);
}

GCTrigger::~GCTrigger() = default;

GCTriggerHeap::GCTriggerHeap(MemStatsType *mem_stats, HeapSpace *heap_space)
    : GCTrigger(heap_space), mem_stats_(mem_stats)
{
}

GCTriggerHeap::GCTriggerHeap(MemStatsType *mem_stats, HeapSpace *heap_space, size_t min_heap_size,
                             uint8_t percent_threshold, size_t min_extra_size, size_t max_extra_size,
                             uint32_t skip_gc_times)
    : GCTrigger(heap_space), mem_stats_(mem_stats), skip_gc_count_(skip_gc_times)
{
    percent_threshold_ = percent_threshold;
    min_extra_size_ = min_extra_size;
    max_extra_size_ = max_extra_size;
    // If we have min_heap_size < 100, we get false positives in IsGcTriggered, since we divide by 100 first
    ASSERT(min_heap_size >= 100);
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    target_footprint_.store((min_heap_size / PERCENT_100) * percent_threshold_, std::memory_order_relaxed);
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeap created, min heap size " << min_heap_size << ", percent threshold "
                           << percent_threshold << ", min_extra_size " << min_extra_size << ", max_extra_size "
                           << max_extra_size;
}

void GCTriggerHeap::SetMinTargetFootprint(size_t target_size)
{
    LOG(DEBUG, GC_TRIGGER) << "SetTempTargetFootprint target_footprint = " << target_size;
    min_target_footprint_ = target_size;
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    target_footprint_.store(target_size, std::memory_order_relaxed);
}

void GCTriggerHeap::RestoreMinTargetFootprint()
{
    min_target_footprint_ = DEFAULT_MIN_TARGET_FOOTPRINT;
}

void GCTriggerHeap::ComputeNewTargetFootprint(const GCTask &task, size_t heap_size_before_gc, size_t heap_size)
{
    GC *gc = Thread::GetCurrent()->GetVM()->GetGC();
    if (gc->IsGenerational() && task.reason_ == GCTaskCause::YOUNG_GC_CAUSE) {
        // we don't want to update heap-trigger on young-gc
        return;
    }

    size_t target = this->ComputeTarget(heap_size_before_gc, heap_size);

    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    target_footprint_.store(target, std::memory_order_relaxed);

    LOG(DEBUG, GC_TRIGGER) << "ComputeNewTargetFootprint target_footprint = " << target;
}

size_t GCTriggerHeap::ComputeTarget(size_t heap_size_before_gc, size_t heap_size)
{
    // Note: divide by 100 first to avoid overflow
    size_t delta = (heap_size / PERCENT_100) * percent_threshold_;

    // heap increased corresponding with previous gc
    if (heap_size > heap_size_before_gc) {
        delta = std::min(delta, max_extra_size_);
    } else {
        // if heap was squeeze from 200mb to 100mb we want to set a target to 150mb, not just 100mb*percent_threshold_
        delta = std::max(delta, (heap_size_before_gc - heap_size) / 2);
    }
    return heap_size + std::max(delta, min_extra_size_);
}

bool GCTriggerHeap::IsGcTriggered()
{
    if (skip_gc_count_ > 0) {
        skip_gc_count_--;
        return false;
    }
    size_t bytes_in_heap = mem_stats_->GetFootprintHeap();
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (UNLIKELY(bytes_in_heap >= target_footprint_.load(std::memory_order_relaxed))) {
        LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeap triggered";
        auto gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
        ASSERT(gc != nullptr);
        gc->PendingGC();
        return true;
    }
    return false;
}

GCAdaptiveTriggerHeap::GCAdaptiveTriggerHeap(MemStatsType *mem_stats, HeapSpace *heap_space, size_t min_heap_size,
                                             uint8_t percent_threshold, uint32_t adaptive_multiplier,
                                             size_t min_extra_size, size_t max_extra_size, uint32_t skip_gc_times)
    : GCTriggerHeap(mem_stats, heap_space, min_heap_size, percent_threshold, min_extra_size, max_extra_size,
                    skip_gc_times),
      adaptive_multiplier_(adaptive_multiplier)
{
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    recent_target_thresholds_.push_back(target_footprint_.load(std::memory_order_relaxed));
}

size_t GCAdaptiveTriggerHeap::ComputeTarget(size_t heap_size_before_gc, size_t heap_size)
{
    auto delta = static_cast<size_t>(static_cast<double>(heap_size) / PERCENT_100_D * percent_threshold_);

    const auto [min_threshold, max_threshold] =
        std::minmax_element(recent_target_thresholds_.begin(), recent_target_thresholds_.end());
    size_t window = *max_threshold - *min_threshold;

    // if recent thresholds localize in "small" window then we need to get out from a location to avoid too many trigger
    if (window <= max_extra_size_) {
        delta = std::max(delta, adaptive_multiplier_ * max_extra_size_);
        delta = std::min(delta, heap_size);
    } else if (heap_size > heap_size_before_gc) {  // heap increased corresponding with previous gc
        delta = std::min(delta, max_extra_size_);
    } else {
        // if heap was squeeze from 200mb to 100mb we want to set a target to 150mb, not just 100mb*percent_threshold_
        delta = std::max(delta, (heap_size_before_gc - heap_size) / 2);
    }
    delta = std::max(delta, min_extra_size_);
    size_t target = heap_size + delta;

    recent_target_thresholds_.push_back(target);

    return target;
}

GCTriggerType GetTriggerType(std::string_view gc_trigger_type)
{
    auto trigger_type = GCTriggerType::INVALID_TRIGGER;
    if (gc_trigger_type == "heap-trigger-test") {
        trigger_type = GCTriggerType::HEAP_TRIGGER_TEST;
    } else if (gc_trigger_type == "heap-trigger") {
        trigger_type = GCTriggerType::HEAP_TRIGGER;
    } else if (gc_trigger_type == "adaptive-heap-trigger") {
        trigger_type = GCTriggerType::ADAPTIVE_HEAP_TRIGGER;
    } else if (gc_trigger_type == "trigger-heap-occupancy") {
        trigger_type = GCTriggerType::TRIGGER_HEAP_OCCUPANCY;
    } else if (gc_trigger_type == "debug") {
        trigger_type = GCTriggerType::DEBUG;
    } else if (gc_trigger_type == "no-gc-for-start-up") {
        trigger_type = GCTriggerType::NO_GC_FOR_START_UP;
    } else if (gc_trigger_type == "debug-never") {
        trigger_type = GCTriggerType::DEBUG_NEVER;
    }
    return trigger_type;
}

GCTrigger *CreateGCTrigger(MemStatsType *mem_stats, HeapSpace *heap_space, const GCTriggerConfig &config,
                           InternalAllocatorPtr allocator)
{
    uint32_t skip_gc_times = config.GetSkipStartupGcCount();

    constexpr size_t DEFAULT_HEAP_SIZE = 8_MB;
    auto trigger_type = GetTriggerType(config.GetGCTriggerType());

    GCTrigger *ret {nullptr};
    switch (trigger_type) {  // NOLINT(hicpp-multiway-paths-covered)
        case GCTriggerType::HEAP_TRIGGER_TEST:
            // TODO(dtrubenkov): replace with permanent allocator when we get it
            ret = allocator->New<GCTriggerHeap>(mem_stats, heap_space);
            break;
        case GCTriggerType::HEAP_TRIGGER:
            ret = allocator->New<GCTriggerHeap>(mem_stats, heap_space, DEFAULT_HEAP_SIZE, config.GetPercentThreshold(),
                                                config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize());
            break;
        case GCTriggerType::ADAPTIVE_HEAP_TRIGGER:
            ret = allocator->New<GCAdaptiveTriggerHeap>(mem_stats, heap_space, DEFAULT_HEAP_SIZE,
                                                        config.GetPercentThreshold(), config.GetAdaptiveMultiplier(),
                                                        config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize());
            break;
        case GCTriggerType::NO_GC_FOR_START_UP:
            ret = allocator->New<GCTriggerHeap>(mem_stats, heap_space, DEFAULT_HEAP_SIZE, config.GetPercentThreshold(),
                                                config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize(),
                                                skip_gc_times);
            break;
        case GCTriggerType::TRIGGER_HEAP_OCCUPANCY:
            ret = allocator->New<GCTriggerHeapOccupancy>(heap_space, config.GetMaxTriggerPercent());
            break;
        case GCTriggerType::DEBUG:
            ret = allocator->New<GCTriggerDebug>(config.GetDebugStart(), heap_space);
            break;
        case GCTriggerType::DEBUG_NEVER:
            ret = allocator->New<GCNeverTrigger>(heap_space);
            break;
        default:
            LOG(FATAL, GC) << "Wrong GCTrigger type";
            break;
    }
    return ret;
}

void GCTriggerHeap::GCStarted([[maybe_unused]] size_t heap_size)
{
    heap_space_->SetIsWorkGC(true);
}

void GCTriggerHeap::GCFinished(const GCTask &task, size_t heap_size_before_gc, size_t heap_size)
{
    this->ComputeNewTargetFootprint(task, heap_size_before_gc, heap_size);
    heap_space_->ComputeNewSize();
}

GCTriggerDebug::GCTriggerDebug(uint64_t debug_start, HeapSpace *heap_space)
    : GCTrigger(heap_space), debug_start_(debug_start)
{
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerDebug created";
}

bool GCTriggerDebug::IsGcTriggered()
{
    bool ret = false;
    static std::atomic<uint64_t> counter = 0;
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerDebug counter " << counter;
    if (counter >= debug_start_) {
        LOG(DEBUG, GC_TRIGGER) << "GCTriggerDebug triggered";
        ret = true;
    }
    counter++;
    return ret;
}

void GCTriggerDebug::GCStarted([[maybe_unused]] size_t heap_size)
{
    heap_space_->SetIsWorkGC(true);
}

void GCTriggerDebug::GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                                [[maybe_unused]] size_t heap_size)
{
    heap_space_->ComputeNewSize();
}

GCTriggerHeapOccupancy::GCTriggerHeapOccupancy(HeapSpace *heap_space, uint32_t max_trigger_percent)
    : GCTrigger(heap_space), max_trigger_percent_(max_trigger_percent / PERCENT_100_D)
{
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeapOccupancy created";
}

void GCTriggerHeapOccupancy::GCStarted([[maybe_unused]] size_t heap_size)
{
    heap_space_->SetIsWorkGC(true);
}

void GCTriggerHeapOccupancy::GCFinished([[maybe_unused]] const GCTask &task,
                                        [[maybe_unused]] size_t heap_size_before_gc, [[maybe_unused]] size_t heap_size)
{
    heap_space_->ComputeNewSize();
}

bool GCTriggerHeapOccupancy::IsGcTriggered()
{
    size_t current_heap_size = heap_space_->GetHeapSize();
    size_t min_heap_size = MemConfig::GetInitialHeapSizeLimit();
    size_t max_heap_size = MemConfig::GetHeapSizeLimit();
    size_t threshold = std::min(min_heap_size, static_cast<size_t>(max_trigger_percent_ * max_heap_size));
    if (current_heap_size > threshold) {
        LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeapOccupancy triggered: current heap size = " << current_heap_size
                               << ", threshold = " << threshold;
        return true;
    }
    return false;
}

}  // namespace panda::mem
