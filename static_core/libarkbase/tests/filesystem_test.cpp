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

#include "libarkbase/os/filesystem.h"

#include <gtest/gtest.h>

namespace ark::os::test {

TEST(NormalizePathTest, CurrentDirectory)
{
    EXPECT_EQ(NormalizePath("."), ".");
    EXPECT_EQ(NormalizePath("./"), ".");
    EXPECT_EQ(NormalizePath("././"), ".");
}

TEST(NormalizePathTest, DotDotCollapsesRealComponent)
{
    EXPECT_EQ(NormalizePath("a/.."), ".");
    EXPECT_EQ(NormalizePath("a/b/.."), "a");
    EXPECT_EQ(NormalizePath("a/b/../c"), "a/c");
    EXPECT_EQ(NormalizePath("a/../b"), "b");
}

TEST(NormalizePathTest, DotDotPreservesDotDotComponent)
{
    EXPECT_EQ(NormalizePath(".."), "..");
    EXPECT_EQ(NormalizePath("../.."), "../..");
    EXPECT_EQ(NormalizePath("../../a"), "../../a");
    EXPECT_EQ(NormalizePath("a/../.."), "..");
}

TEST(NormalizePathTest, AbsolutePaths)
{
    EXPECT_EQ(NormalizePath("/"), "/");
    EXPECT_EQ(NormalizePath("/a/b/../c"), "/a/c");
    EXPECT_EQ(NormalizePath("/a/.."), "/");
    EXPECT_EQ(NormalizePath("/../a"), "/a");
    EXPECT_EQ(NormalizePath("/.."), "/");
}

TEST(NormalizePathTest, EmptyPath)
{
    EXPECT_EQ(NormalizePath(""), "");
}

TEST(NormalizePathTest, DuplicateDelimiters)
{
    EXPECT_EQ(NormalizePath("a//b"), "a/b");
    EXPECT_EQ(NormalizePath("/a//b"), "/a/b");
}

TEST(NormalizePathTest, TrailingDelimiter)
{
    EXPECT_EQ(NormalizePath("a/"), "a");
    EXPECT_EQ(NormalizePath("/a/"), "/a");
    EXPECT_EQ(NormalizePath("../"), "..");
}

TEST(NormalizePathTest, DotSegments)
{
    EXPECT_EQ(NormalizePath("a/./b"), "a/b");
    EXPECT_EQ(NormalizePath("./a"), "a");
    EXPECT_EQ(NormalizePath("a/."), "a");
}

TEST(NormalizePathTest, MixedDotAndDotDot)
{
    EXPECT_EQ(NormalizePath("a/./../b"), "b");
    EXPECT_EQ(NormalizePath("./a/./b/../c"), "a/c");
}

TEST(NormalizePathTest, DotDotCantPopRootMarker)
{
    EXPECT_EQ(NormalizePath("/a/../.."), "/");
    EXPECT_EQ(NormalizePath("/../../a"), "/a");
}

}  // namespace ark::os::test
