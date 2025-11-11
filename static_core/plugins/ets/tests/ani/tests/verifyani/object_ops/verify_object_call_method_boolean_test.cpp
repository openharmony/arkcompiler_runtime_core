/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class ObjectCallMethodBooleanTest : public VerifyAniTest {
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

        std::string_view str("ani");
        ASSERT_EQ(env_->String_NewUTF8(str.data(), str.size(), &r_), ANI_OK);
    }

protected:
    ani_class class_ {};              // NOLINT(misc-non-private-member-variables-in-classes)
    ani_object object_ {};            // NOLINT(misc-non-private-member-variables-in-classes)
    ani_method method_ {};            // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_boolean z_ = ANI_TRUE;  // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_char c_ = 'a';          // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_byte b_ = 0x24;         // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_short s_ = 0x2012;      // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_int i_ = 0xb0ba;        // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_long l_ = 0xb1bab0ba;   // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_float f_ = 0.2609;      // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_double d_ = 21.03;      // NOLINT(misc-non-private-member-variables-in-classes)
    ani_string r_ {};                 // NOLINT(misc-non-private-member-variables-in-classes)
};

class ObjectCallMethodBooleanATest : public ObjectCallMethodBooleanTest {
public:
    // CC-OFFNXT(G.CLS.13) false positive
    void SetUp() override
    {
        ObjectCallMethodBooleanTest::SetUp();

        const int nr0 = 0;
        const int nr1 = 1;
        const int nr2 = 2;
        const int nr3 = 3;
        const int nr4 = 4;
        const int nr5 = 5;
        const int nr6 = 6;
        const int nr7 = 7;
        const int nr8 = 8;

        args_[nr0].z = z_;
        args_[nr1].c = c_;
        args_[nr2].b = b_;
        args_[nr3].s = s_;
        args_[nr4].i = i_;
        args_[nr5].l = l_;
        args_[nr6].f = f_;
        args_[nr7].d = d_;
        args_[nr8].r = r_;
    }

protected:
    static constexpr size_t NR_ARGS = 9;
    std::array<ani_value, NR_ARGS> args_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

class A {
public:
    static ani_method g_method;

    static void NativeFoo(ani_env *env, ani_object)
    {
        ani_class cls {};
        ASSERT_EQ(env->FindClass("verify_object_call_method_boolean_test.Parent", &cls), ANI_OK);

        ASSERT_EQ(env->Class_FindMethod(cls, "voidParamMethod", ":z", &g_method), ANI_OK);
    }

    template <typename TestType>
    static void NativeBaz(ani_env *env, ani_object)
    {
        ani_class cls {};
        ani_object obj {};
        ObjectCallMethodBooleanTest::GetClassAndObject(env, &cls, "verify_object_call_method_boolean_test.Parent",
                                                       &obj);

        ani_boolean result;
        if constexpr (std::is_same_v<TestType, ObjectCallMethodBooleanATest>) {
            ani_value args[2U];
            ASSERT_EQ(env->c_api->Object_CallMethod_Boolean_A(env, obj, g_method, &result, args), ANI_OK);
        } else {
            ASSERT_EQ(env->c_api->Object_CallMethod_Boolean(env, obj, g_method, &result), ANI_OK);
        }
    }

