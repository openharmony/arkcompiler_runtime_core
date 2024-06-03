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

class EtsStaticFunctionException : public EtsInteropTest {};

TEST_F(EtsStaticFunctionException, CheckCustomException)
{
    auto ret = CallEtsMethod<bool>("CheckCustomException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckNameCustomException)
{
    auto ret = CallEtsMethod<bool>("CheckNameCustomException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckMessageCustomException)
{
    auto ret = CallEtsMethod<bool>("CheckMessageCustomException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckBuiltinExceptionInStatic)
{
    auto ret = CallEtsMethod<bool>("CheckBuiltinExceptionInStatic");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckNameBuiltinExceptionInStatic)
{
    auto ret = CallEtsMethod<bool>("CheckNameBuiltinExceptionInStatic");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckMessageBuiltinExceptionInStatic)
{
    auto ret = CallEtsMethod<bool>("CheckMessageBuiltinExceptionInStatic");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, ThrowCustomError)
{
    auto ret = CallEtsMethod<bool>("ThrowCustomError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckThrowNewError)
{
    auto ret = CallEtsMethod<bool>("CheckThrowNewError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, throwTSCustomExceptionWithImportType)
{
    auto ret = CallEtsMethod<bool>("throwTSCustomExceptionWithImportType");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing