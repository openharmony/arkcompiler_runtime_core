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
#ifndef PANDA_RUNTIME_MEM_GC_GC_THRESHOLD_H
#define PANDA_RUNTIME_MEM_GC_GC_THRESHOLD_H

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "libpandabase/macros.h"
#include "libpandabase/utils/ring_buffer.h"
#include "runtime/mem/gc/gc.h"

namespace panda {

class RuntimeOptions;

namespace test {
class GCTriggerTest;
}  // namespace test

namespace mem {

enum class GCTriggerType {
    INVALID_TRIGGER,
    HEAP_TRIGGER_TEST,       // TRIGGER with low thresholds for tests
    HEAP_TRIGGER,            // Standard TRIGGER with production ready thresholds
    ADAPTIVE_HEAP_TRIGGER,   // TRIGGER with adaptive strategy for heap increasing
    NO_GC_FOR_START_UP,      // A non-production strategy, TRIGGER GC after the app starts up
    TRIGGER_HEAP_OCCUPANCY,  // Trigger full GC by heap size threshold
    DEBUG,                   // Debug TRIGGER which always returns true
    DEBUG_NEVER,             // Trigger for testing which never triggers (young-gc can still trigger), for test purpose
    GCTRIGGER_LAST = DEBUG
};

class GCTriggerConfig {
public:
    GCTriggerConfig(const RuntimeOptions &options, panda_file::SourceLang lang);

    std::string_view GetGCTriggerType() const
    {
        return gc_trigger_type_;
    }

    uint64_t GetDebugStart() const
    {
        return debug_start_;
    }

    uint32_t GetPercentThreshold() const
    {
        return percent_threshold_;
    }

    uint32_t GetAdaptiveMultiplier() const
    {
        return adaptive_multiplier_;
    }

    size_t GetMinExtraHeapSize() const
    {
        return min_extra_heap_size_;
    }

    size_t GetMaxExtraHeapSize() const
    {
        return max_extra_heap_size_;
    }

    uint32_t GetMaxTriggerPercent() const
    {
        return max_trigger_percent_;
    }

    uint32_t GetSkipStartupGcCount() const
    {
        return skip_startup_gc_count_;
    }

private:
    std::string gc_trigger_type_;
    uint64_t debug_start_;
    uint32_t percent_threshold_;
    uint32_t adaptive_multiplier_;
    size_t min_extra_heap_size_;
    size_t max_extra_heap_size_;
    uint32_t max_trigger_percent_;
    uint32_t skip_startup_gc_count_;
};

class GCTrigger : public GCListener {
public:
    explicit GCTrigger(HeapSpace *heap_space) : heap_space_(heap_space) {}
    ~GCTrigger() override;
    NO_COPY_SEMANTIC(GCTrigger);
    NO_MOVE_SEMANTIC(GCTrigger);

    /**
     * \brief Checks if GC required
     * @return returns true if GC should be executed
     */
    virtual bool IsGcTriggered() = 0;
    virtual void SetMinTargetFootprint([[maybe_unused]] size_t heap_size) {}
    virtual void RestoreMinTargetFootprint() {}

protected:
    HeapSpace *heap_space_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
private:
    friend class GC;
};

/**
 * Triggers when heap increased by predefined %
 */
class GCTriggerHeap : public GCTrigger {
public:
    explicit GCTriggerHeap(MemStatsType *mem_stats, HeapSpace *heap_space);
    explicit GCTriggerHeap(MemStatsType *mem_stats, HeapSpace *heap_space, size_t min_heap_size,
                           uint8_t percent_threshold, size_t min_extra_size, size_t max_extra_size,
                           uint32_t skip_gc_times = 0);

    bool IsGcTriggered() override;

    void GCStarted(size_t heap_size) override;
    void GCFinished(const GCTask &task, size_t heap_size_before_gc, size_t heap_size) override;
    void SetMinTargetFootprint(size_t target_size) override;
    void RestoreMinTargetFootprint() override;
    void ComputeNewTargetFootprint(const GCTask &task, size_t heap_size_before_gc, size_t heap_size);

