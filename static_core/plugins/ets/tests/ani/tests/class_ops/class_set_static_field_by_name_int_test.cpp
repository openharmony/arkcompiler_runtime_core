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
#include <cmath>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassSetStaticFieldByNameIntTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        const ani_int setTarget = 2U;
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, fieldName, setTarget), ANI_OK);
        ani_int resultValue = 0;
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, fieldName, &resultValue), ANI_OK);
        ASSERT_EQ(resultValue, setTarget);
    }
};

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_capi)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Int(env_, cls, "int_value", setTarget), ANI_OK);
    ani_int resultValue = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Int(env_, cls, "int_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", setTarget), ANI_OK);
    ani_int resultValue = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, setTarget);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "string_value", 1), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_invalid_args_object)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(nullptr, "int_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_invalid_args_field)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, nullptr, setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameIntTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);
    ani_int single = 0;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", static_cast<ani_int>(0)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(0));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", static_cast<ani_int>(NULL)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(NULL));

    ani_int max = std::numeric_limits<ani_int>::max();
    ani_int min = -std::numeric_limits<ani_int>::max();

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", static_cast<ani_int>(max)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(max));

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", static_cast<ani_int>(min)), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(min));
}

TEST_F(ClassSetStaticFieldByNameIntTest, combination_test1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    const ani_short setTarget2 = 3U;
    ani_int single = 0;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", setTarget2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &single), ANI_OK);
        ASSERT_EQ(single, setTarget2);
    }
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", setTarget), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &single), ANI_OK);
    ASSERT_EQ(single, setTarget);
}

TEST_F(ClassSetStaticFieldByNameIntTest, combination_test2)
{
    CheckFieldValue("class_set_static_field_by_name_int_test.PackstaticA", "int_value");
}

TEST_F(ClassSetStaticFieldByNameIntTest, combination_test3)
{
    CheckFieldValue("class_set_static_field_by_name_int_test.PackstaticFinal", "int_value");
}

TEST_F(ClassSetStaticFieldByNameIntTest, invalid_argument1)
{
    ani_class cls {};
    const ani_short setTarget = 2U;
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Packstatic", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "\n", setTarget), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Int(nullptr, cls, "int_value", setTarget), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Parent", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "parentStaticField", intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Parent", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "childStaticField", intValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Parent", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "grandchildStaticField", intValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Child", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "parentStaticField", intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Child", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "childStaticField", intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization5)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Child", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "grandchildStaticField", intValue), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Grandchild", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "parentStaticField", intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Grandchild", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "childStaticField", intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

TEST_F(ClassSetStaticFieldByNameIntTest, check_initialization8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_set_static_field_by_name_int_test.Grandchild", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_FALSE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
    const ani_int intValue = 24;
    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "grandchildStaticField", intValue), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Parent"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Child"));
    ASSERT_TRUE(IsRuntimeClassInitialized("class_set_static_field_by_name_int_test.Grandchild"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
