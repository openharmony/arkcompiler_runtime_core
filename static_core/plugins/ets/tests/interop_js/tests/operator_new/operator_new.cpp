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

class EtsOperatorNew : public EtsInteropTest {};

TEST_F(EtsOperatorNew, noArgsClass)
{
    auto ret = CallEtsMethod<bool>("noArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, noArgsClassBrackes)
{
    auto ret = CallEtsMethod<bool>("noArgsClassBrackes");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, oneArgsClass)
{
    auto ret = CallEtsMethod<bool>("oneArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, twoArgsClass)
{
    auto ret = CallEtsMethod<bool>("twoArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, manyArgsClass)
{
    auto ret = CallEtsMethod<bool>("manyArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithSubClass)
{
    auto ret = CallEtsMethod<bool>("newWithSubClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithException)
{
    auto ret = CallEtsMethod<bool>("newWithException");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithFunction)
{
    auto ret = CallEtsMethod<bool>("newWithFunction");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsOperatorNew, newWithArrey)
{
    auto ret = CallEtsMethod<bool>("newWithArrey");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithPrototype)
{
    auto ret = CallEtsMethod<bool>("newWithPrototype");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithJustFunction)
{
    auto ret = CallEtsMethod<bool>("newWithJustFunction");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithObj)
{
    auto ret = CallEtsMethod<bool>("newWithObj");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithPrimitive)
{
    auto ret = CallEtsMethod<bool>("newWithPrimitive");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
