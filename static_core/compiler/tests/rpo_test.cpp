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
#include <vector>
#include "optimizer/analysis/rpo.h"

namespace panda::compiler {
class RpoTest : public GraphTest {
public:
    void CheckSubsequence(const std::vector<BasicBlock *> &&subsequence)
    {
        auto subseq_iter = subsequence.begin();
        for (auto block : GetGraph()->GetBlocksRPO()) {
            if (block == *subseq_iter) {
                if (++subseq_iter == subsequence.end()) {
                    break;
                }
            }
        }
        EXPECT_TRUE(subseq_iter == subsequence.end());
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RpoTest, OneBlock)
{
    auto block = GetGraph()->CreateStartBlock();
    CheckSubsequence({block});
}

/*
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *                    |       \
 *                    v       v
 *                   [B]  <- [C]
 *                    |       |
 *                    v       v
 *                   [D]  <- [E]
 *                    |
 *                    v
 *                  [exit]
 *
 * Add [M], [N], [K]:
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *            /       |       \
 *            v       v       v
 *           [T]  -> [B]  <- [C]
 *                    |       |
 *                    v       v
 *                   [D]  <- [E] -> [N]
 *                    |              |
 *                    v              |
 *                   [M]             |
 *                    |              |
 *                    v              |
 *                   [K]             |
 *                    |              |
 *                    v              |
 *                  [exit] <---------/
 */
TEST_F(RpoTest, GraphNoCycles)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(4, 3, 6)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(6, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    auto a = &BB(2);
    auto b = &BB(3);
    auto c = &BB(4);
    auto d = &BB(5);
    auto e = &BB(6);
    auto exit = GetGraph()->GetEndBlock();

    CheckSubsequence({a, b, d});
    CheckSubsequence({a, c, e, d});
    CheckSubsequence({a, c, b, d});

    auto m = GetGraph()->CreateEmptyBlock();
    auto n = GetGraph()->CreateEmptyBlock();
    auto ret1 = GetGraph()->CreateInstReturnVoid();
    n->AppendInst(ret1);
    auto k = GetGraph()->CreateEmptyBlock();
    auto ret2 = GetGraph()->CreateInstReturnVoid();
    k->AppendInst(ret2);
    d->AddSucc(m);
    d->RemoveSucc(exit);
    d->RemoveInst(&INS(9));
    exit->RemovePred(d);
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, &INS(0), &INS(1), DataType::Type::INT64, CC_NE);
    e->AppendInst(cmp);
    auto if_inst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, cmp, 0, DataType::BOOL, CC_NE);
    e->AppendInst(if_inst);
    e->AddSucc(n);
    m->AddSucc(k);
    k->AddSucc(exit);
    n->AddSucc(exit);
    // Check handle tree update
    EXPECT_FALSE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    ArenaVector<BasicBlock *> added_blocks({m, k}, GetGraph()->GetAllocator()->Adapter());
    GetGraph()->GetAnalysis<Rpo>().AddVectorAfter(d, added_blocks);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(e, n);

    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();
    CheckSubsequence({a, b, d, m, k});
    CheckSubsequence({a, c, b, d, m, k});
    CheckSubsequence({a, c, e, d, m, k});
    CheckSubsequence({a, c, e, n});

    // Check tree rebuilding
    EXPECT_TRUE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(false);
    CheckSubsequence({a, b, d, m, k});
    CheckSubsequence({a, c, b, d, m, k});
    CheckSubsequence({a, c, e, d, m, k});
    CheckSubsequence({a, c, e, n});
}

/*
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *                    |       \
 *                    v       v
 *                   [B]     [C] <- [M]
 *                    |       |      ^
 *                    V       v     /
 *           [G]<--> [D]  <- [E] --/
 *                    |
 *                    v
 *                  [exit]
 *
 *
 * Add [N], [K]
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *                    |       \
 *                    v       v
 *                   [B]     [C] <- [M]
 *                    |       |      ^
 *                    V       v     /
 *    [N] <- [G]<--> [D]  <- [E] --/
 *     |      ^       |
 *     |     /        v
 *     |    /        [L]
 *     |   /          |
 *     v  /           v
 *    [K]/          [exit]
 */
TEST_F(RpoTest, GraphWithCycles)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(4, 6) {}
        BASIC_BLOCK(6, 5, 7)
        {
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(7, 4) {}
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(5, 9, 8)
        {
            INST(9, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(11, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(8, 5) {}
    }
    auto a = &BB(2);
    auto b = &BB(3);
    auto c = &BB(4);
    auto d = &BB(5);
    auto e = &BB(6);
    auto g = &BB(7);
    auto m = &BB(8);
    auto l = &BB(9);
    auto exit = GetGraph()->GetEndBlock();

    // FIXME {A, B, T, D, exit} doesn't work
    CheckSubsequence({a, b, d, l, exit});
    CheckSubsequence({a, c, e, d, l, exit});
    CheckSubsequence({a, c, e, m, l});

    auto n = GetGraph()->CreateEmptyBlock();
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, &INS(0), &INS(1), DataType::Type::INT64, CC_NE);
    g->AppendInst(cmp);
    auto if_inst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, cmp, 0, DataType::BOOL, CC_NE);
    g->AppendInst(if_inst);
    auto k = GetGraph()->CreateEmptyBlock();
    g->AddSucc(n);
    n->AddSucc(k);
    k->AddSucc(g);

    // Check handle tree update
    EXPECT_FALSE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(g, n);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(n, k);
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();

    CheckSubsequence({a, b, d, l, exit});
    CheckSubsequence({a, c, e, d, l, exit});
    CheckSubsequence({a, c, e, m});

    // Check tree rebuilding
    EXPECT_TRUE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(false);
    CheckSubsequence({a, b, d, l, exit});
    CheckSubsequence({a, c, e, d, l, exit});
    CheckSubsequence({a, c, e, m});
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
