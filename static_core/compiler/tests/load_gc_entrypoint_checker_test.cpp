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

#include "tests/unit_test.h"

#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/graph_cloner.h"

#include <gtest/gtest.h>

namespace ark::compiler {
class LoadGCEntrypointCheckerTest : public GraphTest {
public:
    enum NeedBarrier : uint8_t { NEED_BARRIER, DONT_NEED_BARRIER };

    enum Type : uint8_t { REF, PRIMITIVE };

    enum GCBarrierUseStrategy : uint8_t {
        CORRECT_USE,
        DIRECT_AS_REF_INPUT_ONLY,
        DIRECT_AS_VALUE_INPUT_ONLY,
        INDIRECT_AS_REF_INPUT_ONLY,
        DIRECT_AS_REF_INDIRECT_GC,
        DIRECT_AS_REF_DIRECT_GC,
        INDIRECT_AS_REF_INDIRECT_GC,
        INDIRECT_AS_REF_DIRECT_GC,
        NEW_AS_REF_INDIRECT_GC,
        INDIRECT_AS_REF_NEW_GC
    };

protected:
    void SetUp() override
    {
        builder_->EnableGraphChecker(false);
    }

    static void EnableAddressArithmetic(Graph *graph)
    {
        // Address arithmetic is allowed for fast-path.
        graph->SetMode(GraphMode(0) | GraphMode::FastPath(true));
    }
};

// NOLINTBEGIN(readability-magic-numbers)
#ifdef COMPILER_DEBUG_CHECKS
// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithOneLevelPhi, Graph *graph, std::optional<Opcode> opc, unsigned gcbarrierUser,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType1,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType2, LoadGCEntrypointCheckerTest::Type type)
{
    ASSERT(!opc.has_value() || opc == Opcode::Load || opc == Opcode::LoadArray || opc == Opcode::Store ||
           opc == Opcode::StoreArray);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            if (barrierType1) {
                INST(6U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType1);
            } else {
                INST(6U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(4U, 5U)
        {
            if (barrierType2) {
                INST(7U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType2);
            } else {
                INST(7U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Phi).ptr().Inputs(6U, 7U);
            if (opc.has_value()) {
                switch (type) {
                    case LoadGCEntrypointCheckerTest::REF:
                        INST(9U, Opcode::Load).ref().Inputs(0U, 1U);
                        break;
                    case LoadGCEntrypointCheckerTest::PRIMITIVE:
                        INST(9U, Opcode::Load).i32().Inputs(0U, 1U);
                        break;
                }
            }
            if (opc == Opcode::Load || opc == Opcode::LoadArray) {
                switch (type) {
                    case LoadGCEntrypointCheckerTest::REF:
                        INST(gcbarrierUser, *opc).ref().SetNeedReadBarrier(true).Inputs(9U, 1U, 8U);
                        break;
                    case LoadGCEntrypointCheckerTest::PRIMITIVE:
                        INST(gcbarrierUser, *opc).i32().SetNeedReadBarrier(true).Inputs(0U, 1U, 8U);
                        break;
                }
            } else if (opc == Opcode::Store || opc == Opcode::StoreArray) {
                switch (type) {
                    case LoadGCEntrypointCheckerTest::REF:
                        INST(gcbarrierUser, *opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 9U, 8U);
                        break;
                    case LoadGCEntrypointCheckerTest::PRIMITIVE:
                        INST(gcbarrierUser, *opc).i32().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 9U, 8U);
                        break;
                }
            }
            switch (type) {
                case LoadGCEntrypointCheckerTest::REF:
                    INST(11U, Opcode::Return).ref().Inputs(opc.has_value() ? 9U : 8U);
                    break;
                case LoadGCEntrypointCheckerTest::PRIMITIVE:
                    INST(11U, Opcode::Return).i32().Inputs(opc.has_value() ? 9U : 8U);
                    break;
            }
        }
    }
}

SRC_GRAPH(InvalidGraphNonPointerGCBarrier, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::Load).ref().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Load).ref().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Phi).ref().Inputs(6U, 7U);
            INST(9U, Opcode::Load).ref().Inputs(0U, 1U);
            // 8U is the GC entrypoint input, it must have the type of `DataType::POINTER`.
            INST(10U, Opcode::LoadArray).ref().SetNeedReadBarrier(true).Inputs(9U, 1U, 8U);
            INST(11U, Opcode::Return).ref().Inputs(10U);
        }
    }
}

TEST_F(LoadGCEntrypointCheckerTest, InvalidGraphNonPointerGCBarrier)
{
    src_graph::InvalidGraphNonPointerGCBarrier::CREATE(GetGraph());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithLoadAndPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::Load, 10U, LoadGCEntrypointInst::BarrierType::READ,
                                            LoadGCEntrypointInst::BarrierType::READ, REF);
    ASSERT_TRUE(INS(10U).CastToLoad()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithStoreArrayAndPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::StoreArray, 10U,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE, REF);
    ASSERT_TRUE(INS(10U).CastToStoreArray()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());

    InstSetNeedPostWriteBarrier(&INS(10U), true);  // doesn't matter
    ASSERT_TRUE(INS(10U).CastToStoreArray()->GetNeedPostWriteBarrier());
    GraphChecker checker2(GetGraph());
    ASSERT_TRUE(checker2.Check());

    InstSetNeedPreWriteBarrier(&INS(10U), false);
    ASSERT_FALSE(INS(10U).CastToStoreArray()->GetNeedPreWriteBarrier());  // pre-write barrier is a must.
    ASSERT_TRUE(INS(10U).CastToStoreArray()->GetNeedWriteBarrier());      // doesn't matter without the pre-write one.
    ASSERT_TRUE(INS(10U).CastToStoreArray()->GetNeedPostWriteBarrier());
    GraphChecker checker3(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker3.Check(), "");
