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

class EtsNonInteropCallTest : public EtsInteropTest {};

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropGetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropGetProperty"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropSetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropSetProperty"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropInvokeMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropInvokeMethod"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropInvokeFunction)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropInvokeFunction"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropInstantiate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropInstantiate"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropIsInstanceOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropIsInstanceOf"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropHasOwnProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropHasOwnProperty"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithInteropAsyncFunction)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithInteropAsyncFunction"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropGetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropGetProperty"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropSetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropSetProperty"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropGetPropertyByIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropGetPropertyByIndex"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropInvokeMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropInvokeMethod"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropInvokeFunction)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropInvokeFunction"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropInstantiate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropInstantiate"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropIsInstanceOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropIsInstanceOf"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropHasOwnProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropHasOwnProperty"));
}

TEST_F(EtsNonInteropCallTest, checkEAWorkerWithoutInteropAsyncFunction)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEAWorkerWithoutInteropAsyncFunction"));
}

}  // namespace ark::ets::interop::js::testing
