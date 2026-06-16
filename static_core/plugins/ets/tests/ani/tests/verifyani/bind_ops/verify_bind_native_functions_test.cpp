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
#include <optional>
#include <string_view>
#include <utility>

namespace ark::ets::ani::verify::testing {

class BindNativeFunctionsVerifyTest : public VerifyAniTest {
protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr ani_int staticFooResult = 2;

    void SetUp() override
    {
        VerifyAniTest::SetUp();
        FindModule(&module_);
        FindNamespace(&namespace_);
        FindInstanceClass(&instanceClass_);
        FindStaticClass(&staticClass_);
        FindStaticChildClass(&staticChildClass_);
    }

    static ani_int Sum([[maybe_unused]] ani_env *env, ani_int a, ani_int b)
    {
        return a + b;
    }

    static ani_int InstanceFoo([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
    {
        return 1;
    }

    static ani_int StaticFoo([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class cls)
    {
        return staticFooResult;
    }

    void FindModule(ani_module *module)
    {
        ASSERT_EQ(env_->FindModule("@defModule.verify_bind_native_functions_test", module), ANI_OK);
        ASSERT_NE(*module, nullptr);
    }

    void FindNamespace(ani_namespace *ns)
    {
        ASSERT_EQ(env_->FindNamespace("@defModule.verify_bind_native_functions_test.NativeNs", ns), ANI_OK);
        ASSERT_NE(*ns, nullptr);
    }

    void FindInstanceClass(ani_class *cls)
    {
        ASSERT_EQ(env_->FindClass("@defModule.verify_bind_native_functions_test.InstanceBindClass", cls), ANI_OK);
        ASSERT_NE(*cls, nullptr);
    }

    void FindStaticClass(ani_class *cls)
    {
        ASSERT_EQ(env_->FindClass("@defModule.verify_bind_native_functions_test.StaticBindClass", cls), ANI_OK);
        ASSERT_NE(*cls, nullptr);
    }

    void FindStaticChildClass(ani_class *cls)
    {
        ASSERT_EQ(env_->FindClass("@defModule.verify_bind_native_functions_test.StaticChildBindClass", cls), ANI_OK);
        ASSERT_NE(*cls, nullptr);
    }

    ani_module module_ {};           // NOLINT(misc-non-private-member-variables-in-classes)
    ani_namespace namespace_ {};     // NOLINT(misc-non-private-member-variables-in-classes)
    ani_class instanceClass_ {};     // NOLINT(misc-non-private-member-variables-in-classes)
    ani_class staticClass_ {};       // NOLINT(misc-non-private-member-variables-in-classes)
    ani_class staticChildClass_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_success)
{
    std::array functions = {ani_native_function {"moduleSum", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), functions.size()), ANI_OK);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_wrong_module)
{
    std::array functions = {ani_native_function {"moduleSum", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(nullptr, functions.data(), functions.size()), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"module", "ani_module", "reference is nullptr [ERROR]"},
        {"functions", "const ani_native_function *", "wrong class for native function [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_wrong_functions)
{
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, nullptr, 1U), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"functions", "const ani_native_function *",
         "argument is nullptr, expected const ani_native_function * [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_wrong_function_name)
{
    std::array functions = {ani_native_function {nullptr, "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), functions.size()), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"functions", "const ani_native_function *", "wrong native function name at index 0 [FATAL]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_wrong_function_pointer)
{
    std::array functions = {ani_native_function {"moduleSum", "ii:i", nullptr}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), functions.size()), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"functions", "const ani_native_function *", "wrong native function pointer at index 0 [FATAL]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_lookup_status_is_forwarded)
{
    std::array missing = {ani_native_function {"missing", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, missing.data(), missing.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"functions", "const ani_native_function *", "native function not found at index 0 [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", missingLines);

    std::array invalidDescriptor = {ani_native_function {"moduleSum", "X", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, invalidDescriptor.data(), invalidDescriptor.size()),
              ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"functions", "const ani_native_function *", "wrong native function signature at index 0 [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", invalidDescriptorLines);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_already_bound_status_is_forwarded)
{
    std::array functions = {ani_native_function {"moduleSum", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), functions.size()), ANI_OK);
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), functions.size()), ANI_ALREADY_BINDED);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"module", "ani_module"},
        {"functions", "const ani_native_function *", "native function is already bound at index 0 [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Module_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, module_bind_native_functions_zero_count_keeps_success)
{
    std::array functions = {ani_native_function {"moduleSum", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module_, functions.data(), 0U), ANI_OK);
}

TEST_F(BindNativeFunctionsVerifyTest, namespace_bind_native_functions_success)
{
    std::array functions = {ani_native_function {"nsSum", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(namespace_, functions.data(), functions.size()), ANI_OK);
}

TEST_F(BindNativeFunctionsVerifyTest, namespace_bind_native_functions_wrong_namespace)
{
    std::array functions = {ani_native_function {"nsSum", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(nullptr, functions.data(), functions.size()), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace", "reference is nullptr [ERROR]"},
        {"functions", "const ani_native_function *", "wrong class for native function [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, namespace_bind_native_functions_wrong_functions)
{
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(namespace_, nullptr, 1U), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace"},
        {"functions", "const ani_native_function *",
         "argument is nullptr, expected const ani_native_function * [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, namespace_bind_native_functions_lookup_status_is_forwarded)
{
    std::array missing = {ani_native_function {"missing", "ii:i", reinterpret_cast<void *>(Sum)}};
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(namespace_, missing.data(), missing.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ns", "ani_namespace"},
        {"functions", "const ani_native_function *", "native function not found at index 0 [ERROR]"},
        {"nr_functions", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Namespace_BindNativeFunctions", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_native_methods_success)
{
    std::array methods = {ani_native_function {"foo", ":i", reinterpret_cast<void *>(InstanceFoo)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(instanceClass_, methods.data(), methods.size()), ANI_OK);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_native_methods_wrong_class)
{
    std::array methods = {ani_native_function {"foo", ":i", reinterpret_cast<void *>(InstanceFoo)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(nullptr, methods.data(), methods.size()), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr [ERROR]"},
        {"methods", "const ani_native_function *", "wrong class for native method [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_native_methods_wrong_methods)
{
    ASSERT_EQ(env_->Class_BindNativeMethods(instanceClass_, nullptr, 1U), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "argument is nullptr, expected const ani_native_function * [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_native_methods_lookup_status_is_forwarded)
{
    std::array missing = {ani_native_function {"missing", ":i", reinterpret_cast<void *>(InstanceFoo)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(instanceClass_, missing.data(), missing.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "native method not found at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindNativeMethods", missingLines);

    std::array invalidDescriptor = {ani_native_function {"foo", "X", reinterpret_cast<void *>(InstanceFoo)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(instanceClass_, invalidDescriptor.data(), invalidDescriptor.size()),
              ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "wrong native method signature at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindNativeMethods", invalidDescriptorLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_native_methods_ambiguous_status_is_forwarded)
{
    std::array methods = {ani_native_function {"foo", nullptr, reinterpret_cast<void *>(InstanceFoo)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(instanceClass_, methods.data(), methods.size()), ANI_AMBIGUOUS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "ambiguous native method at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_native_methods_not_native_status_is_forwarded)
{
    std::array methods = {ani_native_function {"notNative", ":i", reinterpret_cast<void *>(InstanceFoo)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(instanceClass_, methods.data(), methods.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "native method is not native at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_success)
{
    std::array methods = {ani_native_function {"foo", ":i", reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticClass_, methods.data(), methods.size()), ANI_OK);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_wrong_class)
{
    std::array methods = {ani_native_function {"foo", ":i", reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(nullptr, methods.data(), methods.size()), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr [ERROR]"},
        {"methods", "const ani_native_function *", "wrong class for static native method [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_wrong_methods)
{
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticClass_, nullptr, 1U), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "argument is nullptr, expected const ani_native_function * [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_lookup_status_is_forwarded)
{
    std::array missing = {ani_native_function {"missing", ":i", reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticClass_, missing.data(), missing.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "static native method not found at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", missingLines);

    std::array wrongKind = {ani_native_function {"instanceFoo", ":i", reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticClass_, wrongKind.data(), wrongKind.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> wrongKindLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "static native method not found at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", wrongKindLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_wrong_bind_to_parent_static_method)
{
    std::array methods = {ani_native_function {"foo", ":i", reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticChildClass_, methods.data(), methods.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "static native method not found at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_ambiguous_status_is_forwarded)
{
    std::array methods = {ani_native_function {"foo", nullptr, reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticClass_, methods.data(), methods.size()), ANI_AMBIGUOUS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "ambiguous static native method at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", testLines);
}

TEST_F(BindNativeFunctionsVerifyTest, class_bind_static_native_methods_not_native_status_is_forwarded)
{
    std::array methods = {ani_native_function {"notNative", ":i", reinterpret_cast<void *>(StaticFoo)}};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(staticClass_, methods.data(), methods.size()), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"methods", "const ani_native_function *", "static native method is not native at index 0 [ERROR]"},
        {"nr_methods", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_BindStaticNativeMethods", testLines);
}

}  // namespace ark::ets::ani::verify::testing
