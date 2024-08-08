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

class EtsConstructorAsArgErrorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionArr"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_arrow)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFunctionArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionArr"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_arrow)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassArrowFunctionArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkParentClassInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkParentClassString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_array)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkParentClassArray"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkParentClassObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_arrow)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkParentClassArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_array)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassArray"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_arrow)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_int)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_string)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_arr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassArr"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_obj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_arrow)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassArrow"));
}

}  // namespace ark::ets::interop::js::testing
