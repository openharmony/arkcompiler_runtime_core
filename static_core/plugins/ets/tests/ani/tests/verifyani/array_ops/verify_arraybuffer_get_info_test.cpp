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

namespace ark::ets::ani::verify::testing {

class ArrayBufferGetInfoVerifyTest : public VerifyAniTest {
protected:
    static constexpr ani_size LENGTH = 8U;

    ani_arraybuffer CreateArrayBuffer()
    {
        void *data {};
        ani_arraybuffer arraybuffer {};
        EXPECT_EQ(env_->CreateArrayBuffer(LENGTH, &data, &arraybuffer), ANI_OK);
        EXPECT_NE(data, nullptr);
        EXPECT_NE(arraybuffer, nullptr);
        return arraybuffer;
    }
};

TEST_F(ArrayBufferGetInfoVerifyTest, arraybuffer_get_info_success)
{
    ani_arraybuffer arraybuffer = CreateArrayBuffer();

    void *data {};
    ani_size length {};
    ASSERT_EQ(env_->ArrayBuffer_GetInfo(arraybuffer, &data, &length), ANI_OK);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, LENGTH);
}

TEST_F(ArrayBufferGetInfoVerifyTest, arraybuffer_get_info_wrong_arraybuffer)
{
    ani_arraybuffer arraybuffer {};
    ASSERT_EQ(env_->GetUndefined(reinterpret_cast<ani_ref *>(&arraybuffer)), ANI_OK);

    void *data {};
    ani_size length {};
    ASSERT_EQ(env_->ArrayBuffer_GetInfo(arraybuffer, &data, &length), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"arraybuffer", "ani_arraybuffer", "wrong reference type: undefined, expected: ani_arraybuffer"},
        {"data_result", "void **"},
        {"length_result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ArrayBuffer_GetInfo", testLines);
}

TEST_F(ArrayBufferGetInfoVerifyTest, arraybuffer_get_info_wrong_data_storage)
{
    ani_arraybuffer arraybuffer = CreateArrayBuffer();
    ani_size length {};
    ASSERT_EQ(env_->ArrayBuffer_GetInfo(arraybuffer, nullptr, &length), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"arraybuffer", "ani_arraybuffer"},
        {"data_result", "void **", "nullptr for storing 'void *'"},
        {"length_result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ArrayBuffer_GetInfo", testLines);
}

TEST_F(ArrayBufferGetInfoVerifyTest, arraybuffer_get_info_wrong_length_storage)
{
    ani_arraybuffer arraybuffer = CreateArrayBuffer();
    void *data {};
    ASSERT_EQ(env_->ArrayBuffer_GetInfo(arraybuffer, &data, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"arraybuffer", "ani_arraybuffer"},
        {"data_result", "void **"},
        {"length_result", "ani_size *", "nullptr for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ArrayBuffer_GetInfo", testLines);
}

TEST_F(ArrayBufferGetInfoVerifyTest, arraybuffer_get_info_pending_error)
{
    ani_arraybuffer arraybuffer = CreateArrayBuffer();
    ThrowError();

    void *data {};
    ani_size length {};
    ASSERT_EQ(env_->ArrayBuffer_GetInfo(arraybuffer, &data, &length), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"arraybuffer", "ani_arraybuffer"},
        {"data_result", "void **"},
        {"length_result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ArrayBuffer_GetInfo", testLines);
}

}  // namespace ark::ets::ani::verify::testing
