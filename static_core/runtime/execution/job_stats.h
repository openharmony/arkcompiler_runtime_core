/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_STATS_H
#define PANDA_RUNTIME_EXECUTION_JOB_STATS_H

#include "runtime/include/histogram-inl.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/execution/utils.h"
#include <array>

namespace ark {

template <class T>
constexpr size_t JOB_STATS_ENUM_SIZE = static_cast<size_t>(T::LAST_ID);

/*
Metrics we are potentially interested in (in terms of both time and memory):
- initialization and finalization overhead of the coroutine manager (coros, workers, system coros, sch, etc.)
- coroutine (+ctx) instance creation, deletion, existance, including stacks
- scheduler: locks, queues management and traversal, migration, stealing, etc.
- scheduler: ctx switch separately
- synchronization: await, locks in promises/events
- data sharing
- various js emulation code: JS promises support, event loop, etc.
*/

/// Coroutine time statistic counter types
enum class JobTimeStats {
    INIT,     ///< coroutine manager initialization, including workers creation
    LAUNCH,   ///< launching a single corotutine
    SCH_ALL,  ///< cumulative scheduler overhead
    // NOTE(konstanting, #IAD5MH): need a separate event for CTX_SWITCH count for payload coros?
    CTX_SWITCH,  ///< context switch, separately
    LOCKS,       ///< time spent locking and unlocking mutexes (precision is questionable...)

    LAST_ID  ///< should be the last one
};

/// Coroutine memory statistic counter types
enum class JobMemStats {
    INSTANCE_SIZE,  ///< coroutine instance size

    LAST_ID  ///< should be the last one
};

/**
 * @brief The basic functionality of coroutine statistic counters implementation.
 *
 * Contains the basic profiling tools functionality:
 * - measuring instant and scoped metrics
 * - accessing the measured values and their aggregates
 * - enable/disable controls for the statistics engine
 * - obtaining the information about metrics
 */
class JobStatsBase {
public:
    NO_COPY_SEMANTIC(JobStatsBase);
    DEFAULT_MOVE_SEMANTIC(JobStatsBase);

    /* supplementary types */
    /// possible aggregate types for the metrics
    enum class AggregateType { MAX, MIN, AVG, COUNT, SUM, LAST_ID };
    /// a single metric description: name, type and suggested meaningful aggregates
    struct MetricDescription {
        const char *prettyName;
        std::vector<AggregateType> neededAggregates;
        /// means that this metric should be measured on per-worker basis
        bool perWorker;
    };
    /// time metric value: expected to be in nanoseconds
    using TimeMetricVal = uint64_t;
    static constexpr TimeMetricVal INVALID_TIME_METRIC_VAL = std::numeric_limits<TimeMetricVal>::max();
    /// memory metric value: expected to be in bytes
    using MemMetricVal = uint64_t;
    using TimeStatsDescriptionMap = std::map<JobTimeStats, MetricDescription>;
    using MemStatsDescriptionMap = std::map<JobMemStats, MetricDescription>;

    JobStatsBase();
    virtual ~JobStatsBase() = default;

    /* enable/disable controls */
    /// enable the gathering of statistics (until that, all profiling methods will be no-ops)
    void Enable()
    {
        enabled_ = true;
    }
    /// disable the gathering of statistics (all profiling methods become no-ops)
    void Disable()
    {
        enabled_ = false;
    }
    bool IsEnabled() const
    {
        return enabled_;
    }

    /* profiling controls */
    /// Scoped profiling: open an interval for the given metric id
    void StartInterval(JobTimeStats id);
    /// Scoped profiling: close an interval for the given metric id
    void FinishInterval(JobTimeStats id);
    /// Scoped profiling: check that an interval for the given metric id is active
    bool IsInInterval(JobTimeStats id) const;
    /// Instant profiling: record the value for the given time-based metric
    void RecordTimeStatsValue(JobTimeStats id, TimeMetricVal value);
    /// Instant profiling: record the value for the given memory-based metric
    void RecordMemStatsValue(JobMemStats id, MemMetricVal value);

