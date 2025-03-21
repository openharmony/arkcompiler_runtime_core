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
class ClassSetStaticFieldBooleanTest : public AniTest {};
TEST_F(ClassSetStaticFieldBooleanTest, set_boolean)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_boolean_test/TestSetBoolean;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldBooleanTest, set_boolean_c_api)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_boolean_test/TestSetBoolean;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Boolean(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->c_api->Class_SetStaticField_Boolean(env_, cls, field, ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Boolean(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldBooleanTest, set_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_boolean_test/TestSetBoolean;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, field, ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldBooleanTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_boolean_test/TestSetBoolean;", &cls), ANI_OK);
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "bool_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->Class_GetStaticField_Boolean(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(nullptr, field, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldBooleanTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_boolean_test/TestSetBoolean;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticField_Boolean(cls, nullptr, ANI_TRUE), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)