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

class ClassFindFieldTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_field_test.TestClass";

    void GetTestObject(ani_object *object)
    {
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(class_, "<ctor>", ":", &ctor), ANI_OK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(class_, ctor, object), ANI_OK);
    }

    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindFieldTest, success_returns_verified_handle)
{
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "field", &field), ANI_OK);
    ASSERT_NE(field, nullptr);

    ani_object object {};
    GetTestObject(&object);
    ani_int result {};
    ASSERT_EQ(env_->Object_GetField_Int(object, field, &result), ANI_OK);
    constexpr ani_int EXPECTED_RESULT = 1;
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(ClassFindFieldTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "missingField", &field), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "instance field not found with given name [ERROR]"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, interface_property_name_is_rejected)
{
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(class_, "clsprop", &field), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "instance field not found with given name [ERROR]"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, wrong_env)
{
    ani_field field {};
    ASSERT_EQ(env_->c_api->Class_FindField(nullptr, class_, "field", &field), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, wrong_class)
{
    ani_field field {};
    ASSERT_EQ(env_->c_api->Class_FindField(env_, nullptr, "field", &field), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr [ERROR]"},
        {"name", "const char *", "instance field requires a class [ERROR]"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, wrong_name)
{
    ani_field field {};
    ASSERT_EQ(env_->c_api->Class_FindField(env_, class_, nullptr, &field), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "argument is nullptr, expected const char * [ERROR]"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindField(env_, class_, "field", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_field *", "nullptr for storing 'ani_field' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_field field {};
    ASSERT_EQ(env_->c_api->Class_FindField(env_, localClass, "field", &field), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"name", "const char *", "instance field requires a class [ERROR]"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

TEST_F(ClassFindFieldTest, pending_error_is_rejected)
{
    ThrowError();

    ani_field field {};
    ASSERT_EQ(env_->c_api->Class_FindField(env_, class_, "field", &field), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_field *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindField", testLines);
}

}  // namespace ark::ets::ani::verify::testing
