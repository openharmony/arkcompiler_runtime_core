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
class StringGetUtf16Test : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        uint16_t input[3] = {0U};  // NOLINT(modernize-avoid-c-arrays)
        auto status = env_->String_NewUTF16(input, 3U, &string_);
        ASSERT_EQ(status, ANI_OK);
    }

protected:
    ani_string string_;        // NOLINT(misc-non-private-member-variables-in-classes)
    uint16_t res_[5];          // NOLINT(misc-non-private-member-variables-in-classes,modernize-avoid-c-arrays)
    const ani_size size_ = 5;  // NOLINT(misc-non-private-member-variables-in-classes)
    ani_size cnt_;             // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(StringGetUtf16Test, wrong_env)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16(nullptr, string_, res_, size_, &cnt_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"string", "ani_string"},
        {"utf16Buffer", "uint16_t *"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);
}

TEST_F(StringGetUtf16Test, wrong_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16(env_, nullptr, res_, size_, &cnt_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference"},
        {"utf16Buffer", "uint16_t *"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);
}

TEST_F(StringGetUtf16Test, null_input_string)
{
    ani_string str;
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&str)), ANI_OK);

    ASSERT_EQ(env_->c_api->String_GetUTF16(env_, str, res_, size_, &cnt_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: null"},
        {"utf16Buffer", "uint16_t *"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);
}

TEST_F(StringGetUtf16Test, undef_input_string)
{
    ani_string str;
    ASSERT_EQ(env_->GetUndefined(reinterpret_cast<ani_ref *>(&str)), ANI_OK);

    ASSERT_EQ(env_->c_api->String_GetUTF16(env_, str, res_, size_, &cnt_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: undefined"},
        {"utf16Buffer", "uint16_t *"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);
}

TEST_F(StringGetUtf16Test, wrong_res)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16(env_, string_, res_, size_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string"},
        {"utf16Buffer", "uint16_t *"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);
}

TEST_F(StringGetUtf16Test, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16(nullptr, nullptr, nullptr, size_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"string", "ani_string", "wrong reference"},
        {"utf16Buffer", "uint16_t *", "wrong pointer to use as argument in 'uint16_t *'"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);
}

TEST_F(StringGetUtf16Test, throw_error)
{
    ThrowError();
    ASSERT_EQ(env_->c_api->String_GetUTF16(env_, string_, res_, size_, &cnt_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"string", "ani_string"},
        {"utf16Buffer", "uint16_t *"},
        {"utf16BufferSize", "ani_size"},
        {"result", "ani_size *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(StringGetUtf16Test, success)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16(env_, string_, res_, size_, &cnt_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
