/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <random>

#include "optimizer/code_generator/codegen.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/if_conversion.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer_run.h"
#include "compiler/inplace_task_runner.h"
#include "compiler/compiler_task_runner.h"

#include "libpandabase/macros.h"
#include "libpandabase/utils/utils.h"
#include "gtest/gtest.h"
#include "unit_test.h"
#include "utils/bit_utils.h"
#include "vixl_exec_module.h"

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 40;
#else
const uint64_t ITERATION = 20000;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
static inline auto g_randomGenerator = std::mt19937_64(SEED);

namespace panda::compiler {

class CodegenTest : public GraphTest {
public:
    CodegenTest() : execModule_(GetAllocator(), GetGraph()->GetRuntime())
    {
#ifndef NDEBUG
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
    }
    ~CodegenTest() override = default;

    NO_COPY_SEMANTIC(CodegenTest);
    NO_MOVE_SEMANTIC(CodegenTest);

    VixlExecModule &GetExecModule()
    {
        return execModule_;
    }

    template <typename T>
    void CheckStoreArray();

    template <typename T>
    void CheckLoadArray();

    template <typename T>
    void CheckStoreArrayPair(bool imm);

    template <typename T>
    void CheckLoadArrayPair(bool imm);

    template <typename T>
    void CheckCmp(bool isFcmpg = false);

    template <typename T>
    void CheckReturnValue(Graph *graph, T expectedValue);

    template <typename T>
    void CheckBounds(size_t count);

    void TestBinaryOperationWithShiftedOperand(Opcode opcode, uint32_t l, uint32_t r, ShiftType shiftType,
                                               uint32_t shift, uint32_t erv);

    void CreateGraphForOverflowTest(Graph *graph, Opcode overflowOpcode);

private:
    VixlExecModule execModule_;
};

bool RunCodegen(Graph *graph)
{
    return graph->RunPass<Codegen>();
}

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)
TEST_F(CodegenTest, SimpleProgramm)
{
    /*
   .function main()<main>{
       movi.64 v0, 100000000           ##      0 -> 3      ##  bb0
       movi.64 v1, 4294967296          ##      1 -> 4      ##  bb0
       ldai 0                          ##      2 -> 5      ##  bb0
   loop:                               ##                  ##
       jeq v0, loop_exit               ##      6, 7, 8     ##  bb1
                                       ##                  ##
       sta.64 v2                       ##      9           ##  bb2
       and.64 v1                       ##      10          ##  bb2
       sta.64 v1                       ##      11          ##  bb2
       lda.64 v2                       ##      12          ##  bb2
       inc                             ##      13          ##  bb2
       jmp loop                        ##      14          ##  bb2
   loop_exit:                          ##                  ##
       lda.64 v1                       ##      14          ##  bb3
       return.64                       ##      15          ##  bb3
   }
   */

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10UL);          // r1
        CONSTANT(1U, 4294967296UL);  // r2
        CONSTANT(2U, 0UL);           // r3 -> acc(3)
        CONSTANT(3U, 0x1UL);         // r20 -> 0x1 (for inc constant)

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(16U, Opcode::Phi).Inputs(2U, 13U).s64();  // PHI acc
            INST(17U, Opcode::Phi).Inputs(1U, 10U).s64();  // PHI  v1
            INST(20U, Opcode::Phi).Inputs(2U, 10U).s64();  // result to return

            // NOTE (igorban): support CMP instr
            INST(18U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 16U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(10U, Opcode::And).Inputs(16U, 17U).s64();  // -> acc
            INST(13U, Opcode::Add).Inputs(16U, 3U).s64();   // -> acc
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::Return).Inputs(20U).s64();
        }
    }

    SetNumVirtRegs(0U);
    SetNumArgs(1U);

    RegAlloc(GetGraph());

    // call codegen
    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto entry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto exit = entry + GetGraph()->GetCode().Size();
    ASSERT(entry != nullptr && exit != nullptr);
    GetExecModule().SetInstructions(entry, exit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 0U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

template <typename T>
void CodegenTest::CheckStoreArray()
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    auto index = graph->AddNewParameter(1U, DataType::INT32);
    auto storeValue = graph->AddNewParameter(2U, TYPE);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));
    storeValue->SetLocationData(graph->GetDataForNativeParam(TYPE));

    auto stArr = graph->CreateInst(Opcode::StoreArray);
    block->AppendInst(stArr);
    stArr->SetType(TYPE);
    stArr->SetInput(0U, array);
    stArr->SetInput(1U, index);
    stArr->SetInput(2U, storeValue);
    auto ret = graph->CreateInst(Opcode::ReturnVoid);
    block->AppendInst(ret);

    SetNumVirtRegs(0U);
    SetNumArgs(3U);

    RegAlloc(graph);

    // call codegen
    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[4U];
    auto defaultValue = CutValue<T>(0U, TYPE);
    for (auto &data : arrayData) {
        data = defaultValue;
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    auto param3 = CutValue<T>(10U, TYPE);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param3);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, arrayData);

    for (size_t i = 0; i < 4U; i++) {
        if (i == 2U) {
            EXPECT_EQ(arrayData[i], param3);
        } else {
            EXPECT_EQ(arrayData[i], defaultValue);
        }
    }
    GetExecModule().FreeArray(param1);
}