    static constexpr uint8_t DEFAULT_PERCENTAGE_THRESHOLD = 20;

protected:
    // TODO(dtrubenkov): change to the proper value when all triggers will be enabled
    static constexpr size_t MIN_HEAP_SIZE_FOR_TRIGGER = 512;
    static constexpr size_t DEFAULT_MIN_TARGET_FOOTPRINT = 256;
    static constexpr size_t DEFAULT_MIN_EXTRA_HEAP_SIZE = 32;      // For heap-trigger-test
    static constexpr size_t DEFAULT_MAX_EXTRA_HEAP_SIZE = 512_KB;  // For heap-trigger-test

    virtual size_t ComputeTarget(size_t heap_size_before_gc, size_t heap_size);

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<size_t> target_footprint_ {MIN_HEAP_SIZE_FOR_TRIGGER};

    /**
     * We'll trigger if heap increased by delta, delta = heap_size_after_last_gc * percent_threshold_ %
     * And the constraint on delta is: min_extra_size_ <= delta <= max_extra_size_
     */
    uint8_t percent_threshold_ {DEFAULT_PERCENTAGE_THRESHOLD};  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t min_extra_size_ {DEFAULT_MIN_EXTRA_HEAP_SIZE};       // NOLINT(misc-non-private-member-variables-in-classes)
    size_t max_extra_size_ {DEFAULT_MAX_EXTRA_HEAP_SIZE};       // NOLINT(misc-non-private-member-variables-in-classes)

private:
    size_t min_target_footprint_ {DEFAULT_MIN_TARGET_FOOTPRINT};
    MemStatsType *mem_stats_;
    uint8_t skip_gc_count_ {0};

    friend class panda::test::GCTriggerTest;
};

/**
 * Triggers when heap increased by adaptive strategy
 */
class GCAdaptiveTriggerHeap : public GCTriggerHeap {
public:
    GCAdaptiveTriggerHeap(MemStatsType *mem_stats, HeapSpace *heap_space, size_t min_heap_size,
                          uint8_t percent_threshold, uint32_t adaptive_multiplier, size_t min_extra_size,
                          size_t max_extra_size, uint32_t skip_gc_times = 0);
    NO_COPY_SEMANTIC(GCAdaptiveTriggerHeap);
    NO_MOVE_SEMANTIC(GCAdaptiveTriggerHeap);
    ~GCAdaptiveTriggerHeap() override = default;

private:
    static constexpr uint32_t DEFAULT_INCREASE_MULTIPLIER = 3U;
    // We save last RECENT_THRESHOLDS_COUNT thresholds for detect too often triggering
    static constexpr size_t RECENT_THRESHOLDS_COUNT = 3;

    size_t ComputeTarget(size_t heap_size_before_gc, size_t heap_size) override;

    RingBuffer<size_t, RECENT_THRESHOLDS_COUNT> recent_target_thresholds_;
    uint32_t adaptive_multiplier_ {DEFAULT_INCREASE_MULTIPLIER};

    friend class panda::test::GCTriggerTest;
};

/**
 * Trigger always returns true after given start
 */
class GCTriggerDebug : public GCTrigger {
public:
    explicit GCTriggerDebug(uint64_t debug_start, HeapSpace *heap_space);

    bool IsGcTriggered() override;

    void GCStarted(size_t heap_size) override;
    void GCFinished(const GCTask &task, size_t heap_size_before_gc, size_t heap_size) override;

private:
    uint64_t debug_start_ = 0;
};

class GCTriggerHeapOccupancy : public GCTrigger {
public:
    explicit GCTriggerHeapOccupancy(HeapSpace *heap_space, uint32_t max_trigger_percent);

    bool IsGcTriggered() override;

    void GCStarted([[maybe_unused]] size_t heap_size) override;
    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                    [[maybe_unused]] size_t heap_size) override;

private:
    double max_trigger_percent_ = 0.0;
};

class GCNeverTrigger : public GCTrigger {
public:
    explicit GCNeverTrigger(HeapSpace *heap_space) : GCTrigger(heap_space) {}

    bool IsGcTriggered() override
    {
        return false;
    }

    void GCStarted([[maybe_unused]] size_t heap_size) override {}
    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                    [[maybe_unused]] size_t heap_size) override
    {
    }
};

GCTrigger *CreateGCTrigger(MemStatsType *mem_stats, HeapSpace *heap_space, const GCTriggerConfig &config,
                           InternalAllocatorPtr allocator);

}  // namespace mem
}  // namespace panda

#endif  // PANDA_RUNTIME_MEM_GC_GC_THRESHOLD_H
