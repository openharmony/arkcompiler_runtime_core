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

class ClassFindStaticMethodTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_static_method_test.TestClass";
    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindStaticMethodTest, success_returns_verified_handle)
{
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "staticMethod", ":i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(class_, method, &result), ANI_OK);
    constexpr ani_int EXPECTED_RESULT = 3;
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(ClassFindStaticMethodTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "missingStaticMethod", ":i", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *", "static method not found [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", missingLines);

    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "staticOverloaded", nullptr, &method), ANI_AMBIGUOUS);
    std::vector<TestLineInfo> ambiguousLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *", "method lookup is ambiguous [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", ambiguousLines);

    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "staticMethod", "X", &method), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *", "method signature descriptor is invalid [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", invalidDescriptorLines);

    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "method", ":i", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> instanceMethodLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *", "static method not found [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", instanceMethodLines);
}

TEST_F(ClassFindStaticMethodTest, old_static_getter_name_is_rejected)
{
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(class_, "<get>staticProp", ":i", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *", "static method not found [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

TEST_F(ClassFindStaticMethodTest, wrong_env)
{
    ani_static_method method {};
    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(nullptr, class_, "staticMethod", ":i", &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

TEST_F(ClassFindStaticMethodTest, wrong_class)
{
    ani_static_method method {};
    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(env_, nullptr, "staticMethod", ":i", &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr [ERROR]"},
        {"name", "const char *"},
        {"signature", "const char *", "wrong class for static method [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

TEST_F(ClassFindStaticMethodTest, wrong_name)
{
    ani_static_method method {};
    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(env_, class_, nullptr, ":i", &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "argument is nullptr, expected const char * [ERROR]"},
        {"signature", "const char *"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

TEST_F(ClassFindStaticMethodTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(env_, class_, "staticMethod", ":i", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_static_method *", "nullptr for storing 'ani_static_method' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

TEST_F(ClassFindStaticMethodTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_static_method method {};
    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(env_, localClass, "staticMethod", ":i", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"name", "const char *"},
        {"signature", "const char *", "wrong class for static method [ERROR]"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

TEST_F(ClassFindStaticMethodTest, pending_error_is_rejected)
{
    ThrowError();

    ani_static_method method {};
    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(env_, class_, "staticMethod", ":i", &method), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"signature", "const char *"},
        {"result", "ani_static_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticMethod", testLines);
}

}  // namespace ark::ets::ani::verify::testing