template <typename T>
void CodegenTest::CheckLoadArray()
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    auto index = graph->AddNewParameter(1U, DataType::INT32);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));

    auto ldArr = graph->CreateInst(Opcode::LoadArray);
    block->AppendInst(ldArr);
    ldArr->SetType(TYPE);
    ldArr->SetInput(0U, array);
    ldArr->SetInput(1U, index);
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(TYPE);
    ret->SetInput(0U, ldArr);
    block->AppendInst(ret);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[4U];
    for (size_t i = 0; i < 4U; i++) {
        arrayData[i] = CutValue<T>((-i), TYPE);
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, arrayData);

    GetExecModule().FreeArray(param1);

    auto retData = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(retData, CutValue<T>(-2L, TYPE));
}

template <typename T>
void CodegenTest::CheckStoreArrayPair(bool imm)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    [[maybe_unused]] auto index = graph->AddNewParameter(1U, DataType::INT32);
    auto val0 = graph->AddNewParameter(2U, TYPE);
    auto val1 = graph->AddNewParameter(3U, TYPE);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));
    val0->SetLocationData(graph->GetDataForNativeParam(TYPE));
    val1->SetLocationData(graph->GetDataForNativeParam(TYPE));

    Inst *stpArr = nullptr;
    if (imm) {
        stpArr = graph->CreateInstStoreArrayPairI(TYPE, INVALID_PC, array, val0, val1, 2U);
        block->AppendInst(stpArr);
    } else {
        stpArr = graph->CreateInstStoreArrayPair(TYPE, INVALID_PC, array, index, val0, val1);
        block->AppendInst(stpArr);
    }

    auto ret = graph->CreateInst(Opcode::ReturnVoid);
    block->AppendInst(ret);

    GraphChecker(graph).Check();

    SetNumVirtRegs(0U);
    SetNumArgs(4U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[6U] = {0U, 0U, 0U, 0U, 0U, 0U};
    auto param1 = GetExecModule().CreateArray(arrayData, 6U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    auto param3 = CutValue<T>(3U, TYPE);
    auto param4 = CutValue<T>(5U, TYPE);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param3);
    GetExecModule().SetParameter(3U, param4);

    GetExecModule().Execute();
    GetExecModule().CopyArray(param1, arrayData);
    GetExecModule().FreeArray(param1);

    T arrayExpected[6U] = {0U, 0U, 3U, 5U, 0U, 0U};

    for (size_t i = 0; i < 6U; ++i) {
        EXPECT_EQ(arrayData[i], arrayExpected[i]);
    }
}

template <typename T>
void CodegenTest::CheckLoadArrayPair(bool imm)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    [[maybe_unused]] auto index = graph->AddNewParameter(1U, DataType::INT32);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));

    Inst *ldpArr = nullptr;
    if (imm) {
        ldpArr = graph->CreateInstLoadArrayPairI(TYPE, INVALID_PC, array, 2U);
        block->AppendInst(ldpArr);
    } else {
        ldpArr = graph->CreateInstLoadArrayPair(TYPE, INVALID_PC, array, index);
        block->AppendInst(ldpArr);
    }

    auto loadHigh = graph->CreateInstLoadPairPart(TYPE, INVALID_PC, ldpArr, 0U);
    block->AppendInst(loadHigh);

    auto loadLow = graph->CreateInstLoadPairPart(TYPE, INVALID_PC, ldpArr, 1U);
    block->AppendInst(loadLow);

    auto sum = graph->CreateInst(Opcode::Add);
    block->AppendInst(sum);
    sum->SetType(TYPE);
    sum->SetInput(0U, loadHigh);
    sum->SetInput(1U, loadLow);

    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(TYPE);
    ret->SetInput(0U, sum);
    block->AppendInst(ret);

    GraphChecker(graph).Check();

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[6U];
    // [ 1, 2, 3, 4, 5, 6] -> 7
    for (size_t i = 0; i < 6U; i++) {
        arrayData[i] = CutValue<T>(i + 1U, TYPE);
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 6U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();
    GetExecModule().FreeArray(param1);

    auto retData = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(retData, CutValue<T>(7U, TYPE));
}

template <typename T>
void CodegenTest::CheckBounds(uint64_t count)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();
    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto param = graph->AddNewParameter(0U, TYPE);

    graph->ResetParameterInfo();
    param->SetLocationData(graph->GetDataForNativeParam(TYPE));

    BinaryImmOperation *lastInst = nullptr;
    // instruction_count + parameter + return
    for (uint64_t i = count - 1U; i > 1U; --i) {
        auto addInst = graph->CreateInstAddI(TYPE, 0U, 1U);
        block->AppendInst(addInst);
        if (lastInst == nullptr) {
            addInst->SetInput(0U, param);
        } else {
            addInst->SetInput(0U, lastInst);
        }
        lastInst = addInst;
    }
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(TYPE);
    ret->SetInput(0U, lastInst);
    block->AppendInst(ret);

#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif
    GraphChecker(graph).Check();

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    auto instsPerByte = GetGraph()->GetEncoder()->MaxArchInstPerEncoded();
    auto maxBitsInInst = GetInstructionSizeBits(GetGraph()->GetArch());
    if (count * instsPerByte * maxBitsInInst > g_options.GetCompilerMaxGenCodeSize()) {
        EXPECT_FALSE(RunCodegen(graph));
    } else {
        ASSERT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        T param = 0;
        GetExecModule().SetParameter(0U, param);
        GetExecModule().Execute();

        auto retData = GetExecModule().GetRetValue<T>();
        EXPECT_EQ(retData, CutValue<T>(count - 2L, TYPE));
    }
}

