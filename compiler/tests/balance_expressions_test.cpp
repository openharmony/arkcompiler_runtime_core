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

#include "unit_test.h"
#include "optimizer/optimizations/balance_expressions.h"

namespace panda::compiler {
class BalanceExpressionsTest : public GraphTest {
};

TEST_F(BalanceExpressionsTest, AddMulParallel)
{
    // Check that independent expression are not mixed with each other and being considered sequentially:
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).u64();
        PARAMETER(5, 5).u64();
        PARAMETER(6, 6).u64();
        PARAMETER(7, 7).u64();

        /**
         * From:
         *  (((((((a + b) + c) + d) + e) + f) + g) + h)
         *  interlined with
         *  (((((((a * b) * c) * d) * e) * f) * g) * h)
         *
         *  (Critical path is 8)
         */
        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Add).u64().Inputs(0, 1);
            INST(9, Opcode::Mul).u64().Inputs(0, 1);

            INST(10, Opcode::Add).u64().Inputs(8, 2);
            INST(11, Opcode::Mul).u64().Inputs(9, 2);

            INST(12, Opcode::Add).u64().Inputs(10, 3);
            INST(13, Opcode::Mul).u64().Inputs(11, 3);

            INST(14, Opcode::Add).u64().Inputs(12, 4);
            INST(15, Opcode::Mul).u64().Inputs(13, 4);

            INST(16, Opcode::Add).u64().Inputs(14, 5);
            INST(17, Opcode::Mul).u64().Inputs(15, 5);

            INST(18, Opcode::Add).u64().Inputs(16, 6);
            INST(19, Opcode::Mul).u64().Inputs(17, 6);

            INST(20, Opcode::Add).u64().Inputs(18, 7);
            INST(21, Opcode::Mul).u64().Inputs(19, 7);

            INST(22, Opcode::Mul).u64().Inputs(21, 20);
            INST(23, Opcode::Return).u64().Inputs(22);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());
    ASSERT_TRUE(CheckUsers(INS(20), {22}));
    ASSERT_TRUE(CheckUsers(INS(22), {23}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).u64();
        PARAMETER(5, 5).u64();
        PARAMETER(6, 6).u64();
        PARAMETER(7, 7).u64();

        /**
         * To:
         *  (((a + b) + (c + d)) + ((e + f) + (g + h)))
         *  followed by
         *  (((a * b) * (c * d)) * ((e * f) * (g * h)))
         *
         *  (Critical path is 4)
         */
        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Add).u64().Inputs(0, 1);
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(12, Opcode::Add).u64().Inputs(8, 10);
            INST(14, Opcode::Add).u64().Inputs(4, 5);
            INST(16, Opcode::Add).u64().Inputs(6, 7);
            INST(18, Opcode::Add).u64().Inputs(14, 16);
            INST(20, Opcode::Add).u64().Inputs(12, 18);

            INST(9, Opcode::Mul).u64().Inputs(0, 1);
            INST(11, Opcode::Mul).u64().Inputs(2, 3);
            INST(13, Opcode::Mul).u64().Inputs(9, 11);
            INST(15, Opcode::Mul).u64().Inputs(4, 5);
            INST(17, Opcode::Mul).u64().Inputs(6, 7);
            INST(19, Opcode::Mul).u64().Inputs(15, 17);
            INST(21, Opcode::Mul).u64().Inputs(13, 19);

            INST(22, Opcode::Mul).u64().Inputs(21, 20);
            INST(23, Opcode::Return).u64().Inputs(22);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(BalanceExpressionsTest, MultipleUsers)
{
    // Instruction with more than one user may create side effects, so it is needed to check that they are considered as
    // sources (thus would not be modified).
    // Also checks that the last operator of an expression has the same users as before its optimization:
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).u64();
        PARAMETER(5, 5).u64();
        PARAMETER(6, 6).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Add).u64().Inputs(0, 1);
            INST(9, Opcode::Add).u64().Inputs(2, 8);

            // Has multiple users:
            INST(10, Opcode::Add).u64().Inputs(3, 9);

            INST(11, Opcode::Add).u64().Inputs(4, 10);
            INST(12, Opcode::Add).u64().Inputs(5, 6);
            INST(13, Opcode::Add).u64().Inputs(11, 12);
            INST(14, Opcode::Add).u64().Inputs(10, 13);

            INST(15, Opcode::Return).u64().Inputs(14);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());

    // The same users as we expect that the second expression would be unchanged
    ASSERT_TRUE(CheckUsers(INS(10), {11, 14}));
    ASSERT_TRUE(CheckUsers(INS(14), {15}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).u64();
        PARAMETER(5, 5).u64();
        PARAMETER(6, 6).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Add).u64().Inputs(3, 2);
            INST(9, Opcode::Add).u64().Inputs(0, 1);

            // Has multiple users:
            INST(10, Opcode::Add).u64().Inputs(8, 9);

            INST(11, Opcode::Add).u64().Inputs(4, 10);
            INST(12, Opcode::Add).u64().Inputs(5, 6);
            INST(13, Opcode::Add).u64().Inputs(11, 12);
            INST(14, Opcode::Add).u64().Inputs(10, 13);

            INST(15, Opcode::Return).u64().Inputs(14);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(BalanceExpressionsTest, SameSource)
{
    // Check that expression with repeated sources are handled correctly:
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Add).u64().Inputs(0, 0);
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);

            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());
    ASSERT_TRUE(CheckUsers(INS(3), {4}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Add).u64().Inputs(0, 0);
            INST(2, Opcode::Add).u64().Inputs(0, 0);
            INST(3, Opcode::Add).u64().Inputs(1, 2);

            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(BalanceExpressionsTest, OddSources)
{
    // Check that expression with odd number of sources are handled correctly:
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).u64();

        /**
         * From:
         *  (a + (e + ((c + d) + b)))
         *
         *  (Critical path is 4)
         */
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::Add).u64().Inputs(2, 3);
            INST(6, Opcode::Add).u64().Inputs(5, 1);
            INST(7, Opcode::Add).u64().Inputs(4, 6);
            INST(8, Opcode::Add).u64().Inputs(0, 7);

            INST(9, Opcode::Return).u64().Inputs(8);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());
    ASSERT_TRUE(CheckUsers(INS(8), {9}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).u64();

        /**
         * To:
         *  (((a + e) + (c + d)) + b)
         *
         *  (Critical path is 3)
         */
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 4);
            INST(6, Opcode::Add).u64().Inputs(2, 3);
            INST(7, Opcode::Add).u64().Inputs(5, 6);
            INST(8, Opcode::Add).u64().Inputs(7, 1);

            INST(9, Opcode::Return).u64().Inputs(8);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
}  // namespace panda::compiler
