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

#include "runtime/coretypes/algorithm/knuth_morris_pratt.h"

#include "libarkbase/utils/utils.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/mem/vm_handle-inl.h"

#include <gtest/gtest.h>
#include <vector>

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::coretypes::algo::test {

class KmpTest : public testing::Test {
public:
    KmpTest()
    {
        // We need to create a runtime instance to be able to use Panda's allocators.
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options_);
    }

    ~KmpTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(KmpTest);
    NO_MOVE_SEMANTIC(KmpTest);

protected:
    void SetUp() override
    {
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        thread_->ManagedCodeEnd();
    }

private:
    ark::MTManagedThread *thread_ {};
    RuntimeOptions options_;
};

template <typename T>
Span<const T> MakeSpan(const std::initializer_list<T> &&list)
{
    return Span(list.begin(), list.end());
}

template <typename T>
Span<const T> MakeSpan()
{
    return Span<const T>();
}

TEST_F(KmpTest, IndexOfEmpty)
{
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan<uint8_t>()));
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan<uint8_t>(), 1_I));
}

TEST_F(KmpTest, IndexOfSingleChar)
{
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan({'a'})));
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan({'b'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan({'b', 'a'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan({'a', 'a'}), 0));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan({'a'})}.IndexOf(MakeSpan({'a', 'a'}), 1_I));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint8_t>({'b'})}.IndexOf(MakeSpan<uint8_t>({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint8_t>({'b'})}.IndexOf(MakeSpan<uint16_t>({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint16_t>({'b'})}.IndexOf(MakeSpan<uint16_t>({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint16_t>({'b'})}.IndexOf(MakeSpan<uint8_t>({'a', 'b'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan<uint8_t>({'a'})}.IndexOf(MakeSpan<uint8_t>({'a', 'b', 'a'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan<uint8_t>({'a'})}.IndexOf(MakeSpan<uint16_t>({'a', 'b', 'a'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan<uint16_t>({'a'})}.IndexOf(MakeSpan<uint16_t>({'a', 'b', 'a'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan<uint16_t>({'a'})}.IndexOf(MakeSpan<uint8_t>({'a', 'b', 'a'})));
}

TEST_F(KmpTest, IndexOfHeterogeneous)
{
    const auto p8 = std::vector<uint8_t> {'b', 'c', 'd'};
    const auto p16 = std::vector<uint16_t> {'b', 'c', 'd'};
    const auto kmp8 = KmpStringMatcher {Span(p8)};
    const auto kmp16 = KmpStringMatcher {Span(p16)};
    const auto str8 = std::vector<uint8_t> {'a', 'b', 'c', 'd', 'z'};
    const auto str16 = std::vector<uint16_t> {'a', 'b', 'c', 'd', 'z'};
    EXPECT_EQ(kmp8.IndexOf(Span(str8), 1_I), kmp16.IndexOf(Span(str8), 1_I));
    EXPECT_EQ(kmp8.IndexOf(Span(str8), 1_I), kmp16.IndexOf(Span(str16), 1_I));
    EXPECT_EQ(kmp8.IndexOf(Span(str8), 2_I), kmp16.IndexOf(Span(str8), 2_I));
    EXPECT_EQ(kmp8.IndexOf(Span(str8), 2_I), kmp16.IndexOf(Span(str16), 2_I));
}

TEST_F(KmpTest, IndexOfHeterogeneous2)
{
    const auto p8 = std::vector<uint8_t> {'w'};
    const auto p16 = std::vector<uint16_t> {'w'};
    const auto kmp8 = KmpStringMatcher {Span(p8)};
    const auto kmp16 = KmpStringMatcher {Span(p16)};
    EXPECT_EQ(kmp8.IndexOf(Span(p8)), kmp16.IndexOf(Span(p8)));
    EXPECT_EQ(kmp8.IndexOf(Span(p16)), kmp16.IndexOf(Span(p16)));
}

TEST_F(KmpTest, IndexOf)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(0, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 1_I));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 1_I));
}

TEST_F(KmpTest, IndexOf2)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'c', 'a', 'b', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(0, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(pattern.size() + 1_I, matcher.IndexOf(Span(string), 1_I));
    EXPECT_EQ(pattern.size() + 1_I, matcher.IndexOf(Span(string), pattern.size() + 1_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), pattern.size() + 2_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), pattern.size() + 3_I));
}

TEST_F(KmpTest, IndexOf3)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'd'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'c', 'd'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(string.size() - pattern.size(), matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(string.size() - pattern.size(), matcher.IndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 1));
}

TEST_F(KmpTest, IndexOf4)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'd'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'c', 'e'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), 0));
}

TEST_F(KmpTest, IndexOf5)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'c', 'a', 'b', 'a'};
    const auto aCharPattern = std::vector<uint8_t> {'a'};
    const auto aCharMatcher = KmpStringMatcher {Span(aCharPattern)};
    EXPECT_EQ(0, aCharMatcher.IndexOf(Span(string), 0));
    EXPECT_EQ(2_I, aCharMatcher.IndexOf(Span(string), 1_I));
    EXPECT_EQ(2_I, aCharMatcher.IndexOf(Span(string), 2_I));
    EXPECT_EQ(4_I, aCharMatcher.IndexOf(Span(string), 3_I));
    EXPECT_EQ(4_I, aCharMatcher.IndexOf(Span(string), 4_I));
    EXPECT_EQ(6_I, aCharMatcher.IndexOf(Span(string), 5_I));
    EXPECT_EQ(string.size() - 1_I, aCharMatcher.IndexOf(Span(string), string.size() - 1_I));
    EXPECT_EQ(-1, aCharMatcher.IndexOf(Span(string), string.size()));

    const auto cCharPattern = std::vector<uint8_t> {'c'};
    const auto cCharMatcher = KmpStringMatcher {Span(cCharPattern)};
    EXPECT_EQ(3_I, cCharMatcher.IndexOf(Span(string), 0));
    EXPECT_EQ(3_I, cCharMatcher.IndexOf(Span(string), 1_I));
    EXPECT_EQ(3_I, cCharMatcher.IndexOf(Span(string), 2_I));
    EXPECT_EQ(3_I, cCharMatcher.IndexOf(Span(string), 3_I));
    EXPECT_EQ(-1, cCharMatcher.IndexOf(Span(string), 4_I));
    EXPECT_EQ(-1, cCharMatcher.IndexOf(Span(string), 5_I));
    EXPECT_EQ(-1, cCharMatcher.IndexOf(Span(string), string.size() - 1));
    EXPECT_EQ(-1, cCharMatcher.IndexOf(Span(string), string.size()));
}

