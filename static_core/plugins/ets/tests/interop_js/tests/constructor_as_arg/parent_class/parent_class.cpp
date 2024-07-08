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

class EtsParentClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsParentClassTsToEtsTest, check_main_class_as_arg)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_main_class_as_arg"));
}

TEST_F(EtsParentClassTsToEtsTest, check_anonymous_class_as_arg)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_anonymous_class_as_arg"));
}

TEST_F(EtsParentClassTsToEtsTest, check_IIFE_class_as_arg)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_IIFE_class_as_arg"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_main_class_with_arg_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_main_class_with_arg_from_ts"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_main_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_main_class_from_ts"));
}

TEST_F(EtsParentClassTsToEtsTest, check_main_class_instance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_main_class_instance"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_anonymous_class_with_arg_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_anonymous_class_with_arg_from_ts"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_anonymous_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_anonymous_class_from_ts"));
}

TEST_F(EtsParentClassTsToEtsTest, check_anonymous_class_instance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_anonymous_class_instance"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_iife_class_from_ts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_create_iife_class_from_ts"));
}

TEST_F(EtsParentClassTsToEtsTest, check_iife_class_instance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_iife_class_instance"));
}

}  // namespace ark::ets::interop::js::testing