    template <typename TestType>
    static void NativeBar(ani_env *env, ani_object, ani_ref o, ani_long m)
    {
        ani_object object = static_cast<ani_object>(o);
        ani_method method = reinterpret_cast<ani_method>(m);

        ani_boolean result;
        if constexpr (std::is_same_v<TestType, ObjectCallMethodBooleanATest>) {
            ani_value args[2U];
            ASSERT_EQ(env->c_api->Object_CallMethod_Boolean_A(env, object, method, &result, args), ANI_OK);
        } else {
            ASSERT_EQ(env->c_api->Object_CallMethod_Boolean(env, object, method, &result), ANI_OK);
        }
    }
};

ani_method A::g_method = nullptr;

TEST_F(ObjectCallMethodBooleanTest, wrong_env)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(nullptr, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_object_0)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, nullptr, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"method", "ani_method", "wrong object"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, cls_1)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&object_)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference type: null"},
        {"method", "ani_method", "wrong object"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_method_0)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object_, reinterpret_cast<ani_method>(method), &result, z_,
                                                     c_, b_, s_, i_, l_, f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong type: ani_static_method, expected: ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_method_1)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    const auto fakeMethod = reinterpret_cast<ani_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, fakeMethod, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_method_2)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, nullptr, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_result)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, nullptr, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_boolean_arg)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, 0xb1ba, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean", "wrong value"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_char_arg)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, 0xb1bab0ba, b_, s_, i_, l_,
                                                     f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char", "wrong value"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_byte_arg)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, 0xb1bab0ba, s_, i_, l_,
                                                     f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte", "wrong value"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_short_arg)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, 0xb1bab0ba, i_, l_,
                                                     f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short", "wrong value"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_ref_arg)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_,
                                                     nullptr),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(nullptr, nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object", "wrong reference"},
        {"method", "ani_method", "wrong method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_return_type)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "intMethod", "zcbsilfdC{std.core.String}:i", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong return type"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, wrong_object_type)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Animal", &object_);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong object for method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, object_from_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Child", &object_);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"method", "ani_method", "wrong object"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean", testLines);
}

TEST_F(ObjectCallMethodBooleanTest, cross_thread_method_call_from_native_method)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ObjectCallMethodBooleanTest>)}};
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, class_, methods.data(), methods.size()), ANI_OK);

    std::thread([&]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ani_object object {};
        GetClassAndObject(env, &cls, "verify_object_call_method_boolean_test.Parent", &object);

        // In the native "foo" method, the "voidParamMethod" method is found
        ASSERT_EQ(env->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    // In the native "baz" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "baz", ":"), ANI_OK);
}

// Issue 30353
TEST_F(ObjectCallMethodBooleanTest, DISABLED_object_from_escape_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateEscapeLocalScope(nr3), ANI_OK);
    ani_object object {};
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Child", &object);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(object, reinterpret_cast<ani_ref *>(&object_)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

// Issue 30353
TEST_F(ObjectCallMethodBooleanTest, DISABLED_call_method_on_global_ref)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_object gobject {};
    ASSERT_EQ(env_->GlobalReference_Create(object_, reinterpret_cast<ani_ref *>(&gobject)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, gobject, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ObjectCallMethodBooleanTest, cross_thread_method_call)
{
    ani_method method;

    std::thread([&]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ani_object object {};
        GetClassAndObject(env, &cls, "verify_object_call_method_boolean_test.Parent", &object);
        ASSERT_EQ(env->Class_FindMethod(cls, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

// Issue 28700
TEST_F(ObjectCallMethodBooleanTest, DISABLED_method_call_from_native_method)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    std::array methods = {ani_native_function {
        "bar", "C{std.core.Object}l:", reinterpret_cast<void *>(A::NativeBar<ObjectCallMethodBooleanTest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(class_, "voidParamMethod", ":z", &method), ANI_OK);

    // In the native "bar" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, object_, "bar", "C{std.core.Object}l:", object_, method),
              ANI_OK);
}

TEST_F(ObjectCallMethodBooleanTest, method_call_from_native_method_1)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ObjectCallMethodBooleanTest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    // In the native "foo" method, the "voidParamMethod" method is found
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "foo", ":"), ANI_OK);
    // In the native "baz" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "baz", ":"), ANI_OK);
}

TEST_F(ObjectCallMethodBooleanTest, method_call_in_local_scope)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(ObjectCallMethodBooleanTest, call_parent_method)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Child", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ObjectCallMethodBooleanTest, success)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Object_CallMethod_Boolean(env_, object_, method_, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_env)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(nullptr, object_, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_object_0)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, nullptr, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"method", "ani_method", "wrong object"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, cls_1)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&object_)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference type: null"},
        {"method", "ani_method", "wrong object"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_method_0)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, reinterpret_cast<ani_method>(method), &result,
                                                       args_.data()),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong type: ani_static_method, expected: ani_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_method_1)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    const auto fakeMethod = reinterpret_cast<ani_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, fakeMethod, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_method_2)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, nullptr, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_result)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, nullptr, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_args)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong arguments value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(nullptr, nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object", "wrong reference"},
        {"method", "ani_method", "wrong method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"args", "ani_value *", "wrong arguments value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_boolean_arg)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    const int fakeBoolean = 0xb1ba;
    args_[0].i = fakeBoolean;
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong method arguments"},
        {"[0]", "ani_boolean", "wrong value"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_return_type)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "intMethod", "zcbsilfdC{std.core.String}:i", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong return type"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, wrong_object_type)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Animal", &object_);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"method", "ani_method", "wrong object for method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, object_from_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Child", &object_);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"method", "ani_method", "wrong object"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_CallMethod_Boolean_A", testLines);
}