template <typename T>
void CodegenTest::CheckCmp(bool isFcmpg)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();
    bool isFloat = DataType::IsFloatType(TYPE);

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto param1 = graph->AddNewParameter(0U, TYPE);
    auto param2 = graph->AddNewParameter(1U, TYPE);

    graph->ResetParameterInfo();
    param1->SetLocationData(graph->GetDataForNativeParam(TYPE));
    param2->SetLocationData(graph->GetDataForNativeParam(TYPE));

    auto fcmp = graph->CreateInst(Opcode::Cmp);
    block->AppendInst(fcmp);
    fcmp->SetType(DataType::INT32);
    fcmp->SetInput(0U, param1);
    fcmp->SetInput(1U, param2);
    static_cast<CmpInst *>(fcmp)->SetOperandsType(TYPE);
    if (isFloat) {
        static_cast<CmpInst *>(fcmp)->SetFcmpg(isFcmpg);
    }
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(DataType::INT32);
    ret->SetInput(0U, fcmp);
    block->AppendInst(ret);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);
    T paramData[3U];
    if (TYPE == DataType::FLOAT32) {
        paramData[0U] = std::nanf("0");
    } else if (TYPE == DataType::FLOAT64) {
        paramData[0U] = std::nan("0");
    } else {
        paramData[0U] = std::numeric_limits<T>::max();
        paramData[2U] = std::numeric_limits<T>::min();
    }
    paramData[1U] = CutValue<T>(2U, TYPE);
    if (isFloat) {
        paramData[2U] = -paramData[1U];
    }

    for (size_t i = 0; i < 3U; i++) {
        for (size_t j = 0; j < 3U; j++) {
            auto param1 = paramData[i];
            auto param2 = paramData[j];
            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);

            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue<int32_t>();
            if ((i == 0U || j == 0U) && isFloat) {
                EXPECT_EQ(retData, isFcmpg ? 1U : -1L);
            } else if (i == j) {
                EXPECT_EQ(retData, 0U);
            } else if (i > j) {
                EXPECT_EQ(retData, -1L);
            } else {
                EXPECT_EQ(retData, 1U);
            }
        }
    }
}

template <typename T>
void CodegenTest::CheckReturnValue(Graph *graph, [[maybe_unused]] T expectedValue)
{
    SetNumVirtRegs(0U);
    RegAlloc(graph);
    EXPECT_TRUE(RunCodegen(graph));

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();

    ASSERT(codeEntry != nullptr && codeExit != nullptr);

    GetExecModule().SetInstructions(codeEntry, codeExit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(rv, expectedValue);
}

void CodegenTest::TestBinaryOperationWithShiftedOperand(Opcode opcode, uint32_t l, uint32_t r, ShiftType shiftType,
                                                        uint32_t shift, uint32_t erv)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, l);
        CONSTANT(1U, r);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, opcode).Shift(shiftType, shift).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), erv);
}

void CodegenTest::CreateGraphForOverflowTest(Graph *graph, Opcode overflowOpcode)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, overflowOpcode).i32().SrcType(DataType::INT32).CC(CC_EQ).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(4U);
        }
    }
}

TEST_F(CodegenTest, Cmp)
{
    CheckCmp<float>(true);
    CheckCmp<float>(false);
    CheckCmp<double>(true);
    CheckCmp<double>(false);
    CheckCmp<uint8_t>();
    CheckCmp<int8_t>();
    CheckCmp<uint16_t>();
    CheckCmp<int16_t>();
    CheckCmp<uint32_t>();
    CheckCmp<int32_t>();
    CheckCmp<uint64_t>();
    CheckCmp<int64_t>();
}

TEST_F(CodegenTest, StoreArray)
{
    CheckStoreArray<bool>();
    CheckStoreArray<int8_t>();
    CheckStoreArray<uint8_t>();
    CheckStoreArray<int16_t>();
    CheckStoreArray<uint16_t>();
    CheckStoreArray<int32_t>();
    CheckStoreArray<uint32_t>();
    CheckStoreArray<int64_t>();
    CheckStoreArray<uint64_t>();
    CheckStoreArray<float>();
    CheckStoreArray<double>();

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u32();  // index
        PARAMETER(2U, 2U).u32();  // store value
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::StoreArray).u32().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::ReturnVoid);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0U);
    SetNumArgs(3U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    ObjectPointerType array[4U] = {0U, 0U, 0U, 0U};
    auto param1 = GetExecModule().CreateArray(array, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    auto param3 = CutValue<ObjectPointerType>(10U, DataType::UINT64);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param3);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, array);

    for (size_t i = 0; i < 4U; i++) {
        if (i == 2U) {
            EXPECT_EQ(array[i], 10U) << "value of i: " << i;
        } else {
            EXPECT_EQ(array[i], 0U) << "value of i: " << i;
        }
    }
    GetExecModule().FreeArray(param1);
}

TEST_F(CodegenTest, StoreArrayPair)
{
    CheckStoreArrayPair<uint32_t>(true);
    CheckStoreArrayPair<int32_t>(false);
    CheckStoreArrayPair<uint64_t>(true);
    CheckStoreArrayPair<int64_t>(false);
    CheckStoreArrayPair<float>(true);
    CheckStoreArrayPair<float>(false);
    CheckStoreArrayPair<double>(true);
    CheckStoreArrayPair<double>(false);
}

