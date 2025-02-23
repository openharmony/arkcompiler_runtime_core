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
#include <iostream>
#include <cstdarg>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CallObjectMethodLongTest : public AniTest {
public:
    void NewObject(ani_class cls, ani_object *result)
    {
        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_C", ":LC;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *result = static_cast<ani_object>(ref);
    }

    ani_status TestFuncV(ani_object object, ani_method method, ani_long *result, ...)
    {
        va_list args;
        va_start(args, result);
        ani_status aniResult = env_->Object_CallMethod_Long_V(object, method, result, args);
        va_end(args);
        return aniResult;
    }
};

// ninja ani_test_object_callmethod_long_gtests
TEST_F(CallObjectMethodLongTest, CallMethodLongTestOK)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LC;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_object obj {};
    this->NewObject(cls, &obj);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_method", "JI:J", &method), ANI_OK);

    ani_long result;
    ani_value arg1;
    ani_value arg2;
    arg1.l = 1L;
    const int testVal = 121;
    arg2.i = testVal;
    ani_long arg3 = 0;
    // Object_CallMethod_Long
    ASSERT_EQ(env_->Object_CallMethod_Long(obj, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg3);
    ASSERT_EQ(env_->c_api->Object_CallMethod_Long(env_, obj, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg3);
    // Object_CallMethod_Long_A
    const std::vector<ani_value> longArgs = {arg1, arg2};
    ASSERT_EQ(env_->Object_CallMethod_Long_A(obj, method, &result, longArgs.data()), ANI_OK);
    ASSERT_EQ(result, arg3);

    // Object_CallMethod_Long_V
    ASSERT_EQ(this->TestFuncV(obj, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg3);
}

TEST_F(CallObjectMethodLongTest, CallMethodLongTestError)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LC;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_object obj {};
    this->NewObject(cls, &obj);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_method", "JI:J", &method), ANI_OK);

    ani_long result;
    ani_value arg1;
    ani_value arg2;
    ASSERT_EQ(env_->Object_CallMethod_Long(obj, method, nullptr, arg1), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethod_Long(obj, nullptr, nullptr, arg1), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethod_Long(nullptr, method, nullptr, arg1), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethod_Long_A(nullptr, method, &result, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(this->TestFuncV(obj, nullptr, &result, arg1, arg2), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
