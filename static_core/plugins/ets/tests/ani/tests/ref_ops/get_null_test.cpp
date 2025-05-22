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

class GetNullTest : public AniTest {};

TEST_F(GetNullTest, get_null)
{
    ani_ref ref;
    ASSERT_EQ(env_->GetNull(&ref), ANI_OK);

    auto isNull = CallEtsFunction<ani_boolean>("get_null_test", "isNull", ref);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(GetNullTest, invalid_argument)
{
    ASSERT_EQ(env_->GetNull(nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
