/*
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

class ReferenceIsUnderfinedTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        ASSERT_EQ(env_->GetUndefined(&undefRef), ANI_OK);

        ASSERT_EQ(env_->FindModule("verify_reference_is_undefined_test", &module), ANI_OK);
        std::array functions = {
            ani_native_function {"foo", "C{std.core.String}:", reinterpret_cast<void *>(Foo)},
        };
        ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);
    }

protected:
    ani_ref undefRef {};
    ani_module module {};

private:
    static void Foo(ani_env *env, ani_ref ref)
    {
        ani_boolean isUndef {};
        ASSERT_EQ(env->c_api->Reference_IsUndefined(env, ref, &isUndef), ANI_OK);
        ASSERT_EQ(isUndef, ANI_TRUE);
    }
};

TEST_F(ReferenceIsUnderfinedTest, wrong_env)
{
    ani_boolean res {};

    ASSERT_EQ(env_->c_api->Reference_IsUndefined(nullptr, undefRef, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsUndefined", testLines);
}

TEST_F(ReferenceIsUnderfinedTest, wrong_ref)
{
    ani_boolean res {};

    ASSERT_EQ(env_->c_api->Reference_IsUndefined(env_, nullptr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsUndefined", testLines);
}

TEST_F(ReferenceIsUnderfinedTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(env_, undefRef, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsUndefined", testLines);
}

TEST_F(ReferenceIsUnderfinedTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsUndefined", testLines);
}

TEST_F(ReferenceIsUnderfinedTest, throw_error)
{
    ThrowError();

    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(env_, undefRef, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"ref", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsUndefined", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(ReferenceIsUnderfinedTest, global_ref)
{
    ani_ref gref {};
    ASSERT_EQ(env_->GlobalReference_Create(undefRef, &gref), ANI_OK);
    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(env_, gref, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->GlobalReference_Delete(gref), ANI_OK);
}

// NOTE: enable when #28700 is resolved
TEST_F(ReferenceIsUnderfinedTest, DISABLED_stack_ref)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "checkStackRef", ":", &fn), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(fn), ANI_OK);
}

TEST_F(ReferenceIsUnderfinedTest, success)
{
    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(env_, undefRef, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_ref nullRef {};
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);

    ASSERT_EQ(env_->c_api->Reference_IsUndefined(env_, nullRef, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

}  // namespace ark::ets::ani::verify::testing
