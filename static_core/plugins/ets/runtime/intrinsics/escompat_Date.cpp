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

#include <chrono>
#include <ctime>
#include <clocale>
#include <sstream>
#include "plugins/ets/runtime/types/ets_string.h"

namespace panda::ets::intrinsics {

constexpr const int32_t MINS_IN_HOUR = 60;
constexpr const int32_t MS_IN_SECOND = 1000;

extern "C" double EscompatDateNow()
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return now_ms.time_since_epoch().count();
}

extern "C" int64_t EscompatDateGetLocalTimezoneOffset()
{
    time_t now;
    struct tm gmt {};
    struct tm local {};
    int64_t seconds;

    std::time(&now);
    local = *localtime(&now);
    gmt = *gmtime(&now);

    seconds = static_cast<int64_t>(difftime(mktime(&gmt), mktime(&local)));

    return seconds / MINS_IN_HOUR;
}

extern "C" EtsString *EscompatDateGetTimezoneName()
{
    std::string s = std::string(*tzname);
    return EtsString::CreateFromMUtf8(s.c_str());
}

extern "C" EtsString *EscompatDateGetLocaleString(EtsString *format, EtsString *locale, int64_t ms, uint8_t is_utc)
{
    PandaVector<uint8_t> buf;
    auto format_s = std::string(format->ConvertToStringView(&buf));
    auto locale_s = std::string(locale->ConvertToStringView(&buf));
    std::time_t seconds = ms / MS_IN_SECOND;
    std::stringstream ss;
    ss.imbue(std::locale(ss.getloc(), new std::time_put_byname<char>(locale_s.c_str())));
    if (static_cast<bool>(is_utc)) {
        ss << std::put_time(std::gmtime(&seconds), format_s.c_str());
    } else {
        ss << std::put_time(std::localtime(&seconds), format_s.c_str());
    }
    return EtsString::CreateFromMUtf8(ss.str().c_str());
}

}  // namespace panda::ets::intrinsics