TEST_F(CodegenTest, Compare)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        for (auto inverse : {true, false}) {
            auto graph = CreateGraphStartEndBlocks();
            RuntimeInterfaceMock runtime;
            graph->SetRuntime(&runtime);

            GRAPH(graph)
            {
                PARAMETER(0U, 0U).u64();
                PARAMETER(1U, 1U).u64();
                CONSTANT(2U, 0U);
                CONSTANT(3U, 1U);
                BASIC_BLOCK(2U, 3U, 4U)
                {
                    INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                    INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0U).Inputs(4U);
                }
                BASIC_BLOCK(3U, -1L)
                {
                    INST(6U, Opcode::Return).b().Inputs(3U);
                }
                BASIC_BLOCK(4U, -1L)
                {
                    INST(7U, Opcode::Return).b().Inputs(2U);
                }
            }
            SetNumVirtRegs(0U);
            SetNumArgs(2U);

            RegAlloc(graph);

            EXPECT_TRUE(RunCodegen(graph));
            auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
            auto codeExit = codeEntry + graph->GetCode().Size();
            ASSERT(codeEntry != nullptr && codeExit != nullptr);
            GetExecModule().SetInstructions(codeEntry, codeExit);

            GetExecModule().SetDump(false);

            bool result;
            auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
            auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);

            result = (cc == CC_NE || cc == CC_GT || cc == CC_GE || cc == CC_B || cc == CC_BE);
            if (inverse) {
                result = !result;
            }

            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param2);
            GetExecModule().SetParameter(1U, param1);

            GetExecModule().Execute();

            result = (cc == CC_NE || cc == CC_LT || cc == CC_LE || cc == CC_A || cc == CC_AE);
            if (inverse) {
                result = !result;
            }

            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param1);

            result = (cc == CC_EQ || cc == CC_LE || cc == CC_GE || cc == CC_AE || cc == CC_BE);
            if (inverse) {
                result = !result;
            }

            GetExecModule().Execute();

            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);
        }
    }
}

TEST_F(CodegenTest, GenIf)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(6U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(7U, Opcode::Return).b().Inputs(2U);
            }
        }
        SetNumVirtRegs(0U);
        SetNumArgs(2U);
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_EQ(INS(0U).GetUsers().Front().GetInst()->GetOpcode(), Opcode::If);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        bool result;
        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

TEST_F(CodegenTest, GenIfImm)
{
    int32_t values[3U] = {-1L, 0U, 1U};
    for (auto value : values) {
        for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
            auto cc = static_cast<ConditionCode>(ccint);
            auto graph = CreateGraphStartEndBlocks();
            RuntimeInterfaceMock runtime;
            graph->SetRuntime(&runtime);
            if ((cc == CC_TST_EQ || cc == CC_TST_NE) && !graph->GetEncoder()->CanEncodeImmLogical(value, WORD_SIZE)) {
                continue;
            }

            GRAPH(graph)
            {
                PARAMETER(0U, 0U).s32();
                CONSTANT(1U, 0U);
                CONSTANT(2U, 1U);
                BASIC_BLOCK(2U, 3U, 4U)
                {
                    INST(3U, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(value).Inputs(0U);
                }
                BASIC_BLOCK(3U, -1L)
                {
                    INST(4U, Opcode::Return).b().Inputs(2U);
                }
                BASIC_BLOCK(4U, -1L)
                {
                    INST(5U, Opcode::Return).b().Inputs(1U);
                }
            }
            SetNumVirtRegs(0U);
            SetNumArgs(2U);

            RegAlloc(graph);

            EXPECT_TRUE(RunCodegen(graph));
            auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
            auto codeExit = codeEntry + graph->GetCode().Size();
            ASSERT(codeEntry != nullptr && codeExit != nullptr);
            GetExecModule().SetInstructions(codeEntry, codeExit);

            GetExecModule().SetDump(false);

            bool result;
            auto param1 = CutValue<uint64_t>(value, DataType::INT32);
            auto param2 = CutValue<uint64_t>(value + 5U, DataType::INT32);
            auto param3 = CutValue<uint64_t>(value - 5L, DataType::INT32);

            GetExecModule().SetParameter(0U, param1);
            result = Compare(cc, param1, param1);
            GetExecModule().Execute();
            auto retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param2);
            GetExecModule().Execute();
            result = Compare(cc, param2, param1);
            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param3);
            result = Compare(cc, param3, param1);
            GetExecModule().Execute();
            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);
        }
    }
}

TEST_F(CodegenTest, If)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(5U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(6U, Opcode::Return).b().Inputs(2U);
            }
        }
        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        bool result;
        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

TEST_F(CodegenTest, AddOverflow)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    CreateGraphForOverflowTest(graph, Opcode::AddOverflow);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    InPlaceCompilerTaskRunner taskRunner;
    taskRunner.GetContext().SetGraph(graph);
    bool success = true;
    taskRunner.AddCallbackOnFail([&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });
    RunOptimizations<INPLACE_MODE>(std::move(taskRunner));
    EXPECT_TRUE(success);

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    int32_t min = std::numeric_limits<int32_t>::min();
    int32_t max = std::numeric_limits<int32_t>::max();
    std::array<int32_t, 7U> values = {0U, 2U, 5U, -7L, -10L, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            int32_t a0 = values[i];
            int32_t a1 = values[j];
            int32_t result;
            auto param1 = CutValue<int32_t>(a0, DataType::INT32);
            auto param2 = CutValue<int32_t>(a1, DataType::INT32);

            if ((a0 > 0 && a1 > max - a0) || (a0 < 0 && a1 < min - a0)) {
                result = 0;
            } else {
                result = a0 + a1;
            }
            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);
            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue<int32_t>();
            EXPECT_EQ(retData, result);
        }
    }
}

