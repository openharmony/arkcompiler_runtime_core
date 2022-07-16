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

#include <string>

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-literals.h"
#include "assembler/assembly-parser.h"
#include "assembler/assembly-program.h"
#include "common.h"
#include "codegen.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimize_bytecode.h"
#include "reg_acc_alloc.h"

namespace panda::bytecodeopt::test {

TEST(ToolTest, BytecodeOptIrInterfaceCommon)
{
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.methods.insert({0, std::string("method")});
    maps.fields.insert({0, std::string("field")});
    maps.classes.insert({0, std::string("class")});
    maps.strings.insert({0, std::string("string")});
    maps.literalarrays.insert({0, std::string("0")});

    pandasm::Program prog;
    prog.literalarray_table.emplace("0", pandasm::LiteralArray());

    BytecodeOptIrInterface interface(&maps, &prog);

    EXPECT_EQ(interface.GetMethodIdByOffset(0), std::string("method"));
    EXPECT_EQ(interface.GetStringIdByOffset(0), std::string("string"));
    EXPECT_EQ(interface.GetLiteralArrayIdByOffset(0), "0");
    EXPECT_EQ(interface.GetTypeIdByOffset(0), std::string("class"));
    EXPECT_EQ(interface.GetFieldIdByOffset(0), std::string("field"));
    interface.StoreLiteralArray("1", pandasm::LiteralArray());
    EXPECT_EQ(interface.GetLiteralArrayTableSize(), 2);
}

TEST(ToolTest, BytecodeOptIrInterfacePcLineNumber)
{
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;

    pandasm::Program prog;
    prog.literalarray_table.emplace("0", pandasm::LiteralArray());

    BytecodeOptIrInterface interface(&maps, &prog);

    auto map = interface.GetPcInsMap();
    EXPECT_EQ(interface.GetLineNumberByPc(0), 0);

    auto ins = pandasm::Ins();
    ins.ins_debug.line_number = 1;
    ASSERT_NE(map, nullptr);
    map->insert({0, &ins});
    EXPECT_EQ(interface.GetLineNumberByPc(0), 1);
    interface.ClearPcInsMap();
    EXPECT_NE(interface.GetLineNumberByPc(0), 1);
}

TEST(ToolTest, BytecodeOptIrInterfaceLiteralArray)
{
    BytecodeOptIrInterface interface(nullptr, nullptr);
#ifdef NDEBUG
    EXPECT_EQ(interface.GetLiteralArrayIdByOffset(0), std::nullopt);
#endif
}

TEST_F(CommonTest, CodeGenBinaryImms)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::AddI).s32().Inputs(0).Imm(2);
            INST(2, Opcode::SubI).s32().Inputs(0).Imm(2);
            INST(3, Opcode::MulI).s32().Inputs(0).Imm(2);
            INST(4, Opcode::DivI).s32().Inputs(0).Imm(2);
            INST(5, Opcode::ModI).s32().Inputs(0).Imm(2);
            INST(6, Opcode::AndI).u32().Inputs(0).Imm(2);
            INST(7, Opcode::OrI).s32().Inputs(0).Imm(2);
            INST(8, Opcode::XorI).s32().Inputs(0).Imm(2);
            INST(10, Opcode::Return).b().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodeGenIfImm)
{
    auto graph = CreateEmptyGraph();

    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(compiler::ConditionCode::CC_EQ).Inputs(0, 1);
            INST(3, Opcode::IfImm)
                .SrcType(compiler::DataType::BOOL)
                .CC(compiler::ConditionCode::CC_NE)
                .Imm(0)
                .Inputs(2);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(10, Opcode::Return).s32().Inputs(0);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).s32().Inputs(0);
        }
    }

