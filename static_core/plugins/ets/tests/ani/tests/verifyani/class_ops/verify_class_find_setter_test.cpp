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

class ClassFindSetterTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_setter_test.TestClass";

    void GetTestObject(ani_object *object)
    {
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(class_, "<ctor>", ":", &ctor), ANI_OK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(class_, ctor, object), ANI_OK);
    }

    ani_class class_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindSetterTest, success_returns_verified_handle)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindSetter(class_, "prop", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    GetTestObject(&object);
    constexpr ani_int PROP_VALUE = 42;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, PROP_VALUE), ANI_OK);
}

TEST_F(ClassFindSetterTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindSetter(class_, "missingProp", &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "wrong setter"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

TEST_F(ClassFindSetterTest, wrong_env)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindSetter(nullptr, class_, "prop", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

TEST_F(ClassFindSetterTest, wrong_class)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindSetter(env_, nullptr, "prop", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference"},
        {"name", "const char *", "wrong class for setter"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

TEST_F(ClassFindSetterTest, wrong_name)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindSetter(env_, class_, nullptr, &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

TEST_F(ClassFindSetterTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindSetter(env_, class_, "prop", nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_method *", "wrong pointer for storing 'ani_method'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

TEST_F(ClassFindSetterTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindSetter(env_, localClass, "prop", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference"},
        {"name", "const char *", "wrong class for setter"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

TEST_F(ClassFindSetterTest, pending_error_is_rejected)
{
    ThrowError();

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindSetter(env_, class_, "prop", &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"class", "ani_class"},
        {"name", "const char *"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindSetter", testLines);
}

}  // namespace ark::ets::ani::verify::testing
