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

#include <cstring>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
namespace ark::ets::ani::verify::testing {

class ValueArrayGetLengthTest : public VerifyAniTest {
protected:
    static constexpr ani_size LENGTH = 3U;
};

TEST_F(ValueArrayGetLengthTest, wrong_env)
{
    ani_valuearray_int array {};
    ASSERT_EQ(env_->ValueArray_New_Int(LENGTH, &array), ANI_OK);

    ani_size result {};
    ASSERT_EQ(env_->c_api->ValueArray_GetLength(nullptr, array, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"array", "ani_valuearray"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ValueArray_GetLength", testLines);
}

TEST_F(ValueArrayGetLengthTest, wrong_array)
{
    ani_size result {};
    ASSERT_EQ(env_->ValueArray_GetLength(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_valuearray", "reference is nullptr [ERROR]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ValueArray_GetLength", testLines);
}

TEST_F(ValueArrayGetLengthTest, wrong_result)
{
    ani_valuearray_int array {};
    ASSERT_EQ(env_->ValueArray_New_Int(LENGTH, &array), ANI_OK);

    ASSERT_EQ(env_->ValueArray_GetLength(array, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_valuearray"},
        {"result", "ani_size *", "nullptr for storing 'ani_size' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ValueArray_GetLength", testLines);
}

TEST_F(ValueArrayGetLengthTest, wrong_reference_type)
{
    ani_string string {};
    const char *data = "test";
    ASSERT_EQ(env_->String_NewUTF8(data, strlen(data), &string), ANI_OK);

    ani_size result {};
    ASSERT_EQ(env_->ValueArray_GetLength(reinterpret_cast<ani_valuearray>(string), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_valuearray", "wrong reference type: ani_string, expected: ani_valuearray [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ValueArray_GetLength", testLines);
}

TEST_F(ValueArrayGetLengthTest, throw_error)
{
    ani_valuearray_int array {};
    ASSERT_EQ(env_->ValueArray_New_Int(LENGTH, &array), ANI_OK);

    ThrowError();

    ani_size result {};
    ASSERT_EQ(env_->ValueArray_GetLength(array, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"array", "ani_valuearray"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ValueArray_GetLength", testLines);
}

TEST_F(ValueArrayGetLengthTest, success)
{
    ani_valuearray_int array {};
    ASSERT_EQ(env_->ValueArray_New_Int(LENGTH, &array), ANI_OK);

    ani_size result {};
    ASSERT_EQ(env_->ValueArray_GetLength(array, &result), ANI_OK);
    ASSERT_EQ(result, LENGTH);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
