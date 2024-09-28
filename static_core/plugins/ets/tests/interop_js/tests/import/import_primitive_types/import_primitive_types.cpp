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

class EtsImportPrimitiveTypesTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeNumber"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeString"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeTrue)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeTrue"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeFalse)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeFalse"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeUndefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeUndefined"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeNull)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeNull"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeSymbol)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeSymbol"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsImportPrimitiveTypesTsToEtsTest, DISABLED_checkChangeBigInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChangeBigInt"));
}

}  // namespace ark::ets::interop::js::testing
