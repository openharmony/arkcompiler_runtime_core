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

class ClassFindIndexableGetterTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_indexable_getter_test.TestClass";
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *signature = "d:C{verify_class_find_indexable_getter_test.TestClass}";
    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindIndexableGetterTest, success_returns_verified_handle)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableGetter(class_, signature, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindIndexableGetterTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableGetter(class_, "i:i", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> missingLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *", "wrong indexable getter"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", missingLines);

    ASSERT_EQ(env_->Class_FindIndexableGetter(class_, "X", &method), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *", "wrong method signature"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", invalidDescriptorLines);
}

TEST_F(ClassFindIndexableGetterTest, wrong_env)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableGetter(nullptr, class_, signature, &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", testLines);
}

TEST_F(ClassFindIndexableGetterTest, wrong_class)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableGetter(env_, nullptr, signature, &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr"},
        {"signature", "const char *", "wrong class for indexable getter"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", testLines);
}

TEST_F(ClassFindIndexableGetterTest, null_signature_is_allowed)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableGetter(class_, nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindIndexableGetterTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindIndexableGetter(env_, class_, signature, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"signature", "const char *"},
        {"result", "ani_method *", "nullptr for storing 'ani_method'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", testLines);
}

TEST_F(ClassFindIndexableGetterTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableGetter(env_, localClass, signature, &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference not found (may be deleted, out of scope, or corrupted)"},
        {"signature", "const char *", "wrong class for indexable getter"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", testLines);
}

TEST_F(ClassFindIndexableGetterTest, pending_error_is_rejected)
{
    ThrowError();

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIndexableGetter(env_, class_, signature, &method), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"class", "ani_class"},
        {"signature", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIndexableGetter", testLines);
}

}  // namespace ark::ets::ani::verify::testing
