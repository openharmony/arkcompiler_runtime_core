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

class EtsDynumicFunctionException : public EtsInteropTest {};

TEST_F(EtsDynumicFunctionException, CheckUserExceptionError)
{
    auto ret = CallEtsMethod<bool>("CheckUserExceptionError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckNameUserExceptionError)
{
    auto ret = CallEtsMethod<bool>("CheckNameUserExceptionError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckMessageUserExceptionError)
{
    auto ret = CallEtsMethod<bool>("CheckMessageUserExceptionError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CatchUserException)
{
    auto ret = CallEtsMethod<bool>("CatchUserException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CatchBuiltinException)
{
    auto ret = CallEtsMethod<bool>("CatchBuiltinException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckNameBuiltinException)
{
    auto ret = CallEtsMethod<bool>("CheckNameBuiltinException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckMessageBuiltinException)
{
    auto ret = CallEtsMethod<bool>("CheckMessageBuiltinException");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing