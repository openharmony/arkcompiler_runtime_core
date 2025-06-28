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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ModuleFindNamespaceTest : public AniTest {
    static ani_int Sum([[maybe_unused]] ani_env *env, ani_int a, ani_int b)
    {
        return a + b;
    }

public:
    void NamespaceFind(ani_namespace ns, ani_env *env)
    {
        ani_variable variable {};
        ASSERT_EQ(env->Namespace_FindVariable(ns, "a", &variable), ANI_OK);
        ASSERT_NE(variable, nullptr);
        ani_int x;
        ASSERT_EQ(env->Variable_GetValue_Int(variable, &x), ANI_OK);
        ASSERT_EQ(x, 1);

        ani_enum aniEnum {};
        ASSERT_EQ(env->Namespace_FindEnum(ns, "LColor;", &aniEnum), ANI_OK);
        ani_class kclass {};
        ASSERT_EQ(env->Namespace_FindClass(ns, "LA;", &kclass), ANI_OK);
        ASSERT_NE(variable, nullptr);

        ani_namespace ns1 {};
        ASSERT_EQ(env->Namespace_FindNamespace(ns, "Lttt;", &ns1), ANI_OK);
        ASSERT_NE(ns1, nullptr);

        ani_function fn {};
        ASSERT_EQ(env->Namespace_FindFunction(ns, "getInitialIntValue", ":I", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
        ani_int value;
        ASSERT_EQ(env->c_api->Function_Call_Int(env, fn, &value), ANI_OK);
        ASSERT_EQ(value, 1);

        std::array functions = {
            ani_native_function {"sum", "II:I", reinterpret_cast<void *>(Sum)},
        };
        ASSERT_EQ(env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_OK);
    }
};

TEST_F(ModuleFindNamespaceTest, find_namespace)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Louter/inner;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lopst;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
}

TEST_F(ModuleFindNamespaceTest, invalid_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    ani_namespace ns1 {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ns1 = ns;
    ASSERT_EQ(env_->Module_FindNamespace(module, "", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindNamespace(module, nullptr, &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(ns, ns1);
    ASSERT_EQ(env_->Module_FindNamespace(module, "ops;", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(ns, ns1);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lmodule_find_class_test/ops", &ns), ANI_NOT_FOUND);
    ASSERT_EQ(ns, ns1);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lmodule_find_class_test/inner;", &ns), ANI_NOT_FOUND);
    ASSERT_EQ(ns, ns1);
}

TEST_F(ModuleFindNamespaceTest, invalid_module)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespaceaaa_test;", &module), ANI_NOT_FOUND);
    ASSERT_EQ(module, nullptr);
    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(ns, nullptr);
}

TEST_F(ModuleFindNamespaceTest, invalid_env)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    ASSERT_EQ(env_->c_api->Module_FindNamespace(nullptr, module, "Lops;", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(ns, nullptr);
}

TEST_F(ModuleFindNamespaceTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    char end = 'j';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        std::string str = "L";
        str += static_cast<char>(random() % (end - 'a') + 'a');
        str += ";";
        ASSERT_EQ(env_->Module_FindNamespace(module, str.c_str(), &ns), ANI_OK);
    }
}

TEST_F(ModuleFindNamespaceTest, find_namespace_and_get_int)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    NamespaceFind(ns, env_);
}

TEST_F(ModuleFindNamespaceTest, find_namespace_inner_and_get_int)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Louter/inner;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    NamespaceFind(ns, env_);
}

TEST_F(ModuleFindNamespaceTest, invalid_arg_result)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lmodule_find_class_test/atest;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindNamespaceTest, check_initialization)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_namespace_test", &module), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_namespace_test", false));
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_namespace_test.outer.inner"));
    ani_namespace nm {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "outer.inner", &nm), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_namespace_test.outer.inner"));
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_namespace_test", false));
}

}  // namespace ark::ets::ani::testing
