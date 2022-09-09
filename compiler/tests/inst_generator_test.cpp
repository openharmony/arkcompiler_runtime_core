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
#include "inst_generator.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"

namespace panda::compiler {
class InstGeneratorTest : public GraphTest {
public:
    InstGeneratorTest() : GraphTest()
    {
        // Disable regalloc verification as generated code may operate on
        // registers/stack slots containing uninitialized values
        options.SetCompilerVerifyRegalloc(false);
    }
};

class CodegenStatisticGenerator : public StatisticGenerator {
public:
    using StatisticGenerator::StatisticGenerator;

    ~CodegenStatisticGenerator() override = default;

    void Generate() override
    {
        for (auto op : inst_generator_.GetMap()) {
            ASSERT(op.first != Opcode::Builtin);
            if (op.first == Opcode::Intrinsic) {
                continue;
            }
            auto it = inst_generator_.Generate(op.first);
            FULL_INST_STAT full_inst_stat = tmplt_;
            for (auto i = it.begin(); i != it.end(); ++i) {
                ASSERT(graph_creator_.GetAllocator()->GetAllocatedSize() == 0);
                auto graph = graph_creator_.GenerateGraph(*i);
                graph->GetAnalysis<LoopAnalyzer>().Run();
                GraphChecker(graph).Check();
                RegAllocLinearScan(graph).Run();
                graph->SetStackSlotsCount(4U);
                bool status = graph->RunPass<Codegen>();
                full_inst_stat[(*i)->GetType()] = status;
                all_inst_number_++;
                positive_inst_number += status;
                // To consume less memory
                graph->~Graph();
                graph_creator_.GetAllocator()->Resize(0);
            }
            statistic_.first.insert({op.first, full_inst_stat});
        }
        auto intrinsics = inst_generator_.Generate(Opcode::Intrinsic);
        for (auto i = intrinsics.begin(); i != intrinsics.end(); ++i) {
            ASSERT(graph_creator_.GetAllocator()->GetAllocatedSize() == 0);
            auto graph = graph_creator_.GenerateGraph(*i);
            graph->GetAnalysis<LoopAnalyzer>().Run();
            GraphChecker(graph).Check();
            RegAllocLinearScan(graph).Run();
            graph->SetStackSlotsCount(2U);
            bool status = graph->RunPass<Codegen>();
            statistic_.second[(*i)->CastToIntrinsic()->GetIntrinsicId()] = status;
            all_inst_number_++;
            positive_inst_number += status;
            graph->~Graph();
            graph_creator_.GetAllocator()->Resize(0);
        }
        for (auto i = 0; i != static_cast<int>(Opcode::NUM_OPCODES); ++i) {
            auto opc = static_cast<Opcode>(i);
            if (opc == Opcode::NOP || opc == Opcode::Intrinsic || opc == Opcode::Builtin) {
                continue;
            }
            all_opcode_number_++;
            implemented_opcode_number_ += (statistic_.first.find(opc) != statistic_.first.end());
        }
    }
};

TEST_F(InstGeneratorTest, AllInstTestARM64)
{
    ArenaAllocator inst_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(inst_alloc);

    ArenaAllocator graph_alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graph_local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graph_creator(graph_alloc, graph_local_alloc);

    // ARM64
    CodegenStatisticGenerator stat_gen_arm64(inst_gen, graph_creator);
    stat_gen_arm64.Generate();
    stat_gen_arm64.GenerateHTMLPage("CodegenStatisticARM64.html");
}

TEST_F(InstGeneratorTest, AllInstTestARM32)
{
    ArenaAllocator inst_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(inst_alloc);

    ArenaAllocator graph_alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graph_local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graph_creator(graph_alloc, graph_local_alloc);

    // ARM32
    graph_creator.SetRuntimeTargetArch(Arch::AARCH32);
    CodegenStatisticGenerator stat_gen_arm32(inst_gen, graph_creator);
    stat_gen_arm32.Generate();
    stat_gen_arm32.GenerateHTMLPage("CodegenStatisticARM32.html");
}

TEST_F(InstGeneratorTest, AllInstTestAMD64)
{
    ArenaAllocator inst_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(inst_alloc);

    ArenaAllocator graph_alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graph_local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graph_creator(graph_alloc, graph_local_alloc);

    // AMD64
    graph_creator.SetRuntimeTargetArch(Arch::X86_64);
    CodegenStatisticGenerator stat_gen_amd64(inst_gen, graph_creator);
    stat_gen_amd64.Generate();
    stat_gen_amd64.GenerateHTMLPage("CodegenStatisticAMD64.html");
}

}  // namespace panda::compiler