TEST_F(CodegenTest, SubOverflow)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    CreateGraphForOverflowTest(graph, Opcode::SubOverflow);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    InPlaceCompilerTaskRunner taskRunner;
    taskRunner.GetContext().SetGraph(graph);
    bool success = true;
    taskRunner.AddCallbackOnFail([&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });
    RunOptimizations<INPLACE_MODE>(std::move(taskRunner));
    EXPECT_TRUE(success);

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    int32_t min = std::numeric_limits<int32_t>::min();
    int32_t max = std::numeric_limits<int32_t>::max();
    std::array<int32_t, 7U> values = {0U, 2U, 5U, -7L, -10L, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            int32_t a0 = values[i];
            int32_t a1 = values[j];
            int32_t result;
            auto param1 = CutValue<int32_t>(a0, DataType::INT32);
            auto param2 = CutValue<int32_t>(a1, DataType::INT32);

            if ((a1 > 0 && a0 < min + a1) || (a1 < 0 && a0 > max + a1)) {
                result = 0;
            } else {
                result = a0 - a1;
            }
            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);
            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue<int32_t>();
            EXPECT_EQ(retData, result);
        }
    }
}

TEST_F(CodegenTest, GenSelect)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();
            PARAMETER(1U, 1U).s64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::If).SrcType(DataType::INT64).CC(cc).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, 4U) {}
            BASIC_BLOCK(4U, -1L)
            {
                INST(5U, Opcode::Phi).b().Inputs({{3U, 3U}, {2U, 2U}});
                INST(6U, Opcode::Return).b().Inputs(5U);
            }
        }
        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        EXPECT_TRUE(graph->RunPass<IfConversion>());
        ASSERT_EQ(INS(6U).GetInput(0U).GetInst()->GetOpcode(), Opcode::Select);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        bool result;
        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

TEST_F(CodegenTest, BoolSelectImm)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                INST(5U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U, 2U, 4U);
                INST(6U, Opcode::Return).b().Inputs(5U);
            }
        }
        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);
        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        bool result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

TEST_F(CodegenTest, Select)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(5U, Opcode::Select).u64().SrcType(DataType::UINT64).CC(cc).Inputs(3U, 2U, 0U, 1U);
                INST(6U, Opcode::Return).u64().Inputs(5U);
            }
        }
        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);
        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        bool result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = (cc == CC_EQ || cc == CC_LE || cc == CC_GE || cc == CC_AE || cc == CC_BE);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

TEST_F(CodegenTest, CompareObj)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_NE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    SetNumVirtRegs(0U);
    SetNumArgs(1U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
    auto param2 = CutValue<uint64_t>(0U, DataType::UINT64);

    GetExecModule().SetParameter(0U, param1);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 1U);

    GetExecModule().SetParameter(0U, param2);

    GetExecModule().Execute();

    retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 0U);
}

TEST_F(CodegenTest, LoadArray)
{
    CheckLoadArray<bool>();
    CheckLoadArray<int8_t>();
    CheckLoadArray<uint8_t>();
    CheckLoadArray<int16_t>();
    CheckLoadArray<uint16_t>();
    CheckLoadArray<int32_t>();
    CheckLoadArray<uint32_t>();
    CheckLoadArray<int64_t>();
    CheckLoadArray<uint64_t>();
    CheckLoadArray<float>();
    CheckLoadArray<double>();

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u32();  // index
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::LoadArray).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    ObjectPointerType array[4U] = {0xffffaaaaU, 0xffffbbbbU, 0xffffccccU, 0xffffddddU};
    auto param1 = GetExecModule().CreateArray(array, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, array);

    GetExecModule().FreeArray(param1);
    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, array[2U]);
}

TEST_F(CodegenTest, LoadArrayPair)
{
    CheckLoadArrayPair<uint32_t>(true);
    CheckLoadArrayPair<int32_t>(false);
    CheckLoadArrayPair<uint64_t>(true);
    CheckLoadArrayPair<int64_t>(false);
    CheckLoadArrayPair<float>(true);
    CheckLoadArrayPair<float>(false);
    CheckLoadArrayPair<double>(true);
    CheckLoadArrayPair<double>(false);
}

#ifndef USE_ADDRESS_SANITIZER
TEST_F(CodegenTest, CheckCodegenBounds)
{
    // Do not try to encode too large graph
    uint64_t instsPerByte = GetGraph()->GetEncoder()->MaxArchInstPerEncoded();
    uint64_t maxBitsInInst = GetInstructionSizeBits(GetGraph()->GetArch());
    uint64_t instCount = g_options.GetCompilerMaxGenCodeSize() / (instsPerByte * maxBitsInInst);

    CheckBounds<uint32_t>(instCount - 1L);
    CheckBounds<uint32_t>(instCount + 1U);

    CheckBounds<uint32_t>(instCount / 2U);
}
#endif

TEST_F(CodegenTest, LenArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LenArray).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0U);
    SetNumArgs(1U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    uint64_t array[4U] = {0U, 0U, 0U, 0U};
    auto param1 = GetExecModule().CreateArray(array, 4U, GetObjectAllocator());
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));

    GetExecModule().Execute();
    GetExecModule().FreeArray(param1);

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 4U);
}

TEST_F(CodegenTest, Parameter)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).s16();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(8U, Opcode::Add).s16().Inputs(1U, 1U);
            INST(15U, Opcode::Return).u64().Inputs(6U);
            // Return parameter_0 + parameter_0
        }
    }
    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    auto graph = GetGraph();

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(1234U, DataType::UINT64);
    auto param2 = CutValue<int16_t>(-1234L, DataType::INT16);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 1234U + 1234U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

