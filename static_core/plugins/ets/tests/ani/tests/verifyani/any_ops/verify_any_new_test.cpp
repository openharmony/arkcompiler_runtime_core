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

class AnyNewTest : public VerifyAniTest {};

TEST_F(AnyNewTest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_ref out {};
    ASSERT_EQ(env_->c_api->Any_New(nullptr, cls, 0U, nullptr, &out), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ctor", "ani_ref", "Static types are not supported"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_New", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyNewTest, wrong_ctor_null)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_ref out {};
    ASSERT_EQ(env_->c_api->Any_New(env_, nullptr, 0U, nullptr, &out), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},    {"ctor", "ani_ref", "wrong reference"}, {"argc", "ani_size"}, {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_New", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyNewTest, wrong_result_null)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Any_New(env_, cls, 0U, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},  {"ctor", "ani_ref", "Static types are not supported"},          {"argc", "ani_size"},
        {"argv", "ani_ref *"}, {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_New", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyNewTest, argv_null_with_argc_positive)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_ref out {};
    ASSERT_EQ(env_->c_api->Any_New(env_, cls, 1U, nullptr, &out), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},    {"ctor", "ani_ref", "Static types are not supported"},
        {"argc", "ani_size"},    {"argv", "ani_ref *", "wrong pointer to use as argument in 'ani_ref *argv'"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_New", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyNewTest, throw_error)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ThrowError();

    ani_ref out {};
    ASSERT_EQ(env_->c_api->Any_New(env_, cls, 0U, nullptr, &out), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"ctor", "ani_ref", "Static types are not supported"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_New", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
