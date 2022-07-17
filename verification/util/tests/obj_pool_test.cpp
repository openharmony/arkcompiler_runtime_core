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

#include "util/obj_pool.h"

#include <gtest/gtest.h>

#include <vector>

namespace panda::verifier::test {

namespace {

struct S {
    int a;
    int b;
};

template <typename I, typename C>
struct Pool : public ObjPool<S, std::vector, I, C> {
    Pool(I i, C c) : ObjPool<S, std::vector, I, C> {i, c} {}
};

}  // namespace

TEST(VerifierTest_ObjPool, Basic)
{
    int result = 0;

    auto &&h = [&](S &s, size_t idx) {
        s.a = idx;
        result += idx;
    };
    Pool pool {h, [&](S &s) { result -= s.a; }};

    {
        auto q = pool.New();
        auto p = pool.New();
        EXPECT_EQ(pool.Count(), 2);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 2);
        EXPECT_EQ(result, 1);
    }

    EXPECT_EQ(pool.Count(), 2);
    EXPECT_EQ(pool.FreeCount(), 2);
    EXPECT_EQ(pool.AccCount(), 0);
    EXPECT_EQ(result, 0);

    {
        auto q = pool.New();
        auto w = pool.New();
        EXPECT_EQ(pool.Count(), 2);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 2);
        EXPECT_EQ(result, 1);
    }

    {
        auto q = pool.New();
        auto w = pool.New();
        EXPECT_EQ(pool.Count(), 2);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 2);
        EXPECT_EQ(result, 1);
        {
            auto p = pool.New();
            EXPECT_EQ(pool.Count(), 3);
            EXPECT_EQ(pool.FreeCount(), 0);
            EXPECT_EQ(pool.AccCount(), 3);
            EXPECT_EQ(result, 3);
        }
        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 1);
        EXPECT_EQ(pool.AccCount(), 2);
        EXPECT_EQ(result, 1);
        {
            auto p = pool.New();
            EXPECT_EQ(pool.Count(), 3);
            EXPECT_EQ(pool.FreeCount(), 0);
            EXPECT_EQ(pool.AccCount(), 3);
            EXPECT_EQ(result, 3);
        }
    }

    EXPECT_EQ(pool.Count(), 3);
    EXPECT_EQ(pool.FreeCount(), 3);
    EXPECT_EQ(pool.AccCount(), 0);
    EXPECT_EQ(result, 0);

    {
        auto q = pool.New();
        auto w = pool.New();
        pool.ShrinkToFit();
        EXPECT_EQ(pool.Count(), 2);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 2);
        EXPECT_EQ(result, 1);
    }

    EXPECT_EQ(pool.Count(), 2);
    EXPECT_EQ(pool.FreeCount(), 2);
    EXPECT_EQ(pool.AccCount(), 0);
    EXPECT_EQ(result, 0);

    {
        auto q = pool.New();
        auto w = pool.New();
        auto p = pool.New();
        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 3);
        EXPECT_EQ(result, 3);

        auto u {p};

        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 4);
        EXPECT_EQ(result, 3);

        auto e {std::move(p)};

        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 4);
        EXPECT_EQ(result, 3);
        EXPECT_FALSE(p);
        EXPECT_TRUE(u);

        p = e;

        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 0);
        EXPECT_EQ(pool.AccCount(), 5);
        EXPECT_EQ(result, 3);
        EXPECT_TRUE(p);

        q = e;

        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 1);
        EXPECT_EQ(pool.AccCount(), 5);
        EXPECT_EQ(result, 3);

        w = std::move(e);

        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 2);
        EXPECT_EQ(pool.AccCount(), 4);
        EXPECT_EQ(result, 2);
        EXPECT_FALSE(e);

        EXPECT_EQ((*w).a, 2);
    }

    EXPECT_EQ(pool.Count(), 3);
    EXPECT_EQ(pool.FreeCount(), 3);
    EXPECT_EQ(pool.AccCount(), 0);
    EXPECT_EQ(result, 0);

    pool.ShrinkToFit();
    EXPECT_EQ(pool.Count(), 0);
    EXPECT_EQ(pool.FreeCount(), 0);
    EXPECT_EQ(pool.AccCount(), 0);
    EXPECT_EQ(result, 0);

    {
        auto q = pool.New();
        auto w = pool.New();
        auto p = pool.New();
        q = std::move(w);
        auto e = p;

        EXPECT_EQ(pool.Count(), 3);
        EXPECT_EQ(pool.FreeCount(), 1);
        EXPECT_EQ(pool.AccCount(), 3);
        EXPECT_EQ(result, 3);

        int sum = 0;
        int prod = 1;
        int num = 0;

        while (auto acc = pool.AllObjects()()) {
            const S &obj = *(*acc);
            sum += obj.a;
            prod *= obj.a;
            ++num;
        }

        EXPECT_EQ(sum, 3);
        EXPECT_EQ(prod, 2);
        EXPECT_EQ(num, 2);
    }

    EXPECT_EQ(pool.Count(), 3);
    EXPECT_EQ(pool.FreeCount(), 3);
    EXPECT_EQ(pool.AccCount(), 0);
    EXPECT_EQ(result, 0);
}

}  // namespace panda::verifier::test
