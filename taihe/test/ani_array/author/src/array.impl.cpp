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
#include "taihe/array.hpp"

#include <algorithm>
#include <numeric>

#include "array_test.Color.proj.0.hpp"
#include "array_test.Data.proj.1.hpp"
#include "array_test.impl.hpp"
#include "stdexcept"
#include "taihe/map.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {
int32_t sumArray(array_view<int32_t> nums, int32_t base)
{
    return std::accumulate(nums.begin(), nums.end(), base);
}

int64_t getArrayValue(array_view<int64_t> nums, int32_t idx)
{
    if (idx >= 0 && idx < nums.size()) {
        return nums[idx];
    }
    throw std::runtime_error("Index out of range");
}

array<string> toStingArray(array_view<int32_t> nums)
{
    auto result = array<string>::make(nums.size(), "");
    std::transform(nums.begin(), nums.end(), result.begin(), [](int32_t n) { return to_string(n); });
    return result;
}

array<int32_t> makeIntArray(int32_t value, int32_t num)
{
    return array<int32_t>::make(num, value);
}

array<::array_test::Color> makeEnumArray(::array_test::Color value, int32_t num)
{
    return array<::array_test::Color>::make(num, value);
}

array<map<string, int64_t>> makeRecordArray(string_view key, int64_t val, int32_t num)
{
    map<string, int64_t> record;
    record.emplace(key, val);
    return array<map<string, int64_t>>::make(num, record);
}

array<::array_test::Data> makeStructArray(string_view a, string_view b, int32_t c, int32_t num)
{
    return array<::array_test::Data>::make(num, ::array_test::Data {a, b, c});
}

array<array<int32_t>> makeIntArray2(array_view<int32_t> value, int32_t num)
{
    return array<array<int32_t>>::make(num, value);
}

array<::array_test::Color> changeEnumArray(array_view<::array_test::Color> value, ::array_test::Color color)
{
    auto result = array<::array_test::Color>::make(value.size(), value[0]);
    std::transform(value.begin(), value.end(), result.begin(), [color](::array_test::Color c) { return color; });
    return result;
}

array<map<string, int64_t>> changeRecordArray(array_view<map<string, int64_t>> value, string_view k, int64_t v)
{
    auto result = array<map<string, int64_t>>::make(value.size(), value[0]);
    map<string, int64_t> record;
    record.emplace(k, v);
    std::transform(value.begin(), value.end(), result.begin(), [record](map<string, int64_t> m) { return record; });
    return result;
}

array<::array_test::Data> changeStructArray(array_view<::array_test::Data> value, string_view a, string_view b,
                                            int32_t c)
{
    auto result = array<::array_test::Data>::make(value.size(), value[0]);
    std::transform(value.begin(), value.end(), result.begin(), [a, b, c](::array_test::Data d) {
        return ::array_test::Data {a, b, c};
    });
    return result;
}

array<float> fetchBinaryDataSync(int32_t num)
{
    if (num <= 0) {
        taihe::set_error("some error happen in fetchBinaryDataSync");
        return array<float>::make(0);
    } else {
        return array<float>::make(num);
    }
}

array<array<::array_test::Data>> makeStructArrayArray(string_view a, string_view b, int32_t c, int32_t num1,
                                                      int32_t num2)
{
    auto arr = array<::array_test::Data>::make(num1, ::array_test::Data {a, b, c});
    return array<array<::array_test::Data>>::make(num2, arr);
}
}  // namespace
// The macros used below are automatically generated code.
// NOLINTBEGIN
TH_EXPORT_CPP_API_sumArray(sumArray);
TH_EXPORT_CPP_API_getArrayValue(getArrayValue);
TH_EXPORT_CPP_API_toStingArray(toStingArray);
TH_EXPORT_CPP_API_makeIntArray(makeIntArray);
TH_EXPORT_CPP_API_makeEnumArray(makeEnumArray);
TH_EXPORT_CPP_API_makeRecordArray(makeRecordArray);
TH_EXPORT_CPP_API_makeStructArray(makeStructArray);
TH_EXPORT_CPP_API_makeIntArray2(makeIntArray2);
TH_EXPORT_CPP_API_changeEnumArray(changeEnumArray);
TH_EXPORT_CPP_API_changeRecordArray(changeRecordArray);
TH_EXPORT_CPP_API_changeStructArray(changeStructArray);
TH_EXPORT_CPP_API_fetchBinaryDataSync(fetchBinaryDataSync);
TH_EXPORT_CPP_API_makeStructArrayArray(makeStructArrayArray);
// NOLINTEND
