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
        LoadModuleAs("module", "number_subtypes/module.js");
    }
};

TEST_F(EtsInteropClassOperationsTest, TestJSCallEmpty)
{
    auto ret = CallEtsMethod<int64_t>("jscall_empty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewEmpty)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_empty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticEmpty)
{
    auto ret = CallEtsMethod<int64_t>("jscall_static_method_empty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallObject)
{
    auto ret = CallEtsMethod<int64_t>("jscall_object");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewObject)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_object");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewObjectSetPropery)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_setproperty_object");
    ASSERT_EQ(ret, 0);
}

// returns nan
TEST_F(EtsInteropClassOperationsTest, TestJSCallMethodObject)
{
    auto ret = CallEtsMethod<int64_t>("jscall_method_object");
    ASSERT_EQ(ret, 0);
}

// simplification of previous failure
TEST_F(EtsInteropClassOperationsTest, TestJSCallMethodObjectSimple)
{
    auto ret = CallEtsMethod<int64_t>("jscall_method_simple");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallString)
{
    auto ret = CallEtsMethod<int64_t>("jscall_string");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewString)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_string");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewSetPropertyString)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_setproperty_string");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticString)
{
    auto ret = CallEtsMethod<int64_t>("jscall_static_method_string");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallArray)
{
    auto ret = CallEtsMethod<int64_t>("jscall_array");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewArray)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_array");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewSetPropertyArray)
{
    auto ret = CallEtsMethod<int64_t>("jsnew_setproperty_array");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticArray)
{
    auto ret = CallEtsMethod<int64_t>("jscall_static_method_array");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestNamespace)
{
    auto ret = CallEtsMethod<int64_t>("test_namespace");
    ASSERT_EQ(ret, 0);
}

}  // namespace ark::ets::interop::js::testing
