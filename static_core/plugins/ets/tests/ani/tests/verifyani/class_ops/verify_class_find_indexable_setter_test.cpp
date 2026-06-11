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

class ClassFindIndexableSetterTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_indexable_setter_test.TestClass";
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *signature = "dC{std.core.String}:";
    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindIndexableSetterTest, success_returns_verified_handle)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableSetter(class_, signature, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindIndexableSetterTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableSetter(class_, "ii:", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *", "indexable setter not found [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", missingLines);

    ASSERT_EQ(env_->Class_FindIndexableSetter(class_, "X", &method), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *", "method signature descriptor is invalid [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", invalidDescriptorLines);
}

TEST_F(ClassFindIndexableSetterTest, wrong_env)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableSetter(nullptr, class_, signature, &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope [ERROR]"},
        {"class", "ani_class"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", testLines);
}

TEST_F(ClassFindIndexableSetterTest, wrong_class)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableSetter(env_, nullptr, signature, &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr [ERROR]"},
        {"signature", "const char *", "wrong class for indexable setter [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", testLines);
}

TEST_F(ClassFindIndexableSetterTest, null_signature_reports_ambiguous_method)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableSetter(class_, nullptr, &method), ANI_AMBIGUOUS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *", "method lookup is ambiguous [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", testLines);
}

TEST_F(ClassFindIndexableSetterTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindIndexableSetter(env_, class_, signature, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *"},
        {"result", "ani_method *", "nullptr for storing 'ani_method' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", testLines);
}

TEST_F(ClassFindIndexableSetterTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableSetter(env_, localClass, signature, &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"signature", "const char *", "wrong class for indexable setter [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", testLines);
}

TEST_F(ClassFindIndexableSetterTest, pending_error_is_rejected)
{
    ThrowError();

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableSetter(env_, class_, signature, &method), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error [ERROR]"},
        {"class", "ani_class"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableSetter", testLines);
}

}  // namespace ark::ets::ani::verify::testing
