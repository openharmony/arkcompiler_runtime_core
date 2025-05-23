/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <iostream>

#include "enum_test.impl.hpp"
#include "stdexcept"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
static constexpr std::size_t COLOR_COUNT = 3;
static constexpr std::size_t WEEKDAY_COUNT = 7;

::enum_test::Color nextEnum(::enum_test::Color color)
{
    return (::enum_test::Color::key_t)(((int)color.get_key() + 1) % COLOR_COUNT);
}

void showEnum(::enum_test::Color color)
{
    std::cout << color << std::endl;
}

::enum_test::Weekday nextEnumWeekday(::enum_test::Weekday day)
{
    return (::enum_test::Weekday::key_t)(((int)day.get_key() + 1) % WEEKDAY_COUNT);
}

void showEnumWeekday(::enum_test::Weekday day)
{
    std::string dayStr;
    if (day.get_key() == ::enum_test::Weekday::key_t::MONDAY) {
        dayStr = "Monday";
    } else if (day.get_key() == ::enum_test::Weekday::key_t::TUESDAY) {
        dayStr = "Tuesday";
    } else if (day.get_key() == ::enum_test::Weekday::key_t::WEDNESDAY) {
        dayStr = "Wednesday";
    } else if (day.get_key() == ::enum_test::Weekday::key_t::THURSDAY) {
        dayStr = "Thursday";
    } else if (day.get_key() == ::enum_test::Weekday::key_t::FRIDAY) {
        dayStr = "Friday";
    } else if (day.get_key() == ::enum_test::Weekday::key_t::SATURDAY) {
        dayStr = "Saturday";
    } else if (day.get_key() == ::enum_test::Weekday::key_t::SUNDAY) {
        dayStr = "Sunday";
    } else {
        dayStr = "Unknown";
    }
    std::cout << dayStr << std::endl;
}
}  // namespace
// The macros used below are automatically generated code.
// NOLINTBEGIN
TH_EXPORT_CPP_API_nextEnum(nextEnum);
TH_EXPORT_CPP_API_showEnum(showEnum);
TH_EXPORT_CPP_API_nextEnumWeekday(nextEnumWeekday);
TH_EXPORT_CPP_API_showEnumWeekday(showEnumWeekday);
// NOLINTEND
