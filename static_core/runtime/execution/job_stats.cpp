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

#include "libarkbase/utils/time.h"
#include "runtime/execution/job_stats.h"
#include <chrono>

namespace ark {

// clang-tidy cannot detect that we are going to initialize intervalStarts_ via fill()
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
JobStatsBase::JobStatsBase()
{
    intervalStarts_.fill(INVALID_TIME_METRIC_VAL);
}

void JobStatsBase::StartInterval(JobTimeStats id)
{
    if (!IsEnabled()) {
        return;
    }
    LOG(DEBUG, COROUTINES) << "JobStatsBase::StartInterval: " << ToIndex(id) << " Stats: " << this;
    ASSERT(!IsInInterval(id));
    TimeMetricVal start = time::GetCurrentTimeInNanos();
    intervalStarts_[ToIndex(id)] = start;
}

void JobStatsBase::FinishInterval(JobTimeStats id)
{
    if (!IsEnabled()) {
        return;
    }
    LOG(DEBUG, COROUTINES) << "JobStatsBase::FinishInterval " << ToIndex(id) << " Stats: " << this;
    ASSERT(IsInInterval(id));
    TimeMetricVal end = time::GetCurrentTimeInNanos();
    TimeMetricVal delay = end - intervalStarts_[ToIndex(id)];
    intervalStarts_[ToIndex(id)] = INVALID_TIME_METRIC_VAL;
    auto &metric = timeStats_[ToIndex(id)];
    metric.AddValue(delay);
}

bool JobStatsBase::IsInInterval(JobTimeStats id) const
{
    return (intervalStarts_[ToIndex(id)] != INVALID_TIME_METRIC_VAL);
}

void JobStatsBase::RecordTimeStatsValue(JobTimeStats id, TimeMetricVal value)
{
    auto &metric = timeStats_[ToIndex(id)];
    metric.AddValue(value);
}

void JobStatsBase::RecordMemStatsValue(JobMemStats id, MemMetricVal value)
{
    auto &metric = memStats_[ToIndex(id)];
    metric.AddValue(value);
}

const SimpleHistogram<JobStatsBase::TimeMetricVal> &JobStatsBase::GetTimeStatValue(JobTimeStats id) const
{
    return timeStats_[ToIndex(id)];
}

PandaString JobStatsBase::GetTimeStatString(JobTimeStats id) const
{
    auto &metric = timeStats_[ToIndex(id)];
    if (metric.GetCount() > 0) {
        return metric.GetGeneralStatistic() + " Count: " + ark::ToPandaString(metric.GetCount());
    }
    return "<no data>";
}

const SimpleHistogram<JobStatsBase::MemMetricVal> &JobStatsBase::GetMemStatValue(JobMemStats id) const
{
    return memStats_[ToIndex(id)];
}

PandaString JobStatsBase::GetMemStatString(JobMemStats id) const
{
    auto &metric = memStats_[ToIndex(id)];
    if (metric.GetCount() > 0) {
        return metric.GetGeneralStatistic();
    }
    return "<no data>";
}

JobStatsBase::TimeStatsDescriptionMap &JobStatsBase::GetTimeStatsDescription()
{
    static TimeStatsDescriptionMap timeMetricsDescription = {
        {JobTimeStats::INIT, {"INIT", {AggregateType::SUM}, false}},
        {JobTimeStats::LAUNCH, {"LAUNCH", {AggregateType::AVG, AggregateType::MAX, AggregateType::COUNT}, true}},
        {JobTimeStats::SCH_ALL, {"SCH_ALL", {AggregateType::SUM}, true}},
        {JobTimeStats::CTX_SWITCH,
         {"CTX_SWITCH", {AggregateType::AVG, AggregateType::MAX, AggregateType::COUNT}, true}},
        {JobTimeStats::LOCKS, {"LOCKS", {AggregateType::SUM}, true}},
    };
    ASSERT(timeMetricsDescription.size() == JOB_STATS_ENUM_SIZE<JobTimeStats>);
    return timeMetricsDescription;
}

std::ostream &operator<<(std::ostream &os, JobStatsBase::AggregateType id)
{
    switch (id) {
        case JobStatsBase::AggregateType::MAX:
            os << "MAX";
            break;
        case JobStatsBase::AggregateType::MIN:
            os << "MIN";
            break;
        case JobStatsBase::AggregateType::AVG:
            os << "AVG";
            break;
        case JobStatsBase::AggregateType::COUNT:
            os << "COUNT";
            break;
        case JobStatsBase::AggregateType::SUM:
            os << "SUM";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

PandaString JobStats::GetBriefStatistics() const
{
    PandaStringStream ss;
    constexpr size_t NS_IN_MICROSECOND = 1000ULL;
    ss << "[General stats]\n";
    ss << "INIT [us]: " << (GetTimeStatValue(JobTimeStats::INIT).GetSum() / NS_IN_MICROSECOND) << "\n";
    return ss.str();
}

JobStats::TimeStatsDataArray JobStats::GenerateTimeStatsDataArray(const PandaVector<JobWorkerStats> &workerStats)
{
    TimeStatsDataArray timeStats;
    for (auto &s : timeStats) {
        s = {
            0,                                    /*MAX*/
            std::numeric_limits<uint64_t>::max(), /*MIN*/
            0,                                    /*AVG*/
            0,                                    /*COUNT*/
            0                                     /*SUM*/
        };
    }
    using MetricAggregates = TimeStatsDataArray::value_type;
    auto initMetric = [](const JobWorkerStats &ws, JobTimeStats id, MetricAggregates &metricData) {
        for (size_t aggrId = 0; aggrId < metricData.size(); ++aggrId) {
            switch (static_cast<AggregateType>(aggrId)) {
                case AggregateType::MAX:
                    metricData[aggrId] = std::max(metricData[aggrId], ws.GetTimeStatValue(id).GetMax());
                    break;
                case AggregateType::MIN:
                    metricData[aggrId] = std::min(metricData[aggrId], ws.GetTimeStatValue(id).GetMin());
                    break;
                case AggregateType::AVG:
                    // should be re-calculated later because of a float data type
                    break;
                case AggregateType::COUNT:
                    metricData[aggrId] += ws.GetTimeStatValue(id).GetCount();
                    break;
                case AggregateType::SUM:
                default:
                    metricData[aggrId] += ws.GetTimeStatValue(id).GetSum();
                    break;
            }
        }
    };
    for (auto &ws : workerStats) {
        for (auto &[id, descr] : GetTimeStatsDescription()) {
            auto &metricData = timeStats[ToIndex(id)];
            initMetric(ws, id, metricData);
        }
    }
    return timeStats;
}

PandaString JobStats::GetFullStatistics(const PandaVector<JobWorkerStats> &workerStats) const
{
    PandaStringStream ss;
    // common stats
    ss << GetBriefStatistics();
    // per worker stats
    ss << "[Per worker stats]\n";
    for (auto &ws : workerStats) {
        ss << "Worker: " << ws.GetName() << "\n";
        for (auto &[id, descr] : GetTimeStatsDescription()) {
            if (ws.GetTimeStatValue(id).GetCount() > 0) {
                ss << "\t" << descr.prettyName << " [ns]: " << ws.GetTimeStatString(id) << "\n";
            }
        }
    }
    // aggregate data for all workers
    ss << "Summary: \n";
    TimeStatsDataArray timeStats = GenerateTimeStatsDataArray(workerStats);
    for (auto &[id, descr] : GetTimeStatsDescription()) {
        auto &metricData = timeStats[ToIndex(id)];
        if ((!descr.perWorker) || (metricData[ToIndex(AggregateType::COUNT)] == 0)) {
            continue;
        }
        ss << "\t" << descr.prettyName << " [ns]: ";
        for (size_t aggrId = 0; aggrId < metricData.size(); ++aggrId) {
            ss << static_cast<AggregateType>(aggrId) << "(";
            if (aggrId == ToIndex(AggregateType::AVG)) {
                ss << static_cast<double>(metricData[ToIndex(AggregateType::SUM)]) /
                          metricData[ToIndex(AggregateType::COUNT)];
            } else {
                ss << metricData[ToIndex(aggrId)];
            }
            ss << ") ";
        }
        ss << "\n";
    }
    return ss.str();
}

}  // namespace ark
