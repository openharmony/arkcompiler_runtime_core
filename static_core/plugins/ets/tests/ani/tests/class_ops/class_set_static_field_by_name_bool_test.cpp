/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class ClassSetStaticFieldByNameBoolTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, fieldName, ANI_TRUE), ANI_OK);
        ani_boolean resultValue = ANI_FALSE;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, ANI_TRUE);
    }
};

TEST_F(ClassSetStaticFieldByNameBoolTest, set_static_field_by_name_bool_capi)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Boolean(env_, cls, "bool_value", ANI_TRUE), ANI_OK);
    ani_boolean resultValue = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Boolean(env_, cls, "bool_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, set_static_field_by_name_bool)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", ANI_TRUE), ANI_OK);
    ani_boolean resultValue = ANI_FALSE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, ANI_TRUE);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, set_static_field_by_name_bool_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "string_value", ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, set_static_field_by_name_bool_invalid_args_object)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(nullptr, "bool_value", ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, set_static_field_by_name_bool_invalid_args_field)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, nullptr, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", static_cast<ani_boolean>(0)), ANI_OK);
    ani_boolean single = ANI_FALSE;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", static_cast<ani_boolean>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", static_cast<ani_boolean>(!'s')), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", static_cast<ani_boolean>(!0)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", true), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_TRUE);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", false), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, combination_test1)
{
    ani_class cls {};
    ani_boolean single = ANI_FALSE;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", ANI_TRUE), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
        ASSERT_EQ(single, ANI_TRUE);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "bool_value", ANI_FALSE), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Boolean(cls, "bool_value", &single), ANI_OK);
    ASSERT_EQ(single, ANI_FALSE);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_bool_test.BoolStaticA", "bool_value");
}

TEST_F(ClassSetStaticFieldByNameBoolTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_bool_test.BoolStaticFinal", "bool_value");
}

TEST_F(ClassSetStaticFieldByNameBoolTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.BoolStatic", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "", ANI_TRUE), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "\n", ANI_TRUE), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Boolean(nullptr, cls, "bool_value", ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Parent", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "parentStaticField", ANI_TRUE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Parent", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "childStaticField", ANI_TRUE), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Parent", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "grandchildStaticField", ANI_TRUE), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Child", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "parentStaticField", ANI_TRUE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Child", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "childStaticField", ANI_TRUE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization5)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Child", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "grandchildStaticField", ANI_TRUE), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Grandchild", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "parentStaticField", ANI_TRUE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Grandchild", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "childStaticField", ANI_TRUE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameBoolTest, check_initialization8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_bool_test.Grandchild", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Boolean(cls, "grandchildStaticField", ANI_TRUE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Parent"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Child"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_bool_test.Grandchild"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
