/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common_components/base/time_utils.h"

#include <chrono>

namespace ark::common_vm {
namespace TimeUtil {
using Sec = std::chrono::seconds;
using Ms = std::chrono::milliseconds;
using Us = std::chrono::microseconds;
using Ns = std::chrono::nanoseconds;

template <typename Unit>
uint64_t ClockTime(clockid_t clock)
{
    struct timespec time = {0, 0};
    clock_gettime(clock, &time);
    auto duration = Sec {time.tv_sec} + Ns {time.tv_nsec};
    return std::chrono::duration_cast<Unit>(duration).count();
}

uint64_t NanoSeconds()
{
    return ClockTime<Ns>(CLOCK_MONOTONIC);
}

uint64_t MilliSeconds()
{
    return ClockTime<Ms>(CLOCK_MONOTONIC);
}

uint64_t MicroSeconds() noexcept
{
    return ClockTime<Us>(CLOCK_MONOTONIC);
}

PandaString PrettyDigitsFormat(uint64_t number) noexcept
{
    constexpr int numDigitsPerSegment = 3;
    PandaString orig = ToPandaString(number);
    int pos = static_cast<int>(orig.length()) - numDigitsPerSegment;
    while (pos > 0) {
        orig.insert(pos, ",");
        pos -= numDigitsPerSegment;
    }
    return orig;
}

}  // namespace TimeUtil

}  // namespace ark::common_vm
