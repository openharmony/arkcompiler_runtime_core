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

class ObjectNewTest : public AniTest {
public:
    void GetTestData(ani_class *clsResult, ani_method *ctorResult, ani_string *modelResult, ani_int *weightResult)
    {
        const char m[] = "Pure P60";
        const ani_int weight = 200;

        ani_class cls;
        ASSERT_EQ(env_->FindClass("LMobilePhone;", &cls), ANI_OK);

        ani_method ctor;
        ASSERT_EQ(env_->Class_GetMethod(cls, "<ctor>", "Lstd/core/String;I:V", &ctor), ANI_OK);

        ani_string model;
        ASSERT_EQ(env_->String_NewUTF8(m, strlen(m), &model), ANI_OK);

        *clsResult = cls;
        *ctorResult = ctor;
        *modelResult = model;
        *weightResult = weight;
    }
};

TEST_F(ObjectNewTest, object_new)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, &phone, model, weight), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkWeight", phone, weight), ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_a)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone;
    ASSERT_EQ(env_->Object_New_A(cls, ctor, &phone, args), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkWeight", phone, weight), ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_v)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->Object_New(cls, ctor, &phone, model, weight), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkWeight", phone, weight), ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_invalid_args1)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->c_api->Object_New(env_, nullptr, ctor, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_invalid_args2)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, nullptr, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_invalid_args3)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, nullptr, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args1)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone;
    ASSERT_EQ(env_->Object_New_A(nullptr, ctor, &phone, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args2)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone;
    ASSERT_EQ(env_->Object_New_A(cls, nullptr, &phone, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args3)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ASSERT_EQ(env_->Object_New_A(cls, ctor, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args4)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->Object_New_A(cls, ctor, &phone, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args1)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->Object_New(nullptr, ctor, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args2)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone;
    ASSERT_EQ(env_->Object_New(cls, nullptr, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args3)
{
    ani_class cls;
    ani_method ctor;
    ani_string model;
    ani_int weight;
    GetTestData(&cls, &ctor, &model, &weight);

    ASSERT_EQ(env_->Object_New(cls, ctor, nullptr, model, weight), ANI_INVALID_ARGS);
}

// NODE: Enable when #22280 is resolved
TEST_F(ObjectNewTest, DISABLED_object_new_string)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);

    ani_method ctor;
    ASSERT_EQ(env_->Class_GetMethod(cls, "<ctor>", ":V", &ctor), ANI_OK);

    ani_string str;
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, reinterpret_cast<ani_object *>(&str)), ANI_OK);

    // NODE: Check the resulting string
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)

}  // namespace ark::ets::ani::testing
