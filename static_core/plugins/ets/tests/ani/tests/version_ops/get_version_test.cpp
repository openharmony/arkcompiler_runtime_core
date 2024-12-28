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

class GetVersionTest : public AniTest {};

TEST_F(GetVersionTest, valid_argument)
{
    uint32_t aniVersion;
    ASSERT_EQ(env_->GetVersion(&aniVersion), ANI_OK);
    ASSERT_EQ(aniVersion, ANI_VERSION_1);
}

TEST_F(GetVersionTest, invalid_argument)
{
    ASSERT_EQ(env_->GetVersion(nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
