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
 * See the License for the specific langusingle governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldByNameBoolTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameBoolTest, get_static_field_bool_c_api_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetBoolStatic;", &cls), ANI_OK);
    ani_boolean single;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Boolean(env_, cls, "single", &single), ANI_OK);
    ASSERT_EQ(single, ani_boolean(ANI_FALSE));
}

TEST_F(ClassGetStaticFieldByNameBoolTest, get_static_field_bool)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetBoolStatic;", &cls), ANI_OK);
    ani_boolean single;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "single", &single), ANI_OK);
    ASSERT_EQ(single, ani_boolean(ANI_FALSE));
}

TEST_F(ClassGetStaticFieldByNameBoolTest, get_static_field_bool_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetBoolStatic;", &cls), ANI_OK);

    ani_boolean single;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "name", &single), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetBoolStatic;", &cls), ANI_OK);
    ani_boolean single;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(nullptr, "name", &single), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetBoolStatic;", &cls), ANI_OK);
    ani_boolean single;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, nullptr, &single), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameBoolTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetBoolStatic;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "name", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)