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

#include "verify_ani_gtest.h"

#include <limits>

namespace ark::ets::ani::verify::testing {

class CreateArrayBufferVerifyTest : public VerifyAniTest {
protected:
    static constexpr ani_size LENGTH = 8U;
};

TEST_F(CreateArrayBufferVerifyTest, create_arraybuffer_success)
{
    void *data {};
    ani_arraybuffer arraybuffer {};
    ASSERT_EQ(env_->CreateArrayBuffer(0U, &data, &arraybuffer), ANI_OK);
    ASSERT_NE(data, nullptr);
    ASSERT_NE(arraybuffer, nullptr);
}

TEST_F(CreateArrayBufferVerifyTest, create_arraybuffer_wrong_env)
{
    void *data {};
    ani_arraybuffer arraybuffer {};
    ASSERT_EQ(env_->c_api->CreateArrayBuffer(nullptr, LENGTH, &data, &arraybuffer), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"length", "ani_size"},
        {"data_result", "void **"},
        {"arraybuffer_result", "ani_arraybuffer *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateArrayBuffer", testLines);
}

TEST_F(CreateArrayBufferVerifyTest, create_arraybuffer_wrong_data_storage)
{
    ani_arraybuffer arraybuffer {};
    ASSERT_EQ(env_->CreateArrayBuffer(LENGTH, nullptr, &arraybuffer), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size"},
        {"data_result", "void **", "nullptr for storing 'void *' [ERROR]"},
        {"arraybuffer_result", "ani_arraybuffer *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateArrayBuffer", testLines);
}

TEST_F(CreateArrayBufferVerifyTest, create_arraybuffer_wrong_arraybuffer_storage)
{
    void *data {};
    ASSERT_EQ(env_->CreateArrayBuffer(LENGTH, &data, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size"},
        {"data_result", "void **"},
        {"arraybuffer_result", "ani_arraybuffer *", "nullptr for storing 'ani_arraybuffer' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateArrayBuffer", testLines);
}

TEST_F(CreateArrayBufferVerifyTest, create_arraybuffer_too_large_length_forwards_status)
{
    void *data {};
    ani_arraybuffer arraybuffer {};
    constexpr size_t TOO_LARGE_LENGTH = static_cast<size_t>(std::numeric_limits<ani_int>::max()) + 1U;
    ASSERT_EQ(env_->CreateArrayBuffer(TOO_LARGE_LENGTH, &data, &arraybuffer), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size", "arraybuffer length is too large [ERROR]"},
        {"data_result", "void **"},
        {"arraybuffer_result", "ani_arraybuffer *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateArrayBuffer", testLines);
}

}  // namespace ark::ets::ani::verify::testing
