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

class EtsMethodConstructorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsMethodConstructorTsToEtsTest, check_method_constructor_main_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodConstructorMainClass"));
}

TEST_F(EtsMethodConstructorTsToEtsTest, check_method_class_parent_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassParentClass"));
}

TEST_F(EtsMethodConstructorTsToEtsTest, check_method_class_child_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassChildClass"));
}

TEST_F(EtsMethodConstructorTsToEtsTest, check_method_class_anonymous_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassAnonymousClass"));
}

TEST_F(EtsMethodConstructorTsToEtsTest, check_method_class_IIFE_class)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodClassIIFEClass"));
}

}  // namespace ark::ets::interop::js::testing
