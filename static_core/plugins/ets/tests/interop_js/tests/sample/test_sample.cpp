/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropJsSampleTest : public EtsInteropTest {};

TEST_F(EtsInteropJsSampleTest, ets_return_int_arg0)
{
    constexpr int ARG0 = 253;
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<int32_t>("ets_return_int_arg0", ARG0);
    ASSERT_EQ(ret, ARG0);
}

TEST_F(EtsInteropJsSampleTest, ets_sum_double)
{
    constexpr double ARG0 = 5.2;
    constexpr double ARG1 = 9.3;
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<double>("ets_sum_double", ARG0, ARG1);
    ASSERT_TRUE(ret.has_value());
    constexpr double SUM = ARG0 + ARG1;
    ASSERT_DOUBLE_EQ(ret.value(), SUM);
}

TEST_F(EtsInteropJsSampleTest, ets_sum_any_types)
{
    constexpr double ARG0 = 934;
    constexpr std::string_view ARG1 = "_input_string_";
    constexpr double ARG2 = 9.435;
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<std::string>("ets_sum_any_types", ARG0, ARG1, ARG2);
    ASSERT_TRUE(ret.has_value());
    constexpr std::string_view RES = "934_input_string_9.435";
    ASSERT_STREQ(ret.value().data(), RES.data());
}

}  // namespace ark::ets::interop::js::testing
