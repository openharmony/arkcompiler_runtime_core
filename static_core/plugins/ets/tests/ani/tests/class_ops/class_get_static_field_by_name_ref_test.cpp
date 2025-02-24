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

class ClassGetStaticFieldByNameRefTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameRefTest, get_static_field_ref_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LMan;", &cls), ANI_OK);

    ani_ref nameRef;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Ref(env_, cls, "name", &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Bob");
}

TEST_F(ClassGetStaticFieldByNameRefTest, get_static_field_ref)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LMan;", &cls), ANI_OK);

    ani_ref nameRef;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "name", &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Bob");
}

TEST_F(ClassGetStaticFieldByNameRefTest, get_static_field_ref_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LMan;", &cls), ANI_OK);
    ani_ref nameRef;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "age", &nameRef), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LMan;", &cls), ANI_OK);
    ani_ref nameRef;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(nullptr, "name", &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LMan;", &cls), ANI_OK);
    ani_ref nameRef;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, nullptr, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameRefTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LMan;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "name", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