#else
    ASSERT_FALSE(checker3.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithAny)
{
    GetGraph()->SetDynamicMethod();
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::StoreObjectDynamic)
                .any()
                .Inputs(0U, 1U, 0U, 1U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::ARRAY);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(INS(2U).CastToStoreObjectDynamic()->IsCompilerBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNeedReadBarrierForNotRefOrAnyAfterPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::LoadArray, 10U, LoadGCEntrypointInst::BarrierType::READ,
                                            LoadGCEntrypointInst::BarrierType::READ, PRIMITIVE);
    ASSERT_TRUE(INS(10U).CastToLoadArray()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

SRC_GRAPH(GraphWithNoNeedBarrierStoreArray, Graph *graph, unsigned storeArray, LoadGCEntrypointCheckerTest::Type type)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::Add).i32().Inputs(1U, 2U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).i32().Inputs(1U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Phi).i32().Inputs(7U, 8U);
            switch (type) {
                case LoadGCEntrypointCheckerTest::REF:
                    INST(storeArray, Opcode::StoreArray).ref().Inputs(0U, 9U, 0U);
                    break;
                case LoadGCEntrypointCheckerTest::PRIMITIVE:
                    INST(storeArray, Opcode::StoreArray).i32().Inputs(0U, 9U, 9U);
                    break;
            }
            INST(11U, Opcode::Return).i32().Inputs(9U);
        }
    }
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoNeedReadBarrierForRef)
{
    src_graph::GraphWithNoNeedBarrierStoreArray::CREATE(GetGraph(), 10U, LoadGCEntrypointCheckerTest::REF);
    INS(10U).CastToStoreArray()->SetNeedWriteBarrier(false);
    ASSERT_FALSE(INS(10U).CastToStoreArray()->GetNeedWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithNoNeedReadBarrierForNonRef)
{
    src_graph::GraphWithNoNeedBarrierStoreArray::CREATE(GetGraph(), 10U, LoadGCEntrypointCheckerTest::PRIMITIVE);
    INS(10U).CastToStoreArray()->SetNeedWriteBarrier(false);
    ASSERT_FALSE(INS(10U).CastToStoreArray()->GetNeedWriteBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNeedWriteBarrierForNotRefOrAnyAfterPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::StoreArray, 10U,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE, PRIMITIVE);
    ASSERT_TRUE(INS(10U).CastToStoreArray()->GetNeedWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithPhiAndSingleStoreGCEntrypoint)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::Add).ptr().Inputs(0U, 2U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).ptr().Inputs(0U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Phi).ptr().Inputs(7U, 8U);
            INST(10U, Opcode::Store).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 9U, 4U);
            INST(11U, Opcode::Return).ref().Inputs(9U);
        }
    }
    ASSERT_TRUE(INS(10U).CastToStore()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithUnresolvedStoreStatic, Graph *graph, unsigned saveUnresolvedId,
          LoadGCEntrypointCheckerTest::GCBarrierUseStrategy useStrategy)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint)
                .ptr()
                .SetBarrierType(useStrategy == LoadGCEntrypointCheckerTest::CORRECT_USE
                                    ? LoadGCEntrypointInst::BarrierType::READ
                                    : LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::Add).i32().Inputs(1U, 2U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).i32().Inputs(1U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Phi).i32().Inputs(7U, 8U);
            INST(10U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            switch (useStrategy) {
                case LoadGCEntrypointCheckerTest::CORRECT_USE:
                    INST(saveUnresolvedId + 1U, Opcode::LoadArray).ref().SetNeedReadBarrier(true).Inputs(0U, 1U, 4U);
                    INST(saveUnresolvedId, Opcode::UnresolvedStoreStatic)
                        .ref()
                        .Inputs(saveUnresolvedId + 1U, 10U)
                        .SetFlag(compiler::inst_flags::REQUIRE_STATE);
                    break;
                case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INPUT_ONLY:
                    INST(saveUnresolvedId, Opcode::UnresolvedStoreStatic)
                        .ref()
                        .Inputs(4U, 10U)
                        .SetFlag(compiler::inst_flags::REQUIRE_STATE);
                    break;
                default:
                    UNREACHABLE();
            }
            INST(saveUnresolvedId + 2U, Opcode::Return).i32().Inputs(9U);
        }
    }
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithUnresolvedStoreStatic)
{
    src_graph::GraphWithUnresolvedStoreStatic::CREATE(GetGraph(), 11U, CORRECT_USE);
    ASSERT_TRUE(INS(11U).CastToUnresolvedStoreStatic()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithUnresolvedStoreStaticToLoadGCEntrypoint)
{
    src_graph::GraphWithUnresolvedStoreStatic::CREATE(GetGraph(), 11U, DIRECT_AS_REF_INPUT_ONLY);
    ASSERT_TRUE(INS(11U).CastToUnresolvedStoreStatic()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithLinearOperation, Graph *graph, Opcode opc, unsigned gcbarrierUser,
          LoadGCEntrypointCheckerTest::NeedBarrier needBarrier,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType,
          LoadGCEntrypointCheckerTest::GCBarrierUseStrategy barrierUseStrategy)
{
    ASSERT(opc == Opcode::Load || opc == Opcode::LoadArray || opc == Opcode::Store || opc == Opcode::StoreArray);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U).i32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            if (barrierType) {
                INST(3U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType);
            } else {
                INST(3U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
            if (opc == Opcode::Load || opc == Opcode::LoadArray) {
                switch (barrierUseStrategy) {
                    case LoadGCEntrypointCheckerTest::CORRECT_USE:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedReadBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(0U, 1U, 3U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INPUT_ONLY:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedReadBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(3U, 1U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_DIRECT_GC:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedReadBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(3U, 1U, 3U);
                        break;
                    default:
                        UNREACHABLE();
                }
                INST(5U, Opcode::Return).ref().Inputs(4U);
            } else if (opc == Opcode::Store || opc == Opcode::StoreArray) {
                switch (barrierUseStrategy) {
                    case LoadGCEntrypointCheckerTest::CORRECT_USE:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedPreWriteBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(0U, 1U, 0U, 3U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INPUT_ONLY:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedPreWriteBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(3U, 1U, 0U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_VALUE_INPUT_ONLY:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedPreWriteBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(0U, 1U, 3U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_DIRECT_GC:
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedPreWriteBarrier(needBarrier == LoadGCEntrypointCheckerTest::NEED_BARRIER)
                            .Inputs(0U, 1U, 3U, 3U);
                        break;
                    default:
                        UNREACHABLE();
                }
                INST(5U, Opcode::ReturnVoid);
            } else {
                UNREACHABLE();
            }
        }
    }
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithLoad)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Load, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    ASSERT_TRUE(INS(4U).CastToLoad()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithLoadFromLoadGCEntrypoint)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Load, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, DIRECT_AS_REF_INPUT_ONLY);
    ASSERT_TRUE(INS(4U).CastToLoad()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoNeedBarrierLoad)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Load, 4U, DONT_NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    ASSERT_FALSE(INS(4U).CastToLoad()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoNeedBarrierLoadArray)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::LoadArray, 4U, DONT_NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    ASSERT_FALSE(INS(4U).CastToLoadArray()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * For instructions with an optional GCBarrierEntrypoint input we must be sure GC barrier entrypoint
 * is only a LoadGCBarrierEntrypoint instruction or Phi.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGCBarrierEntrypointInput)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::LoadArray, 4U, NEED_BARRIER, std::nullopt,
                                                CORRECT_USE);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithTwoLevelsLoop, Graph *graph, Opcode opc, unsigned gcbarrierUser,
          LoadGCEntrypointInst::BarrierType barrierType1, LoadGCEntrypointInst::BarrierType barrierType2,
          LoadGCEntrypointInst::BarrierType barrierType3,
          LoadGCEntrypointCheckerTest::GCBarrierUseStrategy barrierUseStrategy)
{
    ASSERT(opc == Opcode::Load || opc == Opcode::LoadArray || opc == Opcode::Store || opc == Opcode::StoreArray);
    // pointer is an allowed input type for input 0 of Load but not of LoadArray.
    ASSERT(barrierUseStrategy == LoadGCEntrypointCheckerTest::CORRECT_USE || opc == Opcode::Load ||
           opc == Opcode::Store);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(barrierType1);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(9U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(barrierType2);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(barrierType3);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::Phi).ptr().Inputs(9U, 10U, 12U);
        }
        BASIC_BLOCK(7U, 6U, 8U)
        {
            INST(12U, Opcode::Phi).ptr().Inputs(4U, 11U);
            if (opc == Opcode::Load || opc == Opcode::LoadArray) {
                switch (barrierUseStrategy) {
                    case LoadGCEntrypointCheckerTest::CORRECT_USE:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INPUT_ONLY:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(4U, 2U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_INPUT_ONLY:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(12U, 2U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INDIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(4U, 2U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_DIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(4U, 2U, 4U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_INDIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(12U, 2U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_DIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedReadBarrier(true).Inputs(12U, 2U, 4U);
                        break;
                    default:
                        UNREACHABLE();
                }
            } else if (opc == Opcode::Store || opc == Opcode::StoreArray) {
                switch (barrierUseStrategy) {
                    case LoadGCEntrypointCheckerTest::CORRECT_USE:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 2U, 0U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_INPUT_ONLY:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(12U, 2U, 0U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INPUT_ONLY:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(4U, 2U, 0U);
                        break;
                    case LoadGCEntrypointCheckerTest::NEW_AS_REF_INDIRECT_GC:
                        INST(gcbarrierUser + 20U, Opcode::LoadGCEntrypoint)
                            .ptr()
                            .SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedPreWriteBarrier(true)
                            .Inputs(gcbarrierUser + 20U, 2U, 0U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_NEW_GC:
                        INST(gcbarrierUser + 20U, Opcode::LoadGCEntrypoint)
                            .ptr()
                            .SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
                        INST(gcbarrierUser, opc)
                            .ref()
                            .SetNeedPreWriteBarrier(true)
                            .Inputs(12U, 2U, 0U, gcbarrierUser + 20U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_INDIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 2U, 4U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::DIRECT_AS_REF_DIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 2U, 4U, 4U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_INDIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 2U, 12U, 12U);
                        break;
                    case LoadGCEntrypointCheckerTest::INDIRECT_AS_REF_DIRECT_GC:
                        INST(gcbarrierUser, opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 2U, 12U, 4U);
                        break;
                    default:
                        UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            INST(14U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            if (opc == Opcode::Load || opc == Opcode::LoadArray) {
                INST(16U, Opcode::Return).ref().Inputs(gcbarrierUser);
            } else {
                INST(16U, Opcode::ReturnVoid);
            }
        }
    }
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithLoadInLoop)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithLoadFromGCEntrypointInLoop)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ, INDIRECT_AS_REF_INPUT_ONLY);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithLoadFromGCEntrypointInLoop2)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ, DIRECT_AS_REF_INPUT_ONLY);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDirectLoadFromDifferentGCEntrypoint)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ, DIRECT_AS_REF_INDIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDirectLoadFromDifferentGCEntrypoint2)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ, INDIRECT_AS_REF_DIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithLoadFromSameGCEntrypoint)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Load, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, DIRECT_AS_REF_DIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithIndirectLoadFromSameGCEntrypoint)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::READ, INDIRECT_AS_REF_INDIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithStore)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Store, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    ASSERT_TRUE(INS(4U).CastToStore()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoNeedBarrierStore)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Store, 4U, DONT_NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    ASSERT_FALSE(INS(4U).CastToStore()->GetNeedWriteBarrier());
    ASSERT_FALSE(INS(4U).CastToStore()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectBarrierTypeForStore)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Store, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    ASSERT_TRUE(INS(4U).CastToStore()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectBarrierTypeForStoreArray)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::StoreArray, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    ASSERT_TRUE(INS(4U).CastToStoreArray()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithStoreInLoop)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(
        GetGraph(), Opcode::Store, 13U, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        LoadGCEntrypointInst::BarrierType::PRE_WRITE, LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithIndirectStoreDifferentGCEntrypoint)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Store, 13U,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE, INDIRECT_AS_REF_NEW_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithIndirectStoreDifferentGCEntrypoint2)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Store, 13U,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE, NEW_AS_REF_INDIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithStoreSameGCEntrypoint)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Store, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE, DIRECT_AS_REF_DIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithIndirectStoreSameGCEntrypoint)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(
        GetGraph(), Opcode::Store, 13U, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        LoadGCEntrypointInst::BarrierType::PRE_WRITE, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        INDIRECT_AS_REF_INDIRECT_GC);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We would like to allow Phi usages in Load/Store only when the inputs of the Phi are LoadGCEntrypoint instructions
 * with the same barrier type.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInPhiInLoop)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(
        GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We would like to allow Phi usages in Load/Store only when the inputs of the Phi are LoadGCEntrypoint instructions
 * with the same barrier type.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInPhiInLoop2)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(GetGraph(), Opcode::Load, 13U, LoadGCEntrypointInst::BarrierType::READ,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                              LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectBarrierTypeForLoad)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Load, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectBarrierTypeForLoadArray)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::LoadArray, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithStoreToLoadGCEntrypoint)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Store, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE, DIRECT_AS_REF_INPUT_ONLY);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithStoreLoadGCEntrypointValue)
{
    src_graph::GraphWithLinearOperation::CREATE(GetGraph(), Opcode::Store, 4U, NEED_BARRIER,
                                                LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                                DIRECT_AS_VALUE_INPUT_ONLY);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * The Memory instruction must have the NeedBarriers flag even though the GCBarrierEntrypoint has been propagated
 * through a phi instruction.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoNeedBarriersForLoadArrayInLoop)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(
        GetGraph(), Opcode::LoadArray, 13U, LoadGCEntrypointInst::BarrierType::READ,
        LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ, CORRECT_USE);
    INS(13U).CastToLoadArray()->SetNeedReadBarrier(false);
    ASSERT_FALSE(INS(13U).CastToLoadArray()->GetNeedReadBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * The Memory instruction must have the NeedBarriers flag even though the GCBarrierEntrypoint has been propagated
 * through a phi instruction.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoNeedBarriersForStoreInLoop)
{
    src_graph::GraphWithTwoLevelsLoop::CREATE(
        GetGraph(), Opcode::Store, 13U, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        LoadGCEntrypointInst::BarrierType::PRE_WRITE, LoadGCEntrypointInst::BarrierType::PRE_WRITE, CORRECT_USE);
    INS(13U).CastToStore()->SetNeedPreWriteBarrier(false);
    ASSERT_FALSE(INS(13U).CastToStore()->GetNeedPreWriteBarrier());
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithJustReturnForOrdinaryLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U).i32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(3U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithJustReturnForIndirectOrdinaryLoad)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), std::nullopt, 10U, std::nullopt, std::nullopt, REF);
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithGCBarrierFromIndirectOrdinaryLoad)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::Load, 10U, std::nullopt, std::nullopt, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We do not except returning the value of LoadGCEntrypoint as the function's result.
 */
