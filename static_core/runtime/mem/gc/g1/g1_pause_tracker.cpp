/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <algorithm>
#include "g1_pause_tracker.h"
#include "utils/logger.h"
#include "utils/type_converter.h"
#include "libpandabase/os/time.h"

namespace panda::mem {
G1PauseTracker::G1PauseTracker(int64_t gc_pause_interval_ms, int64_t max_gc_time_ms)
    : gc_pause_interval_us_(gc_pause_interval_ms * panda::os::time::MILLIS_TO_MICRO),
      max_gc_time_us_(max_gc_time_ms * panda::os::time::MILLIS_TO_MICRO)
{
}

bool G1PauseTracker::AddPauseInNanos(int64_t start_time_ns, int64_t end_time_ns)
{
    return AddPause(start_time_ns / panda::os::time::MICRO_TO_NANO, end_time_ns / panda::os::time::MICRO_TO_NANO);
}

bool G1PauseTracker::AddPause(int64_t start_time_us, int64_t end_time_us)
{
    RemoveOutOfIntervalEntries(end_time_us);
    pauses_.push_back(PauseEntry(start_time_us, end_time_us));
    auto gc_time = CalculateIntervalPauseInMicros(end_time_us);
    if (gc_time > max_gc_time_us_) {
        LOG(DEBUG, GC) << "Target GC pause was exceeded: "
                       << panda::helpers::TimeConverter(gc_time * panda::os::time::MICRO_TO_NANO) << " > "
                       << panda::helpers::TimeConverter(max_gc_time_us_ * panda::os::time::MICRO_TO_NANO)
                       << " in time interval "
                       << panda::helpers::TimeConverter(gc_pause_interval_us_ * panda::os::time::MICRO_TO_NANO);
        return false;
    }
    return true;
}

void G1PauseTracker::RemoveOutOfIntervalEntries(int64_t now_us)
{
    auto oldest_interval_time = now_us - gc_pause_interval_us_;
    while (!pauses_.empty()) {
        auto oldest_pause = pauses_.front();
        if (oldest_pause.GetEndTimeInMicros() > oldest_interval_time) {
            break;
        }
        pauses_.pop_front();
    }
}

int64_t G1PauseTracker::CalculateIntervalPauseInMicros(int64_t now_us)
{
    int64_t gc_time = 0;
    auto oldest_interval_time = now_us - gc_pause_interval_us_;
    auto end = pauses_.cend();
    for (auto it = pauses_.cbegin(); it != end; ++it) {
        if (it->GetEndTimeInMicros() > oldest_interval_time) {
            gc_time += it->DurationInMicros(oldest_interval_time);
        }
    }
    return gc_time;
}

int64_t G1PauseTracker::MinDelayBeforePauseInMicros(int64_t now_us, int64_t pause_time_us)
{
    pause_time_us = std::min(pause_time_us, max_gc_time_us_);
    auto gc_budget = max_gc_time_us_ - pause_time_us;
    auto new_interval_time = now_us + pause_time_us;
    auto oldest_interval_time = new_interval_time - gc_pause_interval_us_;

    auto rend = pauses_.crend();
    for (auto it = pauses_.crbegin(); it != rend; ++it) {
        if (it->GetEndTimeInMicros() <= oldest_interval_time) {
            break;
        }

        auto duration = it->DurationInMicros(oldest_interval_time);
        if (duration > gc_budget) {
            auto new_oldest_interval_time = it->GetEndTimeInMicros() - gc_budget;
            ASSERT(new_oldest_interval_time >= oldest_interval_time);
            return new_oldest_interval_time - oldest_interval_time;
        }

        gc_budget -= duration;
    }

    return 0;
}

int64_t G1PauseTracker::MinDelayBeforeMaxPauseInMicros(int64_t now_us)
{
    return MinDelayBeforePauseInMicros(now_us, max_gc_time_us_);
}
}  // namespace panda::mem
