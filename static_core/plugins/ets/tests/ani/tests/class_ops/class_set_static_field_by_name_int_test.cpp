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

class ClassSetStaticFieldByNameIntTest : public AniTest {};

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_int_test/Packstatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Int(env_, cls, "int_value", 2U), ANI_OK);
    ani_int resultValue = 0;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Int(env_, cls, "int_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, 2U);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_int_test/Packstatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "int_value", 2U), ANI_OK);
    ani_int resultValue = 0;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Int(cls, "int_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, 2U);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_int_test/Packstatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, "string_value", 1), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_invalid_args_object)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_int_test/Packstatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(nullptr, "int_value", 2U), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameIntTest, set_static_field_by_name_int_invalid_args_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_int_test/Packstatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Int(cls, nullptr, 2U), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)