TEST_F(CodegenTest, RegallocTwoFreeRegs)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);   // const a = 1
        CONSTANT(1U, 10U);  // const b = 10
        CONSTANT(2U, 20U);  // const c = 20

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {3U, 7U}});                      // var a' = a
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 8U}});                      // var b' = b
            INST(5U, Opcode::Compare).b().CC(CC_NE).Inputs(4U, 0U);                        // b' == 1 ?
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);  // if == (b' == 1) -> exit
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(3U, 4U);  // a' = a' * b'
            INST(8U, Opcode::Sub).u64().Inputs(4U, 0U);  // b' = b' - 1
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);  // a' = c + a'
            INST(11U, Opcode::Return).u64().Inputs(10U);  // return a'
        }
    }
    // Create reg_mask with 5 available general registers,
    // 3 of them will be reserved by Reg Alloc.
    {
        RegAllocLinearScan ra(GetGraph());
        ra.SetRegMask(RegMask {0xFFFFF07FU});
        ra.SetVRegMask(VRegMask {0U});
        EXPECT_TRUE(ra.Run());
    }
    GraphChecker(GetGraph()).Check();
    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(0x0U, DataType::UINT64);
    auto param2 = CutValue<uint16_t>(0x0U, DataType::INT32);

    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_TRUE(retData == 10U * 9U * 8U * 7U * 6U * 5U * 4U * 3U * 2U * 1U + 20U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

// NOTE (igorban): Update FillSaveStates() with filling SaveState from SpillFill
TEST_F(CodegenTest, DISABLED_TwoFreeRegsAdditionSaveState)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(11U, 0U).f64();
        PARAMETER(12U, 0U).f32();
        CONSTANT(1U, 12U);
        CONSTANT(2U, -1L);
        CONSTANT(3U, 100000000U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(7U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Sub).u64().Inputs(0U, 2U);
            INST(9U, Opcode::Sub).u64().Inputs(0U, 3U);
            INST(17U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(18U, Opcode::Sub).u64().Inputs(0U, 0U);
            INST(19U, Opcode::Add).u16().Inputs(0U, 1U);
            INST(20U, Opcode::Add).u16().Inputs(0U, 2U);
            INST(10U, Opcode::SaveState)
                .Inputs(0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 11U, 12U, 17U, 18U, 19U, 20U)
                .SrcVregs({0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U});
        }
    }
    // NO RETURN value - will droped down to SaveState block!

    SetNumVirtRegs(0U);
    SetNumArgs(3U);
    // Create reg_mask with 5 available general registers,
    // 3 of them will be reserved by Reg Alloc.
    {
        RegAllocLinearScan ra(GetGraph());
        ra.SetRegMask(RegMask {0xFFFFF07FU});
        ra.SetVRegMask(VRegMask {0U});
        EXPECT_TRUE(ra.Run());
    }
    GraphChecker(GetGraph()).Check();

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(0x12345U, DataType::UINT64);
    auto param2 = CutValue<float>(0x12345U, DataType::FLOAT32);

    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param2);

    GetExecModule().Execute();

    // Main check - return value get from SaveState return DEOPTIMIZATION
    EXPECT_EQ(GetExecModule().GetRetValue(), 1U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

TEST_F(CodegenTest, SaveState)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u64();  // index
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 6U);
            INST(8U, Opcode::StoreArray).u64().Inputs(3U, 5U, 7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(7U, 7U);  // Some return value
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    auto graph = GetGraph();

    RegAlloc(graph);

    // Run codegen
    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // Enable dumping
    GetExecModule().SetDump(false);

    uint64_t arrayData[4U];
    for (size_t i = 0; i < 4U; i++) {
        arrayData[i] = i + 0x20U;
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 4U, GetObjectAllocator());
    auto param2 = CutValue<uint64_t>(1U, DataType::UINT64);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();
    GetExecModule().SetDump(false);
    // End dump

    auto retData = GetExecModule().GetRetValue();
    // NOTE (igorban) : really need to check array changes
    EXPECT_EQ(retData, 4U * 0x21);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
    GetExecModule().FreeArray(param1);
}  // namespace panda::compiler

TEST_F(CodegenTest, DeoptimizeIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::DeoptimizeIf).Inputs(0U, 2U);
            INST(4U, Opcode::Return).b().Inputs(0U);
        }
    }
    RegAlloc(GetGraph());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // param == false [OK]
    auto param = false;
    GetExecModule().SetParameter(0U, param);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param);

    // param == true [INTERPRET]
}

TEST_F(CodegenTest, ZeroCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s64().Inputs(0U, 2U);
            INST(4U, Opcode::Div).s64().Inputs(1U, 3U);
            INST(5U, Opcode::Mod).s64().Inputs(1U, 3U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Add).s64().Inputs(0U, 1U);  // Some return value
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }
    RegAlloc(GetGraph());

    SetNumVirtRegs(GetGraph()->GetVRegsCount());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // param1 < 0 [OK]
    auto param1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::min(), DataType::INT64);
    auto param2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 > 0 [OK]
    param1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    param2 = CutValue<uint64_t>(0U, DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 == 0 [THROW]
}

TEST_F(CodegenTest, NegativeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NegativeCheck).s64().Inputs(0U, 2U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Add).s64().Inputs(0U, 1U);  // Some return value
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }
    RegAlloc(GetGraph());

    SetNumVirtRegs(GetGraph()->GetVRegsCount());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // param1 > 0 [OK]
    auto param1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    auto param2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::min(), DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 == 0 [OK]
    param1 = CutValue<uint64_t>(0U, DataType::INT64);
    param2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 < 0 [THROW]
}

