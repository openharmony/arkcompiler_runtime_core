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

class ClassFindStaticFieldTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_static_field_test.TestClass";
    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindStaticFieldTest, success_returns_verified_handle)
{
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(class_, "staticField", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_int result {};
    ASSERT_EQ(env_->Class_GetStaticField_Int(class_, field, &result), ANI_OK);
    constexpr ani_int EXPECTED_RESULT = 2;
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(ClassFindStaticFieldTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(class_, "missingStaticField", &field), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "static field not found"},
        {"result", "ani_static_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

TEST_F(ClassFindStaticFieldTest, wrong_env)
{
    ani_static_field field {};
    ASSERT_EQ(env_->c_api->Class_FindStaticField(nullptr, class_, "staticField", &field), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_static_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

TEST_F(ClassFindStaticFieldTest, wrong_class)
{
    ani_static_field field {};
    ASSERT_EQ(env_->c_api->Class_FindStaticField(env_, nullptr, "staticField", &field), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr"},
        {"name", "const char *", "wrong class for static field"},
        {"result", "ani_static_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

TEST_F(ClassFindStaticFieldTest, wrong_name)
{
    ani_static_field field {};
    ASSERT_EQ(env_->c_api->Class_FindStaticField(env_, class_, nullptr, &field), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "argument is nullptr, expected const char *"},
        {"result", "ani_static_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

TEST_F(ClassFindStaticFieldTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindStaticField(env_, class_, "staticField", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_static_field *", "nullptr for storing 'ani_static_field'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

TEST_F(ClassFindStaticFieldTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_static_field field {};
    ASSERT_EQ(env_->c_api->Class_FindStaticField(env_, localClass, "staticField", &field), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference not found (may be deleted, out of scope, or corrupted)"},
        {"name", "const char *", "wrong class for static field"},
        {"result", "ani_static_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

TEST_F(ClassFindStaticFieldTest, pending_error_is_rejected)
{
    ThrowError();

    ani_static_field field {};
    ASSERT_EQ(env_->c_api->Class_FindStaticField(env_, class_, "staticField", &field), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_static_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindStaticField", testLines);
}

}  // namespace ark::ets::ani::verify::testing