TEST_F(KmpTest, IndexOf6)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'a', 'a', 'a', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 2_I));
    EXPECT_EQ(3_I, matcher.IndexOf(Span(string), 3_I));
    EXPECT_EQ(4_I, matcher.IndexOf(Span(string), 4_I));
    EXPECT_EQ(string.size() - pattern.size() - 1, matcher.IndexOf(Span(string), string.size() - pattern.size() - 1));
    EXPECT_EQ(string.size() - pattern.size(), matcher.IndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 2_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() + 1_I));
}

TEST_F(KmpTest, IndexOf7)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'a', 'a', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(8_I, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(8_I, matcher.IndexOf(Span(string), 7_I));
    EXPECT_EQ(8_I, matcher.IndexOf(Span(string), 8_I));
    EXPECT_EQ(9_I, matcher.IndexOf(Span(string), 9_I));
    EXPECT_EQ(string.size() - pattern.size() - 1, matcher.IndexOf(Span(string), string.size() - pattern.size() - 1));
    EXPECT_EQ(string.size() - pattern.size(), matcher.IndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 2_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() + 1_I));
}

TEST_F(KmpTest, IndexOf8)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'e'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), 0));
}

TEST_F(KmpTest, IndexOf9)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'c', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    const auto string1 =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'a', 'b', 'c', 'b', 'a', 'b'};
    EXPECT_EQ(-1, matcher.IndexOf(Span(string1), 0));

    const auto string2 =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'c', 'a', 'a', 'b'};
    EXPECT_EQ(6_I, matcher.IndexOf(Span(string2), 0));
    EXPECT_EQ(6_I, matcher.IndexOf(Span(string2), 5_I));
    EXPECT_EQ(6_I, matcher.IndexOf(Span(string2), 6_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string2), 7_I));
}

