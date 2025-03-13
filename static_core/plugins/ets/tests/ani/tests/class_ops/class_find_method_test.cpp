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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {
class ClassFindMethodTest : public AniTest {
protected:
    static constexpr ani_int TEST_EXPECTED_VALUE1 = 5;
    static constexpr ani_int TEST_EXPECTED_VALUE2 = 4;
    static constexpr ani_int TEST_EXPECTED_VALUE3 = 9;
    static constexpr ani_int TEST_EXPECTED_VALUE4 = 2;
    static constexpr ani_int TEST_EXPECTED_VALUE5 = 6;
    static constexpr ani_int TEST_EXPECTED_VALUE6 = 3;

    static constexpr ani_int TEST_ARG1 = 2;
    static constexpr ani_int TEST_ARG2 = 3;
    static constexpr ani_double TEST_ARG3 = 1.0;
    static constexpr ani_boolean TEST_ARG4 = ANI_TRUE;

    static constexpr ani_int TEST_NATIVE_PARAM1 = 2;
    static constexpr ani_int TEST_NATIVE_PARAM2 = 3;

    static constexpr std::string_view ARG_STRING = "abab";

private:
    void GetFunc(ani_function *fn, const char *funcName)
    {
        ani_module module;
        ASSERT_EQ(env_->FindModule("Ltest;", &module), ANI_OK);
        ASSERT_NE(module, nullptr);

        ASSERT_EQ(env_->Module_FindFunction(module, funcName, nullptr, fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
    }

    void GetObject(ani_object *objectResult, ani_class cls)
    {
        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":V", &ctor), ANI_OK);
        ASSERT_EQ(env_->Object_New(cls, ctor, objectResult), ANI_OK);
    }

public:
    template <bool HAS_METHOD>
    void CheckClassFindMethod(const std::string &clsName, const char *methodName, const char *methodSignature,
                              const ani_value *args = nullptr, ani_int expectedResult = TEST_EXPECTED_VALUE1)
    {
        ani_class cls;
        std::string clsDescriptor = "Ltest/" + clsName + ";";
        ASSERT_EQ(env_->FindClass(clsDescriptor.c_str(), &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method method;
        auto status = env_->Class_FindMethod(cls, methodName, methodSignature, &method);

        if constexpr (!HAS_METHOD) {
            ASSERT_EQ(status, ANI_NOT_FOUND);
            return;
        }

        ASSERT_EQ(status, ANI_OK);
        ASSERT_NE(method, nullptr);

        ani_object obj;
        GetObject(&obj, cls);

        ani_int result;
        ASSERT_EQ(env_->Object_CallMethod_Int_A(obj, method, &result, args), ANI_OK);
        ASSERT_EQ(result, expectedResult);
    }

    void TupleCreator(ani_ref *result)
    {
        ani_function fn {};
        ani_value args[2U];
        args[0U].i = TEST_ARG1;
        args[1U].l = TEST_ARG2;
        GetFunc(&fn, "CreateTuple");
        env_->Function_Call_Ref_A(fn, result, args);
    }

    void PartialCreator(ani_ref *result, ani_int arg = TEST_ARG1)
    {
        ani_function fn {};
        GetFunc(&fn, "CreatePartial");
        env_->Function_Call_Ref(fn, result, arg);
    }

    void RequiredCreator(ani_ref *result)
    {
        ani_function fn {};
        GetFunc(&fn, "CreateRequired");
        env_->Function_Call_Ref(fn, result, TEST_ARG1);
    }

    void ReadonlyCreator(ani_ref *result)
    {
        ani_function fn {};
        GetFunc(&fn, "CreateReadonly");
        env_->Function_Call_Ref(fn, result, TEST_ARG1);
    }

    void RecordCreator(ani_class *cls, ani_object *result)
    {
        ASSERT_EQ(env_->FindClass("Lescompat/Record;", cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(*cls, "<ctor>", nullptr, &ctor), ANI_OK);

        ASSERT_EQ(env_->Object_New(*cls, ctor, result), ANI_OK);
    }

    void BigintCreator(ani_class *cls, ani_object *result)
    {
        ASSERT_EQ(env_->FindClass("Lescompat/BigInt;", cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(*cls, "<ctor>", "I:V", &ctor), ANI_OK);

        ani_int arg = TEST_ARG1;

        ASSERT_EQ(env_->Object_New(*cls, ctor, result, arg), ANI_OK);
    }

    void LambdaCreator(ani_ref *result, const char *funcName)
    {
        ani_function fn {};
        GetFunc(&fn, funcName);
        env_->Function_Call_Ref(fn, result);
    }

    void Int(ani_object *obj, ani_int value)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("Lstd/core/Int;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &ctor), ANI_OK);

        ASSERT_EQ(env_->Object_New(cls, ctor, obj, value), ANI_OK);
    }
};

TEST_F(ClassFindMethodTest, has_method__A_B)
{
    ani_value args[2U];
    args[0U].i = TEST_ARG1;
    args[1U].i = TEST_ARG2;

    CheckClassFindMethod<true>("A", "int_method", "II:I", args, TEST_EXPECTED_VALUE1);
    CheckClassFindMethod<true>("A", "int_method", nullptr, args, TEST_EXPECTED_VALUE1);
    CheckClassFindMethod<true>("B", "int_method", "II:I", args, TEST_EXPECTED_VALUE1);
    CheckClassFindMethod<true>("B", "int_method", nullptr, args, TEST_EXPECTED_VALUE1);

    CheckClassFindMethod<true>("A", "int_override_method", "II:I", args, TEST_EXPECTED_VALUE2);
    CheckClassFindMethod<true>("A", "int_override_method", nullptr, args, TEST_EXPECTED_VALUE2);

    CheckClassFindMethod<true>("B", "int_override_method", "II:I", args, TEST_EXPECTED_VALUE3);
    CheckClassFindMethod<true>("B", "int_override_method", nullptr, args, TEST_EXPECTED_VALUE3);
}

TEST_F(ClassFindMethodTest, has_method_C)
{
    ani_value arg;
    arg.i = TEST_ARG1;

    CheckClassFindMethod<true>("C", "imethod", "I:I", &arg, TEST_EXPECTED_VALUE4);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);
    arg.r = string;

    CheckClassFindMethod<true>("C", "imethod", "Lstd/core/String;:I", &arg, TEST_EXPECTED_VALUE2);
}

TEST_F(ClassFindMethodTest, has_method_C_unusual_types)
{
    ani_value args;
    ani_string string {};
    {
        ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);
        args.r = string;

        CheckClassFindMethod<true>("C", "imethod_optional", nullptr, &args, TEST_EXPECTED_VALUE2);

        env_->GetUndefined(&args.r);
        CheckClassFindMethod<true>("C", "imethod_optional", nullptr, &args, TEST_EXPECTED_VALUE1);

        TupleCreator(&args.r);
        CheckClassFindMethod<true>("C", "imethod_tuple", nullptr, &args, TEST_EXPECTED_VALUE1);
    }
    {
        ani_array_int params;
        ASSERT_EQ(env_->Array_New_Int(2U, &params), ANI_OK);
        ani_int nativeParams[] = {TEST_NATIVE_PARAM1, TEST_NATIVE_PARAM2};
        const ani_size offset = 0;
        ASSERT_EQ(env_->Array_SetRegion_Int(params, offset, 2U, nativeParams), ANI_OK);

        args.r = params;
        CheckClassFindMethod<true>("C", "method_rest", nullptr, &args, TEST_EXPECTED_VALUE1);
    }
    {
        args.r = string;
        CheckClassFindMethod<true>("C", "method_union", nullptr, &args, TEST_EXPECTED_VALUE2);
    }
    {
        ani_object intValue;
        Int(&intValue, TEST_ARG1);
        args.r = intValue;
        CheckClassFindMethod<true>("C", "method_union", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, has_method_Derived)
{
    ani_value args;
    args.i = TEST_ARG1;
    CheckClassFindMethod<true>("Derived", "abstract_method", nullptr, &args, TEST_EXPECTED_VALUE4);

    CheckClassFindMethod<true>("Derived", "method", nullptr, &args, TEST_EXPECTED_VALUE2);
}

TEST_F(ClassFindMethodTest, has_method_Overload)
{
    ani_value args;
    args.i = TEST_ARG1;
    CheckClassFindMethod<true>("Overload", "method", "I:I", &args, TEST_EXPECTED_VALUE4);

    ani_value params[2U];
    params[0U].i = TEST_ARG1;
    params[1U].z = TEST_ARG4;
    CheckClassFindMethod<true>("Overload", "method", "IZ:I", params, TEST_EXPECTED_VALUE2);

    params[1U].d = TEST_ARG3;
    CheckClassFindMethod<true>("Overload", "method", "ID:I", params, TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, special_types1)
{
    ani_value args;
    {
        PartialCreator(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "partial_method", nullptr, &args, TEST_EXPECTED_VALUE1);
        PartialCreator(&args.r, TEST_ARG2);
        CheckClassFindMethod<true>("SpecialTypes", "partial_method", nullptr, &args, TEST_EXPECTED_VALUE6);
    }
    {
        RequiredCreator(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "required_method", nullptr, &args, TEST_EXPECTED_VALUE4);

        ReadonlyCreator(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "readonly_method", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
    {
        ani_class cls;
        ani_object mapValue;
        RecordCreator(&cls, &mapValue);
        ani_method setter;
        ASSERT_EQ(env_->Class_FindIndexableSetter(cls, nullptr, &setter), ANI_OK);

        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);

        ani_object intValue;
        Int(&intValue, TEST_ARG1);
        ASSERT_EQ(env_->Object_CallMethod_Void(mapValue, setter, string, intValue), ANI_OK);

        args.r = mapValue;
        CheckClassFindMethod<true>("SpecialTypes", "record_method", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, special_types2)
{
    ani_value args;

    // NOTE(ypigunova) change native args. Issue #23595
    // NOTE(daizihan) remove this test commented out, since the arg was not correct, need wait #23595
    {
        env_->GetUndefined(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "null_method", nullptr, &args, TEST_EXPECTED_VALUE1);
    }
    {
        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);

        args.r = string;
        CheckClassFindMethod<true>("SpecialTypes", "string_literal_method", nullptr, &args, TEST_EXPECTED_VALUE2);
    }
    {
        ani_class cls;
        ani_object bigIntValue;
        BigintCreator(&cls, &bigIntValue);
        args.r = bigIntValue;
        CheckClassFindMethod<true>("SpecialTypes", "bigint_method", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, lambdas)
{
    ani_value arg;

    {
        LambdaCreator(&arg.r, "LambdaOneArg");
        CheckClassFindMethod<true>("LambdaTypes", "one_arg_method", nullptr, &arg, TEST_EXPECTED_VALUE1);
    }
    {
        LambdaCreator(&arg.r, "LambdaTwoArgs");
        CheckClassFindMethod<true>("LambdaTypes", "two_arg_method", nullptr, &arg, TEST_EXPECTED_VALUE6);
    }
    {
        LambdaCreator(&arg.r, "LambdaOptArg");
        CheckClassFindMethod<true>("LambdaTypes", "opt_arg_method", nullptr, &arg, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, generics)
{
    ani_value arg;

    ani_object intValue;
    Int(&intValue, TEST_ARG1);
    arg.r = intValue;
    CheckClassFindMethod<true>("Generic0", "method", nullptr, &arg, TEST_EXPECTED_VALUE1);

    Int(&intValue, TEST_ARG1);
    arg.r = intValue;
    CheckClassFindMethod<true>("Generic1", "method", nullptr, &arg, TEST_EXPECTED_VALUE4);
}

TEST_F(ClassFindMethodTest, method_not_found)
{
    CheckClassFindMethod<false>("A", "bla_bla_bla", nullptr);
    CheckClassFindMethod<false>("A", "int_method", "bla_bla_bla");
    CheckClassFindMethod<false>("Overload", "method", "DD:I");
    CheckClassFindMethod<false>("Overload", "method", "ILstd/core/String;:I");
}

TEST_F(ClassFindMethodTest, invalid_argument_name)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass(nullptr, &cls), ANI_INVALID_ARGS);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, nullptr, nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, invalid_argument_cls)
{
    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(nullptr, "int_method", nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, invalid_argument_result)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Ltest/A;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