TEST_F(ObjectCallMethodBooleanATest, cross_thread_method_call_from_native_method)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ObjectCallMethodBooleanATest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    std::thread([&]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ani_object object {};
        GetClassAndObject(env, &cls, "verify_object_call_method_boolean_test.Parent", &object);

        // In the native "foo" method, the "voidParamMethod" method is found
        ASSERT_EQ(env->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    // In the native "baz" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "baz", ":"), ANI_OK);
}

// Issue 30353
TEST_F(ObjectCallMethodBooleanATest, DISABLED_object_from_escape_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateEscapeLocalScope(nr3), ANI_OK);
    ani_object object {};
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Child", &object);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(object, reinterpret_cast<ani_ref *>(&object_)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_OK);
}

// Issue 30353
TEST_F(ObjectCallMethodBooleanATest, DISABLED_call_method_on_global_ref)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_object gobject {};
    ASSERT_EQ(env_->GlobalReference_Create(object_, reinterpret_cast<ani_ref *>(&gobject)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, gobject, method_, &result, args_.data()), ANI_OK);
}

TEST_F(ObjectCallMethodBooleanATest, cross_thread_method_call)
{
    ani_method method;

    std::thread([&]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ani_object object {};
        GetClassAndObject(env, &cls, "verify_object_call_method_boolean_test.Parent", &object);
        ASSERT_EQ(env->Class_FindMethod(cls, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method, &result, args_.data()), ANI_OK);
}

// Issue 28700
TEST_F(ObjectCallMethodBooleanATest, DISABLED_method_call_from_native_method)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    std::array methods = {ani_native_function {
        "bar", "C{std.core.Object}l:", reinterpret_cast<void *>(A::NativeBar<ObjectCallMethodBooleanATest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(class_, "voidParamMethod", ":z", &method), ANI_OK);

    // In the native "bar" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, object_, "bar", "C{std.core.Object}l:", object_, method),
              ANI_OK);
}

TEST_F(ObjectCallMethodBooleanATest, method_call_from_native_method_1)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);

    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ObjectCallMethodBooleanATest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(class_, methods.data(), methods.size()), ANI_OK);

    // In the native "foo" method, the "voidParamMethod" method is found
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "foo", ":"), ANI_OK);
    // In the native "baz" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object_, "baz", ":"), ANI_OK);
}

TEST_F(ObjectCallMethodBooleanATest, method_call_in_local_scope)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(ObjectCallMethodBooleanATest, call_parent_method)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Child", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_OK);
}

TEST_F(ObjectCallMethodBooleanATest, success)
{
    GetClassAndObject(env_, &class_, "verify_object_call_method_boolean_test.Parent", &object_);
    ASSERT_EQ(env_->Class_FindMethod(class_, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method_), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean_A(env_, object_, method_, &result, args_.data()), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
