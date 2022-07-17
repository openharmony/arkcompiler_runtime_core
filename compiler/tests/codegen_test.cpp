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

#include <random>

#include "optimizer/code_generator/codegen.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/if_conversion.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer_run.h"

#include "gtest/gtest.h"
#include "libpandabase/macros.h"
#include "unit_test.h"
#include "utils/bit_utils.h"
#include "vixl_exec_module.h"

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 40;
#else
const uint64_t ITERATION = 20000;
#endif
static inline auto random_generator = std::mt19937_64(SEED);

namespace panda::compiler {
namespace {
template <typename T>
bool Compare(ConditionCode cc, T lhs, T rhs)
{
    using signed_t = std::make_signed_t<T>;
    using unsigned_t = std::make_unsigned_t<T>;
    unsigned_t lhs_u = bit_cast<unsigned_t>(lhs);
    unsigned_t rhs_u = bit_cast<unsigned_t>(rhs);
    signed_t lhs_s = bit_cast<signed_t>(lhs);
    signed_t rhs_s = bit_cast<signed_t>(rhs);

    switch (cc) {
        case ConditionCode::CC_EQ:
            return lhs_u == rhs_u;
        case ConditionCode::CC_NE:
            return lhs_u != rhs_u;
        case ConditionCode::CC_LT:
            return lhs_s < rhs_s;
        case ConditionCode::CC_LE:
            return lhs_s <= rhs_s;
        case ConditionCode::CC_GT:
            return lhs_s > rhs_s;
        case ConditionCode::CC_GE:
            return lhs_s >= rhs_s;
        case ConditionCode::CC_B:
            return lhs_u < rhs_u;
        case ConditionCode::CC_BE:
            return lhs_u <= rhs_u;
        case ConditionCode::CC_A:
            return lhs_u > rhs_u;
        case ConditionCode::CC_AE:
            return lhs_u >= rhs_u;
        case ConditionCode::CC_TST_EQ:
            return (lhs_u & rhs_u) == 0;
        case ConditionCode::CC_TST_NE:
            return (lhs_u & rhs_u) != 0;
        default:
            UNREACHABLE();
            return false;
    }
}
}  // namespace

class CodegenTest : public GraphTest {
public:
    CodegenTest() : exec_module_(GetAllocator(), GetGraph()->GetRuntime())
    {
#ifndef NDEBUG
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
    }
    ~CodegenTest() override {}

    VixlExecModule &GetExecModule()
    {
        return exec_module_;
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
    void CheckCmp(bool is_fcmpg = false);

    template <typename T>
    void CheckReturnValue(Graph *graph, T expected_value);

    template <typename T>
    void CheckBounds(uint64_t count);

    void TestBinaryOperationWithShiftedOperand(Opcode opcode, uint32_t l, uint32_t r, ShiftType shift_type,
                                               uint32_t shift, uint32_t erv);

private:
    VixlExecModule exec_module_;
};

bool RunCodegen(Graph *graph)
{
    if (!graph->RunPass<Codegen>()) {
        return false;
    }
    return true;
}

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
        CONSTANT(0, 10UL);          // r1
        CONSTANT(1, 4294967296UL);  // r2
        CONSTANT(2, 0UL);           // r3 -> acc(3)
        CONSTANT(3, 0x1UL);         // r20 -> 0x1 (for inc constant)

        BASIC_BLOCK(2, 4, 3)
        {
            INST(16, Opcode::Phi).Inputs(2, 13).s64();  // PHI acc
            INST(17, Opcode::Phi).Inputs(1, 10).s64();  // PHI  v1
            INST(20, Opcode::Phi).Inputs(2, 10).s64();  // result to return

            // TODO (igorban): support CMP instr
            INST(18, Opcode::Compare).b().CC(CC_NE).Inputs(0, 16);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(10, Opcode::And).Inputs(16, 17).s64();  // -> acc
            INST(13, Opcode::Add).Inputs(16, 3).s64();   // -> acc
        }

        BASIC_BLOCK(4, -1)
        {
            INST(19, Opcode::Return).Inputs(20).s64();
        }
    }

    SetNumVirtRegs(0);
    SetNumArgs(1);

    RegAlloc(GetGraph());

    // call codegen
    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto exit = entry + GetGraph()->GetData().Size();
    ASSERT(entry != nullptr && exit != nullptr);
    GetExecModule().SetInstructions(entry, exit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();

    auto ret_data = GetExecModule().GetRetValue();
    EXPECT_EQ(ret_data, 0U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

template <typename T>
void CodegenTest::CheckStoreArray()
{
    constexpr DataType::Type type = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0, DataType::REFERENCE);
    auto index = graph->AddNewParameter(1, DataType::INT32);
    auto store_value = graph->AddNewParameter(2, type);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));
    store_value->SetLocationData(graph->GetDataForNativeParam(type));

    auto st_arr = graph->CreateInst(Opcode::StoreArray);
    block->AppendInst(st_arr);
    st_arr->SetType(type);
    st_arr->SetInput(0, array);
    st_arr->SetInput(1, index);
    st_arr->SetInput(2, store_value);
    auto ret = graph->CreateInst(Opcode::ReturnVoid);
    block->AppendInst(ret);

    SetNumVirtRegs(0);
    SetNumArgs(3);

    RegAlloc(graph);

    // call codegen
    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    T array_data[4];
    auto default_value = CutValue<T>(0, type);
    for (auto i = 0; i < 4; i++) {
        array_data[i] = default_value;
    }
    auto param_1 = GetExecModule().CreateArray(array_data, 4, GetObjectAllocator());
    auto param_2 = CutValue<int32_t>(2, DataType::INT32);
    auto param_3 = CutValue<T>(10, type);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().SetParameter(2U, param_3);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param_1, array_data);

    for (auto i = 0; i < 4; i++) {
        if (i == 2) {
            EXPECT_EQ(array_data[i], param_3);
        } else {
            EXPECT_EQ(array_data[i], default_value);
        }
    }
    GetExecModule().FreeArray(param_1);
}