TEST_F(KmpTest, IndexOf10)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    const auto string =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'a', 'b', 'c', 'b', 'a', 'b'};
    EXPECT_EQ(-1, matcher.IndexOf(Span(string)));
}

TEST_F(KmpTest, IndexOf11)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(0, matcher.IndexOf(Span(pattern)));
    EXPECT_EQ(-1, matcher.IndexOf(Span(pattern), 1_I));
}

TEST_F(KmpTest, IndexOf12)
{
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'b', 'c'})}.IndexOf(MakeSpan({'b'})));
}

TEST_F(KmpTest, IndexOf13)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    const auto string =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'c', 'b', 'a', 'b'};
    EXPECT_EQ(4_I, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(4_I, matcher.IndexOf(Span(string), 4_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), 5_I));
}

TEST_F(KmpTest, IndexOf14)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
    const auto pattern = std::vector<uint8_t> {'d', 'e', 'f'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(3_I, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(3_I, matcher.IndexOf(Span(string), 3_I));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), 4_I));
}

TEST_F(KmpTest, IndexOf15)
{
    const auto string = std::vector<uint8_t> {'a', 'a', 'a', 'a', 'b'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'b'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    ASSERT_EQ(2_I, string.size() - pattern.size());
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 1_I));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 1));
}

TEST_F(KmpTest, IndexOf16)
{
    const auto string = std::vector<uint8_t> {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    ASSERT_EQ(2_I, string.size() - pattern.size());
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 0));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), 1_I));
    EXPECT_EQ(2_I, matcher.IndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.IndexOf(Span(string), string.size() - pattern.size() + 1));
}

TEST_F(KmpTest, LastIndexOfEmpty)
{
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan<uint8_t>()));
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan<uint8_t>(), 1_I));
}

TEST_F(KmpTest, LastIndexOfSingleChar)
{
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'a'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'a'}), 0));
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'b'})));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'b', 'a'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'a', 'a'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'a', 'a'}), 1_I));
    EXPECT_EQ(0, KmpStringMatcher {MakeSpan({'a'})}.LastIndexOf(MakeSpan({'a', 'a'}), 0));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint8_t>({'b'})}.LastIndexOf(MakeSpan<uint8_t>({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint8_t>({'b'})}.LastIndexOf(MakeSpan<uint16_t>({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint16_t>({'b'})}.LastIndexOf(MakeSpan<uint16_t>({'a', 'b'})));
    EXPECT_EQ(1_I, KmpStringMatcher {MakeSpan<uint16_t>({'b'})}.LastIndexOf(MakeSpan<uint8_t>({'a', 'b'})));
    EXPECT_EQ(2_I, KmpStringMatcher {MakeSpan<uint8_t>({'a'})}.LastIndexOf(MakeSpan<uint8_t>({'a', 'b', 'a'})));
    EXPECT_EQ(2_I, KmpStringMatcher {MakeSpan<uint8_t>({'a'})}.LastIndexOf(MakeSpan<uint16_t>({'a', 'b', 'a'})));
    EXPECT_EQ(2_I, KmpStringMatcher {MakeSpan<uint16_t>({'a'})}.LastIndexOf(MakeSpan<uint16_t>({'a', 'b', 'a'})));
    EXPECT_EQ(2_I, KmpStringMatcher {MakeSpan<uint16_t>({'a'})}.LastIndexOf(MakeSpan<uint8_t>({'a', 'b', 'a'})));
}

TEST_F(KmpTest, LastIndexOfHeterogeneous)
{
    const auto p8 = std::vector<uint8_t> {'b', 'c', 'd'};
    const auto p16 = std::vector<uint16_t> {'b', 'c', 'd'};
    const auto kmp8 = KmpStringMatcher {Span(p8)};
    const auto kmp16 = KmpStringMatcher {Span(p16)};
    const auto str8 = std::vector<uint8_t> {'a', 'b', 'c', 'd', 'z'};
    const auto str16 = std::vector<uint16_t> {'a', 'b', 'c', 'd', 'z'};
    EXPECT_EQ(kmp8.LastIndexOf(Span(str8), 1_I), kmp16.LastIndexOf(Span(str8), 1_I));
    EXPECT_EQ(kmp8.LastIndexOf(Span(str8), 1_I), kmp16.LastIndexOf(Span(str16), 1_I));
    EXPECT_EQ(kmp8.LastIndexOf(Span(str8), 0), kmp16.LastIndexOf(Span(str8), 0));
    EXPECT_EQ(kmp8.LastIndexOf(Span(str8), 0), kmp16.LastIndexOf(Span(str16), 0));
}

TEST_F(KmpTest, LastIndexOfHeterogeneous2)
{
    const auto p8 = std::vector<uint8_t> {'w'};
    const auto p16 = std::vector<uint16_t> {'w'};
    const auto kmp8 = KmpStringMatcher {Span(p8)};
    const auto kmp16 = KmpStringMatcher {Span(p16)};
    EXPECT_EQ(kmp8.LastIndexOf(Span(p8)), kmp16.LastIndexOf(Span(p8)));
    EXPECT_EQ(kmp8.LastIndexOf(Span(p16)), kmp16.LastIndexOf(Span(p16)));
}

TEST_F(KmpTest, LastIndexOf)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), 0));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), 1_I));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), string.size() - pattern.size() - 1_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), 2_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 1_I));
}

