/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class EtsErrorJsToEtsTest : public EtsInteropTest {};

TEST_F(EtsErrorJsToEtsTest, test_error_catch_num_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkNumError"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_str_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkStrError"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_obj_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkObjError"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_subclass_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSubClassError"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_error_data)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkErrorData"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_error_msg)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkErrorMsg"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_error_cause)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkErrorCause"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkError"));
}

TEST_F(EtsErrorJsToEtsTest, test_error_catch_ets_error)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkEtsError"));
}

}  // namespace ark::ets::interop::js::testing