template <typename T>
void CodegenTest::CheckLoadArray()
{
    constexpr DataType::Type type = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0, DataType::REFERENCE);
    auto index = graph->AddNewParameter(1, DataType::INT32);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));

    auto ld_arr = graph->CreateInst(Opcode::LoadArray);
    block->AppendInst(ld_arr);
    ld_arr->SetType(type);
    ld_arr->SetInput(0, array);
    ld_arr->SetInput(1, index);
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(type);
    ret->SetInput(0, ld_arr);
    block->AppendInst(ret);

    SetNumVirtRegs(0);
    SetNumArgs(2);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    T array_data[4];
    for (auto i = 0; i < 4; i++) {
        array_data[i] = CutValue<T>((-i), type);
    }
    auto param_1 = GetExecModule().CreateArray(array_data, 4, GetObjectAllocator());
    auto param_2 = CutValue<int32_t>(2, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param_1, array_data);

    GetExecModule().FreeArray(param_1);

    auto ret_data = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(ret_data, CutValue<T>(-2, type));
}

template <typename T>
void CodegenTest::CheckStoreArrayPair(bool imm)
{
    constexpr DataType::Type type = VixlExecModule::GetType<T>();

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

    auto array = graph->AddNewParameter(0, DataType::REFERENCE);
    [[maybe_unused]] auto index = graph->AddNewParameter(1, DataType::INT32);
    auto val0 = graph->AddNewParameter(2, type);
    auto val1 = graph->AddNewParameter(3, type);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));
    val0->SetLocationData(graph->GetDataForNativeParam(type));
    val1->SetLocationData(graph->GetDataForNativeParam(type));

    Inst *stp_arr = nullptr;
    if (imm) {
        stp_arr = graph->CreateInstStoreArrayPairI(2);
        block->AppendInst(stp_arr);
        stp_arr->SetType(type);
        stp_arr->SetInput(0, array);
        stp_arr->SetInput(1, val0);
        stp_arr->SetInput(2, val1);
    } else {
        stp_arr = graph->CreateInstStoreArrayPair();
        block->AppendInst(stp_arr);
        stp_arr->SetType(type);
        stp_arr->SetInput(0, array);
        stp_arr->SetInput(1, index);
        stp_arr->SetInput(2, val0);
        stp_arr->SetInput(3, val1);
    }

    auto ret = graph->CreateInst(Opcode::ReturnVoid);
    block->AppendInst(ret);

    GraphChecker(graph).Check();

    SetNumVirtRegs(0);
    SetNumArgs(4);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    T array_data[6] = {0, 0, 0, 0, 0, 0};
    auto param_1 = GetExecModule().CreateArray(array_data, 6, GetObjectAllocator());
    auto param_2 = CutValue<int32_t>(2, DataType::INT32);
    auto param_3 = CutValue<T>(3, type);
    auto param_4 = CutValue<T>(5, type);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().SetParameter(2U, param_3);
    GetExecModule().SetParameter(3U, param_4);

    GetExecModule().Execute();
    GetExecModule().CopyArray(param_1, array_data);
    GetExecModule().FreeArray(param_1);

    T array_expected[6] = {0, 0, 3, 5, 0, 0};

    for (auto i = 0; i < 6; ++i) {
        EXPECT_EQ(array_data[i], array_expected[i]);
    }
}

template <typename T>
void CodegenTest::CheckLoadArrayPair(bool imm)
{
    constexpr DataType::Type type = VixlExecModule::GetType<T>();

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

    auto array = graph->AddNewParameter(0, DataType::REFERENCE);
    [[maybe_unused]] auto index = graph->AddNewParameter(1, DataType::INT32);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));

    Inst *ldp_arr = nullptr;
    if (imm) {
        ldp_arr = graph->CreateInstLoadArrayPairI(2);
        block->AppendInst(ldp_arr);
        ldp_arr->SetType(type);
        ldp_arr->SetInput(0, array);
    } else {
        ldp_arr = graph->CreateInstLoadArrayPair();
        block->AppendInst(ldp_arr);
        ldp_arr->SetType(type);
        ldp_arr->SetInput(0, array);
        ldp_arr->SetInput(1, index);
    }

    auto load_high = graph->CreateInstLoadPairPart(0);
    block->AppendInst(load_high);
    load_high->SetType(type);
    load_high->SetInput(0, ldp_arr);

    auto load_low = graph->CreateInstLoadPairPart(1);
    block->AppendInst(load_low);
    load_low->SetType(type);
    load_low->SetInput(0, ldp_arr);

    auto sum = graph->CreateInst(Opcode::Add);
    block->AppendInst(sum);
    sum->SetType(type);
    sum->SetInput(0, load_high);
    sum->SetInput(1, load_low);

    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(type);
    ret->SetInput(0, sum);
    block->AppendInst(ret);

    GraphChecker(graph).Check();

    SetNumVirtRegs(0);
    SetNumArgs(2);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    T array_data[6];
    // [ 1, 2, 3, 4, 5, 6] -> 7
    for (auto i = 0; i < 6; i++) {
        array_data[i] = CutValue<T>(i + 1, type);
    }
    auto param_1 = GetExecModule().CreateArray(array_data, 6, GetObjectAllocator());
    auto param_2 = CutValue<int32_t>(2, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);

    GetExecModule().Execute();
    GetExecModule().FreeArray(param_1);

    auto ret_data = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(ret_data, CutValue<T>(7, type));
}

template <typename T>
void CodegenTest::CheckBounds(uint64_t count)
{
    constexpr DataType::Type type = VixlExecModule::GetType<T>();
    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto param = graph->AddNewParameter(0, type);

    graph->ResetParameterInfo();
    param->SetLocationData(graph->GetDataForNativeParam(type));

    BinaryImmOperation *last_inst = nullptr;
    // instruction_count + parameter + return
    for (uint64_t i = count - 1; i > 1; --i) {
        auto add_inst = graph->CreateInstAddI(type, 0, 1);
        block->AppendInst(add_inst);
        if (last_inst == nullptr) {
            add_inst->SetInput(0, param);
        } else {
            add_inst->SetInput(0, last_inst);
        }
        last_inst = add_inst;
    }
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(type);
    ret->SetInput(0, last_inst);
    block->AppendInst(ret);

#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif
    GraphChecker(graph).Check();

    SetNumVirtRegs(0);
    SetNumArgs(2);

    RegAlloc(graph);

    auto insts_per_byte = GetGraph()->GetEncoder()->MaxArchInstPerEncoded();
    auto max_bits_in_inst = GetInstructionSizeBits(GetGraph()->GetArch());
    if (count * insts_per_byte * max_bits_in_inst > options.GetCompilerMaxGenCodeSize()) {
        EXPECT_FALSE(RunCodegen(graph));
    } else {
        ASSERT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);

        GetExecModule().SetDump(false);

        T param = 0;
        GetExecModule().SetParameter(0U, param);
        GetExecModule().Execute();

        auto ret_data = GetExecModule().GetRetValue<T>();
        EXPECT_EQ(ret_data, CutValue<T>(count - 2, type));
    }
}