#ifdef USE_VIXL_ARM64
#include "vixl_exec_module.h"
#include <random>
#include <cstring>

#else
#error "Not supported!"
#endif

namespace panda::compiler {
class ArithGenerator : public CodegenStatisticGenerator {
    // Seed for random generator
    static constexpr uint64_t SEED = 0x1234;

public:
#ifndef PANDA_NIGHTLY_TEST_ON
    static constexpr uint64_t ITERATION = 20;
#else
    static constexpr uint64_t ITERATION = 250;
#endif
    ArithGenerator() = delete;

    explicit ArithGenerator(InstGenerator &inst_generator, GraphCreator &graph_creator)
        : CodegenStatisticGenerator(inst_generator, graph_creator),
          exec_module_(graph_creator_.GetAllocator(), graph_creator_.GetRuntime()) {};

    uint64_t GetRandomData()
    {
        static auto random_gen_ = std::mt19937_64(SEED);
        return random_gen_();
    };

    template <class T>
    void FixParams([[maybe_unused]] T *param_1, [[maybe_unused]] T *param_2, [[maybe_unused]] T *param_3, Opcode opc)
    {
        switch (opc) {
            case Opcode::Neg:
            case Opcode::NegSR:
            case Opcode::Abs:
            case Opcode::Not:
            case Opcode::Cast:
            case Opcode::Min:
            case Opcode::Max: {
                // single instructions - do not need to fix
                return;
            }
            case Opcode::Add:
            case Opcode::Sub:

            case Opcode::And:
            case Opcode::Or:
            case Opcode::Xor:
            case Opcode::AddI:
            case Opcode::SubI:
            case Opcode::AndI:
            case Opcode::OrI:
            case Opcode::XorI:
            case Opcode::AndNot:
            case Opcode::OrNot:
            case Opcode::XorNot: {
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    // shift parameters to prevent overflow
                    *param_1 >>= 2;
                    *param_2 >>= 2;
                    return;
                }
            }
            case Opcode::Mul:
            case Opcode::Div:
            case Opcode::Mod:
            case Opcode::MNeg: {
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    // shift parameters to prevent overflow
                    *param_1 >>= sizeof(T) * 4;
                    *param_2 >>= (sizeof(T) * 4 + 1);
                    if (*param_2 == 0) {
                        *param_2 = *param_1 + 1;
                    }
                    if (*param_2 == 0) {
                        *param_2 = *param_2 + 1;
                    };
                    return;
                }
            }
            case Opcode::MAdd:
            case Opcode::MSub:
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    // shift parameters to prevent overflow
                    *param_1 >>= sizeof(T) * 4;
                    *param_2 >>= (sizeof(T) * 4 + 2);
                    *param_3 >>= sizeof(T) * 4;
                    if (*param_2 == 0) {
                        *param_2 = *param_1 + 1;
                    }
                    if (*param_2 == 0) {
                        *param_2 = *param_2 + 1;
                    };
                    if (*param_3 == 0) {
                        *param_3 = *param_3 + 1;
                    }
                }
                return;
            case Opcode::Shl:
            case Opcode::Shr:
            case Opcode::AShr:
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    *param_1 >>= 2;
                    return;
                }
            case Opcode::ShlI:
            case Opcode::ShrI:
            case Opcode::AShrI:
            case Opcode::AddSR:
            case Opcode::SubSR:
            case Opcode::AndSR:
            case Opcode::OrSR:
            case Opcode::XorSR:
            case Opcode::AndNotSR:
            case Opcode::OrNotSR:
            case Opcode::XorNotSR:
                if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return;
                } else {
                    *param_1 >>= 2;
                    // mask for shift
                    *param_2 &= (sizeof(T) - 1);
                    return;
                }
            default:
                ASSERT_DO(0, std::cerr << (int)opc << "\n");
        }
    }

    bool isImmOps(Opcode opc)
    {
        return (opc == Opcode::AddI || opc == Opcode::SubI || opc == Opcode::ShlI || opc == Opcode::ShrI ||
                opc == Opcode::AShrI || opc == Opcode::AndI || opc == Opcode::OrI || opc == Opcode::XorI);
    }

    bool isUnaryShiftedRegisterOps(Opcode opc)
    {
        return opc == Opcode::NegSR;
    }

    bool isBinaryShiftedRegisterOps(Opcode opc)
    {
        return (opc == Opcode::AddSR || opc == Opcode::SubSR || opc == Opcode::AndSR || opc == Opcode::OrSR ||
                opc == Opcode::XorSR || opc == Opcode::AndNotSR || opc == Opcode::OrNotSR || opc == Opcode::XorNotSR);
    }

    bool isTernary(Opcode opc)
    {
        return opc == Opcode::MAdd || opc == Opcode::MSub;
    }

    std::tuple<uint64_t, uint64_t, uint64_t> GetRandValues()
    {
        return {GetRandomData(), GetRandomData(), GetRandomData()};
    }

    uint64_t GetRandValue()
    {
        return GetRandomData();
    }

    template <class param_type>
    void Generate(Opcode opc, std::pair<param_type, param_type> vals)
    {
        Generate(opc, std::make_tuple(vals.first, vals.second, static_cast<param_type>(0)));
    }

    template <class param_type>
    void Generate(Opcode opc, std::tuple<param_type, param_type, param_type> vals)
    {
        auto it = inst_generator_.Generate(opc);
        for (auto i = it.begin(); i != it.end(); ++i) {
            auto inst = *i;
            auto type = inst->GetType();
            auto shift_type = ShiftType::INVALID_SHIFT;
            if (VixlExecModule::GetType<param_type>() == type) {
                auto param_1 = std::get<0>(vals);
                auto param_2 = std::get<1>(vals);
                auto param_3 = std::get<2>(vals);
                FixParams<param_type>(&param_1, &param_2, &param_3, opc);
                if (isImmOps(opc)) {
                    static_cast<BinaryImmOperation *>(inst)->SetImm(param_2);
                } else if (isUnaryShiftedRegisterOps(opc)) {
                    static_cast<UnaryShiftedRegisterOperation *>(inst)->SetImm(param_2);
                    shift_type = static_cast<UnaryShiftedRegisterOperation *>(inst)->GetShiftType();
                } else if (isBinaryShiftedRegisterOps(opc)) {
                    static_cast<BinaryShiftedRegisterOperation *>(inst)->SetImm(param_3);
                    shift_type = static_cast<BinaryShiftedRegisterOperation *>(inst)->GetShiftType();
                }
                auto graph = graph_creator_.GenerateGraph(*i);

                auto finalizer = [&graph]([[maybe_unused]] void *ptr) {
                    if (graph != nullptr) {
                        graph->~Graph();
                    }
                };
                std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);

                graph->GetAnalysis<LoopAnalyzer>().Run();
                GraphChecker(graph).Check();

                RegAllocLinearScan(graph).Run();
                GraphChecker(graph).Check();

                if (!graph->RunPass<Codegen>()) {
                    return;
                };
                auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
                auto code_exit = code_entry + graph->GetData().Size();
                ASSERT(code_entry != nullptr && code_exit != nullptr);
                exec_module_.SetInstructions(code_entry, code_exit);

                exec_module_.SetDump(false);

                exec_module_.SetParameter(0U, param_1);

                if (!isImmOps(opc)) {
                    exec_module_.SetParameter(1U, param_2);
                }

                if (isTernary(opc)) {
                    exec_module_.SetParameter(2U, param_3);
                }

                exec_module_.Execute();

                struct RetValue {
                    uint64_t data;
                    uint64_t type;
                };

                auto ret_data = exec_module_.GetRetValue<param_type>();
                auto calc_data = DoLogic<param_type>(opc, param_1, param_2, param_3, shift_type,
                                                     DataType::GetTypeSize(type, graph->GetArch()));
                if (calc_data != ret_data) {
                    std::cout << "  data " << ret_data << " sizeof type  " << (uint64_t)(sizeof(param_type) * 8)
                              << " \n";
                    std::cout << std::hex << "parameter_1 = " << param_1 << " parameter_2 = " << param_2
                              << "parameter_3 = " << param_3 << "\n";
                    inst->Dump(&std::cerr);
                    std::cout << "calculated = " << calc_data << " returned " << ret_data << "\n";
                    exec_module_.SetDump(true);
                    exec_module_.PrintInstructions();
                    exec_module_.SetParameter(0U, param_1);
                    exec_module_.SetParameter(1U, param_2);
                    exec_module_.SetParameter(2U, param_3);
                    exec_module_.Execute();
                    exec_module_.SetDump(false);
                }
                ASSERT_EQ(calc_data, ret_data);
            }
        }
    }

    template <class param_type, class result_type>
    void GenCast(param_type val)
    {
        auto opc = Opcode::Cast;

        auto it = inst_generator_.Generate(opc);
        for (auto i = it.begin(); i != it.end(); ++i) {
            auto inst = *i;
            auto type = inst->GetType();
            if (VixlExecModule::GetType<param_type>() == type) {
                param_type param_1 = val;

                auto graph = graph_creator_.GenerateGraph(*i);

                auto finalizer = [&graph]([[maybe_unused]] void *ptr) {
                    if (graph != nullptr) {
                        graph->~Graph();
                    }
                };
                std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);

                // Reset type for Cast-destination:
                (*i)->SetType(VixlExecModule::GetType<result_type>());
                // Fix return value
                for (auto iter = (*i)->GetUsers().begin(); iter != (*i)->GetUsers().end(); ++iter) {
                    iter->GetInst()->SetType(VixlExecModule::GetType<result_type>());
                }

                graph->GetAnalysis<LoopAnalyzer>().Run();
                GraphChecker(graph).Check();
                RegAllocLinearScan(graph).Run();
                GraphChecker(graph).Check();

                if (!graph->RunPass<Codegen>()) {
                    return;
                };
                auto code_entry = reinterpret_cast<char *>(graph->GetData().Data());
                auto code_exit = code_entry + graph->GetData().Size();
                ASSERT(code_entry != nullptr && code_exit != nullptr);
                exec_module_.SetInstructions(code_entry, code_exit);

                exec_module_.SetDump(false);
                exec_module_.SetParameter(0U, param_1);
                exec_module_.Execute();

                auto ret_data = exec_module_.GetRetValue<result_type>();
                result_type calc_data;

                if constexpr ((GetCommonType(VixlExecModule::GetType<result_type>()) == DataType::Type::INT64) &&
                              (std::is_same_v<param_type, float> || std::is_same_v<param_type, double>)) {
                    if (param_1 > (param_type)std::numeric_limits<result_type>::max()) {
                        calc_data = std::numeric_limits<result_type>::max();
                    } else if (param_1 < (param_type)std::numeric_limits<result_type>::min()) {
                        calc_data = std::numeric_limits<result_type>::min();
                    } else {
                        calc_data = static_cast<result_type>(param_1);
                    }
                } else {
                    calc_data = static_cast<result_type>(param_1);
                }

                if (calc_data != ret_data) {
                    if constexpr (std::is_same_v<result_type, double> || std::is_same_v<param_type, double>) {
                        std::cout << std::hex << " in parameter = " << param_1 << "\n";
                        std::cout << std::hex << "parameter_1 = " << (double)param_1 << "\n";
                    }

#ifndef NDEBUG
                    std::cout << " cast from " << DataType::ToString(VixlExecModule::GetType<param_type>()) << " to "
                              << DataType::ToString(VixlExecModule::GetType<result_type>()) << "\n";
#endif
                    std::cout << "  data " << ret_data << " sizeof type  " << (uint64_t)(sizeof(param_type) * 8)
                              << " \n";
                    inst->Dump(&std::cerr);
                    exec_module_.SetDump(true);
                    exec_module_.SetParameter(0U, param_1);
                    exec_module_.Execute();
                    exec_module_.SetDump(false);
                }
                ASSERT_EQ(calc_data, ret_data);
            }
        }
    }

    template <class T>
    T DoShift(T value, ShiftType shift_type, uint64_t scale, uint8_t type_size)
    {
        switch (shift_type) {
            case ShiftType::LSL:
                return static_cast<uint64_t>value << scale;
            case ShiftType::ROR:
                return (static_cast<uint64_t>value >> scale) | (value << (type_size - scale));
            case ShiftType::LSR:
                if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>) {
                    return (static_cast<uint32_t>value) >> static_cast<uint32_t>scale;
                }
                if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
                    return (static_cast<uint64_t>value) >> scale;
                }
            /* fall-through */
            case ShiftType::ASR:
                return static_cast<int64_t>value >> scale;
            default:
                UNREACHABLE();
        }
    }

    // Make logic with parameters (default - first parameter)
    template <class T>
    T DoLogic(Opcode opc, T param_1, [[maybe_unused]] T param_2, [[maybe_unused]] T param_3,
              [[maybe_unused]] ShiftType shift_type, [[maybe_unused]] uint8_t type_size)
    {
        constexpr DataType::Type type = ConstantInst::GetTypeFromCType<T>();
        constexpr bool arithmetic_type = (type == DataType::INT64);
        switch (opc) {
            case Opcode::Neg:
                return -param_1;
            case Opcode::MAdd:
                return param_1 * param_2 + param_3;
            case Opcode::MSub:
                return param_3 - param_1 * param_2;
            case Opcode::Not:
                return (-param_1 - 1);
            case Opcode::Add:
            case Opcode::AddI:
                return (param_1 + param_2);
            case Opcode::Sub:
            case Opcode::SubI:
                return (param_1 - param_2);
            case Opcode::Mul:
                return (param_1 * param_2);
            case Opcode::MNeg:
                return -(param_1 * param_2);
            case Opcode::Div:
                ASSERT_PRINT(param_2 != 0, "If you got this assert, you may change SEED");
                return (param_1 / param_2);
            case Opcode::Mod:
                if constexpr (arithmetic_type) {
                    ASSERT_PRINT(param_2 != 0, "If you got this assert, you may change SEED");
                    return param_1 % param_2;
                } else {
                    return fmod(param_1, param_2);
                }
            case Opcode::Min:
                return std::min(param_1, param_2);
            case Opcode::Max:
                return std::max(param_1, param_2);

            case Opcode::NegSR:
                if constexpr (arithmetic_type) {
                    return -(DoShift(param_1, shift_type, param_2, type_size));
                }
                /* fall-through */
            case Opcode::ShlI:
                if constexpr (arithmetic_type)
                    return DoShift(param_1, ShiftType::LSL, param_2, type_size);
                /* fall-through */
            case Opcode::Shl:
                if constexpr (arithmetic_type) {
                    return DoShift(param_1, ShiftType::LSL, param_2 & (type_size - 1), type_size);
                }
                /* fall-through */
            case Opcode::ShrI:
                if constexpr (arithmetic_type) {
                    return DoShift(param_1, ShiftType::LSR, param_2, type_size);
                }
                /* fall-through */
            case Opcode::Shr:
                if constexpr (arithmetic_type) {
                    return DoShift(param_1, ShiftType::LSR, param_2 & (type_size - 1), type_size);
                }
                /* fall-through */
            case Opcode::AShr:
                if constexpr (arithmetic_type) {
                    return DoShift(param_1, ShiftType::ASR, param_2 & (type_size - 1), type_size);
                }
                /* fall-through */
            case Opcode::AShrI:
                if constexpr (arithmetic_type) {
                    return DoShift(param_1, ShiftType::ASR, param_2, type_size);
                }
                /* fall-through */
            case Opcode::And:
            case Opcode::AndI:
                if constexpr (arithmetic_type) {
                    return param_1 & param_2;
                }
                /* fall-through */
            case Opcode::AndSR:
                if constexpr (arithmetic_type) {
                    return param_1 & DoShift(param_2, shift_type, param_3, type_size);
                }
                /* fall-through */
            case Opcode::Or:
            case Opcode::OrI:
                if constexpr (arithmetic_type) {
                    return param_1 | param_2;
                }
                /* fall-through */
            case Opcode::OrSR:
                if constexpr (arithmetic_type) {
                    return param_1 | DoShift(param_2, shift_type, param_3, type_size);
                }
                /* fall-through */
            case Opcode::Xor:
            case Opcode::XorI:
                if constexpr (arithmetic_type) {
                    return param_1 ^ param_2;
                }
                /* fall-through */
            case Opcode::XorSR:
                if constexpr (arithmetic_type) {
                    return param_1 ^ DoShift(param_2, shift_type, param_3, type_size);
                }
                /* fall-through */
            case Opcode::AndNot:
                if constexpr (arithmetic_type) {
                    return param_1 & (~param_2);
                }
                /* fall-through */
            case Opcode::AndNotSR:
                if constexpr (arithmetic_type) {
                    return param_1 & (~DoShift(param_2, shift_type, param_3, type_size));
                }
                /* fall-through */
            case Opcode::OrNot:
                if constexpr (arithmetic_type) {
                    return param_1 | (~param_2);
                }
                /* fall-through */
            case Opcode::OrNotSR:
                if constexpr (arithmetic_type) {
                    return param_1 | (~DoShift(param_2, shift_type, param_3, type_size));
                }
                /* fall-through */
            case Opcode::XorNot:
                if constexpr (arithmetic_type) {
                    return param_1 ^ (~param_2);
                }
                /* fall-through */
            case Opcode::XorNotSR:
                if constexpr (arithmetic_type) {
                    return param_1 ^ (~DoShift(param_2, shift_type, param_3, type_size));
                }
                /* fall-through */
            case Opcode::Abs:
                if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
                              std::is_same_v<T, int64_t>) {
                    return std::abs(param_1);
                }
            /* fall-through */
            default:
                ASSERT_DO(false, std::cerr << "Unsupported!" << (int)opc << "\n");
                return -1;
        }
    }