#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    graph->RunPass<compiler::Lowering>();
    graph->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(12, Opcode::If).CC(compiler::ConditionCode::CC_EQ).SrcType(compiler::DataType::UINT32).Inputs(1, 0);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(10, Opcode::Return).s32().Inputs(0);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).s32().Inputs(0);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));

    EXPECT_TRUE(expected->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(expected->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenIf)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u32();
            PARAMETER(1, 1).u32();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT32).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s32().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s32().Inputs(0);
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfDynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u32();
            PARAMETER(1, 1).u32();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT32).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s32().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s32().Inputs(0);
            }
        }

        graph->SetDynamicMethod();

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();
            PARAMETER(1, 1).s64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::If).CC(cond).SrcType(compiler::DataType::INT64).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s64().Inputs(0);
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();
            PARAMETER(1, 1).s64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::If).CC(cond).SrcType(compiler::DataType::INT64).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s64().Inputs(0);
            }
        }

        graph->SetDynamicMethod();

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfUINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT64).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).u64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(0);
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfUINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT64).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).u64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(0);
            }
        }

        graph->SetDynamicMethod();

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfREFERENCE)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).ref();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::If).SrcType(compiler::DataType::REFERENCE).CC(cond).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(3, Opcode::ReturnVoid).v0id();
            }
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnVoid).v0id();
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZero)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s32();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::INT32).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s32().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s32().Inputs(0);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroDynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s32();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::INT32).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s32().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s32().Inputs(0);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::INT64).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s64().Inputs(0);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::INT64).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s64().Inputs(0);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroUINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::UINT64).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).u64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(0);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroUINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::UINT64).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).u64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(0);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroREFERENCE)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::REFERENCE).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).ref().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).ref().Inputs(0);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroREFERENCEDynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(0).SrcType(compiler::DataType::REFERENCE).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).ref().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).ref().Inputs(0);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmNonZero)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(1).SrcType(compiler::DataType::UINT64).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).u64().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(0);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmNonZero32)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s32();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(1).SrcType(compiler::DataType::INT32).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s32().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s32().Inputs(0);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmNonZero32Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s32();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::IfImm).CC(cond).Imm(1).SrcType(compiler::DataType::INT32).Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(10, Opcode::Return).s32().Inputs(0);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(11, Opcode::Return).s32().Inputs(0);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenConstantINT64Dynamic)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).s64().Inputs(0);
        }
    }

    graph->SetDynamicMethod();

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenConstantFLOAT64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).f64().Inputs(0);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenConstantFLOAT64Dynamic)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).f64().Inputs(0);
        }
    }

    graph->SetDynamicMethod();

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenConstantINT32Dynamic)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).s32().Inputs(0);
        }
    }

    graph->SetDynamicMethod();

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2F64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(compiler::DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Uint)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u32().SrcType(compiler::DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).u32().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Int16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s16().SrcType(compiler::DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).s16().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Uint16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u16().SrcType(compiler::DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).u16().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Int8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s8().SrcType(compiler::DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).s8().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Uint8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u8().SrcType(compiler::DataType::INT32).Inputs(0);
            INST(2, Opcode::Return).u8().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToInt32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s32().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToUint32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u32().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).u32().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToF64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).f64().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToInt16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s16().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).s16().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToUint16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u16().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).u16().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToInt8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s8().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).s8().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToUint8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u8().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(2, Opcode::Return).u8().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastFloat64ToInt64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s64().SrcType(compiler::DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).s64().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastFloat64ToInt32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s32().SrcType(compiler::DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastFloat64ToUint32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).u32().SrcType(compiler::DataType::FLOAT64).Inputs(0);
            INST(2, Opcode::Return).u32().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(AsmTest, CodegenLoadString)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        using namespace compiler::DataType;
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).NoVregs();

            INST(2, Opcode::LoadString).ref().TypeId(235).Inputs(1);

            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::NullCheck).ref().Inputs(2, 3);
            INST(5, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 4}, {NO_TYPE, 3}});
            INST(6, Opcode::Return).s32().Inputs(5);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));

    graph->SetDynamicMethod();
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenStoreObject64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 5}, {NO_TYPE, 4}});
            INST(8, Opcode::SaveState).NoVregs();
            INST(9, Opcode::NullCheck).ref().Inputs(3, 8);

            INST(10, Opcode::StoreObject).s64().Inputs(9, 7);

            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenStoreObjectReference)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 5}, {NO_TYPE, 4}});
            INST(8, Opcode::SaveState).NoVregs();
            INST(9, Opcode::NullCheck).ref().Inputs(3, 8);

            INST(10, Opcode::StoreObject).ref().Inputs(9, 5);

            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLoadObjectInt)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).s32().Inputs(2);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLoadObjectInt64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).s64().Inputs(2);
            INST(4, Opcode::Return).s64().Inputs(3);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLoadObjectReference)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).ref().Inputs(2);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenInitObject)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;

            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0).TypeId(68);
            INST(3, Opcode::SaveState).NoVregs();
            INST(8, Opcode::InitObject).ref().Inputs({{REFERENCE, 1}, {NO_TYPE, 3}});
            INST(7, Opcode::ReturnVoid).v0id();
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenEncodeSta64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;

            INST(1, Opcode::SaveState).NoVregs();
            INST(3, Opcode::CallVirtual).f64().Inputs({{REFERENCE, 0}, {NO_TYPE, 1}});
            INST(6, Opcode::LoadObject).s64().Inputs(0);
            INST(7, Opcode::Cast).f64().SrcType(INT64).Inputs(6);
            CONSTANT(15, 3.6e+06).f64();
            INST(9, Opcode::Div).f64().Inputs(7, 15);
            INST(10, Opcode::Add).f64().Inputs(9, 3);
            CONSTANT(16, 24).f64();
            INST(12, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).f64().Inputs({{FLOAT64, 10}, {FLOAT64, 16}, {NO_TYPE, 12}});
            INST(14, Opcode::ReturnVoid).v0id();
        }
    }

    RuntimeInterfaceMock runtime(1);
    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenBlockNeedsJump)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(4, 4).ref();
        PARAMETER(7, 7).s32();
        PARAMETER(13, 13).ref();
        PARAMETER(19, 19).ref();
        PARAMETER(35, 35).ref();
        PARAMETER(43, 43).s32();

        using namespace compiler::DataType;

        BASIC_BLOCK(2, 3, 4)
        {
            INST(10, Opcode::IfImm).SrcType(INT32).CC(compiler::CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 5, 7)
        {
            INST(39, Opcode::If).SrcType(REFERENCE).CC(compiler::CC_EQ).Inputs(13, 4);
        }
        BASIC_BLOCK(3, 7, 8)
        {
            INST(40, Opcode::If).SrcType(REFERENCE).CC(compiler::CC_NE).Inputs(19, 4);
        }
        BASIC_BLOCK(8, 5, 7)
        {
            INST(41, Opcode::If).SrcType(INT32).CC(compiler::CC_EQ).Inputs(7, 43);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(28, Opcode::ReturnVoid).v0id();
        }
        BASIC_BLOCK(7, -1)
        {
            INST(37, Opcode::SaveState).NoVregs();
            INST(38, Opcode::Throw).Inputs(35, 37);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenFloat32Constant)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0.0).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).f32().Inputs(0);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(AsmTest, CatchPhi)
{
    auto source = R"(
    .record E1 {}
    .record A {}

    .function i64 main(i64 a0) {
    try_begin:
        movi.64 v0, 100
        lda.64 v0
        div2.64 a0
        sta.64 v0
        div2.64 a0
    try_end:
        jmp exit

    catch_block1_begin:
        lda.64 v0
        return

    catch_block2_begin:
        lda.64 v0
        return

    exit:
        return

    .catch E1, try_begin, try_end, catch_block1_begin
    .catchall try_begin, try_end, catch_block2_begin

    }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(parser.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);
    auto &prog = res.Value();
    ParseToGraph(&prog, "main");

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(GetGraph()->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLdai)
{
    using compiler::DataType::Type;
    std::vector<Type> types {Type::INT32, Type::INT64, Type::FLOAT32, Type::FLOAT64};
    std::vector<pandasm::Opcode> opcodes {pandasm::Opcode::LDAI, pandasm::Opcode::LDAI_64, pandasm::Opcode::FLDAI,
                                          pandasm::Opcode::FLDAI_64};

    for (size_t i = 0; i < types.size(); i++) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s32();
            BASIC_BLOCK(2, -1)
            {
                CONSTANT(1, 159).s32();
                INST(2, Opcode::Add).s32().Inputs(0, 1);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        INS(0).SetType(types[i]);
        INS(1).SetType(types[i]);
        INS(2).SetType(types[i]);
        INS(3).SetType(types[i]);
        graph->RunPass<bytecodeopt::RegAccAlloc>();
        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), panda::panda_file::SourceLang::PANDA_ASSEMBLY);
        IrInterfaceTest interface(nullptr);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
        EXPECT_TRUE(FuncHasInst(&function, opcodes[i]));
    }
}

TEST(TotalTest, OptimizeBytecode)
{
    auto source = R"(
    .function i8 main(){
        ldai 1
        return
    }
    )";
    panda::pandasm::Parser parser;
    const char file_name[] = "opt_bc.bin";
    auto res = parser.Parse(source, file_name);
    ASSERT_TRUE(parser.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);
    auto &prog = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    pandasm::AsmEmitter::Emit(file_name, prog, nullptr, &maps);
    EXPECT_TRUE(OptimizeBytecode(&prog, &maps, file_name));
    EXPECT_TRUE(OptimizeBytecode(&prog, &maps, file_name, true));

    options.SetOptLevel(0);
    EXPECT_FALSE(OptimizeBytecode(&prog, &maps, file_name));
    options.SetOptLevel(1);
    EXPECT_TRUE(OptimizeBytecode(&prog, &maps, file_name));
#ifndef NDEBUG
    options.SetOptLevel(-1);
    EXPECT_DEATH_IF_SUPPORTED(OptimizeBytecode(&prog, &maps, file_name), "");
#endif
    options.SetOptLevel(2);

    options.SetMethodRegex(std::string());
    EXPECT_FALSE(OptimizeBytecode(&prog, &maps, file_name));
}

}  // namespace panda::bytecodeopt::test