template <typename T>
void CodegenTest::CheckCmp(bool is_fcmpg)
{
    constexpr DataType::Type type = VixlExecModule::GetType<T>();
    bool is_float = DataType::IsFloatType(type);

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto param1 = graph->AddNewParameter(0, type);
    auto param2 = graph->AddNewParameter(1, type);

    graph->ResetParameterInfo();
    param1->SetLocationData(graph->GetDataForNativeParam(type));
    param2->SetLocationData(graph->GetDataForNativeParam(type));

    auto fcmp = graph->CreateInst(Opcode::Cmp);
    block->AppendInst(fcmp);
    fcmp->SetType(DataType::INT32);
    fcmp->SetInput(0, param1);
    fcmp->SetInput(1, param2);
    static_cast<CmpInst *>(fcmp)->SetOperandsType(type);
    if (is_float) {
        static_cast<CmpInst *>(fcmp)->SetFcmpg(is_fcmpg);
    }
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(DataType::INT32);
    ret->SetInput(0, fcmp);
    block->AppendInst(ret);

    SetNumVirtRegs(0);
    SetNumArgs(2);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);
    T param_data[3];
    if (type == DataType::FLOAT32) {
        param_data[0] = std::nanf("0");
    } else if (type == DataType::FLOAT64) {
        param_data[0] = std::nan("0");
    } else {
        param_data[0] = std::numeric_limits<T>::max();
        param_data[2] = std::numeric_limits<T>::min();
    }
    param_data[1] = CutValue<T>(2, type);
    if (is_float) {
        param_data[2] = -param_data[1];
    }

    for (auto i = 0; i < 3; i++) {
        for (auto j = 0; j < 3; j++) {
            auto param_1 = param_data[i];
            auto param_2 = param_data[j];
            GetExecModule().SetParameter(0U, param_1);
            GetExecModule().SetParameter(1U, param_2);

            GetExecModule().Execute();

            auto ret_data = GetExecModule().GetRetValue<int32_t>();
            if ((i == 0 || j == 0) && is_float) {
                EXPECT_EQ(ret_data, is_fcmpg ? 1 : -1);
            } else if (i == j) {
                EXPECT_EQ(ret_data, 0);
            } else if (i > j) {
                EXPECT_EQ(ret_data, -1);
            } else {
                EXPECT_EQ(ret_data, 1);
            }
        }
    }
}

template <typename T>
void CodegenTest::CheckReturnValue(Graph *graph, [[maybe_unused]] T expected_value)
{
    SetNumVirtRegs(0);
    RegAlloc(graph);
    EXPECT_TRUE(RunCodegen(graph));

    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();

    ASSERT(code_entry != nullptr && code_exit != nullptr);

    GetExecModule().SetInstructions(code_entry, code_exit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(rv, expected_value);
}

void CodegenTest::TestBinaryOperationWithShiftedOperand(Opcode opcode, uint32_t l, uint32_t r, ShiftType shift_type,
                                                        uint32_t shift, uint32_t erv)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, l);
        CONSTANT(1, r);

        BASIC_BLOCK(2, -1)
        {
            INST(2, opcode).Shift(shift_type, shift).u32().Inputs(0, 1);
            INST(3, Opcode::Return).u32().Inputs(2);
        }
    }

    CheckReturnValue(GetGraph(), erv);
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
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).u32();  // index
        PARAMETER(2, 2).u32();  // store value
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::StoreArray).u32().Inputs(0, 1, 2);
            INST(4, Opcode::ReturnVoid);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0);
    SetNumArgs(3);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    object_pointer_type array[4] = {0, 0, 0, 0};
    auto param_1 = GetExecModule().CreateArray(array, 4, GetObjectAllocator());
    auto param_2 = CutValue<int32_t>(2, DataType::INT32);
    auto param_3 = CutValue<object_pointer_type>(10, DataType::UINT64);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().SetParameter(2U, param_3);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param_1, array);

    for (auto i = 0; i < 4; i++) {
        if (i == 2) {
            EXPECT_EQ(array[i], 10U) << "value of i: " << i;
        } else {
            EXPECT_EQ(array[i], 0U) << "value of i: " << i;
        }
    }
    GetExecModule().FreeArray(param_1);
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
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        for (auto inverse : {true, false}) {
            auto graph = CreateGraphStartEndBlocks();
            RuntimeInterfaceMock runtime;
            graph->SetRuntime(&runtime);

            GRAPH(graph)
            {
                PARAMETER(0, 0).u64();
                PARAMETER(1, 1).u64();
                CONSTANT(2, 0);
                CONSTANT(3, 1);
                BASIC_BLOCK(2, 3, 4)
                {
                    INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                    INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0).Inputs(4);
                }
                BASIC_BLOCK(3, -1)
                {
                    INST(6, Opcode::Return).b().Inputs(3);
                }
                BASIC_BLOCK(4, -1)
                {
                    INST(7, Opcode::Return).b().Inputs(2);
                }
            }
            SetNumVirtRegs(0);
            SetNumArgs(2);

            RegAlloc(graph);

            EXPECT_TRUE(RunCodegen(graph));
            auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
            auto code_exit = code_entry + graph->GetData().Size();
            ASSERT(code_entry != nullptr && code_exit != nullptr);
            GetExecModule().SetInstructions(code_entry, code_exit);

            GetExecModule().SetDump(false);

            bool result;
            auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
            auto param_2 = CutValue<uint64_t>(-1, DataType::UINT64);

            GetExecModule().SetParameter(0U, param_1);
            GetExecModule().SetParameter(1U, param_2);

            result = (cc == CC_NE || cc == CC_GT || cc == CC_GE || cc == CC_B || cc == CC_BE);
            if (inverse) {
                result = !result;
            }

            GetExecModule().Execute();

            auto ret_data = GetExecModule().GetRetValue();
            EXPECT_EQ(ret_data, result);

            GetExecModule().SetParameter(0U, param_2);
            GetExecModule().SetParameter(1U, param_1);

            GetExecModule().Execute();

            result = (cc == CC_NE || cc == CC_LT || cc == CC_LE || cc == CC_A || cc == CC_AE);
            if (inverse) {
                result = !result;
            }

            ret_data = GetExecModule().GetRetValue();
            EXPECT_EQ(ret_data, result);

            GetExecModule().SetParameter(0U, param_1);
            GetExecModule().SetParameter(1U, param_1);

            result = (cc == CC_EQ || cc == CC_LE || cc == CC_GE || cc == CC_AE || cc == CC_BE);
            if (inverse) {
                result = !result;
            }

            GetExecModule().Execute();

            ret_data = GetExecModule().GetRetValue();
            EXPECT_EQ(ret_data, result);
        }
    }
}