TEST_F(KmpTest, LastIndexOf2)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'c', 'a', 'b', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), 0));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), 1_I));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), 2_I));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(string), pattern.size()));
    EXPECT_EQ(pattern.size() + 1, matcher.LastIndexOf(Span(string), pattern.size() + 1_I));
    EXPECT_EQ(pattern.size() + 1, matcher.LastIndexOf(Span(string), pattern.size() + 2_I));
    EXPECT_EQ(pattern.size() + 1, matcher.LastIndexOf(Span(string), pattern.size() + 3_I));
}

TEST_F(KmpTest, LastIndexOf3)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'c', 'a', 'b', 'a'};
    const auto aCharPattern = std::vector<uint8_t> {'a'};
    const auto aCharMatcher = KmpStringMatcher {Span(aCharPattern)};
    EXPECT_EQ(0, aCharMatcher.LastIndexOf(Span(string), 0));
    EXPECT_EQ(0, aCharMatcher.LastIndexOf(Span(string), 1_I));
    EXPECT_EQ(2_I, aCharMatcher.LastIndexOf(Span(string), 2_I));
    EXPECT_EQ(2_I, aCharMatcher.LastIndexOf(Span(string), 3_I));
    EXPECT_EQ(4_I, aCharMatcher.LastIndexOf(Span(string), 4_I));
    EXPECT_EQ(4_I, aCharMatcher.LastIndexOf(Span(string), 5_I));
    EXPECT_EQ(string.size() - 1, aCharMatcher.LastIndexOf(Span(string), string.size() - 1));
    EXPECT_EQ(string.size() - 1, aCharMatcher.LastIndexOf(Span(string), string.size()));

    const auto cCharPattern = std::vector<uint8_t> {'c'};
    const auto cCharMatcher = KmpStringMatcher {Span(cCharPattern)};
    EXPECT_EQ(-1, cCharMatcher.LastIndexOf(Span(string), 0));
    EXPECT_EQ(-1, cCharMatcher.LastIndexOf(Span(string), 1_I));
    EXPECT_EQ(-1, cCharMatcher.LastIndexOf(Span(string), 2_I));
    EXPECT_EQ(3_I, cCharMatcher.LastIndexOf(Span(string), 3_I));
    EXPECT_EQ(3_I, cCharMatcher.LastIndexOf(Span(string), 4_I));
    EXPECT_EQ(3_I, cCharMatcher.LastIndexOf(Span(string), 5_I));
    EXPECT_EQ(3_I, cCharMatcher.LastIndexOf(Span(string), string.size() - 1));
    EXPECT_EQ(3_I, cCharMatcher.LastIndexOf(Span(string), string.size()));
}

TEST_F(KmpTest, LastIndexOf4)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'd'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'c', 'd'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string)));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), string.size() - pattern.size() - 1));
}

TEST_F(KmpTest, LastIndexOf5)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'd'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'c', 'e'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 0));
}

TEST_F(KmpTest, LastIndexOf6)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c'};
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c', 'e'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 0));
}

