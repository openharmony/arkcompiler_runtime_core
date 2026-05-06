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

class GetVMTest : public VerifyAniTest {};

TEST_F(GetVMTest, wrong_env)
{
    ani_vm *resultVm {};
    ASSERT_EQ(env_->c_api->GetVM(nullptr, &resultVm), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result", "ani_vm **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetVM", testLines);
}

TEST_F(GetVMTest, wrong_result_ptr)
{
    ASSERT_EQ(env_->c_api->GetVM(env_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"result", "ani_vm **", "wrong pointer for storing 'ani_vm *'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetVM", testLines);
}

TEST_F(GetVMTest, success)
{
    ani_vm *resultVm {};
    ASSERT_EQ(env_->c_api->GetVM(env_, &resultVm), ANI_OK);
    ASSERT_NE(resultVm, nullptr);
}

TEST_F(GetVMTest, success_with_pending_error)
{
    ani_vm *resultVm {};
    ThrowError();
    ASSERT_EQ(env_->c_api->GetVM(env_, &resultVm), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing