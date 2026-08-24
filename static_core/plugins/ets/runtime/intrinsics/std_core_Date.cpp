/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <chrono>
#include <ctime>
#include <clocale>
#include <sstream>
#include "unicode/timezone.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

static constexpr int32_t MS_PER_SEC = 1000;
static constexpr int32_t SEC_PER_MIN = 60;

static constexpr int32_t MS_IN_MINUTES = 60000;
static constexpr int64_t MS_IN_SECOND = 1000;
static constexpr int32_t MINUTES_IN_HOUR = 60;
static constexpr int32_t SECONDS_IN_MINUTE = 60;
static constexpr int32_t HOURS_IN_DAY = 24;
static constexpr int32_t DECIMAL_BASE = 10;
static constexpr int32_t SECONDS_IN_HOUR = MINUTES_IN_HOUR * SECONDS_IN_MINUTE;
static constexpr int64_t MS_IN_DAY = HOURS_IN_DAY * MINUTES_IN_HOUR * MS_IN_MINUTES;
static constexpr int32_t TIME_HOUR_TENS_POS = 0;
static constexpr int32_t TIME_HOUR_ONES_POS = 1;
static constexpr int32_t TIME_MINUTE_TENS_POS = 3;
static constexpr int32_t TIME_MINUTE_ONES_POS = 4;
static constexpr int32_t TIME_SECOND_TENS_POS = 6;
static constexpr int32_t TIME_SECOND_ONES_POS = 7;
static constexpr int32_t TZ_SIGN_POS = 12;
static constexpr int32_t TZ_HOUR_TENS_POS = 13;
static constexpr int32_t TZ_HOUR_ONES_POS = 14;
static constexpr int32_t TZ_MINUTE_TENS_POS = 15;
static constexpr int32_t TZ_MINUTE_ONES_POS = 16;

extern "C" int64_t StdCoreDateNow()
{
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return nowMs.time_since_epoch().count();
}

extern "C" int64_t StdCoreDateGetLocalTimezoneOffset(int64_t ms)
{
    auto utcSeconds = static_cast<time_t>(ms / MS_PER_SEC);
    struct tm localTm = {};

    if (localtime_r(&utcSeconds, &localTm) == nullptr) {
        return 0;
    }
    return static_cast<int64_t>(-localTm.tm_gmtoff / SEC_PER_MIN);
}

extern "C" EtsString *StdCoreDateFormatTimeString(int64_t ms, int64_t tzOffset)
{
    int64_t timeInDay = (ms - tzOffset * MS_IN_MINUTES) % MS_IN_DAY;
    if (timeInDay < 0) {
        timeInDay += MS_IN_DAY;
    }

    auto totalSeconds = static_cast<int32_t>(timeInDay / MS_IN_SECOND);
    auto hour = totalSeconds / SECONDS_IN_HOUR;
    auto minute = totalSeconds / SECONDS_IN_MINUTE % MINUTES_IN_HOUR;
    auto second = totalSeconds % SECONDS_IN_MINUTE;

    int64_t displayOffset = -tzOffset;
    char sign = displayOffset < 0 ? '-' : '+';
    if (displayOffset < 0) {
        displayOffset = -displayOffset;
    }
    auto offsetHour = static_cast<int32_t>(displayOffset / MINUTES_IN_HOUR);
    auto offsetMinute = static_cast<int32_t>(displayOffset % MINUTES_IN_HOUR);

    std::string result = "00:00:00 GMT+0000";
    result[TIME_HOUR_TENS_POS] = static_cast<char>('0' + hour / DECIMAL_BASE);
    result[TIME_HOUR_ONES_POS] = static_cast<char>('0' + hour % DECIMAL_BASE);
    result[TIME_MINUTE_TENS_POS] = static_cast<char>('0' + minute / DECIMAL_BASE);
    result[TIME_MINUTE_ONES_POS] = static_cast<char>('0' + minute % DECIMAL_BASE);
    result[TIME_SECOND_TENS_POS] = static_cast<char>('0' + second / DECIMAL_BASE);
    result[TIME_SECOND_ONES_POS] = static_cast<char>('0' + second % DECIMAL_BASE);
    result[TZ_SIGN_POS] = sign;
    result[TZ_HOUR_TENS_POS] = static_cast<char>('0' + offsetHour / DECIMAL_BASE);
    result[TZ_HOUR_ONES_POS] = static_cast<char>('0' + offsetHour % DECIMAL_BASE);
    result[TZ_MINUTE_TENS_POS] = static_cast<char>('0' + offsetMinute / DECIMAL_BASE);
    result[TZ_MINUTE_ONES_POS] = static_cast<char>('0' + offsetMinute % DECIMAL_BASE);
    return EtsString::CreateFromMUtf8(result.c_str());
}

extern "C" EtsString *StdCoreDateGetTimezoneName(int64_t ms)
{
    UErrorCode success = U_ZERO_ERROR;
    int32_t stdOffset;
    int32_t dstOffset;
    icu::TimeZone *tzlocal = icu::TimeZone::createDefault();
    icu::UnicodeString s;
    std::string result;
    tzlocal->getOffset(ms, 0, stdOffset, dstOffset, success);
    bool inDayLight = (dstOffset != 0);
    tzlocal->getDisplayName(static_cast<UBool>(inDayLight), icu::TimeZone::EDisplayType::LONG, s);
    s.toUTF8String(result);
    delete tzlocal;
    return EtsString::CreateFromMUtf8(result.c_str());
}

extern "C" int64_t ChronoGetCpuTime()
{
    // NOTE(ipetrov, #15499): Need to change approach when coroutine can migrate to other executionCtx
    return ark::os::time::GetClockTimeInThreadCpuTime();
}

}  // namespace ark::ets::intrinsics
