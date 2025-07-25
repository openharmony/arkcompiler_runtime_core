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
#include <thread>
#include <stdexcept>

namespace ark::ets::ani::testing {

class GetEnvTest : public AniTest {};

TEST_F(GetEnvTest, testGetEnv)
{
    ani_vm *vm = nullptr;
    ASSERT_EQ(env_->GetVM(&vm), ANI_OK);
    ani_env *env = nullptr;
    ASSERT_EQ(vm->GetEnv(ANI_VERSION_1, &env), ANI_OK);
    ASSERT_NE(env, nullptr);
    uint32_t version;
    ASSERT_EQ(env->GetVersion(&version), ANI_OK);
}

TEST_F(GetEnvTest, different_versions)
{
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ani_env *env = nullptr;
        if (i == ANI_VERSION_1) {
            ASSERT_EQ(vm_->GetEnv(i, &env), ANI_OK);
            ASSERT_NE(env, nullptr);
        } else {
            ASSERT_EQ(vm_->GetEnv(i, &env), ANI_ERROR);
        }
    }
}

TEST_F(GetEnvTest, invalid_argument)
{
    ani_env *env = nullptr;
    ASSERT_EQ(vm_->c_api->GetEnv(nullptr, ANI_VERSION_1, &env), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing