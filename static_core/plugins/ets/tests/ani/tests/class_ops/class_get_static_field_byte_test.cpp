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

class ClassGetStaticFieldByteTest : public AniTest {};

TEST_F(ClassGetStaticFieldByteTest, get_byte)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LTestByte;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    const ani_byte target = 126;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldByteTest, get_byte_c_api)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LTestByte;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    const ani_byte target = 126;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Byte(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldByteTest, get_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LTestByte;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LTestByte;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_byte result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LTestByte;", &cls), ANI_OK);
    ani_byte result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByteTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LTestByte;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "byte_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Byte(cls, field, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)