TEST_F(CodegenTest, NullCheckBoundsCheck)
{
    static const unsigned ARRAY_LEN = 10;

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u64();  // index
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 6U);
            INST(8U, Opcode::StoreArray).u64().Inputs(3U, 5U, 7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(7U, 7U);  // Some return value
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
    SetNumVirtRegs(0U);
    SetNumArgs(2U);
    RegAlloc(GetGraph());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // NOTE (igorban) : fill Frame array == nullptr [THROW]

    uint64_t array[ARRAY_LEN];
    for (auto i = 0U; i < ARRAY_LEN; i++) {
        array[i] = i + 0x20U;
    }
    auto param1 = GetExecModule().CreateArray(array, ARRAY_LEN, GetObjectAllocator());
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));

    // 0 <= index < ARRAY_LEN [OK]
    auto index = CutValue<uint64_t>(1U, DataType::UINT64);
    GetExecModule().SetParameter(1U, index);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), array[index] * 4U);

    /*
    NOTE (igorban) : fill Frame
    // index < 0 [THROW]
    */
    GetExecModule().FreeArray(param1);
}

TEST_F(CodegenTest, ResolveParamSequence)
{
    ArenaVector<std::pair<uint8_t, uint8_t>> someSequence(GetAllocator()->Adapter());
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(0U, 3U));
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(1U, 0U));
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(3U, 2U));

    auto result = ResoveParameterSequence(&someSequence, 13U, GetAllocator());
    EXPECT_TRUE(someSequence.empty());
    ArenaVector<std::pair<uint8_t, uint8_t>> resultSequence(GetAllocator()->Adapter());
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(1U, 0U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(0U, 3U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(13U, 2U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(3U, 13U));

    EXPECT_EQ(result, resultSequence);

    {
        // Special loop-only case
        ArenaVector<std::pair<uint8_t, uint8_t>> someSequence(GetAllocator()->Adapter());
        someSequence.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
        someSequence.emplace_back(std::pair<uint8_t, uint8_t>(1U, 2U));
        someSequence.emplace_back(std::pair<uint8_t, uint8_t>(4U, 1U));
        someSequence.emplace_back(std::pair<uint8_t, uint8_t>(0U, 4U));
        someSequence.emplace_back(std::pair<uint8_t, uint8_t>(3U, 0U));

        auto result = ResoveParameterSequence(&someSequence, 13U, GetAllocator());
        EXPECT_TRUE(someSequence.empty());
        ArenaVector<std::pair<uint8_t, uint8_t>> resultSequence(GetAllocator()->Adapter());
        resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(13U, 2U));
        resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
        resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(3U, 0U));
        resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(0U, 4U));
        resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(4U, 1U));
        resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(1U, 13U));

        EXPECT_EQ(result, resultSequence);
    }
    const uint32_t regSize = 30;
    const uint8_t tmpReg = regSize + 5U;
    for (uint64_t i = 0; i < ITERATION; ++i) {
        EXPECT_TRUE(someSequence.empty());

        std::vector<uint8_t> iters;
        for (uint8_t j = 0; j < regSize; ++j) {
            iters.push_back(j);
        }
        std::shuffle(iters.begin(), iters.end(), g_randomGenerator);
        std::vector<std::pair<uint8_t, uint8_t>> origVector;
        for (uint8_t j = 0; j < regSize; ++j) {
            auto gen {g_randomGenerator()};
            auto randomValue = gen % regSize;
            origVector.emplace_back(std::pair<uint8_t, uint8_t>(iters[j], randomValue));
        }
        for (auto &pair : origVector) {
            someSequence.emplace_back(pair);
        }
        resultSequence = ResoveParameterSequence(&someSequence, tmpReg, GetAllocator());
        std::vector<std::pair<uint8_t, uint8_t>> result;
        for (auto &pair : resultSequence) {
            result.emplace_back(pair);
        }

        // First analysis - there are no dst before src
        for (uint8_t j = 0; j < regSize; ++j) {
            auto dst = result[j].first;
            for (uint8_t k = j + 1U; k < regSize; ++k) {
                if (result[k].second == dst && result[k].second != tmpReg) {
                    std::cerr << " first = " << result[k].first << " tmp = " << (regSize + 5U) << "\n";
                    std::cerr << " Before:\n";
                    for (auto &it : origVector) {
                        std::cerr << " " << (size_t)it.first << "<-" << (size_t)it.second << "\n";
                    }
                    std::cerr << " After:\n";
                    for (auto &it : result) {
                        std::cerr << " " << (size_t)it.first << "<-" << (size_t)it.second << "\n";
                    }
                    std::cerr << "Fault on " << (size_t)j << " and " << (size_t)k << "\n";
                    EXPECT_NE(result[k].second, dst);
                }
            }
        }

        // Second analysis - if remove all same moves - there will be
        // only " move tmp <- reg " & " mov reg <- tmp"
    }
}

TEST_F(CodegenTest, BoundsCheckI)
{
    uint64_t arrayData[4098U];
    for (unsigned i = 0; i < 4098U; i++) {
        arrayData[i] = i;
    }

    for (unsigned index = 4095U; index <= 4097U; index++) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();  // array
            BASIC_BLOCK(2U, -1L)
            {
                INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
                INST(3U, Opcode::LenArray).s32().Inputs(2U);
                INST(4U, Opcode::BoundsCheckI).s32().Inputs(3U, 1U).Imm(index);
                INST(5U, Opcode::LoadArrayI).u64().Inputs(2U).Imm(index);
                INST(6U, Opcode::Return).u64().Inputs(5U);
            }
        }

        SetNumVirtRegs(0U);
        SetNumArgs(1U);

        RegAlloc(graph);

        // Run codegen
        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        // Enable dumping
        GetExecModule().SetDump(false);

        auto param = GetExecModule().CreateArray(arrayData, index + 1U, GetObjectAllocator());
        GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param));

        GetExecModule().Execute();
        GetExecModule().SetDump(false);
        // End dump

        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, index);

        GetExecModule().FreeArray(param);
    }
}

