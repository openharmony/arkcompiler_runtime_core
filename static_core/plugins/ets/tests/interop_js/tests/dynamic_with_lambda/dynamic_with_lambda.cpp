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

class EtsInteropJsDynamicWithLambda : public EtsInteropTest {};

TEST_F(EtsInteropJsDynamicWithLambda, TestArgs0)
{
    auto ret = CallEtsMethod<uint32_t>("TestArgs0");
    ASSERT_EQ(ret, 25U);
}

TEST_F(EtsInteropJsDynamicWithLambda, TestArgs1)
{
    auto ret = CallEtsMethod<uint32_t>("TestArgs1");
    ASSERT_EQ(ret, 25U);
}

TEST_F(EtsInteropJsDynamicWithLambda, TestArgs2)
{
    auto ret = CallEtsMethod<uint32_t>("TestArgs2");
    ASSERT_EQ(ret, 25U);
}

// NOTE: vpukhov. Function cast #14626
TEST_F(EtsInteropJsDynamicWithLambda, DISABLED_TestLambdaJSValue)
{
    auto ret = CallEtsMethod<uint32_t>("TestLambdaJSValue");
    ASSERT_EQ(ret, 25U);
}

// NOTE: vpukhov. Function cast #14626
TEST_F(EtsInteropJsDynamicWithLambda, DISABLED_TestLambdaJSValueCast)
{
    auto ret = CallEtsMethod<uint32_t>("TestLambdaJSValueCast");
    ASSERT_EQ(ret, 25U);
}

// NOTE: vpukhov. Function cast #14626
TEST_F(EtsInteropJsDynamicWithLambda, DISABLED_TestLambdaJSValueCache)
{
    auto ret = CallEtsMethod<uint32_t>("TestLambdaJSValueCache");
    ASSERT_EQ(ret, 25U);
}

// NOTE(itrubachev) this test can be enabled after fixing NOTE in checker::Type *TSAsExpression::Check in es2panda
TEST_F(EtsInteropJsDynamicWithLambda, DISABLED_TestLambdaJSValueCastCallAsArgument)
{
    auto ret = CallEtsMethod<uint32_t>("TestLambdaJSValueCastCallAsArgument");
    ASSERT_EQ(ret, 25U);
}

}  // namespace ark::ets::interop::js::testing