TEST_F(CodegenTest, GenIf)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(6, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(7, Opcode::Return).b().Inputs(2);
            }
        }
        SetNumVirtRegs(0);
        SetNumArgs(2);
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_EQ(INS(0).GetUsers().Front().GetInst()->GetOpcode(), Opcode::If);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);

        GetExecModule().SetDump(false);

        bool result;
        auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
        auto param_2 = CutValue<uint64_t>(-1, DataType::UINT64);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_2);
        result = Compare(cc, param_1, param_2);
        GetExecModule().Execute();
        auto ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_2);
        GetExecModule().SetParameter(1U, param_1);
        GetExecModule().Execute();
        result = Compare(cc, param_2, param_1);
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_1);
        result = Compare(cc, param_1, param_1);
        GetExecModule().Execute();
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);
    }
}

TEST_F(CodegenTest, GenIfImm)
{
    int32_t values[3] = {-1, 0, 1};
    for (auto value : values) {
        for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
            ConditionCode cc = static_cast<ConditionCode>(ccint);
            auto graph = CreateGraphStartEndBlocks();
            RuntimeInterfaceMock runtime;
            graph->SetRuntime(&runtime);
            if ((cc == CC_TST_EQ || cc == CC_TST_NE) && !graph->GetEncoder()->CanEncodeImmLogical(value, WORD_SIZE)) {
                continue;
            }

            GRAPH(graph)
            {
                PARAMETER(0, 0).s32();
                CONSTANT(1, 0);
                CONSTANT(2, 1);
                BASIC_BLOCK(2, 3, 4)
                {
                    INST(3, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(value).Inputs(0);
                }
                BASIC_BLOCK(3, -1)
                {
                    INST(4, Opcode::Return).b().Inputs(2);
                }
                BASIC_BLOCK(4, -1)
                {
                    INST(5, Opcode::Return).b().Inputs(1);
                }
            }
            SetNumVirtRegs(0);
            SetNumArgs(2);

            RegAlloc(graph);

            EXPECT_TRUE(RunCodegen(graph));
            auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
            auto code_exit = code_entry + graph->GetData().Size();
            ASSERT(code_entry != nullptr && code_exit != nullptr);
            GetExecModule().SetInstructions(code_entry, code_exit);

            GetExecModule().SetDump(false);

            bool result;
            auto param_1 = CutValue<uint64_t>(value, DataType::INT32);
            auto param_2 = CutValue<uint64_t>(value + 5, DataType::INT32);
            auto param_3 = CutValue<uint64_t>(value - 5, DataType::INT32);

            GetExecModule().SetParameter(0U, param_1);
            result = Compare(cc, param_1, param_1);
            GetExecModule().Execute();
            auto ret_data = GetExecModule().GetRetValue();
            EXPECT_EQ(ret_data, result);

            GetExecModule().SetParameter(0U, param_2);
            GetExecModule().Execute();
            result = Compare(cc, param_2, param_1);
            ret_data = GetExecModule().GetRetValue();
            EXPECT_EQ(ret_data, result);

            GetExecModule().SetParameter(0U, param_3);
            result = Compare(cc, param_3, param_1);
            GetExecModule().Execute();
            ret_data = GetExecModule().GetRetValue();
            EXPECT_EQ(ret_data, result);
        }
    }
}

TEST_F(CodegenTest, If)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(5, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(6, Opcode::Return).b().Inputs(2);
            }
        }
        SetNumVirtRegs(0);
        SetNumArgs(2);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);

        GetExecModule().SetDump(false);

        bool result;
        auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
        auto param_2 = CutValue<uint64_t>(-1, DataType::UINT64);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_2);
        result = Compare(cc, param_1, param_2);
        GetExecModule().Execute();
        auto ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_2);
        GetExecModule().SetParameter(1U, param_1);
        GetExecModule().Execute();
        result = Compare(cc, param_2, param_1);
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_1);
        result = Compare(cc, param_1, param_1);
        GetExecModule().Execute();
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);
    }
}

TEST_F(CodegenTest, AddOverflow)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    GRAPH(graph)
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::AddOverflow).i32().SrcType(DataType::INT32).CC(CC_EQ).Inputs(0, 1);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Return).b().Inputs(4);
        }
    }
    SetNumVirtRegs(0);
    SetNumArgs(2);

    EXPECT_TRUE(RunOptimizations(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    int32_t min = std::numeric_limits<int32_t>::min();
    int32_t max = std::numeric_limits<int32_t>::max();
    std::array<int32_t, 7> values = {0, 2, 5, -7, -10, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            int32_t a0 = values[i];
            int32_t a1 = values[j];
            int32_t result;
            auto param_1 = CutValue<int32_t>(a0, DataType::INT32);
            auto param_2 = CutValue<int32_t>(a1, DataType::INT32);

            if ((a0 > 0 && a1 > max - a0) || (a0 < 0 && a1 < min - a0)) {
                result = 0;
            } else {
                result = a0 + a1;
            }
            GetExecModule().SetParameter(0U, param_1);
            GetExecModule().SetParameter(1U, param_2);
            GetExecModule().Execute();

            auto ret_data = GetExecModule().GetRetValue<int32_t>();
            EXPECT_EQ(ret_data, result);
        }
    }
}

TEST_F(CodegenTest, SubOverflow)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    GRAPH(graph)
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SubOverflow).i32().SrcType(DataType::INT32).CC(CC_EQ).Inputs(0, 1);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Return).b().Inputs(4);
        }
    }
    SetNumVirtRegs(0);
    SetNumArgs(2);

    EXPECT_TRUE(RunOptimizations(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    int32_t min = std::numeric_limits<int32_t>::min();
    int32_t max = std::numeric_limits<int32_t>::max();
    std::array<int32_t, 7> values = {0, 2, 5, -7, -10, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            int32_t a0 = values[i];
            int32_t a1 = values[j];
            int32_t result;
            auto param_1 = CutValue<int32_t>(a0, DataType::INT32);
            auto param_2 = CutValue<int32_t>(a1, DataType::INT32);

            if ((a1 > 0 && a0 < min + a1) || (a1 < 0 && a0 > max + a1)) {
                result = 0;
            } else {
                result = a0 - a1;
            }
            GetExecModule().SetParameter(0U, param_1);
            GetExecModule().SetParameter(1U, param_2);
            GetExecModule().Execute();

            auto ret_data = GetExecModule().GetRetValue<int32_t>();
            EXPECT_EQ(ret_data, result);
        }
    }
}

