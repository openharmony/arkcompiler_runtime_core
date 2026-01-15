/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class GetNullTest : public VerifyAniTest {};

TEST_F(GetNullTest, wrong_env)
{
    ani_ref ref {};
    ASSERT_EQ(env_->c_api->GetNull(nullptr, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetNull", testLines);
}

TEST_F(GetNullTest, wrong_ref)
{
    ASSERT_EQ(env_->c_api->GetNull(env_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetNull", testLines);
}

TEST_F(GetNullTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->GetNull(nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetNull", testLines);
}

TEST_F(GetNullTest, success)
{
    ani_ref ref {};
    ASSERT_EQ(env_->c_api->GetNull(env_, &ref), ANI_OK);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(ref, &result), ANI_OK);
    ASSERT_TRUE(result == ANI_TRUE);
}

TEST_F(GetNullTest, throw_error)
{
    ThrowError();

    ani_ref ref {};
    ASSERT_EQ(env_->c_api->GetNull(env_, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetNull", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
