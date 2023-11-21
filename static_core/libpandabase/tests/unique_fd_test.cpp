/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "os/unique_fd.h"

#include <gtest/gtest.h>
#include <utility>
#include <unistd.h>

namespace panda::os::unique_fd {

enum TestValue { DEFAULT_VALUE = -1L, STDIN_VALUE, STDOUT_VALUE, STDERR_VALUE };

struct DuplicateFD {
    // CHECKER_IGNORE_NEXTLINE(AF0005)
    // NOLINTNEXTLINE(android-cloexec-dup)
    int stdin_value = ::dup(STDIN_VALUE);
    // CHECKER_IGNORE_NEXTLINE(AF0005)
    // NOLINTNEXTLINE(android-cloexec-dup)
    int stdout_value = ::dup(STDOUT_VALUE);
    // CHECKER_IGNORE_NEXTLINE(AF0005)
    // NOLINTNEXTLINE(android-cloexec-dup)
    int stferr_value = ::dup(STDERR_VALUE);
};

TEST(UniqueFd, Construct)
{
    DuplicateFD dup_df;
    auto fd_a = UniqueFd();
    auto fd_b = UniqueFd(dup_df.stdin_value);
    auto fd_c = UniqueFd(dup_df.stdout_value);
    auto fd_d = UniqueFd(dup_df.stferr_value);

    EXPECT_EQ(fd_a.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_b.Get(), dup_df.stdin_value);
    EXPECT_EQ(fd_c.Get(), dup_df.stdout_value);
    EXPECT_EQ(fd_d.Get(), dup_df.stferr_value);

    auto fd_e = std::move(fd_a);
    auto fd_f = std::move(fd_b);
    auto fd_g = std::move(fd_c);
    auto fd_h = std::move(fd_d);

    // NOLINTBEGIN(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    EXPECT_EQ(fd_a.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_b.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_c.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_d.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_e.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_f.Get(), dup_df.stdin_value);
    EXPECT_EQ(fd_g.Get(), dup_df.stdout_value);
    EXPECT_EQ(fd_h.Get(), dup_df.stferr_value);
}

TEST(UniqueFd, Equal)
{
    DuplicateFD dup_df;
    auto fd_a = UniqueFd();
    auto fd_b = UniqueFd(dup_df.stdin_value);
    auto fd_c = UniqueFd(dup_df.stdout_value);
    auto fd_d = UniqueFd(dup_df.stferr_value);

    auto fd_e = UniqueFd();
    auto fd_f = UniqueFd();
    auto fd_g = UniqueFd();
    auto fd_h = UniqueFd();
    fd_e = std::move(fd_a);
    fd_f = std::move(fd_b);
    fd_g = std::move(fd_c);
    fd_h = std::move(fd_d);

    EXPECT_EQ(fd_a.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_b.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_c.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_d.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_e.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_f.Get(), dup_df.stdin_value);
    EXPECT_EQ(fd_g.Get(), dup_df.stdout_value);
    EXPECT_EQ(fd_h.Get(), dup_df.stferr_value);
    // NOLINTEND(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
}

TEST(UniqueFd, Release)
{
    DuplicateFD dup_df;
    auto fd_a = UniqueFd();
    auto fd_b = UniqueFd(dup_df.stdin_value);
    auto fd_c = UniqueFd(dup_df.stdout_value);
    auto fd_d = UniqueFd(dup_df.stferr_value);

    auto num_a = fd_a.Release();
    auto num_b = fd_b.Release();
    auto num_c = fd_c.Release();
    auto num_d = fd_d.Release();

    EXPECT_EQ(fd_a.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_b.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_c.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_d.Get(), DEFAULT_VALUE);
    EXPECT_EQ(num_a, DEFAULT_VALUE);
    EXPECT_EQ(num_b, dup_df.stdin_value);
    EXPECT_EQ(num_c, dup_df.stdout_value);
    EXPECT_EQ(num_d, dup_df.stferr_value);
}

TEST(UniqueFd, Reset)
{
    DuplicateFD dup_df;

    auto num_a = DEFAULT_VALUE;
    auto num_b = dup_df.stdin_value;
    auto num_c = dup_df.stdout_value;
    auto num_d = dup_df.stferr_value;

    auto fd_a = UniqueFd();
    auto fd_b = UniqueFd();
    auto fd_c = UniqueFd();
    auto fd_d = UniqueFd();

    fd_a.Reset(num_a);
    fd_b.Reset(num_b);
    fd_c.Reset(num_c);
    fd_d.Reset(num_d);

    EXPECT_EQ(fd_a.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fd_b.Get(), dup_df.stdin_value);
    EXPECT_EQ(fd_c.Get(), dup_df.stdout_value);
    EXPECT_EQ(fd_d.Get(), dup_df.stferr_value);
}

}  // namespace panda::os::unique_fd
