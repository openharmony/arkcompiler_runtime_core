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

#include <array>
#include <string_view>

namespace ark::ets::ani::verify::testing {

class FunctionalObjectCallTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindModule(moduleName, &module_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *moduleName = "verify_functional_object_call_test";
    static constexpr std::string_view testString = "test";  // NOLINT(readability-identifier-naming)
    static constexpr size_t oneArgument = 1U;               // NOLINT(readability-identifier-naming)

    void GetModuleFunctionResult(const char *name, ani_ref *result)
    {
        ASSERT_NE(result, nullptr);
        ani_function function {};
        ASSERT_EQ(env_->Module_FindFunction(module_, name, ":C{std.core.Object}", &function), ANI_OK);
        ASSERT_EQ(env_->Function_Call_Ref(function, result), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    }

    void CheckString(ani_ref ref)
    {
        std::string result;
        GetStdString(static_cast<ani_string>(ref), result);
        ASSERT_EQ(result, testString);
    }

    ani_module module_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(FunctionalObjectCallTest, success_returns_verified_handle)
{
    ani_ref fnRef {};
    GetModuleFunctionResult("getFnObj", &fnRef);
    auto fnObj = reinterpret_cast<ani_fn_object>(fnRef);
    ani_string arg {};
    ASSERT_EQ(env_->String_NewUTF8(testString.data(), testString.size(), &arg), ANI_OK);
    std::array<ani_ref, oneArgument> args {static_cast<ani_ref>(arg)};

    ani_ref result {};
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    CheckString(result);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(FunctionalObjectCallTest, wrong_env)
{
    ani_ref fnRef {};
    GetModuleFunctionResult("getFnObj", &fnRef);
    auto fnObj = reinterpret_cast<ani_fn_object>(fnRef);
    ani_ref result {};
    ASSERT_EQ(env_->c_api->FunctionalObject_Call(nullptr, fnObj, 0, nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"fn", "ani_fn_object"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", testLines);
}

TEST_F(FunctionalObjectCallTest, wrong_functional_object)
{
    ani_ref result {};
    ASSERT_EQ(env_->FunctionalObject_Call(nullptr, 0, nullptr, &result), ANI_INVALID_ARGS);
    // clang-format off
    std::vector<TestLineInfo> nullLines {
        {"env", "ani_env *"},
        {"fn", "ani_fn_object", "reference is nullptr [ERROR]"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", nullLines);

    ani_ref nonFnRef {};
    GetModuleFunctionResult("getNonFnObj", &nonFnRef);
    auto nonFnObj = reinterpret_cast<ani_fn_object>(nonFnRef);
    ASSERT_EQ(env_->FunctionalObject_Call(nonFnObj, 0, nullptr, &result), ANI_INVALID_TYPE);
    // clang-format off
    std::vector<TestLineInfo> wrongTypeLines {
        {"env", "ani_env *"},
        {"fn", "ani_fn_object", "wrong functional object [ERROR]"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", wrongTypeLines);
}

TEST_F(FunctionalObjectCallTest, wrong_argv)
{
    ani_ref fnRef {};
    GetModuleFunctionResult("getFnObj", &fnRef);
    auto fnObj = reinterpret_cast<ani_fn_object>(fnRef);
    ani_ref result {};
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, oneArgument, nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},    {"fn", "ani_fn_object"},
        {"argc", "ani_size"},    {"argv", "ani_ref *", "wrong pointer to use as argument in 'ani_ref *' [ERROR]"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", testLines);
}

TEST_F(FunctionalObjectCallTest, non_functional_object_with_wrong_argv)
{
    ani_ref nonFnRef {};
    GetModuleFunctionResult("getNonFnObj", &nonFnRef);
    auto nonFnObj = reinterpret_cast<ani_fn_object>(nonFnRef);
    std::array<ani_ref, oneArgument> args {nullptr};

    ani_ref result {};
    ASSERT_EQ(env_->FunctionalObject_Call(nonFnObj, args.size(), args.data(), &result), ANI_INVALID_TYPE);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"fn", "ani_fn_object", "wrong functional object [ERROR]"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", testLines);
}

TEST_F(FunctionalObjectCallTest, wrong_result_storage)
{
    ani_ref fnRef {};
    GetModuleFunctionResult("getFnObj", &fnRef);
    auto fnObj = reinterpret_cast<ani_fn_object>(fnRef);
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, 0, nullptr, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"fn", "ani_fn_object"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *", "nullptr for storing 'ani_ref' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", testLines);
}

TEST_F(FunctionalObjectCallTest, pending_error_is_rejected)
{
    ani_ref fnRef {};
    GetModuleFunctionResult("getFnObj", &fnRef);
    auto fnObj = reinterpret_cast<ani_fn_object>(fnRef);
    ThrowError();

    ani_ref result {};
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, 0, nullptr, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"fn", "ani_fn_object"},
        {"argc", "ani_size"},
        {"argv", "ani_ref *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FunctionalObject_Call", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