private:
    VixlExecModule exec_module_;
};

void OneTest(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<int8_t>(opc, gen.GetRandValues());
    gen.Generate<int16_t>(opc, gen.GetRandValues());
    gen.Generate<int32_t>(opc, gen.GetRandValues());
    gen.Generate<int64_t>(opc, gen.GetRandValues());
    gen.Generate<uint8_t>(opc, gen.GetRandValues());
    gen.Generate<uint16_t>(opc, gen.GetRandValues());
    gen.Generate<uint32_t>(opc, gen.GetRandValues());
    gen.Generate<uint64_t>(opc, gen.GetRandValues());
}

void OneTestCast(ArithGenerator &gen)
{
    gen.GenCast<int8_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int8_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int8_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int8_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int8_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int8_t, float>(gen.GetRandValue());
    gen.GenCast<int8_t, double>(gen.GetRandValue());

    gen.GenCast<int16_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int16_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int16_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int16_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int16_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int16_t, float>(gen.GetRandValue());
    gen.GenCast<int16_t, double>(gen.GetRandValue());

    gen.GenCast<int32_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int32_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int32_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int32_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int32_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int32_t, float>(gen.GetRandValue());
    gen.GenCast<int32_t, double>(gen.GetRandValue());

    gen.GenCast<int64_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<int64_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<int64_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<int64_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int8_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int16_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int32_t>(gen.GetRandValue());
    gen.GenCast<int64_t, int64_t>(gen.GetRandValue());
    gen.GenCast<int64_t, float>(gen.GetRandValue());
    gen.GenCast<int64_t, double>(gen.GetRandValue());

    gen.GenCast<uint8_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint8_t, float>(gen.GetRandValue());
    gen.GenCast<uint8_t, double>(gen.GetRandValue());

    gen.GenCast<uint16_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint16_t, float>(gen.GetRandValue());
    gen.GenCast<uint16_t, double>(gen.GetRandValue());

    gen.GenCast<uint32_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint32_t, float>(gen.GetRandValue());
    gen.GenCast<uint32_t, double>(gen.GetRandValue());

    gen.GenCast<uint64_t, uint8_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, uint16_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, uint32_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, uint64_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int8_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int16_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int32_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, int64_t>(gen.GetRandValue());
    gen.GenCast<uint64_t, float>(gen.GetRandValue());
    gen.GenCast<uint64_t, double>(gen.GetRandValue());

    // We DON'T support cast from float32 to int8/16.
    gen.GenCast<float, uint32_t>(gen.GetRandValue());
    gen.GenCast<float, uint64_t>(gen.GetRandValue());
    gen.GenCast<float, int32_t>(gen.GetRandValue());
    gen.GenCast<float, int64_t>(gen.GetRandValue());
    gen.GenCast<float, float>(gen.GetRandValue());
    gen.GenCast<float, double>(gen.GetRandValue());

    // We DON'T support cast from float64 to int8/16.
    gen.GenCast<double, uint32_t>(gen.GetRandValue());
    gen.GenCast<double, uint64_t>(gen.GetRandValue());
    gen.GenCast<double, int32_t>(gen.GetRandValue());
    gen.GenCast<double, int64_t>(gen.GetRandValue());
    gen.GenCast<double, float>(gen.GetRandValue());
    gen.GenCast<double, double>(gen.GetRandValue());
}

