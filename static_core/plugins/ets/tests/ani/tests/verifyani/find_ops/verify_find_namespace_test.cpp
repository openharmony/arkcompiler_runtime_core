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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class FindNamespaceTest : public VerifyAniTest {};

TEST_F(FindNamespaceTest, find_namespace_success)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("verify_find_namespace_test.Ns", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(FindNamespaceTest, find_namespace_wrong_descriptor)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace(nullptr, &result), ANI_ERROR);
    std::vector<TestLineInfo> nullDescriptorLines {
        {"env", "ani_env *"},
        {"namespace_descriptor", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"result", "ani_namespace *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindNamespace", nullDescriptorLines);

    ASSERT_EQ(env_->FindNamespace("Lverify_find_namespace_test/ns;", &result), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"namespace_descriptor", "const char *", "invalid namespace descriptor"},
        {"result", "ani_namespace *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindNamespace", invalidDescriptorLines);
}

TEST_F(FindNamespaceTest, find_namespace_wrong_result_storage)
{
    ASSERT_EQ(env_->FindNamespace("verify_find_namespace_test.Ns", nullptr), ANI_ERROR);
    std::vector<TestLineInfo> namespaceLines {
        {"env", "ani_env *"},
        {"namespace_descriptor", "const char *"},
        {"result", "ani_namespace *", "wrong pointer for storing 'ani_namespace'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindNamespace", namespaceLines);
}

}  // namespace ark::ets::ani::verify::testing
