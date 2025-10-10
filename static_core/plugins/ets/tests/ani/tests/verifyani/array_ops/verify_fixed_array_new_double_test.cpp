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

class FixedArrayNewDoubleTest : public VerifyAniTest {
protected:
    ani_size length_ = 3U;
};

TEST_F(FixedArrayNewDoubleTest, wrong_env)
{
    ani_fixedarray_double res {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Double(nullptr, length_, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_double *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Double", testLines);
}

TEST_F(FixedArrayNewDoubleTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->FixedArray_New_Double(env_, length_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_double *", "wrong pointer for storing 'ani_fixedarray_double'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Double", testLines);
}

TEST_F(FixedArrayNewDoubleTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->FixedArray_New_Double(nullptr, length_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_double *", "wrong pointer for storing 'ani_fixedarray_double'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Double", testLines);
}

TEST_F(FixedArrayNewDoubleTest, throw_error)
{
    ThrowError();

    ani_fixedarray_double res {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Double(env_, length_, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"length", "ani_size"},
        {"result", "ani_fixedarray_double *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Double", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(FixedArrayNewDoubleTest, success)
{
    ani_fixedarray_double res {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Double(env_, length_, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

}  // namespace ark::ets::ani::verify::testing
