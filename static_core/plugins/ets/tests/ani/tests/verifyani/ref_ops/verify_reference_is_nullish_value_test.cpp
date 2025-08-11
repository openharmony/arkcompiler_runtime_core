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

class ReferenceIsNullishValueTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);

        ASSERT_EQ(env_->FindModule("verify_reference_is_nullish_value_test", &module_), ANI_OK);
        std::array functions = {
            ani_native_function {"foo", "X{C{std.core.Null}C{std.core.String}}:l", reinterpret_cast<void *>(Foo)},
            ani_native_function {"anotherFrame", "C{std.core.String}:l", reinterpret_cast<void *>(AnotherFrame)},
            ani_native_function {"checkRefFromAnotherFrame", ":l", reinterpret_cast<void *>(CheckRefFromAnotherFrame)},
            ani_native_function {"funcAddress", "C{std.core.String}:l", reinterpret_cast<void *>(FuncAddress)},
            ani_native_function {"badStackRef", ":l", reinterpret_cast<void *>(BadStackRef)},
        };
        ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), functions.size()), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Module_FindFunction(module_, "getVar", ":C{std.core.String}", &fn), ANI_OK);

        ASSERT_EQ(env_->Function_Call_Ref(fn, &var_), ANI_OK);
    }

protected:
    ani_ref nullRef {};
    static ani_module module_;
    static ani_ref someRef_;
    ani_ref var_;

private:
    static void Foo(ani_env *env, ani_ref ref)
    {
        ani_boolean isNull {};
        ASSERT_EQ(env->c_api->Reference_IsNullishValue(env, ref, &isNull), ANI_OK);
        ASSERT_EQ(isNull, ANI_TRUE);
    }

    static ani_long AnotherFrame(ani_env *env, ani_ref ref)
    {
        someRef_ = ref;

        ani_function fn;
        auto status = env->Module_FindFunction(module_, "checkRefFromAnotherFrame", nullptr, &fn);
        if (status != ANI_OK) {
            return status;
        }
        ani_long res {};
        status = env->Function_Call_Long(fn, &res);
        if (status != ANI_OK) {
            return status;
        }
        return res;
    }

    static ani_long CheckRefFromAnotherFrame(ani_env *env)
    {
        ani_boolean isNullish = ANI_FALSE;
        ani_status status = env->c_api->Reference_IsNullishValue(env, someRef_, &isNullish);
        return status;
    }

    static ani_long FuncAddress(ani_env *env, ani_ref ref)
    {
        ani_boolean res {};
        auto status = env->c_api->Reference_IsNullishValue(env, ref - sizeof(void *), &res);
        return status;
    }

    static ani_long BadStackRef(ani_env *env)
    {
        ani_boolean res {};
        const auto fakeRef = reinterpret_cast<ani_ref>(static_cast<long>(0x0ff00));  // NOLINT(google-runtime-int)

        auto status = env->c_api->Reference_IsNullishValue(env, fakeRef, &res);
        return status;
    }
};

ani_ref ReferenceIsNullishValueTest::someRef_ {};
ani_module ReferenceIsNullishValueTest::module_ {};

TEST_F(ReferenceIsNullishValueTest, wrong_env)
{
    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(nullptr, nullRef, &res), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, wrong_ref)
{
    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, nullptr, &res), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, nullRef, nullptr), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(nullptr, nullptr, nullptr), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, success)
{
    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, nullRef, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ReferenceIsNullishValueTest, global_ref)
{
    ani_ref gref {};
    ASSERT_EQ(env_->GlobalReference_Create(nullRef, &gref), ANI_OK);
    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, gref, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->GlobalReference_Delete(gref), ANI_OK);
}

TEST_F(ReferenceIsNullishValueTest, stack_ref)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module_, "checkStackRef", ":", &fn), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(fn), ANI_OK);
}

TEST_F(ReferenceIsNullishValueTest, anotherFrame)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module_, "anotherFrame", nullptr, &fn), ANI_OK);

    ani_long res;
    ASSERT_EQ(env_->Function_Call_Long(fn, &res, var_), ANI_OK);
    ASSERT_EQ(res, ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, funcAddress)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module_, "funcAddress", nullptr, &fn), ANI_OK);

    ani_long res {};
    ASSERT_EQ(env_->Function_Call_Long(fn, &res, var_), ANI_OK);
    ASSERT_EQ(res, ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, bad_stack_ref1)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module_, "badStackRef", nullptr, &fn), ANI_OK);

    ani_long res {};
    ASSERT_EQ(env_->Function_Call_Long(fn, &res, var_), ANI_OK);
    ASSERT_EQ(res, ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, bad_stack_ref2)
{
    const auto fakeRef = reinterpret_cast<ani_ref>(static_cast<long>(0x0ff00));  // NOLINT(google-runtime-int)

    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, fakeRef, &res), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, bad_local_ref)
{
    const auto fakeRef = reinterpret_cast<ani_ref>(static_cast<long>(0x0ff01));  // NOLINT(google-runtime-int)

    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, fakeRef, &res), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);
}

TEST_F(ReferenceIsNullishValueTest, throw_error)
{
    ThrowError();

    ani_boolean res {};
    ASSERT_EQ(env_->c_api->Reference_IsNullishValue(env_, nullRef, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"ref", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsNullishValue", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
