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

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class StringNewUtf8Test : public VerifyAniTest {
protected:
    const char *buffer_ = "abcdef";  // NOLINT(misc-non-private-member-variables-in-classes)
    ani_size size_ = 6U;             // NOLINT(misc-non-private-member-variables-in-classes)
    ani_string res_;                 // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(StringNewUtf8Test, wrong_env)
{
    ASSERT_EQ(env_->c_api->String_NewUTF8(nullptr, buffer_, size_, &res_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"utf8String", "const char *"},
        {"utf8Size", "ani_size"},
        {"result", "ani_string *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF8", testLines);
}

TEST_F(StringNewUtf8Test, wrong_input_string)
{
    ASSERT_EQ(env_->c_api->String_NewUTF8(env_, nullptr, size_, &res_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"utf8String", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"utf8Size", "ani_size"},
        {"result", "ani_string *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF8", testLines);
}

TEST_F(StringNewUtf8Test, wrong_res)
{
    ASSERT_EQ(env_->c_api->String_NewUTF8(env_, buffer_, size_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"utf8String", "const char *"},
        {"utf8Size", "ani_size"},
        {"result", "ani_string *", "wrong pointer for storing 'ani_string'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF8", testLines);
}

TEST_F(StringNewUtf8Test, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->String_NewUTF8(nullptr, nullptr, size_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"utf8String", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"utf8Size", "ani_size"},
        {"result", "ani_string *", "wrong pointer for storing 'ani_string'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF8", testLines);
}

TEST_F(StringNewUtf8Test, throw_error)
{
    ThrowError();
    ASSERT_EQ(env_->c_api->String_NewUTF8(env_, buffer_, size_, &res_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"utf8String", "const char *"},
        {"utf8Size", "ani_size"},
        {"result", "ani_string *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF8", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(StringNewUtf8Test, success)
{
    ASSERT_EQ(env_->c_api->String_NewUTF8(env_, buffer_, size_, &res_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
