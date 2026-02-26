/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "utils/perfMetrics.h"

#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/type_converter.h"
#include "libpandabase/mem/mem.h"
#include <chrono>
#include <mutex>
#include <forward_list>
#include <optional>
#include <sstream>
#include <iomanip>

#ifdef PANDA_TARGET_UNIX
#include "sys/resource.h"
#endif

namespace panda {

static int64_t PerfMetricsGetTimeNS()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

[[maybe_unused]] static int64_t PerfMetricsGetMaxRSS()
{
#ifdef PANDA_TARGET_UNIX
    struct rusage ru {};
    if (getrusage(RUSAGE_SELF, &ru) != 0) {
        LOG(FATAL, COMPILER) << "getrusage failed";
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return static_cast<int64_t>(ru.ru_maxrss) * 1_KB;
#else
    return 0;
#endif
}

class PerfMetricValue {
public:
    static PerfMetricValue Zero()
    {
        return PerfMetricValue();
    }

    static PerfMetricValue Now()
    {
        PerfMetricValue st;
        st.time_ = PerfMetricsGetTimeNS();
        st.mem_ = PerfMetricsGetMaxRSS();
        return st;
    }

    static PerfMetricValue Accumulate(PerfMetricValue const &acc, PerfMetricValue const &begin,
                                      PerfMetricValue const &end)
    {
        PerfMetricValue diff = Combine(end, begin, false);
        return Combine(acc, diff, true);
    }

    int64_t GetTimeNanoseconds() const
    {
        return time_;
    }

    int64_t GetMemorySize() const
    {
        return mem_;
    }

    DEFAULT_COPY_SEMANTIC(PerfMetricValue);
    DEFAULT_MOVE_SEMANTIC(PerfMetricValue);
    ~PerfMetricValue() = default;

private:
    PerfMetricValue() = default;

    static PerfMetricValue Combine(PerfMetricValue const &st1, PerfMetricValue const &st2, bool sum)
    {
        int mult = sum ? 1 : -1;
        PerfMetricValue res;
        res.time_ = st1.time_ + mult * st2.time_;
        res.mem_ = st1.mem_ + mult * st2.mem_;
        return res;
    }

    int64_t time_ {};
    int64_t mem_ {};
};

class PerfMetricRecord {
public:
    explicit PerfMetricRecord(std::string_view name) : name_(name) {}

    PerfMetricValue const &GetStats() const
    {
        return acc_;
    }

    std::string const &GetName() const
    {
        return name_;
    }

    void BeginTrace()
    {
        invokationsCount_++;
        if (nesting_ == 0) {
            begin_ = PerfMetricValue::Now();
        }
        if (++nesting_ > maxNesting_) {
            maxNesting_ = nesting_;
        }
    }

    void EndTrace()
    {
        if (--nesting_ == 0) {
            acc_ = PerfMetricValue::Accumulate(acc_, begin_, PerfMetricValue::Now());
        }
    }

    size_t GetMaxNesting() const
    {
        return maxNesting_;
    }

    size_t GetInvokationsCount() const
    {
        return invokationsCount_;
    }

    NO_COPY_SEMANTIC(PerfMetricRecord);
    DEFAULT_MOVE_SEMANTIC(PerfMetricRecord);
    ~PerfMetricRecord() = default;

private:
    size_t nesting_ {};
    size_t maxNesting_ {};
    size_t invokationsCount_ {};
    std::string name_;
    PerfMetricValue acc_ = PerfMetricValue::Zero();
    PerfMetricValue begin_ = PerfMetricValue::Zero();
};

PerfMetricScope::PerfMetricScope(PerfMetricRecord *record) : record_(record)
{
    record_->BeginTrace();
}

PerfMetricScope::~PerfMetricScope()
{
    record_->EndTrace();
}

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
static std::forward_list<std::pair<os::thread::ThreadId, std::forward_list<PerfMetricRecord>>> g_perfMetricsLists;
static std::mutex g_perfMetricsListsMtx;
// NOLINTEND(fuchsia-statically-constructed-objects)

std::forward_list<PerfMetricRecord> *GetTLSPerfMetricsData()
{
    thread_local auto tls = ([]() {
        std::lock_guard lk(g_perfMetricsListsMtx);
        g_perfMetricsLists.push_front({os::thread::GetCurrentThreadId(), std::forward_list<PerfMetricRecord>()});
        return &g_perfMetricsLists.begin()->second;
    })();

    return tls;
}

PerfMetricRecord *RegisterPerfMetricRecord(std::string_view name)
{
    auto *tls = GetTLSPerfMetricsData();
    tls->emplace_front(PerfMetricRecord(name));
    return &tls->front();
}

static std::string PrettyTimeNs(uint64_t duration)
{
    // CC-OFF(G.NAM.03-CPP) project code style
    constexpr std::array<std::string_view, 4U> TIME_UNITS = {"ns", "us", "ms", "s"};
    // CC-OFF(G.NAM.03-CPP) project code style
    constexpr double K_SCALE = 1E3;

    if (duration == 0) {
        return "0s";
    }

    double val = duration;
    size_t unitIdx = 0;

    for (;; ++unitIdx) {
        double nextVal = val / K_SCALE;
        if (unitIdx == TIME_UNITS.size() || nextVal < 1.0) {
            break;
        }
        val = nextVal;
    }

    std::stringstream ss;
    ss << std::setprecision(3U) << val << TIME_UNITS[unitIdx];
    return ss.str();
}

static void DumpPerfMetricRecord(std::stringstream &ss, PerfMetricRecord const *rec)
{
    // CC-OFFNXT(G.FMT.14-CPP) project code style
    auto const metric = [&ss](std::string_view name, unsigned w) -> std::stringstream& {
        ss << " " << name << "=" << std::left << std::setw(w);
        return ss;
    };

    auto const &stats = rec->GetStats();
    // NOLINTBEGIN(readability-magic-numbers)
    ss << ":" << std::left << std::setw(50U) << rec->GetName() << ": ";
    metric("time", 10U) << PrettyTimeNs(stats.GetTimeNanoseconds());
    metric("mem", 10U) << stats.GetMemorySize() << "B";
    if (rec->GetMaxNesting() > 1) {
        metric("nesting", 6U) << rec->GetMaxNesting();
    }
    if (rec->GetInvokationsCount() > 1) {
        metric("count", 8U) << rec->GetInvokationsCount();
    }
    // NOLINTEND(readability-magic-numbers)
}

static void DumpPerfMetricsImpl(std::optional<std::string_view> prefix)
{
    std::lock_guard lk(g_perfMetricsListsMtx);  // prevents new thread from appearing

    if (g_perfMetricsLists.begin() == g_perfMetricsLists.end()) {
        return;
    }

    auto getSortVal = [](PerfMetricRecord *rec) { return rec->GetStats().GetTimeNanoseconds(); };

    std::stringstream ss;
    ss << "============================ es2panda perf metrics =============================" << std::endl;

    for (auto &tlsdata : g_perfMetricsLists) {
        std::vector<PerfMetricRecord *> records;
        for (auto &rec : tlsdata.second) {
            if (prefix.has_value() && rec.GetName().rfind(*prefix, 0) != std::string::npos) {
                records.push_back(&rec);
            }
        }
        std::sort(records.begin(), records.end(),
                  [getSortVal](PerfMetricRecord *r1, PerfMetricRecord *r2) { return getSortVal(r1) > getSortVal(r2); });

        ss << "==== Thread [" << tlsdata.first << "]" << std::endl;

        for (auto &rec : records) {
            DumpPerfMetricRecord(ss, rec);
            ss << std::endl;
        }
    }

    std::cout << ss.str();
}

void DumpPerfMetrics()
{
    DumpPerfMetricsImpl("@");
}

// You may want to add __attribute__((destructor)) for the debugging needs
[[maybe_unused]] static void DbgDumpPerfMetrics()
{
    DumpPerfMetricsImpl("%");
}

}  // namespace panda
