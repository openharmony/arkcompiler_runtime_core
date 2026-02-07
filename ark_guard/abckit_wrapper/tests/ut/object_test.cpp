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

#include <gtest/gtest.h>
#include "abckit_wrapper/file_view.h"
#include "abckit_wrapper/object.h"
#include "abckit_wrapper/class.h"
#include "abckit_wrapper/method.h"
#include "test_util/assert.h"

using namespace testing::ext;
namespace {
using namespace abckit_wrapper;
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/object_test.abc";
abckit_wrapper::FileView fileView;
/**
 * @brief Test class for Object fully qualified name functionality
 */
class TestObject : public testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override {}
};

/**
 * @tc.name: get_fully_qualified_name_001
 * @tc.desc: Test case for Object::GetFullyQualifiedName method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestObject, get_fully_qualified_name_001, TestSize.Level0)
{
    // Get a class object
    const auto clazz = fileView.Get<Class>("object_test.ClassA");
    ASSERT_TRUE(clazz.has_value());

    // First call to GetFullyQualifiedName
    const std::string fqn1 = clazz.value()->GetFullyQualifiedName();

    // Second call should use cache
    const std::string fqn2 = clazz.value()->GetFullyQualifiedName();

    // Verify both calls return the same result and name is correct
    ASSERT_EQ(fqn1, fqn2);
    ASSERT_EQ(fqn1, "object_test.ClassA");
}
};  // namespace