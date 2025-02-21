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

class ClassGetStaticFieldByNameByteTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameByteTest, get_static_field_byte_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetbyteStatic;", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Byte(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_byte(0));
}

TEST_F(ClassGetStaticFieldByNameByteTest, get_static_field_byte)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetbyteStatic;", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_byte(0));
}

TEST_F(ClassGetStaticFieldByNameByteTest, get_static_field_byte_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetbyteStatic;", &cls), ANI_OK);

    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetbyteStatic;", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetbyteStatic;", &cls), ANI_OK);
    ani_byte age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameByteTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetbyteStatic;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Byte(cls, "name", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
