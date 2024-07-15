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

class EtsInteropClassOperationsTest : public EtsInteropTest {
public:
    void SetUp() override
    {
        interopJsTestPath_ = std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES");
        // This object is used to save global js names
        if (!SetGtestEnv()) {
            std::abort();
        }
        LoadModuleAs("module", "module.js");
    }
};

TEST_F(EtsInteropClassOperationsTest, TestJSCallEmpty)
{
    auto ret = CallEtsMethod<int64_t>("jscallEmpty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewEmpty)
{
    auto ret = CallEtsMethod<int64_t>("jsnewEmpty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticEmpty)
{
    auto ret = CallEtsMethod<int64_t>("jscallStaticMethodEmpty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallObject)
{
    auto ret = CallEtsMethod<int64_t>("jscallObject");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewObject)
{
    auto ret = CallEtsMethod<int64_t>("jsnewObject");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewObjectSetProperty)
{
    auto ret = CallEtsMethod<int64_t>("jsnewSetPropertyObject");
    ASSERT_EQ(ret, 0);
}

// returns nan
TEST_F(EtsInteropClassOperationsTest, TestJSCallMethodObject)
{
    auto ret = CallEtsMethod<int64_t>("jscallMethodObject");
    ASSERT_EQ(ret, 0);
}

// simplification of previous failure
TEST_F(EtsInteropClassOperationsTest, TestJSCallMethodObjectSimple)
{
    auto ret = CallEtsMethod<int64_t>("jscallMethodSimple");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallString)
{
    auto ret = CallEtsMethod<int64_t>("jscallString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewString)
{
    auto ret = CallEtsMethod<int64_t>("jsnewString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestjsnewSetPropertyString)
{
    auto ret = CallEtsMethod<int64_t>("jsnewSetPropertyString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticString)
{
    auto ret = CallEtsMethod<int64_t>("jscallStaticMethodString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallArray)
{
    auto ret = CallEtsMethod<int64_t>("jscallArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewArray)
{
    auto ret = CallEtsMethod<int64_t>("jsnewArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestjsnewSetPropertyArray)
{
    auto ret = CallEtsMethod<int64_t>("jsnewSetPropertyArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticArray)
{
    auto ret = CallEtsMethod<int64_t>("jscallStaticMethodArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestNamespace)
{
    auto ret = CallEtsMethod<int64_t>("testNamespace");
    ASSERT_EQ(ret, 0);
}

}  // namespace ark::ets::interop::js::testing
