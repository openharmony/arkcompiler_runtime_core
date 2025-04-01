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

class ClassGetStaticFieldFloatTest : public AniTest {};

TEST_F(ClassGetStaticFieldFloatTest, get_float)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_float_test/TestFloat;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    const ani_float target = 18.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldFloatTest, get_float_c_api)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_float_test/TestFloat;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    const ani_float target = 18.0F;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Float(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, target);
}

TEST_F(ClassGetStaticFieldFloatTest, get_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_float_test/TestFloat;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_float_test/TestFloat;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_float_test/TestFloat;", &cls), ANI_OK);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldFloatTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_float_test/TestFloat;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "float_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Float(cls, field, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)