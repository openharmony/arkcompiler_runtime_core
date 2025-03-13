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

class EnumFindTest : public AniTest {};

TEST_F(EnumFindTest, find_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("LToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("LToFindInt;", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);

    ani_enum aniEnumString {};
    ASSERT_EQ(env_->FindEnum("LToFindString;", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);
}

TEST_F(EnumFindTest, invalid_arg_enum)
{
    ani_enum en {};
    ASSERT_EQ(env_->FindEnum(nullptr, &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->FindEnum("LToFind;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(EnumFindTest, invalid_arg_name)
{
    ani_enum en {};
    ASSERT_EQ(env_->FindEnum("LNotFound;", &en), ANI_NOT_FOUND);
}

}  // namespace ark::ets::ani::testing
