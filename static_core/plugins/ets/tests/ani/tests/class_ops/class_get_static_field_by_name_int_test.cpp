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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldByNameIntTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameIntTest, get_static_field_int_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetIntStatic;", &cls), ANI_OK);
    ani_int age;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Int(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_int(20U));
}

TEST_F(ClassGetStaticFieldByNameIntTest, get_static_field_int)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetIntStatic;", &cls), ANI_OK);
    ani_int age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_int(20U));
}

TEST_F(ClassGetStaticFieldByNameIntTest, get_static_field_int_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetIntStatic;", &cls), ANI_OK);

    ani_int age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetIntStatic;", &cls), ANI_OK);
    ani_int age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetIntStatic;", &cls), ANI_OK);
    ani_int age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameIntTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetIntStatic;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "name", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