TEST_F(CodegenTest, GenSelect)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();
            PARAMETER(1, 1).s64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::If).SrcType(DataType::INT64).CC(cc).Inputs(0, 1);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, -1)
            {
                INST(5, Opcode::Phi).b().Inputs({{3, 3}, {2, 2}});
                INST(6, Opcode::Return).b().Inputs(5);
            }
        }
        SetNumVirtRegs(0);
        SetNumArgs(2);

        EXPECT_TRUE(graph->RunPass<IfConversion>());
        ASSERT_EQ(INS(6).GetInput(0).GetInst()->GetOpcode(), Opcode::Select);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);

        GetExecModule().SetDump(false);

        bool result;
        auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
        auto param_2 = CutValue<uint64_t>(-1, DataType::UINT64);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_2);
        result = Compare(cc, param_1, param_2);
        GetExecModule().Execute();
        auto ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_2);
        GetExecModule().SetParameter(1U, param_1);
        GetExecModule().Execute();
        result = Compare(cc, param_2, param_1);
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_1);
        result = Compare(cc, param_1, param_1);
        GetExecModule().Execute();
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);
    }
}

TEST_F(CodegenTest, BoolSelectImm)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, -1)
            {
                INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(5, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3, 2, 4);
                INST(6, Opcode::Return).b().Inputs(5);
            }
        }
        SetNumVirtRegs(0);
        SetNumArgs(2);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);
        GetExecModule().SetDump(false);

        auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
        auto param_2 = CutValue<uint64_t>(-1, DataType::UINT64);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_2);
        bool result = Compare(cc, param_1, param_2);
        GetExecModule().Execute();
        auto ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_2);
        GetExecModule().SetParameter(1U, param_1);
        GetExecModule().Execute();
        result = Compare(cc, param_2, param_1);
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_1);
        result = Compare(cc, param_1, param_1);
        GetExecModule().Execute();
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);
    }
}

TEST_F(CodegenTest, Select)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, -1)
            {
                INST(5, Opcode::Select).u64().SrcType(DataType::UINT64).CC(cc).Inputs(3, 2, 0, 1);
                INST(6, Opcode::Return).u64().Inputs(5);
            }
        }
        SetNumVirtRegs(0);
        SetNumArgs(2);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);
        GetExecModule().SetDump(false);

        auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
        auto param_2 = CutValue<uint64_t>(-1, DataType::UINT64);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_2);
        bool result = Compare(cc, param_1, param_2);
        GetExecModule().Execute();
        auto ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_2);
        GetExecModule().SetParameter(1U, param_1);
        GetExecModule().Execute();
        result = Compare(cc, param_2, param_1);
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);

        GetExecModule().SetParameter(0U, param_1);
        GetExecModule().SetParameter(1U, param_1);
        result = (cc == CC_EQ || cc == CC_LE || cc == CC_GE || cc == CC_AE || cc == CC_BE);
        GetExecModule().Execute();
        ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, result);
    }
}

TEST_F(CodegenTest, CompareObj)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_NE).Inputs(0, 1);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }
    SetNumVirtRegs(0);
    SetNumArgs(1);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    auto param_1 = CutValue<uint64_t>(1, DataType::UINT64);
    auto param_2 = CutValue<uint64_t>(0, DataType::UINT64);

    GetExecModule().SetParameter(0U, param_1);

    GetExecModule().Execute();

    auto ret_data = GetExecModule().GetRetValue();
    EXPECT_EQ(ret_data, 1);

    GetExecModule().SetParameter(0U, param_2);

    GetExecModule().Execute();

    ret_data = GetExecModule().GetRetValue();
    EXPECT_EQ(ret_data, 0);
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
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).u32();  // index
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::LoadArray).u32().Inputs(0, 1);
            INST(3, Opcode::Return).u32().Inputs(2);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0);
    SetNumArgs(2);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    object_pointer_type array[4] = {0xffffaaaa, 0xffffbbbb, 0xffffcccc, 0xffffdddd};
    auto param_1 = GetExecModule().CreateArray(array, 4, GetObjectAllocator());
    auto param_2 = CutValue<int32_t>(2, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param_1, array);

    GetExecModule().FreeArray(param_1);
    auto ret_data = GetExecModule().GetRetValue();
    EXPECT_EQ(ret_data, array[2]);
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
    uint64_t insts_per_byte = GetGraph()->GetEncoder()->MaxArchInstPerEncoded();
    uint64_t max_bits_in_inst = GetInstructionSizeBits(GetGraph()->GetArch());
    uint64_t inst_count = options.GetCompilerMaxGenCodeSize() / (insts_per_byte * max_bits_in_inst);

    CheckBounds<uint32_t>(inst_count - 1);
    CheckBounds<uint32_t>(inst_count + 1);

    CheckBounds<uint32_t>(inst_count / 2);
}
#endif

TEST_F(CodegenTest, LenArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::LenArray).s32().Inputs(0);
            INST(2, Opcode::Return).s32().Inputs(1);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0);
    SetNumArgs(1);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    uint64_t array[4] = {0, 0, 0, 0};
    auto param_1 = GetExecModule().CreateArray(array, 4, GetObjectAllocator());
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));

    GetExecModule().Execute();
    GetExecModule().FreeArray(param_1);

    auto ret_data = GetExecModule().GetRetValue();
    EXPECT_EQ(ret_data, 4U);
}

