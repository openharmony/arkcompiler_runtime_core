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

class EtsFunctionTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsFunctionTsToEtsTest, check_create_class_function_main)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionMain"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_function_parent)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionParent"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_function_child)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionChild"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_function_anonymous)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionAnonymous"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_function_IIFE)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionIIFE"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_arrow_function_main)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionMain"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_arrow_function_parent)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionParent"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_arrow_function_child)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionChild"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_arrow_function_anonymous)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionAnonymous"));
}

TEST_F(EtsFunctionTsToEtsTest, check_create_class_arrow_function_IIFE)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionIIFE"));
}

}  // namespace ark::ets::interop::js::testing