TEST_F(CodegenTest, MultiplyAddInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 42U);
        CONSTANT(2U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MAdd).s64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), 433U);
}

TEST_F(CodegenTest, MultiplyAddFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10.0_D);
        CONSTANT(1U, 42.0_D);
        CONSTANT(2U, 13.0_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MAdd).f64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).f64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), 433.0_D);
}

TEST_F(CodegenTest, MultiplySubtractInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 42U);
        CONSTANT(2U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MSub).s64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), -407L);
}

TEST_F(CodegenTest, MultiplySubtractFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10.0_D);
        CONSTANT(1U, 42.0_D);
        CONSTANT(2U, 13.0_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MSub).f64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).f64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), -407.0_D);
}

TEST_F(CodegenTest, MultiplyNegateInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 5U);
        CONSTANT(1U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::MNeg).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), -25L);
}

TEST_F(CodegenTest, MultiplyNegateFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 5.0_D);
        CONSTANT(1U, 5.0_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::MNeg).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), -25.0_D);
}

TEST_F(CodegenTest, OrNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x0000beefU);
        CONSTANT(1U, 0x2152ffffU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::OrNot).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), 0xdeadbeefU);
}

TEST_F(CodegenTest, AndNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xf0000003U);
        CONSTANT(1U, 0x1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AndNot).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), 0xf0000002U);
}

TEST_F(CodegenTest, XorNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xf0f1ffd0U);
        CONSTANT(1U, 0xcf0fc0f1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::XorNot).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), 0xc001c0deU);
}

TEST_F(CodegenTest, AddSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AddSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::AddSR, 10U, 2U, ShiftType::LSL, 1U, 14U);
}

TEST_F(CodegenTest, SubSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "SubSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::SubSR, 10U, 4U, ShiftType::LSR, 2U, 9U);
}

TEST_F(CodegenTest, AndSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AndSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::AndSR, 1U, 1U, ShiftType::LSL, 1U, 0U);
}

TEST_F(CodegenTest, OrSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "OrSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::OrSR, 1U, 1U, ShiftType::LSL, 1U, 3U);
}

TEST_F(CodegenTest, XorSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "XorSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::XorSR, 3U, 1U, ShiftType::LSL, 1U, 1U);
}

TEST_F(CodegenTest, AndNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AndNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::AndNotSR, 6U, 12U, ShiftType::LSR, 2U, 4U);
}

TEST_F(CodegenTest, OrNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "OrNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::OrNotSR, 1U, 12U, ShiftType::LSR, 2U, 0xfffffffdU);
}

TEST_F(CodegenTest, XorNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "XorNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::XorNotSR, -1L, 12U, ShiftType::LSR, 2U, 3U);
}

TEST_F(CodegenTest, NegSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "NegSR instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x80000000U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::NegSR).Shift(ShiftType::ASR, 1U).u32().Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }

    CheckReturnValue(GetGraph(), 0x40000000U);
}

TEST_F(CodegenTest, LoadArrayPairLivenessInfo)
{
    auto graph = GetGraph();

    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::LoadArrayPair).s32().Inputs(0U, 1U);
            INST(4U, Opcode::LoadPairPart).s32().Inputs(2U).Imm(0U);
            INST(5U, Opcode::LoadPairPart).s32().Inputs(2U).Imm(1U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(10U, Opcode::LoadClass)
                .ref()
                .Inputs(12U)
                .TypeId(42U)
                .Class(reinterpret_cast<RuntimeInterface::ClassPtr>(1U));
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 10U, 12U).TypeId(42U);
            INST(6U, Opcode::Cast).s32().SrcType(DataType::BOOL).Inputs(3U);
            INST(7U, Opcode::Add).s32().Inputs(4U, 5U);
            INST(8U, Opcode::Add).s32().Inputs(7U, 6U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
    }

    SetNumVirtRegs(0U);
    SetNumArgs(2U);
    RegAlloc(graph);
    EXPECT_TRUE(RunCodegen(graph));

    RegMask ldpRegs {};

    auto cg = Codegen(graph);
    for (auto &bb : graph->GetBlocksLinearOrder()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->GetOpcode() == Opcode::LoadArrayPair) {
                ldpRegs.set(inst->GetDstReg(0U));
                ldpRegs.set(inst->GetDstReg(1U));
            } else if (inst->GetOpcode() == Opcode::IsInstance) {
                auto liveRegs = cg.GetLiveRegisters(inst).first;
                // Both dst registers should be alive during IsInstance call
                ASSERT_EQ(ldpRegs & liveRegs, ldpRegs);
            }
        }
    }
}

TEST_F(CodegenTest, CompareAnyTypeInst)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
    graph->SetDynamicStub();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U);
        INS(0U).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::CompareAnyType).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

    SetNumVirtRegs(0U);
    ASSERT_TRUE(RegAlloc(graph));
    ASSERT_TRUE(RunCodegen(graph));

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();

    ASSERT(codeEntry != nullptr && codeExit != nullptr);

    GetExecModule().SetInstructions(codeEntry, codeExit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<bool>();
    EXPECT_EQ(rv, true);
}

TEST_F(CodegenTest, CastAnyTypeValueInst)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
    graph->SetDynamicStub();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U);
        INS(0U).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::CastAnyTypeValue).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

    SetNumVirtRegs(0U);
    ASSERT_TRUE(RegAlloc(graph));
    ASSERT_TRUE(RunCodegen(graph));

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();

    ASSERT(codeEntry != nullptr && codeExit != nullptr);

    GetExecModule().SetInstructions(codeEntry, codeExit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<uint32_t>();
    EXPECT_EQ(rv, 0U);
}
// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace panda::compiler
