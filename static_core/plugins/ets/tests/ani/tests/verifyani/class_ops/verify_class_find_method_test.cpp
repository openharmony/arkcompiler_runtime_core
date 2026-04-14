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

class ClassFindMethodTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_method_test.TestClass";

    void GetTestObject(ani_object *object)
    {
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(class_, "<ctor>", ":", &ctor), ANI_OK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(class_, ctor, object), ANI_OK);
    }

    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindMethodTest, success_returns_verified_handle)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(class_, "method", ":i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    GetTestObject(&object);
    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Int(object, method, &result), ANI_OK);
    constexpr ani_int EXPECTED_RESULT = 1;
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(ClassFindMethodTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(class_, "missingMethod", ":i", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},       {"class", "ani_class"},
        {"name", "const char *"},   {"signature", "const char *", "wrong method"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", missingLines);

    ASSERT_EQ(env_->Class_FindMethod(class_, "overloaded", nullptr, &method), ANI_AMBIGUOUS);
    std::vector<TestLineInfo> ambiguousLines {
        {"env", "ani_env *"},       {"class", "ani_class"},
        {"name", "const char *"},   {"signature", "const char *", "ambiguous method"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", ambiguousLines);

    ASSERT_EQ(env_->Class_FindMethod(class_, "method", "X", &method), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},       {"class", "ani_class"},
        {"name", "const char *"},   {"signature", "const char *", "wrong method signature"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", invalidDescriptorLines);

    ASSERT_EQ(env_->Class_FindMethod(class_, "staticMethod", ":i", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> staticMethodLines {
        {"env", "ani_env *"},       {"class", "ani_class"},
        {"name", "const char *"},   {"signature", "const char *", "wrong method"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", staticMethodLines);
}

TEST_F(ClassFindMethodTest, wrong_env)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindMethod(nullptr, class_, "method", ":i", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", testLines);
}

TEST_F(ClassFindMethodTest, wrong_class)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindMethod(env_, nullptr, "method", ":i", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},       {"class", "ani_class", "wrong reference"},
        {"name", "const char *"},   {"signature", "const char *", "wrong class for method"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", testLines);
}

TEST_F(ClassFindMethodTest, wrong_name)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindMethod(env_, class_, nullptr, ":i", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", testLines);
}

TEST_F(ClassFindMethodTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindMethod(env_, class_, "method", ":i", nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_method *", "wrong pointer for storing 'ani_method'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", testLines);
}

TEST_F(ClassFindMethodTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindMethod(env_, localClass, "method", ":i", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},       {"class", "ani_class", "wrong reference"},
        {"name", "const char *"},   {"signature", "const char *", "wrong class for method"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", testLines);
}

TEST_F(ClassFindMethodTest, pending_error_is_rejected)
{
    ThrowError();

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindMethod(env_, class_, "method", ":i", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindMethod", testLines);
}

}  // namespace ark::ets::ani::verify::testing
