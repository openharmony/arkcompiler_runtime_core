/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

class ObjectGetFieldBooleanTest : public VerifyAniTest {
public:
    static void GetClassAndObject(ani_env *env, ani_class *cls, const char *className, ani_object *object)
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env->FindClass(className, cls), ANI_OK);
        ASSERT_NE(*cls, nullptr);

        ani_method ctor {};
        ASSERT_EQ(env->Class_FindMethod(*cls, "<ctor>", ":", &ctor), ANI_OK);

        ASSERT_EQ(env->Object_New(*cls, ctor, object), ANI_OK);
        ASSERT_NE(*object, nullptr);
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    }

    void SetUp() override
    {
        VerifyAniTest::SetUp();
    }

protected:
    ani_class class_ {};    // NOLINT(misc-non-private-member-variables-in-classes)
    ani_object object_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

class A {
public:
    static ani_field g_field;

    static void NativeFoo(ani_env *env, ani_object)
    {
        ani_class cls {};
        ASSERT_EQ(env->FindClass("verify_object_get_field_boolean_test.Parent", &cls), ANI_OK);

        ASSERT_EQ(env->Class_FindField(cls, "booleanField", &g_field), ANI_OK);
    }

    static void NativeBaz(ani_env *env, ani_object)
    {
        ani_class cls {};
        ani_object object {};
        ObjectGetFieldBooleanTest::GetClassAndObject(env, &cls, "verify_object_get_field_boolean_test.Parent", &object);

        ani_boolean result;
        ASSERT_EQ(env->c_api->Object_GetField_Boolean(env, object, g_field, &result), ANI_OK);
    }

    static void NativeBar(ani_env *env, ani_object, ani_ref o, ani_long m)
    {
        ani_object object = static_cast<ani_object>(o);
        ani_field field = reinterpret_cast<ani_field>(m);

        ani_boolean result;
        ASSERT_EQ(env->c_api->Object_GetField_Boolean(env, object, field, &result), ANI_OK);
    }
};

ani_field A::g_field = nullptr;

TEST_F(ObjectGetFieldBooleanTest, wrong_env)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(nullptr, object_, field, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object"},
        {"field", "ani_field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_object_0)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, nullptr, field, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"field", "ani_field", "wrong object for field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_object_1)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&object_)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference type: null"},
        {"field", "ani_field", "wrong object for field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_field_0)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(class_, "staticBooleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, reinterpret_cast<ani_field>(field), &result),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"field", "ani_field", "wrong type: ani_static_field, expected: ani_field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_field_1)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    const auto fakeField = reinterpret_cast<ani_field>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, fakeField, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"field", "ani_field", "wrong field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_field_2)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, nullptr, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"field", "ani_field", "wrong field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_result)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"field", "ani_field"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object", "wrong reference"},
        {"field", "ani_field", "wrong field"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_type)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "intField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"field", "ani_field", "wrong return type"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, wrong_object_type)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Animal", &object_);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"field", "ani_field", "wrong object for field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, object_from_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Child", &object_);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"field", "ani_field", "wrong object for field"},
        {"result", "ani_boolean *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetField_Boolean", testLines);
}

TEST_F(ObjectGetFieldBooleanTest, cross_thread_field_get_from_native_method)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    std::array methods = {ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
                          ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    std::thread([this]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ani_object object {};
        GetClassAndObject(env, &cls, "verify_object_get_field_boolean_test.Parent", &object);

        // In the native "foo" method, the "booleanField" field is found
        ASSERT_EQ(env->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    // In the native "baz" method, the "booleanField" field is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "baz", ":"), ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, object_from_escape_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateEscapeLocalScope(nr3), ANI_OK);
    ani_object object {};
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Child", &object);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(object, reinterpret_cast<ani_ref *>(&object_)), ANI_OK);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, get_field_on_global_ref)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_object gobject {};
    ASSERT_EQ(env_->GlobalReference_Create(object_, reinterpret_cast<ani_ref *>(&gobject)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, gobject, field, &result), ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, cross_thread_field_get)
{
    ani_field field;

    std::thread([this, &field]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ani_object object {};
        GetClassAndObject(env, &cls, "verify_object_get_field_boolean_test.Parent", &object);
        ASSERT_EQ(env->Class_FindField(cls, "booleanField", &field), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_OK);
}

// Issue 28700
TEST_F(ObjectGetFieldBooleanTest, DISABLED_field_get_from_native_method)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);

    std::array methods = {ani_native_function {"bar", "C{std.core.Object}l:", reinterpret_cast<void *>(A::NativeBar)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    ani_field field;
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    // In the native "bar" method, the "booleanField" field is called
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, object_, "bar", "C{std.core.Object}l:", object_, field),
              ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, field_get_from_native_method_1)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);

    std::array methods = {ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
                          ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    // In the native "foo" method, the "booleanField" field is found
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "foo", ":"), ANI_OK);
    // In the native "baz" method, the "booleanField" field is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "baz", ":"), ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, field_get_in_local_scope)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, get_parent_field)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Child", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_OK);
}

TEST_F(ObjectGetFieldBooleanTest, success)
{
    GetClassAndObject(env_, &class_, "verify_object_get_field_boolean_test.Parent", &object_);
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "booleanField", &field), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(env_, object_, field, &result), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
