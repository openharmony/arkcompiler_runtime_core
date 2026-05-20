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

class ThrowErrorTest : public VerifyAniTest {};

TEST_F(ThrowErrorTest, wrong_env)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Error", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{std.core.ErrorOptions}:", &ctor), ANI_OK);
    ani_object err {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &err, undef, undef), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)

    ASSERT_EQ(env_->c_api->ThrowError(nullptr, static_cast<ani_error>(err)), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"error", "ani_error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ThrowError", testLines);

    ASSERT_EQ(env_->Reference_Delete(err), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

TEST_F(ThrowErrorTest, wrong_error)
{
    ASSERT_EQ(env_->c_api->ThrowError(env_, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"error", "ani_error", "wrong reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ThrowError", testLines);
}

TEST_F(ThrowErrorTest, wrong_error_type)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

    ASSERT_EQ(env_->c_api->ThrowError(env_, reinterpret_cast<ani_error>(undef)), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"error", "ani_error", "wrong reference type: undefined"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ThrowError", testLines);

    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

TEST_F(ThrowErrorTest, success)
{
    ThrowError();

    ani_boolean exists = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&exists), ANI_OK);
    ASSERT_EQ(exists, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(ThrowErrorTest, throw_error)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Error", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{std.core.ErrorOptions}:", &ctor), ANI_OK);
    ani_object errObj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &errObj, undef, undef), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    auto err = static_cast<ani_error>(errObj);

    ThrowError();

    ASSERT_EQ(env_->c_api->ThrowError(env_, err), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(err), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