TEST_F(CodegenTest, Parameter)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).s16();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 0);
            INST(8, Opcode::Add).s16().Inputs(1, 1);
            INST(15, Opcode::Return).u64().Inputs(6);
            // Return parameter_0 + parameter_0
        }
    }
    SetNumVirtRegs(0);
    SetNumArgs(2);

    auto graph = GetGraph();

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    auto param_1 = CutValue<uint64_t>(1234, DataType::UINT64);
    auto param_2 = CutValue<int16_t>(-1234, DataType::INT16);
    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);

    GetExecModule().Execute();

    auto ret_data = GetExecModule().GetRetValue();
    EXPECT_EQ(ret_data, 1234U + 1234U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

TEST_F(CodegenTest, RegallocTwoFreeRegs)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);   // const a = 1
        CONSTANT(1, 10);  // const b = 10
        CONSTANT(2, 20);  // const c = 20

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {3, 7}});                        // var a' = a
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 8}});                        // var b' = b
            INST(5, Opcode::Compare).b().CC(CC_NE).Inputs(4, 0);                        // b' == 1 ?
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);  // if == (b' == 1) -> exit
        }

        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);  // a' = a' * b'
            INST(8, Opcode::Sub).u64().Inputs(4, 0);  // b' = b' - 1
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);   // a' = c + a'
            INST(11, Opcode::Return).u64().Inputs(10);  // return a'
        }
    }
    // Create reg_mask with 5 available general registers,
    // 3 of them will be reserved by Reg Alloc.
    {
        RegAllocLinearScan ra(GetGraph());
        ra.SetRegMask(RegMask {0xFFFFF07F});
        ra.SetVRegMask(VRegMask {0});
        EXPECT_TRUE(ra.Run());
    }
    GraphChecker(GetGraph()).Check();
    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto code_entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    auto param_1 = CutValue<uint64_t>(0x0, DataType::UINT64);
    auto param_2 = CutValue<uint16_t>(0x0, DataType::INT32);

    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);

    GetExecModule().Execute();

    auto ret_data = GetExecModule().GetRetValue();
    EXPECT_TRUE(ret_data == 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1 + 20);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

// TODO (igorban): Update FillSaveStates() with filling SaveState from SpillFill
TEST_F(CodegenTest, DISABLED_TwoFreeRegs_SaveState)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(11, 0).f64();
        PARAMETER(12, 0).f32();
        CONSTANT(1, 12);
        CONSTANT(2, -1);
        CONSTANT(3, 100000000);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            INST(5, Opcode::Add).u64().Inputs(0, 2);
            INST(6, Opcode::Add).u64().Inputs(0, 3);
            INST(7, Opcode::Sub).u64().Inputs(0, 1);
            INST(8, Opcode::Sub).u64().Inputs(0, 2);
            INST(9, Opcode::Sub).u64().Inputs(0, 3);
            INST(17, Opcode::Add).u64().Inputs(0, 0);
            INST(18, Opcode::Sub).u64().Inputs(0, 0);
            INST(19, Opcode::Add).u16().Inputs(0, 1);
            INST(20, Opcode::Add).u16().Inputs(0, 2);
            INST(10, Opcode::SaveState)
                .Inputs(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 17, 18, 19, 20)
                .SrcVregs({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14});
        }
    }
    // NO RETURN value - will droped down to SaveState block!

    SetNumVirtRegs(0);
    SetNumArgs(3);
    // Create reg_mask with 5 available general registers,
    // 3 of them will be reserved by Reg Alloc.
    {
        RegAllocLinearScan ra(GetGraph());
        ra.SetRegMask(RegMask {0xFFFFF07F});
        ra.SetVRegMask(VRegMask {0});
        EXPECT_TRUE(ra.Run());
    }
    GraphChecker(GetGraph()).Check();

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto code_entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    GetExecModule().SetDump(false);

    auto param_1 = CutValue<uint64_t>(0x12345, DataType::UINT64);
    auto param_2 = CutValue<float>(0x12345, DataType::FLOAT32);

    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().SetParameter(2U, param_2);

    GetExecModule().Execute();

    // Main check - return value get from SaveState return DEOPTIMIZATION
    EXPECT_EQ(GetExecModule().GetRetValue(), 1);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

TEST_F(CodegenTest, SaveState)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).u64();  // index
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 6);
            INST(8, Opcode::StoreArray).u64().Inputs(3, 5, 7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(7, 7);  // Some return value
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }

    SetNumVirtRegs(0);
    SetNumArgs(2);

    auto graph = GetGraph();

    RegAlloc(graph);

    // Run codegen
    EXPECT_TRUE(RunCodegen(graph));
    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    // Enable dumping
    GetExecModule().SetDump(false);

    uint64_t array_data[4];
    for (auto i = 0; i < 4; i++) {
        array_data[i] = i + 0x20;
    }
    auto param_1 = GetExecModule().CreateArray(array_data, 4, GetObjectAllocator());
    auto param_2 = CutValue<uint64_t>(1, DataType::UINT64);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));
    GetExecModule().SetParameter(1U, param_2);

    GetExecModule().Execute();
    GetExecModule().SetDump(false);
    // End dump

    auto ret_data = GetExecModule().GetRetValue();
    // TODO (igorban) : really need to check array changes
    EXPECT_EQ(ret_data, 4U * 0x21);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
    GetExecModule().FreeArray(param_1);
}  // namespace panda::compiler

TEST_F(CodegenTest, DeoptimizeIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(3, Opcode::DeoptimizeIf).Inputs(0, 2);
            INST(4, Opcode::Return).b().Inputs(0);
        }
    }
    RegAlloc(GetGraph());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto code_entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

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
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).s64().Inputs(0, 2);
            INST(4, Opcode::Div).s64().Inputs(1, 3);
            INST(5, Opcode::Mod).s64().Inputs(1, 3);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(6, Opcode::Add).s64().Inputs(0, 1);  // Some return value
            INST(7, Opcode::Return).s64().Inputs(6);
        }
    }
    RegAlloc(GetGraph());

    SetNumVirtRegs(GetGraph()->GetVRegsCount());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto code_entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    // param1 < 0 [OK]
    auto param_1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::min(), DataType::INT64);
    auto param_2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param_1 + param_2);

    // param1 > 0 [OK]
    param_1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    param_2 = CutValue<uint64_t>(0, DataType::INT64);
    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param_1 + param_2);

    // param1 == 0 [THROW]
}

TEST_F(CodegenTest, NegativeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NegativeCheck).s64().Inputs(0, 2);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(6, Opcode::Add).s64().Inputs(0, 1);  // Some return value
            INST(7, Opcode::Return).s64().Inputs(6);
        }
    }
    RegAlloc(GetGraph());

    SetNumVirtRegs(GetGraph()->GetVRegsCount());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto code_entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    // param1 > 0 [OK]
    auto param_1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    auto param_2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::min(), DataType::INT64);
    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param_1 + param_2);

    // param1 == 0 [OK]
    param_1 = CutValue<uint64_t>(0, DataType::INT64);
    param_2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    GetExecModule().SetParameter(0U, param_1);
    GetExecModule().SetParameter(1U, param_2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param_1 + param_2);

    // param1 < 0 [THROW]
}