TEST_F(KmpTest, LastIndexOf7)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'a', 'a', 'a', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 0));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 1_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), 2_I));
    EXPECT_EQ(3_I, matcher.LastIndexOf(Span(string), 3_I));
    EXPECT_EQ(4_I, matcher.LastIndexOf(Span(string), 4_I));
    EXPECT_EQ(string.size() - pattern.size() - 1,
              matcher.LastIndexOf(Span(string), string.size() - pattern.size() - 1_I));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 2_I));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size()));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() + 1_I));
}

TEST_F(KmpTest, LastIndexOf8)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'a', 'a', 'a'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 0));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 7_I));
    EXPECT_EQ(string.size() - pattern.size() - 1,
              matcher.LastIndexOf(Span(string), string.size() - pattern.size() - 1));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 2_I));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size()));
    EXPECT_EQ(string.size() - pattern.size(), matcher.LastIndexOf(Span(string), string.size() + 1_I));
}

TEST_F(KmpTest, LastIndexOf9)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'c', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    const auto string1 =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'a', 'b', 'c', 'b', 'a', 'b'};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string1), 0));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string1)));

    const auto string2 =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'c', 'a', 'a', 'b'};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string2), 0));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string2), 5_I));
    EXPECT_EQ(6_I, matcher.LastIndexOf(Span(string2), 6_I));
    EXPECT_EQ(6_I, matcher.LastIndexOf(Span(string2), 7_I));
    EXPECT_EQ(6_I, matcher.LastIndexOf(Span(string2), 8_I));
    EXPECT_EQ(6_I, matcher.LastIndexOf(Span(string2), string2.size() - 1));
    EXPECT_EQ(6_I, matcher.LastIndexOf(Span(string2), string2.size()));
}

TEST_F(KmpTest, LastIndexOf10)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    const auto string =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'a', 'b', 'c', 'b', 'a', 'b'};
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string)));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), 0));
}

TEST_F(KmpTest, LastIndexOf11)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(0, matcher.LastIndexOf(Span(pattern)));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(pattern), 0));
    EXPECT_EQ(0, matcher.LastIndexOf(Span(pattern), 1_I));
}

TEST_F(KmpTest, LastIndexOf12)
{
    EXPECT_EQ(-1, KmpStringMatcher {MakeSpan({'b', 'c'})}.LastIndexOf(MakeSpan({'b'})));
}

TEST_F(KmpTest, LastIndexOf13)
{
    const auto pattern = std::vector<uint8_t> {'a', 'b', 'a', 'b', 'a', 'b', 'a'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    const auto string =
        std::vector<uint8_t> {'b', 'a', 'c', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'c', 'b', 'a', 'b'};
    EXPECT_EQ(4_I, matcher.LastIndexOf(Span(string), string.size()));
    EXPECT_EQ(4_I, matcher.LastIndexOf(Span(string), 4_I));
    EXPECT_EQ(4_I, matcher.LastIndexOf(Span(string), 5_I));
}

TEST_F(KmpTest, LastIndexOf14)
{
    const auto string = std::vector<uint8_t> {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
    const auto pattern = std::vector<uint8_t> {'d', 'e', 'f'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    EXPECT_EQ(3_I, matcher.LastIndexOf(Span(string), string.size()));
    EXPECT_EQ(3_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 2_I));
    EXPECT_EQ(3_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(3_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
}

TEST_F(KmpTest, LastIndexOf15)
{
    const auto string = std::vector<uint8_t> {'a', 'a', 'a', 'a', 'b'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'b'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    ASSERT_EQ(2_I, string.size() - pattern.size());
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() + 1_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size()));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), 3_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), string.size() - pattern.size() - 1_I));
}

TEST_F(KmpTest, LastIndexOf16)
{
    const auto string = std::vector<uint8_t> {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b'};
    const auto pattern = std::vector<uint8_t> {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b'};
    const auto matcher = KmpStringMatcher {Span(pattern)};
    ASSERT_EQ(2_I, string.size() - pattern.size());
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() + 1_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size()));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), 7_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size() + 1_I));
    EXPECT_EQ(2_I, matcher.LastIndexOf(Span(string), string.size() - pattern.size()));
    EXPECT_EQ(-1, matcher.LastIndexOf(Span(string), string.size() - pattern.size() - 1_I));
}

}  // namespace ark::coretypes::algo::test

// NOLINTEND(readability-magic-numbers)
