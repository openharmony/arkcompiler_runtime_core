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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class ClassSetStaticFieldByNameRefTest : public AniTest {};

// NOTE: Enable when #22354 is resolved
TEST_F(ClassSetStaticFieldByNameRefTest, static_field_by_name_ref_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_ref_test/BoxStatic;", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->c_api->Class_SetStaticFieldByName_Ref(env_, cls, "string_value", string), ANI_OK);
    ani_ref resultValue;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Ref(env_, cls, "string_value", &resultValue), ANI_OK);

    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 7U> buffer {};
    ani_size resSize;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 6U);
    ASSERT_STREQ(buffer.data(), "abcdef");
}

TEST_F(ClassSetStaticFieldByNameRefTest, static_field_by_name_ref)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_ref_test/BoxStatic;", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", string), ANI_OK);
    ani_ref resultValue;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "string_value", &resultValue), ANI_OK);

    auto name = static_cast<ani_string>(resultValue);
    std::array<char, 7U> buffer {};
    ani_size resSize;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &resSize), ANI_OK);
    ASSERT_EQ(resSize, 6U);
    ASSERT_STREQ(buffer.data(), "abcdef");
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_ref_test/BoxStatic;", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "int_value", string), ANI_INVALID_TYPE);
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_args_object)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_ref_test/BoxStatic;", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(nullptr, "string_value", string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_args_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_ref_test/BoxStatic;", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, nullptr, string), ANI_INVALID_ARGS);
}

TEST_F(ClassSetStaticFieldByNameRefTest, set_static_field_by_name_ref_invalid_args_value)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_set_static_field_by_name_ref_test/BoxStatic;", &cls), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Class_SetStaticFieldByName_Ref(cls, "string_value", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)