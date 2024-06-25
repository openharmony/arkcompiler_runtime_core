/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

class EtsPassingOptionalType : public EtsInteropTest {};

TEST_F(EtsPassingOptionalType, optionalAnyParam)
{
    auto ret = CallEtsMethod<bool>("optionalAnyParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalAnyWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalAnyWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalAnyObjectParam)
{
    auto ret = CallEtsMethod<bool>("optionalAnyObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalAnyObjectWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalAnyObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralParam)
{
    auto ret = CallEtsMethod<bool>("optionalLiteralParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalLiteralWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralObjectParam)
{
    auto ret = CallEtsMethod<bool>("optionalLiteralObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralObjectWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalLiteralObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetParam)
{
    auto ret = CallEtsMethod<bool>("optionalExtraSetParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalExtraSetWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectReduseParam)
{
    auto ret = CallEtsMethod<bool>("optionalSubSetObjectReduseParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectPartialParam)
{
    auto ret = CallEtsMethod<bool>("optionalSubSetObjectPartialParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetObjectParam)
{
    auto ret = CallEtsMethod<bool>("optionalExtraSetObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetObjectWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalExtraSetObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectParam)
{
    auto ret = CallEtsMethod<bool>("optionalSubSetObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalSubSetObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionStringParam)
{
    auto ret = CallEtsMethod<bool>("optionalUnionStringParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionStringWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalUnionStringWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionNumberParam)
{
    auto ret = CallEtsMethod<int64_t>("optionalUnionNumberParam");
    constexpr int EXPECTED_VALUE = 999;
    ASSERT_EQ(ret, EXPECTED_VALUE);
}

TEST_F(EtsPassingOptionalType, optionalUnionWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalUnionWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionStringObjectParam)
{
    auto ret = CallEtsMethod<bool>("optionalUnionStringObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionNumberObjectParam)
{
    auto ret = CallEtsMethod<int64_t>("optionalUnionNumberObjectParam");
    constexpr int EXPECTED_VALUE = 777;
    ASSERT_EQ(ret, EXPECTED_VALUE);
}

TEST_F(EtsPassingOptionalType, optionalUnionObjectWithoutParam)
{
    auto ret = CallEtsMethod<bool>("optionalUnionObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing