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

class ModuleNamespaceFindMemberTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindModule(moduleName, &module_), ANI_OK);
        ASSERT_EQ(env_->FindNamespace(namespaceName, &ns_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *moduleName = "verify_module_namespace_find_member_test";
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_module_namespace_find_member_test.NotModule";
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *namespaceName = "verify_module_namespace_find_member_test.Ops";
    static constexpr ani_int moduleFunctionResult = 11;     // NOLINT(readability-identifier-naming)
    static constexpr ani_int moduleValue = 17;              // NOLINT(readability-identifier-naming)
    static constexpr ani_int namespaceFunctionResult = 23;  // NOLINT(readability-identifier-naming)
    static constexpr ani_int namespaceValue = 29;           // NOLINT(readability-identifier-naming)
    ani_module module_ {};                                  // NOLINT(misc-non-private-member-variables-in-classes)
    ani_namespace ns_ {};                                   // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ModuleNamespaceFindMemberTest, module_find_function_success_returns_verified_handle)
{
    ani_function function {};
    ASSERT_EQ(env_->Module_FindFunction(module_, "moduleFunction", ":i", &function), ANI_OK);
    ASSERT_NE(function, nullptr);

    ani_int result {};
    ASSERT_EQ(env_->Function_Call_Int(function, &result), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(result, moduleFunctionResult);
}

TEST_F(ModuleNamespaceFindMemberTest, module_find_variable_success_returns_verified_handle)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module_, "moduleValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_int result {};
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_EQ(result, moduleValue);
}

TEST_F(ModuleNamespaceFindMemberTest, namespace_find_function_success_returns_verified_handle)
{
    ani_function function {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns_, "namespaceFunction", ":i", &function), ANI_OK);
    ASSERT_NE(function, nullptr);

    ani_int result {};
    ASSERT_EQ(env_->Function_Call_Int(function, &result), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(result, namespaceFunctionResult);
}

TEST_F(ModuleNamespaceFindMemberTest, namespace_find_variable_success_returns_verified_handle)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "namespaceValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_int result {};
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_EQ(result, namespaceValue);
}

TEST_F(ModuleNamespaceFindMemberTest, function_lookup_status_is_forwarded_without_verify_abort)
{
    ani_function function {};
    ASSERT_EQ(env_->Module_FindFunction(module_, "missingFunction", ":i", &function), ANI_NOT_FOUND);
    std::vector<TestLineInfo> moduleMissingLines {
        {"env", "ani_env *"},         {"module", "ani_module"},
        {"name", "const char *"},     {"signature", "const char *", "wrong function"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindFunction", moduleMissingLines);

    ASSERT_EQ(env_->Namespace_FindFunction(ns_, "overloaded", nullptr, &function), ANI_AMBIGUOUS);
    std::vector<TestLineInfo> namespaceAmbiguousLines {
        {"env", "ani_env *"},         {"ns", "ani_namespace"},
        {"name", "const char *"},     {"signature", "const char *", "ambiguous method"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindFunction", namespaceAmbiguousLines);

    ASSERT_EQ(env_->Module_FindFunction(module_, "moduleFunction", "X", &function), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},         {"module", "ani_module"},
        {"name", "const char *"},     {"signature", "const char *", "wrong method signature"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindFunction", invalidDescriptorLines);
}

TEST_F(ModuleNamespaceFindMemberTest, variable_lookup_status_is_forwarded_without_verify_abort)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module_, "missingValue", &variable), ANI_NOT_FOUND);
    std::vector<TestLineInfo> moduleMissingLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"name", "const char *", "wrong variable"},
        {"result", "ani_variable *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindVariable", moduleMissingLines);

    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "missingValue", &variable), ANI_NOT_FOUND);
    std::vector<TestLineInfo> namespaceMissingLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace"},
        {"name", "const char *", "wrong variable"},
        {"result", "ani_variable *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindVariable", namespaceMissingLines);
}

TEST_F(ModuleNamespaceFindMemberTest, wrong_module_handle_kind_is_rejected)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
    auto wrongModule = reinterpret_cast<ani_module>(cls);

    ani_function function {};
    ASSERT_EQ(env_->Module_FindFunction(wrongModule, "staticFunction", ":i", &function), ANI_ERROR);
    std::vector<TestLineInfo> functionLines {
        {"env", "ani_env *"},         {"module", "ani_module", "wrong reference"},
        {"name", "const char *"},     {"signature", "const char *", "wrong class for function"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindFunction", functionLines);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(wrongModule, "value", &variable), ANI_ERROR);
    std::vector<TestLineInfo> variableLines {
        {"env", "ani_env *"},
        {"module", "ani_module", "wrong reference"},
        {"name", "const char *", "wrong class for variable"},
        {"result", "ani_variable *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindVariable", variableLines);
}

TEST_F(ModuleNamespaceFindMemberTest, wrong_namespace_handle_kind_is_rejected)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
    auto wrongNamespace = reinterpret_cast<ani_namespace>(cls);

    ani_function function {};
    ASSERT_EQ(env_->Namespace_FindFunction(wrongNamespace, "staticFunction", ":i", &function), ANI_ERROR);
    std::vector<TestLineInfo> functionLines {
        {"env", "ani_env *"},         {"ns", "ani_namespace", "wrong reference"},
        {"name", "const char *"},     {"signature", "const char *", "wrong class for function"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindFunction", functionLines);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(wrongNamespace, "value", &variable), ANI_ERROR);
    std::vector<TestLineInfo> variableLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace", "wrong reference"},
        {"name", "const char *", "wrong class for variable"},
        {"result", "ani_variable *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindVariable", variableLines);
}

TEST_F(ModuleNamespaceFindMemberTest, wrong_name_is_rejected)
{
    ani_function function {};
    ASSERT_EQ(env_->c_api->Module_FindFunction(env_, module_, nullptr, ":i", &function), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> functionLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"signature", "const char *"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindFunction", functionLines);

    ani_variable variable {};
    ASSERT_EQ(env_->c_api->Namespace_FindVariable(env_, ns_, nullptr, &variable), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> variableLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"result", "ani_variable *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindVariable", variableLines);
}

TEST_F(ModuleNamespaceFindMemberTest, wrong_result_storage_is_rejected)
{
    ASSERT_EQ(env_->c_api->Module_FindFunction(env_, module_, "moduleFunction", ":i", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> functionLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_function *", "wrong pointer for storing 'ani_function'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_FindFunction", functionLines);

    ASSERT_EQ(env_->c_api->Namespace_FindVariable(env_, ns_, "namespaceValue", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> variableLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace"},
        {"name", "const char *"},
        {"result", "ani_variable *", "wrong pointer for storing 'ani_variable'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindVariable", variableLines);
}

TEST_F(ModuleNamespaceFindMemberTest, pending_error_is_rejected)
{
    ThrowError();

    ani_function function {};
    ASSERT_EQ(env_->c_api->Namespace_FindFunction(env_, ns_, "namespaceFunction", ":i", &function), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"ns", "ani_namespace"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_function *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_FindFunction", testLines);
}

}  // namespace ark::ets::ani::verify::testing