void OneTestSign(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<int8_t>(opc, gen.GetRandValues());
    gen.Generate<int16_t>(opc, gen.GetRandValues());
    gen.Generate<int32_t>(opc, gen.GetRandValues());
    gen.Generate<int64_t>(opc, gen.GetRandValues());
}

void OneTestFP(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<float>(opc, gen.GetRandValues());
    gen.Generate<double>(opc, gen.GetRandValues());
}

void OneTestShift(ArithGenerator &gen, Opcode opc)
{
    gen.Generate<uint64_t>(opc, {0x8899aabbccddeeff, 32});
    gen.Generate<uint64_t>(opc, {0x8899aabbccddeeff, 32 + 64});
    gen.Generate<int64_t>(opc, {0x8899aabbccddeeff, 32});
    gen.Generate<int64_t>(opc, {0x8899aabbccddeeff, 32 + 64});
    gen.Generate<uint32_t>(opc, {0xccddeeff, 16});
    gen.Generate<uint32_t>(opc, {0xccddeeff, 16 + 32});
    gen.Generate<int32_t>(opc, {0xccddeeff, 0xffffffff});
    gen.Generate<int32_t>(opc, {0xccddeeff, 16});
    gen.Generate<int32_t>(opc, {0xccddeeff, 16 + 32});
    gen.Generate<uint16_t>(opc, {0xeeff, 8});
    gen.Generate<uint16_t>(opc, {0xeeff, 8 + 16});
    gen.Generate<int16_t>(opc, {0xeeff, 8});
    gen.Generate<int16_t>(opc, {0xeeff, 8 + 16});
    gen.Generate<uint8_t>(opc, {0xff, 4});
    gen.Generate<uint8_t>(opc, {0xff, 4 + 8});
    gen.Generate<int8_t>(opc, {0xff, 4});
    gen.Generate<int8_t>(opc, {0xff, 4 + 8});
}

