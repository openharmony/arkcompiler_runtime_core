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

class EtsForOfSimpleArrayTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsForOfSimpleArrayTsToEtsTest, checkEmptyArray)
{
    ASSERT_EQ(true, CallEtsFunction<bool>("simple_array", "checkEmptyArray"));
}

TEST_F(EtsForOfSimpleArrayTsToEtsTest, checkNumberArray)
{
    ASSERT_EQ(true, CallEtsFunction<bool>("simple_array", "checkNumberArray"));
}

TEST_F(EtsForOfSimpleArrayTsToEtsTest, checkStringArray)
{
    ASSERT_EQ(true, CallEtsFunction<bool>("simple_array", "checkStringArray"));
}

}  // namespace ark::ets::interop::js::testing
