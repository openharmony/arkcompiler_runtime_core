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

class EtsInteropCallableTest : public EtsInteropTest {};

TEST_F(EtsInteropCallableTest, TestNamedFunction)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestNamedFunction");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropCallableTest, TestAnonymousFunction)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestAnonymousFunction");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropCallableTest, TestArrowFunction)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestArrowFunction");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropCallableTest, TestConstructedFunction)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestConstructedFunction");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropCallableTest, TestCallBoundFunction)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestCallBoundFunction");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