// There is not enough memory in the arena allocator, so it is divided into 2 parts
void RandomTestsPart1()
{
    ArenaAllocator alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(alloc);
    GraphCreator graph_creator(alloc, local_alloc);
    ArithGenerator stat_gen(inst_gen, graph_creator);

    OneTest(stat_gen, Opcode::Add);
    OneTestFP(stat_gen, Opcode::Add);

    OneTest(stat_gen, Opcode::Sub);
    OneTestFP(stat_gen, Opcode::Sub);

    OneTest(stat_gen, Opcode::Mul);
    OneTestFP(stat_gen, Opcode::Mul);

    OneTest(stat_gen, Opcode::Div);
    OneTestFP(stat_gen, Opcode::Div);

    OneTest(stat_gen, Opcode::Mod);
    // Disabled, because external fmod() call is currently x86_64 -- incompatible with aarch64 runtime :(
    // stat_gen  Opcode::Mod

    OneTest(stat_gen, Opcode::Min);
    OneTestFP(stat_gen, Opcode::Min);

    OneTest(stat_gen, Opcode::Max);
    OneTestFP(stat_gen, Opcode::Max);

    OneTest(stat_gen, Opcode::Shl);
    OneTest(stat_gen, Opcode::Shr);
    OneTest(stat_gen, Opcode::AShr);
}

