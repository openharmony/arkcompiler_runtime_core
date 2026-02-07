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

#include "abckit_wrapper/object.h"

using namespace testing::ext;

// Mock class that implements the abstract methods of abckit_wrapper::Object
class MockObject : public abckit_wrapper::Object {
public:
    explicit MockObject(const std::string &name) : name_(name) {}

    std::string GetName() const override
    {
        return name_;
    }

    bool SetName(const std::string &name) override
    {
        name_ = name;
        InvalidateFullyQualifiedNameCache();
        return true;
    }

    void InvalidateCache()
    {
        InvalidateFullyQualifiedNameCache();
    }

private:
    std::string name_;
};

/**
 * @tc.name: object_get_fully_qualified_name_test_001
 * @tc.desc: test GetFullyQualifiedName method for single object
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ObjectUtilTest, object_get_fully_qualified_name_test_001, TestSize.Level1)
{
    MockObject obj("TestObject");
    const std::string expected = "TestObject";
    const std::string result = obj.GetFullyQualifiedName();
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: object_get_fully_qualified_name_test_002
 * @tc.desc: test GetFullyQualifiedName method with parent-child relationship
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ObjectUtilTest, object_get_fully_qualified_name_test_002, TestSize.Level1)
{
    MockObject parent("Parent");
    MockObject child("Child");
    child.parent_ = &parent;

    const std::string expected = "Parent.Child";
    const std::string result = child.GetFullyQualifiedName();
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: object_get_fully_qualified_name_test_003
 * @tc.desc: test GetFullyQualifiedName method with multi-level hierarchy
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ObjectUtilTest, object_get_fully_qualified_name_test_003, TestSize.Level1)
{
    MockObject grandparent("Grandparent");
    MockObject parent("Parent");
    MockObject child("Child");

    parent.parent_ = &grandparent;
    child.parent_ = &parent;

    const std::string expected = "Grandparent.Parent.Child";
    const std::string result = child.GetFullyQualifiedName();
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: object_invalidate_cache_test_001
 * @tc.desc: test InvalidateFullyQualifiedNameCache method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ObjectUtilTest, object_invalidate_cache_test_001, TestSize.Level1)
{
    MockObject obj("TestObject");

    // First call to populate cache
    const std::string firstResult = obj.GetFullyQualifiedName();

    // Invalidate cache
    obj.InvalidateCache();

    // Second call should still return the same result
    const std::string secondResult = obj.GetFullyQualifiedName();
    EXPECT_EQ(firstResult, secondResult);
}

/**
 * @tc.name: object_invalidate_cache_test_002
 * @tc.desc: test InvalidateFullyQualifiedNameCache method with children
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ObjectUtilTest, object_invalidate_cache_test_002, TestSize.Level1)
{
    MockObject parent("Parent");
    MockObject child("Child");
    child.parent_ = &parent;

    // First call to populate cache
    const std::string parentResult = parent.GetFullyQualifiedName();
    const std::string childResult = child.GetFullyQualifiedName();

    // Invalidate cache on parent (should recursively invalidate child)
    parent.InvalidateCache();

    // Second call should still return the same result
    const std::string parentResultAfter = parent.GetFullyQualifiedName();
    const std::string childResultAfter = child.GetFullyQualifiedName();

    EXPECT_EQ(parentResult, parentResultAfter);
    EXPECT_EQ(childResult, childResultAfter);
}
