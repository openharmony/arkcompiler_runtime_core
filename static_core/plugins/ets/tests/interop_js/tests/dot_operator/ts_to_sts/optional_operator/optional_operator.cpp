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

class EtsOptionalOperatorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsOptionalOperatorTsToEtsTest, checkOptionalOperatorWithValue)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorWithValue"));
}
// NOTE (17745) - enable after fix alias undefined
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorReturnUndefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorReturnUndefined"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_check_optional_operator_with_value_by_string_index)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("check_optional_operator_with_value_by_string_index"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorReturnUndefined_by_string_index)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorReturnUndefined_by_string_index"));
}

TEST_F(EtsOptionalOperatorTsToEtsTest, checkOptionalOperatorInsideObjectWithValue)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorInsideObjectWithValue"));
}
// NOTE (17745) - enable after fix alias undefined
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorInsideObjectReturnUndefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorInsideObjectReturnUndefined"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorInsideObjectWithValueByStringIndex)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorInsideObjectWithValueByStringIndex"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorInsideReturnUndefinedByStringIndex)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorInsideReturnUndefinedByStringIndex"));
}
// NOTE  (alexanderpolenov) issue(18447) enable after fix call function with ?. operator
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunction)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorFunction"));
}

TEST_F(EtsOptionalOperatorTsToEtsTest, checkOptionalOperatorFunctionUndefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorFunctionUndefined"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunctionByStringIndex)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorFunctionByStringIndex"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunctionReturnUndefinedByStringIndex)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkOptionalOperatorFunctionReturnUndefinedByStringIndex"));
}

}  // namespace ark::ets::interop::js::testing
