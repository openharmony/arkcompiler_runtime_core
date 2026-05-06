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

class GetVersionTest : public VerifyAniTest {};

TEST_F(GetVersionTest, wrong_env)
{
    uint32_t version {};
    ASSERT_EQ(env_->c_api->GetVersion(nullptr, &version), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result", "uint32_t *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetVersion", testLines);
}

TEST_F(GetVersionTest, wrong_result_ptr)
{
    ASSERT_EQ(env_->c_api->GetVersion(env_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"result", "uint32_t *", "wrong pointer for storing 'uint32_t'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetVersion", testLines);
}

TEST_F(GetVersionTest, success)
{
    uint32_t version {};
    ASSERT_EQ(env_->c_api->GetVersion(env_, &version), ANI_OK);
    ASSERT_EQ(version, ANI_VERSION_1);
}

TEST_F(GetVersionTest, success_with_pending_error)
{
    uint32_t version {};
    ThrowError();
    ASSERT_EQ(env_->c_api->GetVersion(env_, &version), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing