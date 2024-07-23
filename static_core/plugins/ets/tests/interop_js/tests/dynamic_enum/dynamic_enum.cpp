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

class EtsInteropEnum : public EtsInteropTest {};

TEST_F(EtsInteropEnum, Test1)
{
    auto ret = CallEtsMethod<bool>("Test1");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropEnum, Test2)
{
    auto ret = CallEtsMethod<bool>("Test2");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropEnum, Test3)
{
    auto ret = CallEtsMethod<bool>("Test3");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropEnum, Test4)
{
    auto ret = CallEtsMethod<bool>("Test4");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropEnum, Test5)
{
    auto ret = CallEtsMethod<bool>("Test5");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropEnum, Test6)
{
    auto ret = CallEtsMethod<bool>("Test6");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
