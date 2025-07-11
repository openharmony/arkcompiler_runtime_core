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

class FixedArrayNewShortTest : public VerifyAniTest {
protected:
    ani_size length_ = 3U;
};

TEST_F(FixedArrayNewShortTest, wrong_env)
{
    ani_fixedarray_short res {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Short(nullptr, length_, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_short *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Short", testLines);
}

TEST_F(FixedArrayNewShortTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->FixedArray_New_Short(env_, length_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_short *", "wrong pointer for storing 'ani_fixedarray_short'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Short", testLines);
}

TEST_F(FixedArrayNewShortTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->FixedArray_New_Short(nullptr, length_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_short *", "wrong pointer for storing 'ani_fixedarray_short'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Short", testLines);
}

TEST_F(FixedArrayNewShortTest, throw_error)
{
    ThrowError();

    ani_fixedarray_short res {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Short(env_, length_, &res), ANI_ERROR);
    ASSERT_EQ(env_->ResetError(), ANI_OK);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_short *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Short", testLines);
}

TEST_F(FixedArrayNewShortTest, success)
{
    ani_fixedarray_short res {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Short(env_, length_, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

}  // namespace ark::ets::ani::verify::testing
