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

class ClassSetStaticFieldByNameDoubleTest : public AniTest {};

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double_capi)
{
    ani_class cls;
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_double_test/DoubleStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Double(env_, cls, "double_value", age), ANI_OK);
    ani_double resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Double(env_, cls, "double_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, age);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double)
{
    ani_class cls;
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_double_test/DoubleStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "double_value", age), ANI_OK);
    ani_double resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Double(cls, "double_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, age);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double_invalid_field_type)
{
    ani_class cls;
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_double_test/DoubleStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, "string_value", age), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double_invalid_args_object)
{
    ani_class cls;
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_double_test/DoubleStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(nullptr, "double_value", age), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameDoubleTest, set_static_field_by_name_double_invalid_args_field)
{
    ani_class cls;
    const ani_double age = 2.0;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_double_test/DoubleStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Double(cls, nullptr, age), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)