void RandomTestsPart2()
{
    ArenaAllocator alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(alloc);
    GraphCreator graph_creator(alloc, local_alloc);
    ArithGenerator stat_gen(inst_gen, graph_creator);

    OneTest(stat_gen, Opcode::And);
    // Float unsupported

    OneTest(stat_gen, Opcode::Or);
    // Float unsupported

    OneTest(stat_gen, Opcode::Xor);
    // Float unsupported

    OneTest(stat_gen, Opcode::Neg);

    OneTestSign(stat_gen, Opcode::Abs);

    OneTest(stat_gen, Opcode::Not);

    OneTest(stat_gen, Opcode::AddI);

    OneTest(stat_gen, Opcode::SubI);

    OneTest(stat_gen, Opcode::ShlI);

    OneTest(stat_gen, Opcode::ShrI);

    OneTest(stat_gen, Opcode::AShrI);

    OneTest(stat_gen, Opcode::AndI);

    OneTest(stat_gen, Opcode::OrI);

    OneTest(stat_gen, Opcode::XorI);

    // Special case for Case-instruction - generate inputs types.
    OneTestCast(stat_gen);
}

void NotRandomTests()
{
    ArenaAllocator alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(alloc);
    GraphCreator graph_creator(alloc, local_alloc);
    ArithGenerator stat_gen(inst_gen, graph_creator);

    stat_gen.Generate<uint64_t>(Opcode::Min, {UINT64_MAX, 0});
    stat_gen.Generate<uint64_t>(Opcode::Min, {0, UINT64_MAX});
    stat_gen.Generate<int64_t>(Opcode::Min, {0, UINT64_MAX});
    stat_gen.Generate<int64_t>(Opcode::Min, {0, UINT64_MAX});
    OneTestShift(stat_gen, Opcode::Shl);
    OneTestShift(stat_gen, Opcode::Shr);
    OneTestShift(stat_gen, Opcode::AShr);
}