TEST_F(LoadGCEntrypointCheckerTest, GraphWithJustReturn)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(2U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
            INST(3U, Opcode::Return).ptr().Inputs(2U);
        }
    }
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We do not except returning the value of LoadGCEntrypoint as the function's result even indirectly (through a Phi).
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithJustReturnForIndirectGCEntrypoint)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), std::nullopt, 10U, LoadGCEntrypointInst::BarrierType::READ,
                                            LoadGCEntrypointInst::BarrierType::READ, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We do not except any pointer arithmetic on the result of a LoadGCEntrypoint instruction.
 */
TEST_F(LoadGCEntrypointCheckerTest, GraphWithArithUser)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U).i32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(3U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
            INST(4U, Opcode::Add).ptr().Inputs(3U, 1U);
            INST(5U, Opcode::Return).ptr().Inputs(4U);
        }
    }
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We would like to allow Phi usages in Load/Store only when the inputs of the Phi are LoadGCEntrypoint instructions
 * with the same barrier type.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInPhiWithNoFinalUser)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), std::nullopt, 10U, LoadGCEntrypointInst::BarrierType::READ,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We would like to allow Phi usages in Load/Store only when the inputs of the Phi are LoadGCEntrypoint instructions
 * with the same barrier type.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::Store, 10U, LoadGCEntrypointInst::BarrierType::READ,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * We would like to allow Phi usages in Load/Store only when the inputs of the Phi are LoadGCEntrypoint instructions
 * with the same barrier type.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInPhi2)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::Store, 10U,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                            LoadGCEntrypointInst::BarrierType::READ, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithThreeVariantPhis, Graph *graph, std::optional<Opcode> opc, unsigned gcbarrierUser,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType1,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType2,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType3)
{
    ASSERT(!opc.has_value() || opc == Opcode::Load || opc == Opcode::LoadArray || opc == Opcode::Store ||
           opc == Opcode::StoreArray);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 6U)
        {
            if (barrierType1.has_value()) {
                INST(6U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType1);
            } else {
                INST(6U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            if (barrierType2.has_value()) {
                INST(7U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType2);
            } else {
                INST(7U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            if (barrierType3.has_value()) {
                INST(10U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType3);
            } else {
                INST(10U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(11U, Opcode::Phi).ptr().Inputs(6U, 7U, 10U);
            if (opc == Opcode::Load || opc == Opcode::LoadArray) {
                INST(gcbarrierUser, *opc).ref().SetNeedReadBarrier(true).Inputs(0U, 1U, 11U);
            } else if (opc == Opcode::Store || opc == Opcode::StoreArray) {
                INST(gcbarrierUser, *opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 0U, 11U);
            }
            INST(13U, Opcode::Return).ref().Inputs(0U);
        }
    }
}

/*
 * We would like to allow Phi usages in Load/Store only when the inputs of the Phi are LoadGCEntrypoint instructions
 * with the same barrier type.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInThreeInputsOfPhi)
{
    src_graph::GraphWithThreeVariantPhis::CREATE(
        GetGraph(), Opcode::Store, 12U, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithSameBarrierTypesInPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::Store, 10U,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE,
                                            LoadGCEntrypointInst::BarrierType::PRE_WRITE, REF);
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

/*
 * All the phis between the 'LoadGCEntrypoint Rb' instructions and their final user are correct but the final user
 * is a 'Store' instruction that requires a pre-write GC barrier.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithSameButWrongBarrierTypesInPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), Opcode::Store, 10U, LoadGCEntrypointInst::BarrierType::READ,
                                            LoadGCEntrypointInst::BarrierType::READ, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * If Phi is a user of a LoadGCEntrypoint instruction, all the input of the Phi must be either LoadGCEntrypoint
 * instructions or phis.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentSourcesOfPhi)
{
    src_graph::GraphWithOneLevelPhi::CREATE(GetGraph(), std::nullopt, 0U, LoadGCEntrypointInst::BarrierType::READ,
                                            std::nullopt, REF);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * If '0.ptr Phi' is a user of a LoadGCEntrypoint instruction, the inputs of all the phis that are inputs of the '0.ptr
 * Phi' must be either LoadGCEntrypoint instructions with the same barrier type or Phis that are subjects to the same
 * rule.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentSourcesOfPhiOnSecondLevel)
{
    src_graph::GraphWithThreeVariantPhis::CREATE(GetGraph(), std::nullopt, 0U, std::nullopt,
                                                 LoadGCEntrypointInst::BarrierType::READ,
                                                 LoadGCEntrypointInst::BarrierType::READ);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentSourcesOfPhiOnSecondLevel2)
{
    src_graph::GraphWithThreeVariantPhis::CREATE(GetGraph(), std::nullopt, 0U, LoadGCEntrypointInst::BarrierType::READ,
                                                 std::nullopt, LoadGCEntrypointInst::BarrierType::READ);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentSourcesOfPhiOnSecondLevel3)
{
    src_graph::GraphWithThreeVariantPhis::CREATE(GetGraph(), std::nullopt, 0U, LoadGCEntrypointInst::BarrierType::READ,
                                                 LoadGCEntrypointInst::BarrierType::READ, std::nullopt);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithTwoLevelsPhi, Graph *graph, std::optional<Opcode> opc, unsigned gcbarrierUser,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType1,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType2,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType3,
          std::optional<LoadGCEntrypointInst::BarrierType> barrierType4)
{
    ASSERT(!opc.has_value() || opc == Opcode::Load || opc == Opcode::LoadArray || opc == Opcode::Store ||
           opc == Opcode::StoreArray);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 7U, 8U)
        {
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, 9U)
        {
            if (barrierType1.has_value()) {
                INST(10U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType1);
            } else {
                INST(10U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(6U, 9U)
        {
            if (barrierType2.has_value()) {
                INST(11U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType2);
            } else {
                INST(11U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(7U, 10U)
        {
            if (barrierType3.has_value()) {
                INST(12U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType3);
            } else {
                INST(12U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(8U, 10U)
        {
            if (barrierType4.has_value()) {
                INST(13U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(*barrierType4);
            } else {
                INST(13U, Opcode::Load).ptr().Inputs(0U, 1U);
            }
        }
        BASIC_BLOCK(9U, 11U)
        {
            INST(14U, Opcode::Phi).ptr().Inputs(10U, 11U);
        }
        BASIC_BLOCK(10U, 11U)
        {
            INST(15U, Opcode::Phi).ptr().Inputs(12U, 13U);
        }
        BASIC_BLOCK(11U, -1L)
        {
            INST(16U, Opcode::Phi).ptr().Inputs(14U, 15U);
            if (opc == Opcode::Load || opc == Opcode::LoadArray) {
                INST(gcbarrierUser, *opc).ref().SetNeedReadBarrier(true).Inputs(0U, 1U, 16U);
            } else if (opc == Opcode::Store || opc == Opcode::StoreArray) {
                INST(gcbarrierUser, *opc).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 0U, 16U);
            }
            INST(18U, Opcode::Return).ref().Inputs(0U);
        }
    }
}

/*
 * If '0.ptr Phi' is a user of a LoadGCEntrypoint instruction, the inputs of all the phis that are inputs of the '0.ptr
 * Phi' must be either LoadGCEntrypoint instructions with the same barrier type or Phis that are subjects to the same
 * rule.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentSourcesOfPhiOnSecondLevel4)
{
    src_graph::GraphWithTwoLevelsPhi::CREATE(GetGraph(), std::nullopt, 0U, LoadGCEntrypointInst::BarrierType::READ,
                                             LoadGCEntrypointInst::BarrierType::READ, std::nullopt, std::nullopt);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithSameBarrierTypesInSourcesOfPhiOnSecondLevel)
{
    src_graph::GraphWithThreeVariantPhis::CREATE(
        GetGraph(), Opcode::LoadArray, 12U, LoadGCEntrypointInst::BarrierType::READ,
        LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ);
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithSameBarrierTypesInSourcesOfPhiOnSecondLevel2)
{
    src_graph::GraphWithTwoLevelsPhi::CREATE(
        GetGraph(), Opcode::LoadArray, 17U, LoadGCEntrypointInst::BarrierType::READ,
        LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ,
        LoadGCEntrypointInst::BarrierType::READ);
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

/*
 * If '0.ptr Phi' is a user of a LoadGCEntrypoint instruction, the inputs of all the phis that are inputs of the '0.ptr
 * Phi' must be either LoadGCEntrypoint instructions with the same barrier type or Phis that are subjects to the same
 * rule.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInSourcesOfPhiOnSecondLevel)
{
    src_graph::GraphWithThreeVariantPhis::CREATE(
        GetGraph(), std::nullopt, 0U, LoadGCEntrypointInst::BarrierType::PRE_WRITE,
        LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * If '0.ptr Phi' is a user of a LoadGCEntrypoint instruction, all the inputs of the phis that are input of the '0.ptr
 * Phi' must be LoadGCEntrypoint instruction with the same barrier type or a Phi that is subject to the same rule.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDifferentBarrierTypesInSourcesOfPhiOnSecondLevel2)
{
    src_graph::GraphWithTwoLevelsPhi::CREATE(
        GetGraph(), std::nullopt, 0U, LoadGCEntrypointInst::BarrierType::READ, LoadGCEntrypointInst::BarrierType::READ,
        LoadGCEntrypointInst::BarrierType::PRE_WRITE, LoadGCEntrypointInst::BarrierType::PRE_WRITE);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * Only LoadGCEntrypoint instructions are an available source of the GC barrier entrypoint input for Loads and Stores
 * that use the GC barrier.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNoLoadGCEntrypointInGcBarrierSourcesOfPhiOnSecondLevel)
{
    src_graph::GraphWithTwoLevelsPhi::CREATE(GetGraph(), Opcode::Store, 17U, std::nullopt, std::nullopt, std::nullopt,
                                             std::nullopt);
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/*
 * SafePoint (8) is a runtime call on a path from LoadGCEntrypoint (4) and Store (11).
 */
// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNonDominatingRuntimeCallOnPath)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::Add).ptr().Inputs(0U, 1U);
            INST(8U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, Opcode::Sub).ptr().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).ptr().Inputs(7U, 9U);
            INST(11U, Opcode::Store).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 10U, 4U);
            INST(12U, Opcode::Return).ref().Inputs(10U);
        }
    }

    GetGraph()->RunPass<DominatorsTree>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<DominatorsTree>());
    decltype(auto) gcep = INS(4U);
    ASSERT_TRUE(gcep.Is(Opcode::LoadGCEntrypoint));
    decltype(auto) rtcall = INS(8U);
    decltype(auto) user = INS(11U);
    ASSERT_TRUE((user.IsStore() || user.IsLoad()) && GetInstGCBarrierEntrypoint(&user) == &gcep);
    ASSERT_TRUE(rtcall.IsRuntimeCall());  // SafePoint is a runtime call.

    EXPECT_TRUE(gcep.IsDominate(&user));
    EXPECT_TRUE(gcep.IsDominate(&rtcall));
    EXPECT_FALSE(rtcall.IsDominate(&user));

    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithNonDominatingRuntimeCallOnLongPath)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 8U)
        {
            INST(7U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(8U, Opcode::Add).ptr().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, Opcode::Sub).ptr().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::Sub).ptr().Inputs(9U, 1U);
        }
        BASIC_BLOCK(6U, 7U) {}
        BASIC_BLOCK(7U, 8U)
        {
            INST(11U, Opcode::Sub).ptr().Inputs(0U, 1U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(12U, Opcode::Phi).ptr().Inputs(8U, 11U);
            INST(13U, Opcode::Sub).ptr().Inputs(12U, 2U);
            INST(14U, Opcode::Store).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 13U, 4U);
            INST(15U, Opcode::Return).ref().Inputs(13U);
        }
    }

    ASSERT_TRUE(INS(7U).IsRuntimeCall());  // SafePoint is a runtime call.
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDominatingRuntimeCallOnLinearPath)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Call)
                .ptr()
                .Inputs({{DataType::POINTER, 0U}, {DataType::NO_TYPE, 5U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(7U, Opcode::Add).i32().Inputs(1U, 2U);
            INST(8U, Opcode::Sub).ptr().Inputs(6U, 7U);
            INST(9U, Opcode::Store).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 8U, 4U);
            INST(10U, Opcode::Return).ptr().Inputs(8U);
        }
    }

    ASSERT_TRUE(INS(6U).IsRuntimeCall());  // A not inlined Call is a runtime call.
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithDominatingRuntimeCallAfterTheLast)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(6U, Opcode::Add).ptr().Inputs(0U, 2U);
            INST(7U, Opcode::Sub).ptr().Inputs(6U, 2U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Store).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 7U, 4U);
            INST(9U, Opcode::Return).ptr().Inputs(7U);
        }
    }

    ASSERT_TRUE(INS(5U).IsRuntimeCall());  // SafePoint is a runtime call.
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithSingleDominatingRuntimeCallInBB)
{
    EnableAddressArithmetic(GetGraph());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::Add).ptr().Inputs(0U, 2U);
            INST(6U, Opcode::Sub).ptr().Inputs(5U, 2U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(7U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Store).ref().SetNeedPreWriteBarrier(true).Inputs(0U, 1U, 6U, 4U);
            INST(9U, Opcode::Return).ptr().Inputs(6U);
        }
    }

    ASSERT_TRUE(INS(7U).IsRuntimeCall());  // SafePoint is a runtime call.
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, СorrectGraphWithNonDominatingRuntimeCallOnPathToPhiOnly)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::PRE_WRITE);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::Add).i32().Inputs(1U, 2U);
            INST(8U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, Opcode::Sub).i32().Inputs(1U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).i32().Inputs(7U, 9U);
            INST(11U, Opcode::Return).i32().Inputs(10U);
        }
    }
    ASSERT_TRUE(INS(8U).IsRuntimeCall());  // SafePoint is a runtime call.

    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithRuntimeCallInLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(9U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::Phi).ptr().Inputs(9U, 10U, 12U);
        }
        BASIC_BLOCK(7U, 6U, 8U)
        {
            INST(12U, Opcode::Phi).ptr().Inputs(4U, 11U);
            INST(13U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
            INST(14U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
            INST(15U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
            INST(16U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(17U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(19U, Opcode::Return).ref().Inputs(13U);
        }
    }
    ASSERT_TRUE(INS(16U).IsRuntimeCall());  // SafePoint is a runtime call.

    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithRuntimeCallInSelfLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).ptr().Inputs(4U, 5U);
            INST(6U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 5U);
            INST(7U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 5U);
            INST(8U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 5U);
            INST(9U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(10U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::Return).ref().Inputs(8U);
        }
    }
    ASSERT_TRUE(INS(9U).IsRuntimeCall());  // SafePoint is a runtime call.

    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithRuntimeCallBetweenPhis)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(9U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::Phi).ptr().Inputs(9U, 10U, 13U);
            INST(12U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(7U, 6U, 8U)
        {
            INST(13U, Opcode::Phi).ptr().Inputs(4U, 11U);
            INST(14U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 13U);
            INST(15U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 13U);
            INST(16U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 13U);
            INST(17U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(19U, Opcode::Return).ref().Inputs(14U);
        }
    }
    ASSERT_TRUE(INS(12U).IsRuntimeCall());  // SafePoint is a runtime call.

    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

/**
 * This graph is correct because 9.ref LoadArray gets a new GC barrier from either 6.ptr LoadGCEntrypoint or
 * 7.ptr LoadGCEntrypoint on every loop iteration. There is no way to bypass any of LoadGCEntrypoint and reuse
 * the previous value which is passed though the runtime call to the loop header (3. SafePoint).
 */
// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithRuntimeCallInLoopHeader)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        // Loop header
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(6U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(7U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(8U, Opcode::Phi).ptr().Inputs(6U, 7U);
            INST(9U, Opcode::LoadArray).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 8U);
        }
        // Latch
        BASIC_BLOCK(7U, 2U, 8U)
        {
            INST(10U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        // Exit
        BASIC_BLOCK(8U, -1L)
        {
            INST(12U, Opcode::Return).ref().Inputs(9U);
        }
    }
    ASSERT_TRUE(INS(3U).IsRuntimeCall());  // SafePoint is a runtime call.

    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}

/**
 * Back-edge in a loop through a sibling BB of the one where the initial LoadGCEntrypoint instruction takes place.
 * There is a non-simple path 3.ptr LoadGCEntrypoint -> 9p.ptr Phi v3, v7 -> 10.ref LoadArray ...,v9 -->
 * (back-edge) --> 4. SafePoint -> 9p.ptr Phi v3, v7 -> 10.ref LoadArray ...,v9 which passes through a runtime call
 * (SafePoint).
 */
// CC-OFFNXT(G.FUD.05) solid test logic
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithRuntimeCallInHeader)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        // A strange loop pre-header (bypasses the loop header)
        BASIC_BLOCK(2U, 4U)
        {
            INST(3U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        // Loop header
        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            // BB 4 has two predecessors: 2 and 3, and we must check both because hypothetically there may be a
            // back-edge through the sibling:
            //                |
            //  ->SafePoint [ LoadGCEntrypoint ]
            // |        \  /
            // |         Load
            // <--------/
            // And this initial LoadGCEntrypoint is reused on every loop iteration.
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(5U, 7U) {}
        BASIC_BLOCK(6U, 7U)
        {
            INST(7U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(7U, 8U)
        {
            INST(9U, Opcode::Phi).ptr().Inputs(3U, 7U);
            INST(10U, Opcode::LoadArray).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 9U);
        }
        // Latch
        BASIC_BLOCK(8U, 3U, 9U)
        {
            INST(11U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        // Exit
        BASIC_BLOCK(9U, -1L)
        {
            INST(13U, Opcode::Return).ref().Inputs(10U);
        }
    }
    ASSERT_TRUE(INS(4U).IsRuntimeCall());  // SafePoint is a runtime call.
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(GraphWithRuntimeCallAtExit, Graph *graph, unsigned runtimeCallId, bool useGCEntrypointInReturn)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U).i32();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(9U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::LoadGCEntrypoint).ptr().SetBarrierType(LoadGCEntrypointInst::BarrierType::READ);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::Phi).ptr().Inputs(9U, 10U, 12U);
        }
        BASIC_BLOCK(7U, 6U, 8U)
        {
            INST(12U, Opcode::Phi).ptr().Inputs(4U, 11U);
            INST(13U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
            INST(14U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
            INST(15U, Opcode::Load).ref().SetNeedReadBarrier(true).Inputs(0U, 2U, 12U);
            INST(16U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(1U, 2U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(16U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            // A runtime call placed after the final `Load` user, but `Return` is a disallowed user of LoadGCEntrypoint.
            INST(runtimeCallId, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(19U, Opcode::Return).ptr().Inputs(useGCEntrypointInReturn ? 12U : 0U);
        }
    }
}

/**
 * Even though it is OK to have a runtime call after all users of a LoadGCEntrypoint instruction, the Return instruction
 * at the end of the function is a disallowed user.
 */
TEST_F(LoadGCEntrypointCheckerTest, IncorrectGraphWithRuntimeCallAtExit)
{
    src_graph::GraphWithRuntimeCallAtExit::CREATE(GetGraph(), 18U, true);
    ASSERT_TRUE(INS(18U).IsRuntimeCall());  // SafePoint is a runtime call.
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
}

TEST_F(LoadGCEntrypointCheckerTest, CorrectGraphWithRuntimeCallAtExit)
{
    src_graph::GraphWithRuntimeCallAtExit::CREATE(GetGraph(), 18U, false);
    ASSERT_TRUE(INS(18U).IsRuntimeCall());  // SafePoint is a runtime call.
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
}
#else
TEST_F(LoadGCEntrypointCheckerTest, RequiresCompilerDebugChecks)
{
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
}
#endif
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
