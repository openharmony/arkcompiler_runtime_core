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

#include "verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class StringNewUtf16Test : public VerifyAniTest {
protected:
    ani_size size_ = 1U;
    const uint16_t input_[1U] = {0x0065};
    ani_string res_;
};

TEST_F(StringNewUtf16Test, wrong_env)
{
    ASSERT_EQ(env_->c_api->String_NewUTF16(nullptr, input_, size_, &res_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"utf16String", "const uint16_t *"},
        {"utf16Size", "ani_size"},
        {"result", "ani_string *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF16", testLines);
}

TEST_F(StringNewUtf16Test, wrong_input_string)
{
    ASSERT_EQ(env_->c_api->String_NewUTF16(env_, nullptr, size_, &res_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"utf16String", "const uint16_t *", " wrong pointer to use as argument in 'const uint16_t *'"},
        {"utf16Size", "ani_size"},
        {"result", "ani_string *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF16", testLines);
}

TEST_F(StringNewUtf16Test, wrong_res)
{
    ASSERT_EQ(env_->c_api->String_NewUTF16(env_, input_, size_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"utf16String", "const uint16_t *"},
        {"utf16Size", "ani_size"},
        {"result", "ani_string *", "wrong pointer for storing 'ani_string'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF16", testLines);
}

TEST_F(StringNewUtf16Test, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->String_NewUTF16(nullptr, nullptr, size_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"utf16String", "const uint16_t *", "wrong pointer to use as argument in 'const uint16_t *'"},
        {"utf16Size", "ani_size"},
        {"result", "ani_string *", "wrong pointer for storing 'ani_string'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF16", testLines);
}

TEST_F(StringNewUtf16Test, throw_error)
{
    ThrowError();
    ASSERT_EQ(env_->c_api->String_NewUTF16(env_, input_, size_, &res_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"utf16String", "const uint16_t *"},
        {"utf16Size", "ani_size"},
        {"result", "ani_string *"},
    };
    // clang-format on
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_ERROR_ANI_ARGS_MSG("String_NewUTF16", testLines);
}

TEST_F(StringNewUtf16Test, success)
{
    ASSERT_EQ(env_->c_api->String_NewUTF16(env_, input_, size_, &res_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
