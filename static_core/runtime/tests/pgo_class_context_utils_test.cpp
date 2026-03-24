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

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "jit/libprofile/pgo_class_context_utils.h"

namespace ark::test {

TEST(PgoClassContextUtilsTest, ParseEmptyContext)
{
    std::map<std::string, std::string> entries;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Parse("", entries, &error));
    EXPECT_TRUE(entries.empty());
    EXPECT_TRUE(error.empty());
}

TEST(PgoClassContextUtilsTest, ParseValidContextWithEmptyTokens)
{
    std::map<std::string, std::string> entries;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Parse(":/a/b/c.abc*111::/x/y/z.abc*222:", entries, &error));
    ASSERT_EQ(entries.size(), 2U);
    EXPECT_EQ(entries["/a/b/c.abc"], "111");
    EXPECT_EQ(entries["/x/y/z.abc"], "222");
}

TEST(PgoClassContextUtilsTest, ParseInvalidContextEntry)
{
    std::map<std::string, std::string> entries;
    std::string error;
    EXPECT_FALSE(pgo::PgoClassContextUtils::Parse("/a/b/c.abc*", entries, &error));
    EXPECT_NE(error.find("invalid class context entry"), std::string::npos);
}

TEST(PgoClassContextUtilsTest, MergeCompatibleContexts)
{
    std::vector<std::string_view> contexts;
    contexts.emplace_back("/a.abc*11:/b.abc*22");
    contexts.emplace_back("/b.abc*22:/c.abc*33");
    std::string merged;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge(contexts, merged, &error));
    EXPECT_EQ(merged, "/a.abc*11:/b.abc*22:/c.abc*33");
}

TEST(PgoClassContextUtilsTest, MergeChecksumMismatch)
{
    std::vector<std::string_view> contexts;
    contexts.emplace_back("/a.abc*11:/b.abc*22");
    contexts.emplace_back("/b.abc*99");
    std::string merged;
    std::string error;
    EXPECT_FALSE(pgo::PgoClassContextUtils::Merge(contexts, merged, &error));
    EXPECT_FALSE(error.empty());
}

TEST(PgoClassContextUtilsTest, MergeAllEmptyContexts)
{
    std::vector<std::string_view> contexts;
    contexts.emplace_back("");
    contexts.emplace_back("");
    std::string merged;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge(contexts, merged, &error));
    EXPECT_TRUE(merged.empty());
}

TEST(PgoClassContextUtilsTest, MergeDuplicatePathWithSameChecksumAllowed)
{
    std::vector<std::string_view> contexts;
    contexts.emplace_back("/a.abc*11:/a.abc*11");
    contexts.emplace_back("/a.abc*11:/b.abc*22");
    std::string merged;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge(contexts, merged, &error));
    EXPECT_EQ(merged, "/a.abc*11:/b.abc*22");
}

TEST(PgoClassContextUtilsTest, MergeDuplicatePathWithDifferentChecksumRejected)
{
    std::vector<std::string_view> contexts;
    contexts.emplace_back("/a.abc*11:/a.abc*22");
    std::string merged;
    std::string error;
    EXPECT_FALSE(pgo::PgoClassContextUtils::Merge(contexts, merged, &error));
    EXPECT_NE(error.find("checksum mismatch"), std::string::npos);
}

TEST(PgoClassContextUtilsTest, MergeUsesCanonicalPathOrder)
{
    std::vector<std::string_view> contexts;
    contexts.emplace_back("/c.abc*33:/a.abc*11");
    contexts.emplace_back("/b.abc*22:/a.abc*11:/d.abc*44");
    contexts.emplace_back("/d.abc*44:/e.abc*55");
    std::string merged;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge(contexts, merged, &error));
    EXPECT_EQ(merged, "/a.abc*11:/b.abc*22:/c.abc*33:/d.abc*44:/e.abc*55");
}

TEST(PgoClassContextUtilsTest, MergeOrderIndependent)
{
    // Commutativity.
    std::vector<std::string_view> contexts1 {"/a.abc*11:/b.abc*22", "/c.abc*33:/a.abc*11"};
    std::vector<std::string_view> contexts2 {"/c.abc*33:/a.abc*11", "/a.abc*11:/b.abc*22"};
    std::string merged1;
    std::string merged2;
    std::string error;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge(contexts1, merged1, &error));
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge(contexts2, merged2, &error));
    EXPECT_EQ(merged1, merged2);
}

TEST(PgoClassContextUtilsTest, MergeGroupingIndependent)
{
    // Associativity.
    std::string a = "/c.abc*33:/a.abc*11";
    std::string b = "/b.abc*22:/a.abc*11";
    std::string c = "/d.abc*44:/b.abc*22";
    std::string error;

    std::string mergedAbc;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge({a, b, c}, mergedAbc, &error));

    std::string mergedAb;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge({a, b}, mergedAb, &error));
    std::string mergedAbThenC;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge({mergedAb, c}, mergedAbThenC, &error));

    std::string mergedBc;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge({b, c}, mergedBc, &error));
    std::string mergedAThenBc;
    ASSERT_TRUE(pgo::PgoClassContextUtils::Merge({a, mergedBc}, mergedAThenBc, &error));

    EXPECT_EQ(mergedAbc, mergedAbThenC);
    EXPECT_EQ(mergedAbc, mergedAThenBc);
}

}  // namespace ark::test
