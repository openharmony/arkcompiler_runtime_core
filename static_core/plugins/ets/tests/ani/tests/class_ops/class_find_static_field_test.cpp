/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

class ClassFindStaticFieldTest : public AniTest {};

TEST_F(ClassFindStaticFieldTest, get_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_find_static_field_test/Singleton;", &cls), ANI_OK);

    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, "instance", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
}

TEST_F(ClassFindStaticFieldTest, invalid_argument1)
{
    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(nullptr, "instance", &field), ANI_INVALID_ARGS);
}

TEST_F(ClassFindStaticFieldTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_find_static_field_test/Singleton;", &cls), ANI_OK);

    ani_static_field field;
    ASSERT_EQ(env_->Class_FindStaticField(cls, nullptr, &field), ANI_INVALID_ARGS);
}

TEST_F(ClassFindStaticFieldTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_find_static_field_test/Singleton;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_FindStaticField(cls, "instance", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassFindStaticFieldTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_find_static_field_test.Singleton", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_static_field_test.Singleton"));
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(cls, "instance", &field), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_static_field_test.Singleton"));
}

}  // namespace ark::ets::ani::testing
