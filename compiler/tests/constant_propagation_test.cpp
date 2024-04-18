/*
 * Copyright (c) 2024 Shenzhen Kaihong Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "compiler/optimizer/ir/basicblock.h"
#include "graph.h"
#include "graph_test.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/constant_propagation/constant_propagation.h"

using namespace testing::ext;

namespace panda::compiler {
class ConstantProgagationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

enum class ExpectType {
    EXPECT_LDTRUE,
    EXPECT_LDFALSE,
    EXPECT_BOTH_TRUE_FALSE,
    EXPECT_CONST,
    EXPECT_PHI,
    EXPECT_OTHER,
};

static void VisitBlock(BasicBlock *bb, ExpectType type)
{
    if (!bb) {
        return;
    }
    for (auto inst : bb->Insts()) {
        if (!inst->IsIntrinsic() ||
            inst->CastToIntrinsic()->GetIntrinsicId() != RuntimeInterface::IntrinsicId::CALLARG1_IMM8_V8) {
            continue;
        }
        auto input = inst->GetInput(0).GetInst();

        switch (type) {
            case ExpectType::EXPECT_CONST:
                ASSERT_TRUE(input->IsConst());
                break;
            case ExpectType::EXPECT_PHI:
                ASSERT_TRUE(input->IsPhi());
                break;
            case ExpectType::EXPECT_LDTRUE: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDTRUE);
                break;
            }
            case ExpectType::EXPECT_LDFALSE: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDFALSE);
                break;
            }
            case ExpectType::EXPECT_BOTH_TRUE_FALSE: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDFALSE ||
                            id == RuntimeInterface::IntrinsicId::LDTRUE);
                break;
            }
            case ExpectType::EXPECT_OTHER: {
                ASSERT_TRUE(input->IsIntrinsic());
                auto id = input->CastToIntrinsic()->GetIntrinsicId();
                EXPECT_TRUE(id != RuntimeInterface::IntrinsicId::LDFALSE &&
                            id != RuntimeInterface::IntrinsicId::LDTRUE);
                break;
            }
        }
    }
}

static void VisitBlockCheckIf(BasicBlock *bb, bool optimize = false)
{
    if (!bb) {
        return;
    }
    for (auto inst : bb->Insts()) {
        if (inst->GetOpcode() != Opcode::IfImm) {
            continue;
        }
        auto input = inst->GetInput(0).GetInst();
        if (!optimize) {
            EXPECT_TRUE(input->GetOpcode() == Opcode::Compare);
        } else {
            auto id = input->CastToIntrinsic()->GetIntrinsicId();
            EXPECT_TRUE(id == RuntimeInterface::IntrinsicId::LDFALSE || id == RuntimeInterface::IntrinsicId::LDTRUE);
        }
    }
}

/**
 * @tc.name: constant_progagation_test_001
 * @tc.desc: Verify the pass to fold greater.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_001, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreater";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_002
 * @tc.desc: Verify the pass to fold greater but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_002, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreaterNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_003
 * @tc.desc: Verify the pass to fold greatereq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_003, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreaterEq";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_004
 * @tc.desc: Verify the pass to fold greatereq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_004, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldGreaterEqNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_005
 * @tc.desc: Verify the pass to fold less.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_005, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLess";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_006
 * @tc.desc: Verify the pass to fold less but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_006, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLessNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_007
 * @tc.desc: Verify the pass to fold lesseq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_007, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLessEq";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_008
 * @tc.desc: Verify the pass to fold lesseq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_008, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldLessEqNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_009
 * @tc.desc: Verify the pass to fold eq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_009, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldEq";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_010
 * @tc.desc: Verify the pass to fold eq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_010, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldEqNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_011
 * @tc.desc: Verify the pass to fold stricteq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_011, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEq";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_012
 * @tc.desc: Verify the pass to fold eq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_012, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEqNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_013
 * @tc.desc: Verify the pass to fold noteq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_013, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEq";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_014
 * @tc.desc: Verify the pass to fold noteq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_014, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictEqNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_015
 * @tc.desc: Verify the pass to fold strictnoteq.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_015, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictNotEq";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_016
 * @tc.desc: Verify the pass to fold strictnoteq but has no effect.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_016, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpFoldStrictNotEqNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }

        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        graph->RunPass<Cleanup>();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_017
 * @tc.desc: Verify the pass without any effect with nan.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_017, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpNoEffectNan";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_018
 * @tc.desc: Verify the optimizion to Compare.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_018, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpIfCheck";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlockCheckIf(bb, false);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlockCheckIf(bb, true);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_019
 * @tc.desc: Verify the optimizion to Phi.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_019, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpPhi";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_PHI);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_BOTH_TRUE_FALSE);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_020
 * @tc.desc: Verify the optimizion to Phi.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_020, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpPhiNoEffect";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_PHI);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_PHI);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_021
 * @tc.desc: Verify the optimizion to Phi.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_021, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpPhi2";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_PHI);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_CONST);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_022
 * @tc.desc: Verify the pass without any effect with inf.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_022, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpNoEffectInfinity";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: constant_progagation_test_023
 * @tc.desc: Verify the pass without any effect with math calculation.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ConstantProgagationTest, constant_progagation_test_023, TestSize.Level1)
{
    std::string file = GRAPH_TEST_ABC_DIR "constantProgagation.abc";
    const char *test_method_name = "sccpNoEffectArithmetic";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(file, [test_method_name, &status](Graph *graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
        auto &bbs = graph->GetVectorBlocks();
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
        EXPECT_TRUE(graph->RunPass<ConstantPropagation>());
        for (auto bb : bbs) {
            VisitBlock(bb, ExpectType::EXPECT_OTHER);
        }
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler
