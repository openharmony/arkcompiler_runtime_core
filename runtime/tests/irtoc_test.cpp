/**
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

#include <array>
#include "gtest/gtest.h"
#include "utils/arch.h"

extern "C" int IrtocTestAddValues(int64_t, int64_t);
extern "C" int IrtocTestIncMaxValue(uint64_t, uint64_t);
extern "C" int IrtocTestIncMaxValueLabels(uint64_t, uint64_t);
extern "C" unsigned IrtocTestSeqLabels(uint64_t);
extern "C" uint64_t IrtocTestCfg(void *, uint64_t);
extern "C" uint64_t IrtocTestCfgLabels(void *, uint64_t);
extern "C" size_t IrtocTestLabels(size_t);
extern "C" size_t IrtocTestReturnBeforeLabel(size_t);

using TestCfgFunc = uint64_t (*)(void *, uint64_t);

namespace panda::test {

TEST(Irtoc, AddValues)
{
    ASSERT_EQ(IrtocTestAddValues(1, 2), 3);
    ASSERT_EQ(IrtocTestAddValues(-1, -2), -3);
}

TEST(Irtoc, IncMaxValue)
{
    ASSERT_EQ(IrtocTestIncMaxValue(10, 9), 11);
    ASSERT_EQ(IrtocTestIncMaxValue(4, 8), 9);
    ASSERT_EQ(IrtocTestIncMaxValue(4, 4), 5);
}

TEST(Irtoc, IncMaxValueLabels)
{
    ASSERT_EQ(IrtocTestIncMaxValueLabels(10, 9), 11);
    ASSERT_EQ(IrtocTestIncMaxValueLabels(4, 8), 9);
    ASSERT_EQ(IrtocTestIncMaxValueLabels(4, 4), 5);
}

template <size_t N>
uint64_t ModifyArrayForLoopTest(std::array<uint64_t, N> *data)
{
    uint64_t res = 0;
    for (size_t i = 0; i < data->size(); i++) {
        if ((i % 2) == 0) {
            if (((*data)[i] % 2) == 0) {
                (*data)[i] += 2;
                res += 2;
            } else {
                (*data)[i] += 1;
                res += 1;
            }
        } else {
            (*data)[i] -= 1;
            res -= 1;
        }
    }
    return res;
}

void TestLoop(TestCfgFunc func)
{
    std::array<uint64_t, 8> buf;
    for (size_t i = 0; i < buf.size(); i++) {
        buf[i] = i + i % 3;
    }
    std::array<uint64_t, 8> buf_expected = buf;
    auto expected = ModifyArrayForLoopTest(&buf_expected);
    uint64_t res = func(static_cast<void *>(buf.data()), buf.size());
    ASSERT_EQ(expected, res);
    ASSERT_EQ(buf_expected, buf);
}

TEST(Irtoc, Loop)
{
    if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP();
    }
    TestLoop(IrtocTestCfg);
    TestLoop(IrtocTestCfgLabels);
}

TEST(Irtoc, SeqLabels)
{
    if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP();
    }
    EXPECT_EQ(IrtocTestSeqLabels(0), 1);
    EXPECT_EQ(IrtocTestSeqLabels(5), 6);
    EXPECT_EQ(IrtocTestSeqLabels(10), 11);
    EXPECT_EQ(IrtocTestSeqLabels(11), 13);
    EXPECT_EQ(IrtocTestSeqLabels(55), 57);
    EXPECT_EQ(IrtocTestSeqLabels(100), 102);
    EXPECT_EQ(IrtocTestSeqLabels(101), 104);
    EXPECT_EQ(IrtocTestSeqLabels(1010), 1013);
    EXPECT_EQ(IrtocTestSeqLabels(54545), 54548);
}

extern "C" int IrtocTestRelocations(int);
extern "C" int TestCall(int n)
{
    return n + 2;
}

TEST(Irtoc, Relocations)
{
    ASSERT_EQ(IrtocTestRelocations(1), 5);
    ASSERT_EQ(IrtocTestRelocations(2), 10);
    ASSERT_EQ(IrtocTestRelocations(3), 17);
}

extern "C" size_t IncrementInt(size_t n)
{
    return n + 1;
}

extern "C" double IncrementFloat(double n)
{
    return n + 1;
}

extern "C" size_t IrtocTestRelocations2(size_t a0, size_t a1, double f0, size_t a2, size_t a3, size_t a4, double f1,
                                        double f2, size_t a5, size_t a6, double f3, size_t a7, size_t a8, size_t a9,
                                        double f4);

TEST(Irtoc, RelocationsParams)
{
    if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP();
    }
    ASSERT_EQ(IrtocTestRelocations2(0, 1, 2.0, 3, 4, 5, 6.0, 7.0, 8, 9, 10.0, 11, 12, 13, 14.0), 120);
}

TEST(Irtoc, Labels)
{
    EXPECT_EQ(IrtocTestLabels(0), 0);
    EXPECT_EQ(IrtocTestLabels(1), 1);
    EXPECT_EQ(IrtocTestLabels(2), 3);
    EXPECT_EQ(IrtocTestLabels(3), 6);
    EXPECT_EQ(IrtocTestLabels(4), 10);
    EXPECT_EQ(IrtocTestLabels(5), 15);
    EXPECT_EQ(IrtocTestLabels(100), 5050);
}

TEST(Irtoc, ReturnBeforeLabel)
{
    EXPECT_EQ(IrtocTestReturnBeforeLabel(42), 2);
    EXPECT_EQ(IrtocTestReturnBeforeLabel(146), 1);
}

}  // namespace panda::test
