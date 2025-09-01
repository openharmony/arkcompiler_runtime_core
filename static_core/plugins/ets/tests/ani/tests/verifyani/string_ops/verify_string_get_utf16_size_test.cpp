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

class StringGetUtf16SizeTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        uint16_t str[3] = {0U};
        auto status = env_->String_NewUTF16(str, 3U, &string_);
        ASSERT_EQ(status, ANI_OK);
    }

protected:
    ani_string string_;
    ani_size size_;
};

TEST_F(StringGetUtf16SizeTest, wrong_env)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16Size(nullptr, string_, &size_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"string", "ani_string"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, wrong_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16Size(env_, nullptr, &size_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, null_input_string)
{
    ani_string str;
    ASSERT_EQ(env_->c_api->GetNull(env_, reinterpret_cast<ani_ref *>(&str)), ANI_OK);

    ASSERT_EQ(env_->c_api->String_GetUTF16Size(env_, str, &size_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: ani_ref"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, undef_input_string)
{
    ani_string str;
    ASSERT_EQ(env_->c_api->GetUndefined(env_, reinterpret_cast<ani_ref *>(&str)), ANI_OK);

    ASSERT_EQ(env_->c_api->String_GetUTF16Size(env_, str, &size_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: ani_ref"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16Size(env_, string_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16Size(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"string", "ani_string", "wrong reference"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, throw_error)
{
    ThrowError();
    ASSERT_EQ(env_->c_api->String_GetUTF16Size(env_, string_, &size_), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"string", "ani_string"},
        {"result", "ani_size *"},
    };
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16Size", testLines);
}

TEST_F(StringGetUtf16SizeTest, success)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16Size(env_, string_, &size_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