TEST_F(CodegenTest, NullCheckBoundsCheck)
{
    static const unsigned ARRAY_LEN = 10;

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).u64();  // index
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 6);
            INST(8, Opcode::StoreArray).u64().Inputs(3, 5, 7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(7, 7);  // Some return value
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }
    SetNumVirtRegs(0);
    SetNumArgs(2);
    RegAlloc(GetGraph());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto code_entry = reinterpret_cast<char *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    GetExecModule().SetInstructions(code_entry, code_exit);

    // TODO (igorban) : fill Frame array == nullptr [THROW]

    uint64_t array[ARRAY_LEN];
    for (auto i = 0U; i < ARRAY_LEN; i++) {
        array[i] = i + 0x20;
    }
    auto param_1 = GetExecModule().CreateArray(array, ARRAY_LEN, GetObjectAllocator());
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param_1));

    // 0 <= index < ARRAY_LEN [OK]
    auto index = CutValue<uint64_t>(1, DataType::UINT64);
    GetExecModule().SetParameter(1U, index);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), array[index] * 4);

    /*
    TODO (igorban) : fill Frame
    // index < 0 [THROW]
    */
    GetExecModule().FreeArray(param_1);
}

TEST_F(CodegenTest, ResolveParamSequence)
{
    ArenaVector<std::pair<uint8_t, uint8_t>> some_sequence(GetAllocator()->Adapter());
    some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(0, 3));
    some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(1, 0));
    some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(2, 3));
    some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(3, 2));

    auto result = ResoveParameterSequence(&some_sequence, 13, GetAllocator());
    EXPECT_TRUE(some_sequence.empty());
    ArenaVector<std::pair<uint8_t, uint8_t>> result_sequence(GetAllocator()->Adapter());
    result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(1, 0));
    result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(0, 3));
    result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(13, 2));
    result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(2, 3));
    result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(3, 13));

    EXPECT_EQ(result, result_sequence);

    {
        // Special loop-only case
        ArenaVector<std::pair<uint8_t, uint8_t>> some_sequence(GetAllocator()->Adapter());
        some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(2, 3));
        some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(1, 2));
        some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(4, 1));
        some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(0, 4));
        some_sequence.emplace_back(std::pair<uint8_t, uint8_t>(3, 0));

        auto result = ResoveParameterSequence(&some_sequence, 13, GetAllocator());
        EXPECT_TRUE(some_sequence.empty());
        ArenaVector<std::pair<uint8_t, uint8_t>> result_sequence(GetAllocator()->Adapter());
        result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(13, 2));
        result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(2, 3));
        result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(3, 0));
        result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(0, 4));
        result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(4, 1));
        result_sequence.emplace_back(std::pair<uint8_t, uint8_t>(1, 13));

        EXPECT_EQ(result, result_sequence);
    }
    const uint32_t REG_SIZE = 30;
    const uint8_t TMP_REG = REG_SIZE + 5;
    for (uint64_t i = 0; i < ITERATION; ++i) {
        EXPECT_TRUE(some_sequence.empty());

        std::vector<uint8_t> iters;
        for (uint8_t j = 0; j < REG_SIZE; ++j) {
            iters.push_back(j);
        }
        std::shuffle(iters.begin(), iters.end(), random_generator);
        std::vector<std::pair<uint8_t, uint8_t>> orig_vector;
        for (uint8_t j = 0; j < REG_SIZE; ++j) {
            auto gen {random_generator()};
            auto random_value = gen % REG_SIZE;
            orig_vector.push_back(std::pair<uint8_t, uint8_t>(iters[j], random_value));
        }
        for (auto &pair : orig_vector) {
            some_sequence.emplace_back(pair);
        }
        result_sequence = ResoveParameterSequence(&some_sequence, TMP_REG, GetAllocator());
        std::vector<std::pair<uint8_t, uint8_t>> result;
        for (auto &pair : result_sequence) {
            result.emplace_back(pair);
        }

        // First analysis - there are no dst before src
        for (uint8_t j = 0; j < REG_SIZE; ++j) {
            auto dst = result[j].first;
            for (uint8_t k = j + 1; k < REG_SIZE; ++k) {
                if (result[k].second == dst && result[k].second != TMP_REG) {
                    std::cerr << " first = " << result[k].first << " tmp = " << REG_SIZE + 5 << "\n";
                    std::cerr << " Before:\n";
                    for (auto &it : orig_vector) {
                        std::cerr << " " << (size_t)it.first << "<-" << (size_t)it.second << "\n";
                    }
                    std::cerr << " After:\n";
                    for (auto &it : result) {
                        std::cerr << " " << (size_t)it.first << "<-" << (size_t)it.second << "\n";
                    }
                    std::cerr << " Fault on " << (size_t)j << " and " << (size_t)k << "\n";
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
    uint64_t array_data[4098];
    for (unsigned i = 0; i < 4098; i++) {
        array_data[i] = i;
    }

    for (unsigned index = 4095; index <= 4097; index++) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();  // array
            BASIC_BLOCK(2, -1)
            {
                INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
                INST(3, Opcode::LenArray).s32().Inputs(2);
                INST(4, Opcode::BoundsCheckI).s32().Inputs(3, 1).Imm(index);
                INST(5, Opcode::LoadArrayI).u64().Inputs(2).Imm(index);
                INST(6, Opcode::Return).u64().Inputs(5);
            }
        }

        SetNumVirtRegs(0);
        SetNumArgs(1);

        RegAlloc(graph);

        // Run codegen
        EXPECT_TRUE(RunCodegen(graph));
        auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
        auto code_exit = code_entry + graph->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        GetExecModule().SetInstructions(code_entry, code_exit);

        // Enable dumping
        GetExecModule().SetDump(false);

        auto param = GetExecModule().CreateArray(array_data, index + 1, GetObjectAllocator());
        GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param));

        GetExecModule().Execute();
        GetExecModule().SetDump(false);
        // End dump

        auto ret_data = GetExecModule().GetRetValue();
        EXPECT_EQ(ret_data, index);

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
        CONSTANT(0, 10);
        CONSTANT(1, 42);
        CONSTANT(2, 13);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::MAdd).s64().Inputs(0, 1, 2);
            INST(4, Opcode::Return).s64().Inputs(3);
        }
    }

    CheckReturnValue(GetGraph(), 433);
}

TEST_F(CodegenTest, MultiplyAddFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 10.0);
        CONSTANT(1, 42.0);
        CONSTANT(2, 13.0);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::MAdd).f64().Inputs(0, 1, 2);
            INST(4, Opcode::Return).f64().Inputs(3);
        }
    }

    CheckReturnValue(GetGraph(), 433.0);
}

