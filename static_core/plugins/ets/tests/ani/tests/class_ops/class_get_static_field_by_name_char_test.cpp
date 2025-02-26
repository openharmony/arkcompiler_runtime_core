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
 * See the License for the specific languname governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldByNameCharTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameCharTest, get_static_field_char_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);
    ani_char name;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Char(env_, cls, "name", &name), ANI_OK);
    ASSERT_EQ(name, ani_char('b'));
}

TEST_F(ClassGetStaticFieldByNameCharTest, get_static_field_char)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);
    ani_char name;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "name", &name), ANI_OK);
    ASSERT_EQ(name, ani_char('b'));
}

TEST_F(ClassGetStaticFieldByNameCharTest, not_found)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);
    ani_char name;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "nameChar", &name), ANI_NOT_FOUND);
}

TEST_F(ClassGetStaticFieldByNameCharTest, get_static_field_char_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);

    ani_char name;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "age", &name), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);
    ani_char name;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(nullptr, "name", &name), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);
    ani_char name;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, nullptr, &name), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameCharTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetCharStatic;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "name", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
