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

class ClassGetStaticFieldByNameDoubleTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameDoubleTest, get_static_field_double_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LWoman;", &cls), ANI_OK);
    ani_double age;
    const ani_double expectedAge = 20.0;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Double(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_double(expectedAge));
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, get_static_field_double)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LWoman;", &cls), ANI_OK);
    ani_double age;
    const ani_double expectedAge = 20.0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_double(expectedAge));
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, get_static_field_double_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LWoman;", &cls), ANI_OK);

    ani_double age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LWoman;", &cls), ANI_OK);
    ani_double age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LWoman;", &cls), ANI_OK);
    ani_double age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameDoubleTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LWoman;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "name", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
