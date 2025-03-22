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

class ClassSetStaticFieldByNameCharTest : public AniTest {};

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_capi)
{
    ani_class cls;
    const ani_char target = 'b';
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_char_test/CharStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Char(env_, cls, "char_value", target), ANI_OK);
    ani_char resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Char(env_, cls, "char_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, target);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char)
{
    ani_class cls;
    const ani_char target = 'b';
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_char_test/CharStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "char_value", target), ANI_OK);
    ani_char resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Char(cls, "char_value", &resultValue), ANI_OK);
    ASSERT_EQ(resultValue, target);
}

TEST_F(ClassSetStaticFieldByNameCharTest, not_found)
{
    ani_class cls;
    const ani_char target = 'b';
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_char_test/CharStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "charValue", target), ANI_NOT_FOUND);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_char_test/CharStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, "string_value", 'b'), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_invalid_args_object)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_char_test/CharStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(nullptr, "char_value", 'b'), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameCharTest, set_static_field_by_name_char_invalid_args_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_char_test/CharStatic;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Char(cls, nullptr, 'b'), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