    /* accessors */
    const SimpleHistogram<TimeMetricVal> &GetTimeStatValue(JobTimeStats id) const;
    /// obtain a summary string for the given time metric id
    PandaString GetTimeStatString(JobTimeStats id) const;
    const SimpleHistogram<MemMetricVal> &GetMemStatValue(JobMemStats id) const;
    /// obtain a summary string for the given memory metric id
    PandaString GetMemStatString(JobMemStats id) const;

    /// @return metrics description
    static TimeStatsDescriptionMap &GetTimeStatsDescription();

private:
    bool enabled_ = false;
    std::array<TimeMetricVal, JOB_STATS_ENUM_SIZE<JobTimeStats>> intervalStarts_;
    std::array<SimpleHistogram<TimeMetricVal>, JOB_STATS_ENUM_SIZE<JobTimeStats>> timeStats_;
    std::array<SimpleHistogram<MemMetricVal>, JOB_STATS_ENUM_SIZE<JobMemStats>> memStats_;
};

std::ostream &operator<<(std::ostream &os, JobStatsBase::AggregateType id);

/**
 * @brief Per-worker statistics.
 *
 * Should be used for gathering per worker metrics only.
 */
class JobWorkerStats : public JobStatsBase {
public:
    NO_COPY_SEMANTIC(JobWorkerStats);
    DEFAULT_MOVE_SEMANTIC(JobWorkerStats);

    explicit JobWorkerStats(PandaString name) : workerName_(std::move(name)) {}
    ~JobWorkerStats() override = default;

    /// @return name of the worker being profiled
    PandaString GetName() const
    {
        return workerName_;
    }

private:
    PandaString workerName_;
};

/// @brief The helper RAII-style class that facilitates the gathering of scoped time metrics for some arbitrary scope
class ScopedJobStats {
public:
    NO_COPY_SEMANTIC(ScopedJobStats);
    NO_MOVE_SEMANTIC(ScopedJobStats);

    /**
     * @brief The constructor
     *
     * @param stats The statistics instance to use
     * @param id The time metric id to measure
     * @param avoidNesting Iff this parameter is set to true, do not count anything in case if the measuring
     * scope for the given metric is already open.
     */
    explicit ScopedJobStats(JobStatsBase *stats, JobTimeStats id, bool avoidNesting = false) : id_(id), stats_(stats)
    {
        ASSERT(stats != nullptr);
        if (avoidNesting && stats_->IsInInterval(id)) {
            doNothing_ = true;
            return;
        }
        stats_->StartInterval(id_);
    }

    ~ScopedJobStats()
    {
        if (doNothing_) {
            return;
        }
        stats_->FinishInterval(id_);
    }

private:
    JobTimeStats id_ = JobTimeStats::LAST_ID;
    JobStatsBase *stats_ = nullptr;
    bool doNothing_ = false;
};

/**
 * @brief Global statistics.
 *
 * Should be used for measuring global (not per-worker) concurrency statistics.
 * Suppors statistics report generation.
 */
class JobStats : public JobStatsBase {
public:
    NO_COPY_SEMANTIC(JobStats);
    NO_MOVE_SEMANTIC(JobStats);

    /* supplementary types */
    /// all the aggregates for one metric
    using TimeStatsAggregateArray = std::array<TimeMetricVal, JOB_STATS_ENUM_SIZE<AggregateType>>;
    /// all the aggregates for all the metrics for all the workers (does not include global metrics)
    using TimeStatsDataArray = std::array<TimeStatsAggregateArray, JOB_STATS_ENUM_SIZE<JobTimeStats>>;

    JobStats() = default;
    ~JobStats() override = default;

    /// generate the summary of global metrics
    PandaString GetBriefStatistics() const;
    /**
     * @brief Generate the full statistics report
     *
     * This method generates the full report that includes the global metrics, the per-worker metrics and
     * the summary, which includes the aggregated data from all workers.
     *
     * @param workerStats a vector that holds per-worker statistic instance pointers
     */
    PandaString GetFullStatistics(const PandaVector<JobWorkerStats> &workerStats) const;

protected:
    /// Aggregates data for the workers. Does not include global (not per-worker) metrics!
    static TimeStatsDataArray GenerateTimeStatsDataArray(const PandaVector<JobWorkerStats> &workerStats);
};

}  // namespace ark

#endif /* PANDA_RUNTIME_EXECUTION_JOB_STATS_H */
