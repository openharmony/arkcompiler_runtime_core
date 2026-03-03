/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(readability-magic-numbers)
namespace ark::ets::ani::testing {

class GetVMTest : public AniTest {};

TEST_F(GetVMTest, valid_argument)
{
    ani_vm *vm = nullptr;
    ASSERT_EQ(env_->GetVM(&vm), ANI_OK);
    ASSERT_NE(vm, nullptr);
}

TEST_F(GetVMTest, invalid_argument)
{
    ASSERT_EQ(env_->GetVM(nullptr), ANI_INVALID_ARGS);
    ani_vm *vm = nullptr;
    ASSERT_EQ(env_->c_api->GetVM(nullptr, &vm), ANI_INVALID_ARGS);
}

TEST_F(GetVMTest, get_vm_under_pending_error)
{
    std::string longString(10000U, 'a');
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(longString.c_str(), longString.size(), &strRef), ANI_OK);
    ani_ref anyStringRef {};
    ASSERT_EQ(env_->Any_New(strRef, 0U, nullptr, &anyStringRef), ANI_PENDING_ERROR);

    ani_vm *vm = nullptr;
    ASSERT_EQ(env_->GetVM(&vm), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(readability-magic-numbers)
