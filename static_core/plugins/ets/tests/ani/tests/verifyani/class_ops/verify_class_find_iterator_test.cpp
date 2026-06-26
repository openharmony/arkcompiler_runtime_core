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

class ClassFindIteratorTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindClass(className, &class_), ANI_OK);
        ASSERT_EQ(env_->FindClass(noIteratorName, &noIteratorClass_), ANI_OK);
    }

protected:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *className = "verify_class_find_iterator_test.TestClass";
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr const char *noIteratorName = "verify_class_find_iterator_test.NoIterator";
    ani_class class_ {};            // NOLINT(misc-non-private-member-variables-in-classes)
    ani_class noIteratorClass_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ClassFindIteratorTest, success_returns_verified_handle)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIterator(class_, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindIteratorTest, lookup_status_is_forwarded_without_verify_abort)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIterator(noIteratorClass_, &method), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "iterator not found [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIterator", testLines);
}

TEST_F(ClassFindIteratorTest, wrong_env)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIterator(nullptr, class_, &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"class", "ani_class"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIterator", testLines);
}

TEST_F(ClassFindIteratorTest, wrong_class)
{
    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIterator(env_, nullptr, &method), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference is nullptr [ERROR]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIterator", testLines);
}

TEST_F(ClassFindIteratorTest, wrong_result_storage)
{
    ASSERT_EQ(env_->c_api->Class_FindIterator(env_, class_, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"result", "ani_method *", "nullptr for storing 'ani_method' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIterator", testLines);
}

TEST_F(ClassFindIteratorTest, local_class_reference_is_rejected)
{
    ani_class localClass {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindClass(className, &localClass), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIterator(env_, localClass, &method), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIterator", testLines);
}

TEST_F(ClassFindIteratorTest, pending_error_is_rejected)
{
    ThrowError();

    ani_method method {};
    ASSERT_EQ(env_->c_api->Class_FindIterator(env_, class_, &method), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"class", "ani_class"},
        {"result", "ani_method *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Class_FindIterator", testLines);
}

}  // namespace ark::ets::ani::verify::testing
