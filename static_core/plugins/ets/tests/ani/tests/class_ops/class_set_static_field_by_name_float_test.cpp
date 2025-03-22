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

namespace ark::ets::ani::testing {

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
class ClassSetStaticFieldByNameFloatTest : public AniTest {};

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_float_test/FloatStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Float(env_, cls, "float_value", 2.0F), ANI_OK);
    ani_float resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Float(env_, cls, "float_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, 2.0F);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_float_test/FloatStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "float_value", 2.0F), ANI_OK);
    ani_float resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Float(cls, "float_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, 2.0F);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_float_test/FloatStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, "string_value", 2.0F), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_invalid_args_object)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_float_test/FloatStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(nullptr, "float_value", 2.0F), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameFloatTest, set_static_field_by_name_long_invalid_args_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_float_test/FloatStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Float(cls, nullptr, 2.0F), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)