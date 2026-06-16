/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <thread>

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class NullVenvTest : public VerifyAniTest {};

TEST_F(NullVenvTest, call_from_unattached_thread)
{
    uint32_t aniVersion = 0;

    std::thread([this, &aniVersion]() { ASSERT_EQ(env_->GetVersion(&aniVersion), ANI_ERROR); }).join();

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "current native thread is not attached [FATAL]"},
        {"result", "uint32_t *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetVersion", testLines);
}

TEST_F(NullVenvTest, success)
{
    uint32_t aniVersion = 0;
    ASSERT_EQ(env_->GetVersion(&aniVersion), ANI_OK);
    ASSERT_EQ(aniVersion, ANI_VERSION_1);
}

}  // namespace ark::ets::ani::verify::testing