TEST_F(CodegenTest, MultiplySubtractInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        CONSTANT(1, 42);
        CONSTANT(2, 13);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::MSub).s64().Inputs(0, 1, 2);
            INST(4, Opcode::Return).s64().Inputs(3);
        }
    }

    CheckReturnValue(GetGraph(), -407);
}

TEST_F(CodegenTest, MultiplySubtractFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 10.0);
        CONSTANT(1, 42.0);
        CONSTANT(2, 13.0);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::MSub).f64().Inputs(0, 1, 2);
            INST(4, Opcode::Return).f64().Inputs(3);
        }
    }

    CheckReturnValue(GetGraph(), -407.0);
}

TEST_F(CodegenTest, MultiplyNegateInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 5);
        CONSTANT(1, 5);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::MNeg).s64().Inputs(0, 1);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }

    CheckReturnValue(GetGraph(), -25);
}

TEST_F(CodegenTest, MultiplyNegateFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 5.0);
        CONSTANT(1, 5.0);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::MNeg).f64().Inputs(0, 1);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }

    CheckReturnValue(GetGraph(), -25.0);
}

TEST_F(CodegenTest, OrNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x0000beef);
        CONSTANT(1, 0x2152ffff);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::OrNot).u32().Inputs(0, 1);
            INST(3, Opcode::Return).u32().Inputs(2);
        }
    }

    CheckReturnValue(GetGraph(), 0xdeadbeef);
}

TEST_F(CodegenTest, AndNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 0xf0000003);
        CONSTANT(1, 0x1);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AndNot).u32().Inputs(0, 1);
            INST(3, Opcode::Return).u32().Inputs(2);
        }
    }

    CheckReturnValue(GetGraph(), 0xf0000002);
}

TEST_F(CodegenTest, XorNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 0xf0f1ffd0);
        CONSTANT(1, 0xcf0fc0f1);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::XorNot).u32().Inputs(0, 1);
            INST(3, Opcode::Return).u32().Inputs(2);
        }
    }

    CheckReturnValue(GetGraph(), 0xc001c0de);
}

TEST_F(CodegenTest, AddSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AddSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::AddSR, 10, 2, ShiftType::LSL, 1, 14);
}

TEST_F(CodegenTest, SubSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "SubSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::SubSR, 10, 4, ShiftType::LSR, 2, 9);
}

TEST_F(CodegenTest, AndSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AndSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::AndSR, 1, 1, ShiftType::LSL, 1, 0);
}

TEST_F(CodegenTest, OrSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "OrSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::OrSR, 1, 1, ShiftType::LSL, 1, 3);
}

TEST_F(CodegenTest, XorSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "XorSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::XorSR, 3, 1, ShiftType::LSL, 1, 1);
}

TEST_F(CodegenTest, AndNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AndNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::AndNotSR, 6, 12, ShiftType::LSR, 2, 4);
}

TEST_F(CodegenTest, OrNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "OrNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::OrNotSR, 1, 12, ShiftType::LSR, 2, 0xfffffffd);
}

TEST_F(CodegenTest, XorNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "XorNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand(Opcode::XorNotSR, -1, 12, ShiftType::LSR, 2, 3);
}

TEST_F(CodegenTest, NegSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "NegSR instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x80000000);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::NegSR).Shift(ShiftType::ASR, 1).u32().Inputs(0);
            INST(2, Opcode::Return).u32().Inputs(1);
        }
    }

    CheckReturnValue(GetGraph(), 0x40000000);
}

TEST_F(CodegenTest, LoadArrayPairLivenessInfo)
{
    auto graph = GetGraph();

    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::LoadArrayPair).s32().Inputs(0, 1);
            INST(4, Opcode::LoadPairPart).s32().Inputs(2).Imm(0);
            INST(5, Opcode::LoadPairPart).s32().Inputs(2).Imm(1);
            INST(12, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(10, Opcode::LoadClass)
                .ref()
                .Inputs(12)
                .TypeId(42)
                .Class(reinterpret_cast<RuntimeInterface::ClassPtr>(1));
            INST(3, Opcode::IsInstance).b().Inputs(0, 10, 12).TypeId(42);
            INST(6, Opcode::Cast).s32().SrcType(DataType::BOOL).Inputs(3);
            INST(7, Opcode::Add).s32().Inputs(4, 5);
            INST(8, Opcode::Add).s32().Inputs(7, 6);
            INST(9, Opcode::Return).s32().Inputs(8);
        }
    }

    SetNumVirtRegs(0);
    SetNumArgs(2);
    RegAlloc(graph);
    EXPECT_TRUE(RunCodegen(graph));

    RegMask ldp_regs {};

    auto cg = Codegen(graph);
    for (auto &bb : graph->GetBlocksLinearOrder()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->GetOpcode() == Opcode::LoadArrayPair) {
                ldp_regs.set(inst->GetDstReg(0));
                ldp_regs.set(inst->GetDstReg(1));
            } else if (inst->GetOpcode() == Opcode::IsInstance) {
                auto live_regs = cg.GetLiveRegisters(inst).first;
                // Both dst registers should be alive during IsInstance call
                ASSERT_EQ(ldp_regs & live_regs, ldp_regs);
            }
        }
    }
}

TEST_F(CodegenTest, CompareAnyTypeInst)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
    GRAPH(graph)
    {
        PARAMETER(0, 0);
        INS(0).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::CompareAnyType).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }

    SetNumVirtRegs(0);
    ASSERT_TRUE(RegAlloc(graph));
    ASSERT_TRUE(RunCodegen(graph));

    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();

    ASSERT(code_entry != nullptr && code_exit != nullptr);

    GetExecModule().SetInstructions(code_entry, code_exit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<bool>();
    EXPECT_EQ(rv, true);
}

TEST_F(CodegenTest, CastAnyTypeValueInst)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
    GRAPH(graph)
    {
        PARAMETER(0, 0);
        INS(0).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::CastAnyTypeValue).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }

    SetNumVirtRegs(0);
    ASSERT_TRUE(RegAlloc(graph));
    ASSERT_TRUE(RunCodegen(graph));

    auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
    auto code_exit = code_entry + graph->GetData().Size();

    ASSERT(code_entry != nullptr && code_exit != nullptr);

    GetExecModule().SetInstructions(code_entry, code_exit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<uint32_t>();
    EXPECT_EQ(rv, 0);
}

}  // namespace panda::compiler
