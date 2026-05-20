/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <thread>

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

// NOLINTBEGIN(readability-identifier-naming)
namespace ark::ets::ani::verify::testing {

class DetachCurrentThreadTest : public VerifyAniTest {};

TEST_F(DetachCurrentThreadTest, wrong_vm)
{
    ASSERT_EQ(vm_->c_api->DetachCurrentThread(nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *", "wrong VM pointer"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DetachCurrentThread", testLines);
}

TEST_F(DetachCurrentThreadTest, wrong_vm_pointer)
{
    constexpr uintptr_t wrongVmAddress = 0x21243;
    auto *wrongVm = reinterpret_cast<ani_vm *>(wrongVmAddress);
    ASSERT_EQ(vm_->c_api->DetachCurrentThread(wrongVm), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *", "wrong VM pointer"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DetachCurrentThread", testLines);
}

TEST_F(DetachCurrentThreadTest, thread_is_not_attached)
{
    std::thread([this]() {
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_ERROR);
        std::string msg = GetAndClearAbortMessage();
        ASSERT_THAT(msg, ::testing::HasSubstr("Cannot detach current thread, thread is not attached"));
    }).join();
}

TEST_F(DetachCurrentThreadTest, success)
{
    std::thread([this]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);
        ASSERT_NE(env, nullptr);

        ani_class cls {};
        ASSERT_EQ(env->FindClass("std.core.String", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        ASSERT_NO_ABORT_MESSAGE();

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
        ASSERT_NO_ABORT_MESSAGE();
    }).join();
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(readability-identifier-naming)