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

class ClassFindFieldTest : public AniTest {};

TEST_F(ClassFindFieldTest, get_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPoint;", &cls), ANI_OK);

    ani_field fieldX;
    ASSERT_EQ(env_->Class_FindField(cls, "x", &fieldX), ANI_OK);
    ASSERT_NE(fieldX, nullptr);

    ani_field fieldY;
    ASSERT_EQ(env_->Class_FindField(cls, "y", &fieldY), ANI_OK);
    ASSERT_NE(fieldY, nullptr);
}

TEST_F(ClassFindFieldTest, invalid_argument1)
{
    ani_field field;
    ASSERT_EQ(env_->Class_FindField(nullptr, "x", &field), ANI_INVALID_ARGS);
}

TEST_F(ClassFindFieldTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPoint;", &cls), ANI_OK);

    ani_field field;
    ASSERT_EQ(env_->Class_FindField(cls, nullptr, &field), ANI_INVALID_ARGS);
}

TEST_F(ClassFindFieldTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPoint;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_FindField(cls, "x", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
