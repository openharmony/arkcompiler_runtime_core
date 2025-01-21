/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ErrorHandlingTest : public AniTest {
public:
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "errorThrow", "I:I", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }
};

TEST_F(ErrorHandlingTest, exist_unhandled_error_test)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int errorResult;
    ani_boolean result;
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &errorResult, 5U), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ErrorHandlingTest, reset_error_test)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int errorResult;
    ani_boolean result;
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &errorResult, 5U), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
