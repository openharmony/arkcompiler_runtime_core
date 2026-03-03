/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class EtsMimicIteratorEtsToTsTest : public EtsInteropTest {};

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorNumberForOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberForOf"));
}

// NOTE(zaytsevaekaterina): disabled, fix in #33284
TEST_F(EtsMimicIteratorEtsToTsTest, DISABLED_testMimicIteratorNumberKeys)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberKeys"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorNumberValues)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberValues"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorString"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorBoolean"));
}

// NOTE(zaytsevaekaterina): disabled, fix in #33284
TEST_F(EtsMimicIteratorEtsToTsTest, DISABLED_testMimicIteratorBooleanEntries)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorBooleanEntries"));
}

// NOTE(zaytsevaekaterina): disabled, fix in #33284
TEST_F(EtsMimicIteratorEtsToTsTest, DISABLED_testMimicIteratorStringEntries)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorStringEntries"));
}

// NOTE(zaytsevaekaterina): disabled, fix in #33284
TEST_F(EtsMimicIteratorEtsToTsTest, DISABLED_testMimicIteratorNumberEntries)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberEntries"));
}

}  // namespace ark::ets::interop::js::testing
