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
#include <cmath>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassGetStaticFieldIntTest : public AniTest {
public:
    void CheckFieldValue(const char *className, const char *fieldName)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_field field {};
        ASSERT_EQ(env_->Class_FindStaticField(cls, fieldName, &field), ANI_OK);
        ASSERT_NE(field, nullptr);
        ani_int result = 0U;
        const ani_int target = 3U;
        ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, target);
        const ani_int setTar = 2U;
        ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, setTar), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar);
    }
};

TEST_F(ClassGetStaticFieldIntTest, get_int)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
}

TEST_F(ClassGetStaticFieldIntTest, get_int_c_api)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Int(env_, cls, field, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
}

TEST_F(ClassGetStaticFieldIntTest, get_invalid_field_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "string_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldIntTest, invalid_argument1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Int(nullptr, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldIntTest, invalid_argument2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_int result = 0;
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldIntTest, invalid_argument3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldIntTest, invalid_argument4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field = nullptr;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticField_Int(nullptr, cls, field, &result), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldIntTest, special_values)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    ani_int single = 0;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia1", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, static_cast<ani_int>(0));
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia3", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia4", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_INVALID_TYPE);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "specia5", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_INVALID_TYPE);
    ani_int max = std::numeric_limits<ani_int>::max();
    ani_int min = std::numeric_limits<ani_int>::min();
    ASSERT_EQ(env_->Class_FindStaticField(cls, "min", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, min);
    ASSERT_EQ(env_->Class_FindStaticField(cls, "max", &field), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &single), ANI_OK);
    ASSERT_EQ(single, max);
}

TEST_F(ClassGetStaticFieldIntTest, combination_test1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_get_static_field_int_test/TestInt;", &cls), ANI_OK);
    ani_static_field field {};
    const ani_int setTar = 1024;
    const ani_int setTar2 = 10;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "int_value", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int result = 0;
    const int32_t loopNum = 3;
    for (int32_t i = 0; i < loopNum; i++) {
        ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, setTar2), ANI_OK);
        ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
        ASSERT_EQ(result, setTar2);
    }
    ASSERT_EQ(env_->Class_SetStaticField_Int(cls, field, setTar), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(cls, field, &result), ANI_OK);
    ASSERT_EQ(result, setTar);
}

TEST_F(ClassGetStaticFieldIntTest, combination_test2)
{
    CheckFieldValue("Lclass_get_static_field_int_test/TestInt;", "int_value");
}

TEST_F(ClassGetStaticFieldIntTest, combination_test3)
{
    CheckFieldValue("Lclass_get_static_field_int_test/TestIntA;", "int_value");
}

TEST_F(ClassGetStaticFieldIntTest, combination_test4)
{
    CheckFieldValue("Lclass_get_static_field_int_test/TestIntFinal;", "int_value");
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)