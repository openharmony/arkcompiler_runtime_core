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

class EtsInteropJsTestFrontend : public EtsInteropTest {
public:
    void SetUp() override
    {
        EtsInteropTest::SetUp();
        LoadModuleAs("test_frontend", "index.js");
    }
};

TEST_F(EtsInteropJsTestFrontend, test_newcall)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("Test.test_newcall"));
}

TEST_F(EtsInteropJsTestFrontend, test_dyncall)
{
    auto ret = CallEtsMethod<uint32_t>("Test.test_dyncall");
    ASSERT_EQ(ret, 10U);
}

TEST_F(EtsInteropJsTestFrontend, test_dyncall_by_value)
{
    auto ret = CallEtsMethod<uint32_t>("Test.test_dyncall_by_value");
    ASSERT_EQ(ret, 43U);
}

}  // namespace ark::ets::interop::js::testing
