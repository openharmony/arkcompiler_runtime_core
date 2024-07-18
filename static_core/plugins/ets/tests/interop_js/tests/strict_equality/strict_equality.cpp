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

class EtsInteropStrictEqualityTest : public EtsInteropTest {};

TEST_F(EtsInteropStrictEqualityTest, TestEqualTrue)
{
    auto ret = CallEtsMethod<bool>("TestEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestEqualFalse)
{
    auto ret = CallEtsMethod<bool>("TestEqualFalse");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotEqualTrue)
{
    auto ret = CallEtsMethod<bool>("TestNotEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotEqualFalse)
{
    auto ret = CallEtsMethod<bool>("TestNotEqualFalse");
    ASSERT_EQ(ret, true);
}

// Behaviour for equal is the same as strict equal
TEST_F(EtsInteropStrictEqualityTest, TestNotStrictEqualTrue)
{
    auto ret = CallEtsMethod<bool>("TestNotStrictEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotStrictEqualFalse)
{
    auto ret = CallEtsMethod<bool>("TestNotStrictEqualFalse");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotStrictNotEqualTrue)
{
    auto ret = CallEtsMethod<bool>("TestNotStrictNotEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotStrictNotEqualFalse)
{
    auto ret = CallEtsMethod<bool>("TestNotStrictNotEqualFalse");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
