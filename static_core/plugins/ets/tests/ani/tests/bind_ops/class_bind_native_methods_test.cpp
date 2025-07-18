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
#include <array>
#include <string_view>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassBindNativeMethodsTest : public AniTest {};

constexpr std::string_view MODULE_NAME = "@defModule.class_bind_native_methods_test";

// NOLINTNEXTLINE(readability-named-parameter)
static ani_int NativeMethodsFooNative(ani_env *, ani_class)
{
    const ani_int answer = 42U;
    return answer;
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_int NativeMethodsFooNativeOverride(ani_env *, ani_class)
{
    const ani_int answer = 43U;
    return answer;
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_long NativeMethodsLongFooNative(ani_env *, ani_class)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    return static_cast<ani_long>(84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_002)
{
    ani_namespace ns {};
    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindNamespace(module, "test002A", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "test002B", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(result, "TestA002", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_003)
{
    ani_class cls;
    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "TestB003", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_004)
{
    ani_class cls {};
    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "TestA004", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_005)
{
    ani_class cls {};

    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "TestA005", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindNativeMethodsTest, BindNativesInheritanceBTest)
{
    ani_class cls {};

    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array method = {
        ani_native_function {"method_A", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method1", ":Iface", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method2", "C{@defModule.class_bind_native_methods_test.Impl}:",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };

    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    std::array methods = {
        ani_native_function {"method1", ":C{@defModule.class_bind_native_methods_test.Impl}",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"method2", "C{@defModule.class_bind_native_methods_test.Iface}:",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
}

TEST_F(ClassBindNativeMethodsTest, BindNativesInheritanceCTest)
{
    ani_class cls {};

    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array method = {
        ani_native_function {"method_A", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method_B", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method1", ":C{@defModule.class_bind_native_methods_test.Iface}",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method2", "C{@defModule.class_bind_native_methods_test.Iface}:",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_007)
{
    ani_class cls {};

    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "TestA006", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", "ii:i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"foo", "iii:i", reinterpret_cast<void *>(NativeMethodsFooNativeOverride)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method fooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", "ii:i", &fooMethod), ANI_OK);
    ASSERT_NE(fooMethod, nullptr);

    ani_method fooMethodOverride {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", "iii:i", &fooMethodOverride), ANI_OK);
    ASSERT_NE(fooMethodOverride, nullptr);

    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, fooMethod, &result, 0, 1), ANI_OK);
    ASSERT_EQ(result, 42U);

    ASSERT_EQ(env_->Object_CallMethod_Int(object, fooMethodOverride, &result, 0, 1, 2U), ANI_OK);
    ASSERT_EQ(result, 43U);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_008)
{
    ani_class cls {};

    ani_module module;
    ASSERT_EQ(env_->FindModule(MODULE_NAME.data(), &module), ANI_OK);
    ASSERT_EQ(env_->Module_FindClass(module, "TestA006", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo1", "ii:i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"foo2", "iii:i", reinterpret_cast<void *>(NativeMethodsFooNativeOverride)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method fooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo1", "ii:i", &fooMethod), ANI_OK);
    ASSERT_NE(fooMethod, nullptr);

    ani_method fooMethodOverride {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo2", "iii:i", &fooMethodOverride), ANI_OK);
    ASSERT_NE(fooMethodOverride, nullptr);

    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, fooMethod, &result, 0, 1), ANI_OK);
    ASSERT_EQ(result, 42U);

    ASSERT_EQ(env_->Object_CallMethod_Int(object, fooMethodOverride, &result, 0, 1, 2U), ANI_OK);
    ASSERT_EQ(result, 43U);
}

static void Ctor([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object) {}

TEST_F(ClassBindNativeMethodsTest, bind_constructor)
{
    std::string_view className = "@defModule.class_bind_native_methods_test.D";

    ani_class cls {};

    ASSERT_EQ(env_->FindClass(className.data(), &cls), ANI_OK);

    std::array methods = {
        ani_native_function {"<ctor>", nullptr, reinterpret_cast<void *>(Ctor)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