TEST_F(InstGeneratorTest, GenArithVixlCode)
{
    for (uint64_t i = 0; i < ArithGenerator::ITERATION; ++i) {
        RandomTestsPart1();
        RandomTestsPart2();
    }
    NotRandomTests();
}

/**
 * Check that all possible instructions that introduce a reference as a result
 * are handled in analysis.  On failed test add a case to AliasVisitor.
 */
TEST_F(InstGeneratorTest, AliasAnalysisSupportTest)
{
    ArenaAllocator inst_alloc(SpaceType::SPACE_TYPE_COMPILER);
    InstGenerator inst_gen(inst_alloc);

    ArenaAllocator graph_alloc(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator graph_local_alloc(SpaceType::SPACE_TYPE_COMPILER);
    GraphCreator graph_creator(graph_alloc, graph_local_alloc);

    for (auto op : inst_gen.GetMap()) {
        auto it = inst_gen.Generate(op.first);
        for (auto i = it.begin(); i != it.end(); ++i) {
            if ((*i)->GetType() != DataType::REFERENCE) {
                continue;
            }
            auto graph = graph_creator.GenerateGraph(*i);

            auto finalizer = [&graph]([[maybe_unused]] void *ptr) {
                if (graph != nullptr) {
                    graph->~Graph();
                }
            };
            std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);

            graph->RunPass<AliasAnalysis>();
            EXPECT_TRUE(graph->IsAnalysisValid<AliasAnalysis>());
        }
    }
}
}  // namespace panda::compiler
