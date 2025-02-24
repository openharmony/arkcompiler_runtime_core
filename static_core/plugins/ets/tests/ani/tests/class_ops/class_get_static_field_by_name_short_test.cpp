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

class ClassGetStaticFieldByNameShortTest : public AniTest {};

TEST_F(ClassGetStaticFieldByNameShortTest, get_static_field_short_capi)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetShortStatic;", &cls), ANI_OK);
    ani_short age;
    ASSERT_EQ(env_->c_api->Class_GetStaticFieldByName_Short(env_, cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_short(20U));
}

TEST_F(ClassGetStaticFieldByNameShortTest, get_static_field_short)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetShortStatic;", &cls), ANI_OK);
    ani_short age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "age", &age), ANI_OK);
    ASSERT_EQ(age, ani_short(20U));
}

TEST_F(ClassGetStaticFieldByNameShortTest, get_static_field_short_invalid_field_type)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetShortStatic;", &cls), ANI_OK);

    ani_short age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetShortStatic;", &cls), ANI_OK);
    ani_short age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(nullptr, "name", &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetShortStatic;", &cls), ANI_OK);
    ani_short age;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ClassGetStaticFieldByNameShortTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LGetShortStatic;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Short(cls, "name", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
