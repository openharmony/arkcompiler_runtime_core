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

#include <functional>
#include <string>
#include <unordered_map>

#include <regex>
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/codegen_native.h"
#include "optimizer/code_generator/method_properties.h"
#include "aarch64/decoder-aarch64.h"
#include "aarch64/disasm-aarch64.h"
#include "aarch64/operands-aarch64.h"
#include "aarch64/disasm-aarch64.h"
#include "tests/unit_test.h"

namespace panda::compiler {

class VixlDisasmTest : public GraphTest {
public:
    VixlDisasmTest() : decoder_(GetAllocator()) {}

    auto &GetDecoder()
    {
        return decoder_;
    }

    auto CreateDisassembler()
    {
        return vixl::aarch64::Disassembler(asm_buf_, sizeof(asm_buf_));
    }

private:
    vixl::aarch64::Decoder decoder_;
    char asm_buf_[vixl::aarch64::Disassembler::GetDefaultBufferSize()];
};

class CodegenCallerSavedRegistersTest : public VixlDisasmTest {
};

class DecoderVisitor : public vixl::aarch64::DecoderVisitor {
public:
#define DECLARE(A) \
    virtual void Visit##A([[maybe_unused]] const vixl::aarch64::Instruction *instr) {}
    VISITOR_LIST(DECLARE)
#undef DECLARE

    void Visit(vixl::aarch64::Metadata *metadata, const vixl::aarch64::Instruction *instr) final
    {
        using FormToVisitorFnMap =
            std::unordered_map<std::string, std::function<void(DecoderVisitor *, const vixl::aarch64::Instruction *)>>;
        static const FormToVisitorFnMap form_to_visitor {DEFAULT_FORM_TO_VISITOR_MAP(DecoderVisitor)};

        auto visitor_it {form_to_visitor.find((*metadata)["form"])};
        ASSERT(visitor_it != std::end(form_to_visitor));

        const auto &visitor {visitor_it->second};
        ASSERT(visitor != nullptr);
        visitor(this, instr);
    }
};

class LoadStoreRegistersCollector : public DecoderVisitor {
public:
    // use the same body for all VisitXXX methods to simplify visitor's implementation
#define DECLARE(A)                                                                                             \
    void Visit##A(const vixl::aarch64::Instruction *instr) override                                            \
    {                                                                                                          \
        if (std::string(#A) == "LoadStorePairOffset") {                                                        \
            if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_x) == vixl::aarch64::LoadStorePairOp::LDP_x || \
                instr->Mask(vixl::aarch64::LoadStorePairOp::STP_x) == vixl::aarch64::LoadStorePairOp::STP_x) { \
                regs.set(instr->GetRt());                                                                      \
                regs.set(instr->GetRt2());                                                                     \
                return;                                                                                        \
            }                                                                                                  \
            if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_d) == vixl::aarch64::LoadStorePairOp::LDP_d || \
                instr->Mask(vixl::aarch64::LoadStorePairOp::STP_d) == vixl::aarch64::LoadStorePairOp::STP_d) { \
                vregs.set(instr->GetRt());                                                                     \
                vregs.set(instr->GetRt2());                                                                    \
                return;                                                                                        \
            }                                                                                                  \
        }                                                                                                      \
        if (std::string(#A) == "LoadStoreUnscaledOffset" || std::string(#A) == "LoadStoreUnsignedOffset" ||    \
            std::string(#A) == "LoadStoreRegisterOffset") {                                                    \
            if (instr->Mask(vixl::aarch64::LoadStoreOp::LDR_x) == vixl::aarch64::LoadStoreOp::LDR_x ||         \
                instr->Mask(vixl::aarch64::LoadStoreOp::STR_x) == vixl::aarch64::LoadStoreOp::STR_x) {         \
                regs.set(instr->GetRt());                                                                      \
                return;                                                                                        \
            }                                                                                                  \
            if (instr->Mask(vixl::aarch64::LoadStoreOp::LDR_d) == vixl::aarch64::LoadStoreOp::LDR_d ||         \
                instr->Mask(vixl::aarch64::LoadStoreOp::STR_d) == vixl::aarch64::LoadStoreOp::STR_d) {         \
                vregs.set(instr->GetRt());                                                                     \
                return;                                                                                        \
            }                                                                                                  \
        }                                                                                                      \
    }

    VISITOR_LIST(DECLARE)
#undef DECLARE
    RegMask GetAccessedRegisters()
    {
        return regs;
    }

    VRegMask GetAccessedVRegisters()
    {
        return vregs;
    }

private:
    RegMask regs;
    VRegMask vregs;
};

class LoadStoreInstCollector : public DecoderVisitor {
public:
#define DECLARE(A)                                                                                                    \
    void Visit##A(const vixl::aarch64::Instruction *instr) override                                                   \
    {                                                                                                                 \
        if (std::string(#A) == "LoadStorePairOffset") {                                                               \
            if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_x) == vixl::aarch64::LoadStorePairOp::LDP_x) {        \
                ldp_++;                                                                                               \
                regs.set(instr->GetRt());                                                                             \
                regs.set(instr->GetRt2());                                                                            \
            } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::STP_x) == vixl::aarch64::LoadStorePairOp::STP_x) { \
                stp_++;                                                                                               \
                regs.set(instr->GetRt());                                                                             \
                regs.set(instr->GetRt2());                                                                            \
            } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_d) == vixl::aarch64::LoadStorePairOp::LDP_d) { \
                ldp_v_++;                                                                                             \
                vregs.set(instr->GetRt());                                                                            \
                vregs.set(instr->GetRt2());                                                                           \
            } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::STP_d) == vixl::aarch64::LoadStorePairOp::STP_d) { \
                stp_v_++;                                                                                             \
                vregs.set(instr->GetRt());                                                                            \
                vregs.set(instr->GetRt2());                                                                           \
            }                                                                                                         \
        }                                                                                                             \
    }

    VISITOR_LIST(DECLARE)
#undef DECLARE
    auto GetLdpX()
    {
        return ldp_;
    }

    auto GetStpX()
    {
        return stp_;
    }

    auto GetLdpD()
    {
        return ldp_v_;
    }

    auto GetStpD()
    {
        return stp_v_;
    }

    RegMask GetAccessedPairRegisters()
    {
        return regs;
    }

    VRegMask GetAccessedPairVRegisters()
    {
        return vregs;
    }

private:
    size_t ldp_ {0};
    size_t stp_ {0};
    size_t ldp_v_ {0};
    size_t stp_v_ {0};
    RegMask regs;
    VRegMask vregs;
};

TEST_F(CodegenCallerSavedRegistersTest, SaveOnlyLiveRegisters)
{
    options.SetCompilerSaveOnlyLiveRegisters(true);
    constexpr auto ARGS_COUNT = 8;
    GRAPH(GetGraph())
    {
        for (int i = 0; i < ARGS_COUNT; i++) {
            PARAMETER(i, i).u64();
        }
        for (int i = 0; i < ARGS_COUNT; i++) {
            PARAMETER(i + ARGS_COUNT, i + ARGS_COUNT).f64();
        }

        BASIC_BLOCK(2, -1)
        {
            INST(16, Opcode::Add).u64().Inputs(0, 1);
            INST(17, Opcode::Add).u64().Inputs(16, 2);
            INST(18, Opcode::Add).u64().Inputs(17, 3);
            INST(19, Opcode::Add).u64().Inputs(18, 4);
            INST(20, Opcode::Add).u64().Inputs(19, 5);
            INST(21, Opcode::Add).u64().Inputs(20, 6);
            INST(22, Opcode::Add).u64().Inputs(21, 7);
            INST(23, Opcode::Add).f64().Inputs(8, 9);
            INST(24, Opcode::Add).f64().Inputs(23, 10);
            INST(25, Opcode::Add).f64().Inputs(24, 11);
            INST(26, Opcode::Add).f64().Inputs(25, 12);
            INST(27, Opcode::Add).f64().Inputs(26, 13);
            INST(28, Opcode::Add).f64().Inputs(27, 14);
            INST(29, Opcode::Add).f64().Inputs(28, 15);
            INST(30, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(29);
            INST(31, Opcode::Add).u64().Inputs(30, 22);

            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(32, Opcode::NewArray).ref().TypeId(8).Inputs(44, 31);
            INST(33, Opcode::Return).ref().Inputs(32);
        }
    }

    SetNumArgs(ARGS_COUNT * 2);
    SetNumVirtRegs(0);
    GraphChecker(GetGraph()).Check();
    RegAlloc(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Codegen>());

    auto code_entry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetData().Data());
    auto code_exit = code_entry + GetGraph()->GetData().Size();
    ASSERT(code_entry != nullptr && code_exit != nullptr);
    auto &decoder {GetDecoder()};
    LoadStoreRegistersCollector visitor;
    vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&visitor});
    for (auto instr = code_entry; instr < code_exit; instr += vixl::aarch64::kInstructionSize) {
        decoder.Decode(instr);
    }
    // not using reg lists from vixl::aarch64 to check only r0-r7
    constexpr auto CALLER_REGS = RegMask((1 << ARGS_COUNT) - 1);
    EXPECT_EQ(visitor.GetAccessedRegisters() & CALLER_REGS, RegMask {0});
    EXPECT_TRUE((visitor.GetAccessedVRegisters() & CALLER_REGS).none());
}

class CodegenSpillFillCoalescingTest : public VixlDisasmTest {
public:
    CodegenSpillFillCoalescingTest()
    {
        options.SetCompilerSpillFillPair(true);
        options.SetCompilerVerifyRegalloc(false);
    }

    void CheckSpillFillCoalescingForEvenRegsNumber(bool aligned)
    {
        GRAPH(GetGraph())
        {
            BASIC_BLOCK(2, -1)
            {
                INST(0, Opcode::SpillFill);
                INST(1, Opcode::ReturnVoid);
            }
        }

        int alignment_offset = aligned ? 1 : 0;

        auto sf_inst = INS(0).CastToSpillFill();
        sf_inst->AddSpill(0, 0 + alignment_offset, DataType::Type::INT64);
        sf_inst->AddSpill(1, 1 + alignment_offset, DataType::Type::INT64);
        sf_inst->AddSpill(0, 2 + alignment_offset, DataType::Type::FLOAT64);
        sf_inst->AddSpill(1, 3 + alignment_offset, DataType::Type::FLOAT64);
        sf_inst->AddFill(4 + alignment_offset, 3, DataType::Type::INT64);
        sf_inst->AddFill(5 + alignment_offset, 2, DataType::Type::INT64);
        sf_inst->AddFill(6 + alignment_offset, 3, DataType::Type::FLOAT64);
        sf_inst->AddFill(7 + alignment_offset, 2, DataType::Type::FLOAT64);

        SetNumArgs(0);
        SetNumVirtRegs(0);
        GraphChecker(GetGraph()).Check();
        GetGraph()->SetStackSlotsCount(8U + alignment_offset);
        RegAlloc(GetGraph());
        ASSERT_TRUE(GetGraph()->RunPass<Codegen>());

        auto code_entry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetData().Data());
        auto code_exit = code_entry + GetGraph()->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        auto &decoder {GetDecoder()};
        LoadStoreInstCollector visitor;
        vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&visitor});
        for (auto instr = code_entry; instr < code_exit; instr += vixl::aarch64::kInstructionSize) {
            decoder.Decode(instr);
        }
        EXPECT_EQ(visitor.GetStpX(), 1 /* 1 use pre increment and not counted */);
        EXPECT_EQ(visitor.GetLdpX(), 1 /* 1 use post increment and not counted */);
        EXPECT_EQ(visitor.GetLdpD(), 1);
        EXPECT_EQ(visitor.GetStpD(), 1);

        constexpr auto TEST_REGS = RegMask(0xF);
        EXPECT_EQ(visitor.GetAccessedPairRegisters() & TEST_REGS, RegMask {0xF});
        EXPECT_EQ(visitor.GetAccessedPairVRegisters() & TEST_REGS, RegMask {0xF});
    }

    void CheckSpillFillCoalescingForOddRegsNumber(bool aligned)
    {
        GRAPH(GetGraph())
        {
            BASIC_BLOCK(2, -1)
            {
                INST(0, Opcode::SpillFill);
                INST(1, Opcode::ReturnVoid);
            }
        }

        int alignment_offset = aligned ? 1 : 0;

        auto sf_inst = INS(0).CastToSpillFill();
        sf_inst->AddSpill(0, 0 + alignment_offset, DataType::Type::INT64);
        sf_inst->AddSpill(1, 1 + alignment_offset, DataType::Type::INT64);
        sf_inst->AddSpill(2, 2 + alignment_offset, DataType::Type::INT64);
        sf_inst->AddSpill(0, 3 + alignment_offset, DataType::Type::FLOAT64);
        sf_inst->AddSpill(1, 4 + alignment_offset, DataType::Type::FLOAT64);
        sf_inst->AddSpill(2, 5 + alignment_offset, DataType::Type::FLOAT64);
        sf_inst->AddFill(6 + alignment_offset, 3, DataType::Type::INT64);
        sf_inst->AddFill(7 + alignment_offset, 4, DataType::Type::INT64);
        sf_inst->AddFill(8 + alignment_offset, 5, DataType::Type::INT64);
        sf_inst->AddFill(9 + alignment_offset, 3, DataType::Type::FLOAT64);
        sf_inst->AddFill(10 + alignment_offset, 4, DataType::Type::FLOAT64);
        sf_inst->AddFill(11 + alignment_offset, 5, DataType::Type::FLOAT64);

        SetNumArgs(0);
        SetNumVirtRegs(0);
        GraphChecker(GetGraph()).Check();
        GetGraph()->SetStackSlotsCount(12U + alignment_offset);
        RegAlloc(GetGraph());
        ASSERT_TRUE(GetGraph()->RunPass<Codegen>());

        auto code_entry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetData().Data());
        auto code_exit = code_entry + GetGraph()->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        auto &decoder {GetDecoder()};
        LoadStoreInstCollector visitor;
        vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&visitor});
        for (auto instr = code_entry; instr < code_exit; instr += vixl::aarch64::kInstructionSize) {
            decoder.Decode(instr);
        }
        EXPECT_EQ(visitor.GetStpX(), 1 /* 1 use pre increment and not counted */);
        EXPECT_EQ(visitor.GetLdpX(), 1 /* 1 use post increment and not counted */);
        EXPECT_EQ(visitor.GetLdpD(), 1);
        EXPECT_EQ(visitor.GetStpD(), 1);

        constexpr auto TEST_REGS = RegMask(0x3F);
        if (aligned) {
            EXPECT_EQ(visitor.GetAccessedPairRegisters() & TEST_REGS, RegMask {0b11011});
            EXPECT_EQ(visitor.GetAccessedPairVRegisters() & TEST_REGS, RegMask {0b110110});
        } else {
            EXPECT_EQ(visitor.GetAccessedPairRegisters() & TEST_REGS, RegMask {0b110110});
            EXPECT_EQ(visitor.GetAccessedPairVRegisters() & TEST_REGS, RegMask {0b11011});
        }
    }
};

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToUnalignedNeighborSlotsEvenRegsNumber)
{
    CheckSpillFillCoalescingForEvenRegsNumber(false);
}

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToAlignedNeighborSlotsEvenRegsNumber)
{
    CheckSpillFillCoalescingForEvenRegsNumber(true);
}

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToUnalignedNeighborSlotsOddRegsNumber)
{
    CheckSpillFillCoalescingForOddRegsNumber(false);
}

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToAlignedNeighborSlotsOddRegsNumber)
{
    CheckSpillFillCoalescingForOddRegsNumber(true);
}

// clang-format off
class CodegenLeafPrologueTest : public VixlDisasmTest {
public:
    CodegenLeafPrologueTest()
    {
        options.SetCompilerVerifyRegalloc(false);
#ifndef NDEBUG
        graph_->SetLowLevelInstructionsEnabled();
#endif
    }

    void CheckLeafPrologue()
    {
        // RedundantOps::inc()
        RuntimeInterface::FieldPtr i = (void *)(0xDEADBEEF);
        GRAPH(GetGraph())
        {
            PARAMETER(0, 0).ref();
            BASIC_BLOCK(2, 3)
            {
                INST(1, Opcode::LoadObject).s64().Inputs(0).TypeId(208U).ObjField(i);
                INST(2, Opcode::AddI).s64().Inputs(1).Imm(1);
                INST(3, Opcode::StoreObject).s64().Inputs(0, 2).TypeId(208U).ObjField(i);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(4, Opcode::ReturnVoid);
            }
        }
        SetNumArgs(1);

        std::vector<std::string> expected_asm = {
#ifdef PANDA_COMPILER_CFI
            "stp x29, x30, [sp, #-16]!",  // prolog save FP and LR
            "mov x29, sp",                // prolog set FP
            "stp x19, x20, [sp, #-80]",   // prolog save callee-saved
#else
            "stp x19, x20, [sp, #-96]",   // prolog save callee-saved
#endif
            "ldr x19, [x1]",
            "add x19, x19, #0x1 // (1)",
            "str x19, [x1]",
#ifdef PANDA_COMPILER_CFI
            "ldp x19, x20, [sp, #-80]",   // epilog restore callee-saved
            "ldp x29, x30, [sp], #16",    // epilog restore FP and LR
#else
            "ldp x19, x20, [sp, #-96]",   // epilog restore callee-saved
#endif
            "ret"};

        GraphChecker(GetGraph()).Check();
        GetGraph()->RunPass<RegAllocLinearScan>();
        bool setup_frame = GetGraph()->GetMethodProperties().GetRequireFrameSetup();
        ASSERT_TRUE(setup_frame ? GetGraph()->RunPass<Codegen>() : GetGraph()->RunPass<CodegenNative>());
        ASSERT_TRUE(GetGraph()->GetData().Size() == expected_asm.size() * vixl::aarch64::kInstructionSize);
        auto code_entry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetData().Data());
        auto code_exit = code_entry + GetGraph()->GetData().Size();
        size_t code_items = (code_exit - code_entry) / vixl::aarch64::kInstructionSize;
        ASSERT_TRUE(code_items == expected_asm.size());

        auto& decoder {GetDecoder()};
        auto disasm(CreateDisassembler());
        vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&disasm});
        for (size_t item = 0; item < code_items; ++item) {
            decoder.Decode(code_entry + item * vixl::aarch64::kInstructionSize);
            EXPECT_EQ(expected_asm.at(item), disasm.GetOutput());
        }
    }

    void CheckLeafWithParamsOnStackPrologue()
    {
        // RedundantOps::sum()
        RuntimeInterface::FieldPtr i = (void *)(0xDEADBEEF);
        GRAPH(GetGraph())
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s64();
            PARAMETER(2, 2).s64();
            PARAMETER(3, 3).s64();
            PARAMETER(4, 4).s64();
            PARAMETER(5, 5).s64();
            PARAMETER(6, 6).s64();
            PARAMETER(7, 7).s64();
            PARAMETER(8, 8).s64();

            BASIC_BLOCK(2, -1)
            {
                INST(10, Opcode::Add).s64().Inputs(1, 2);
                INST(11, Opcode::Add).s64().Inputs(3, 4);
                INST(12, Opcode::Add).s64().Inputs(5, 6);
                INST(13, Opcode::Add).s64().Inputs(7, 8);
                INST(14, Opcode::Add).s64().Inputs(10, 11);
                INST(15, Opcode::Add).s64().Inputs(12, 13);
                INST(16, Opcode::Add).s64().Inputs(14, 15);
                INST(17, Opcode::StoreObject).s64().Inputs(0, 16).TypeId(301U).ObjField(i);
                INST(18, Opcode::ReturnVoid);
            }
        }
        SetNumArgs(9);

        // In this case two parameters are passed on stack,
        // thus to address them SP needs to be adjusted in prolog/epilog.
        std::vector<std::string> expected_asm = {
#ifdef PANDA_COMPILER_CFI
            "stp x29, x30, [sp, #-16]!",    // prolog save FP and LR
            "mov x29, sp",                  // prolog set FP
            "stp x19, x20, [sp, #-112]",    // prolog callee-saved
            "stp x21, x22, [sp, #-96]",     // prolog callee-saved
            "stp x23, x24, [sp, #-80]",     // prolog callee-saved
            "sub sp, sp, #0x230 // (560)",  // prolog adjust SP
#else
            "stp x19, x20, [sp, #-128]",    // prolog callee-saved
            "stp x21, x22, [sp, #-112]",    // prolog callee-saved
            "stp x23, x24, [sp, #-96]",     // prolog callee-saved
            "sub sp, sp, #0x240 // (576)",  // prolog adjust SP
#endif
            "add",                          // "add x19, x2, x3"
            "add",                          // "add x21, x4, x5"
            "add",                          // "add x22, x6, x7"
            "add x16, sp, #0x240 // (576)", // load params from stack
            "ldp x23, x24, [x16]",          // load params from stack
            "add",                          // "add x23, x23, x24"
            "add",                          // "add x19, x19, x21"
            "add",                          // "add x21, x22, x23"
            "add",                          // "add x19, x19, x21"
            "str x19, [x1]",
            "ldp x19, x20, [sp, #448]",     // restore callee-saved
            "ldp x21, x22, [sp, #464]",     // restore callee-saved
            "ldp x23, x24, [sp, #480]",     // restore callee-saved
#ifdef PANDA_COMPILER_CFI
            "add sp, sp, #0x230 // (560)",  // epilog adjust SP
            "ldp x29, x30, [sp], #16",      // epilog restore FP and LR
#else
            "add sp, sp, #0x240 // (576)",  // epilog adjust SP
#endif
            "ret"};

        std::regex add_regex("^add[[:blank:]]+x[0-9]+,[[:blank:]]+x[0-9]+,[[:blank:]]+x[0-9]+",
                             std::regex::egrep | std::regex::icase);

        GraphChecker(GetGraph()).Check();
        GetGraph()->RunPass<RegAllocLinearScan>();
        bool setup_frame = GetGraph()->GetMethodProperties().GetRequireFrameSetup();
        ASSERT_TRUE(setup_frame ? GetGraph()->RunPass<Codegen>() : GetGraph()->RunPass<CodegenNative>());
        ASSERT_TRUE(GetGraph()->GetData().Size() == expected_asm.size() * vixl::aarch64::kInstructionSize);
        auto code_entry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetData().Data());
        auto code_exit = code_entry + GetGraph()->GetData().Size();
        size_t code_items = (code_exit - code_entry) / vixl::aarch64::kInstructionSize;
        ASSERT_TRUE(code_items == expected_asm.size());

        auto& decoder {GetDecoder()};
        auto disasm(CreateDisassembler());
        vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&disasm});
        for (size_t item = 0; item < code_items; ++item) {
            decoder.Decode(code_entry + item * vixl::aarch64::kInstructionSize);
            // replace 'add rx, ry, rz' with 'add' to make comparison independent of regalloc
            std::string s = std::regex_replace(disasm.GetOutput(), add_regex, "add");
            EXPECT_EQ(expected_asm.at(item), s);
        }
    }
};
// clang-format on

TEST_F(CodegenLeafPrologueTest, LeafPrologueGeneration)
{
    CheckLeafPrologue();
}

TEST_F(CodegenLeafPrologueTest, LeafWithParamsOnStackPrologueGeneration)
{
    CheckLeafWithParamsOnStackPrologue();
}

class CodegenTest : public VixlDisasmTest {
public:
    template <typename T, size_t len>
    void AssertCode(const T (&expected_code)[len])
    {
        auto code_entry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetData().Data());
        auto code_exit = code_entry + GetGraph()->GetData().Size();
        ASSERT(code_entry != nullptr && code_exit != nullptr);
        auto &decoder {GetDecoder()};
        auto disasm(CreateDisassembler());
        vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&disasm});

        size_t index = 0;
        for (auto instr = code_entry; instr < code_exit; instr += vixl::aarch64::kInstructionSize) {
            decoder.Decode(instr);
            auto output = disasm.GetOutput();
            if (index == 0) {
                if (std::strncmp(output, expected_code[index], std::strlen(expected_code[index])) == 0) {
                    index++;
                }
                continue;
            }
            if (index >= len) {
                break;
            }
            ASSERT_TRUE(std::strncmp(output, expected_code[index], std::strlen(expected_code[index])) == 0);
            index++;
        }
        ASSERT_EQ(index, len);
    }
};

TEST_F(CodegenTest, CallVirtual)
{
    auto graph = GetGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::CallVirtual).v0id().InputsAutoType(0, 1, 2);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }
    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());
    // exclude offset from verification to avoid test modifications
    const char *expected_code[] = {"ldr w0, [x1, #", "ldr x0, [x0, #", "ldr x30, [x0, #",
                                   "blr x30"};  // CallVirtual is encoded without tmp reg
    AssertCode(expected_code);
}

TEST_F(CodegenTest, EncodeMemCopy)
{
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i32().DstReg(0U);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SpillFill);
            INST(3, Opcode::Return).i32().Inputs(0).DstReg(0U);
        }
    }
    auto spill_fill = INS(2).CastToSpillFill();
    // Add moves chain: R0 -> S0 -> S1 -> R0 [u32]
    spill_fill->AddSpillFill(Location::MakeRegister(0), Location::MakeStackSlot(0), DataType::INT32);
    spill_fill->AddSpillFill(Location::MakeStackSlot(0), Location::MakeStackSlot(1), DataType::INT32);
    spill_fill->AddSpillFill(Location::MakeStackSlot(1), Location::MakeRegister(0), DataType::INT32);

    graph->SetStackSlotsCount(2U);
#ifndef NDEBUG
    graph->SetRegAllocApplied();
#endif
    EXPECT_TRUE(graph->RunPass<Codegen>());

    // Check that stack slots are 64-bit wide
    const char *expected_code[] = {"str x0, [sp, #16]", "ldr x16, [sp, #16]", "str x16, [sp, #8]", "ldr w0, [sp, #8]"};
    AssertCode(expected_code);
}

TEST_F(CodegenTest, EncodeWithZeroReg)
{
    // MAdd a, b, c <=> c + a * b

    // a = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();
            PARAMETER(2, 1).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MAdd).i64().Inputs(0, 1, 2);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mov x0, x2"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // b = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();
            PARAMETER(2, 1).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MAdd).i64().Inputs(1, 0, 2);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mov x0, x2"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // c = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();
            PARAMETER(2, 1).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MAdd).i64().Inputs(1, 2, 0);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mul x0, x1, x2"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // MSub a, b, c <=> c - a * b

    // a = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();
            PARAMETER(2, 1).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MSub).i64().Inputs(0, 1, 2);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mov x0, x2"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // b = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();
            PARAMETER(2, 1).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MSub).i64().Inputs(1, 0, 2);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mov x0, x2"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // c = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();
            PARAMETER(2, 1).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MSub).i64().Inputs(1, 2, 0);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mneg x0, x1, x2"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // MNeg a, b <=> -(a * b)

    // a = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MNeg).i64().Inputs(0, 1);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mov x0, #0"};
        AssertCode(expected_code);
        ResetGraph();
    }

    // b = 0
    {
        auto graph = GetGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 0).i64();
            PARAMETER(1, 0).i64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MNeg).i64().Inputs(1, 0);
                INST(4, Opcode::Return).i64().Inputs(3);
            }
        }

        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());

        const char *expected_code[] = {"mov x0, #0"};
        AssertCode(expected_code);
        ResetGraph();
    }
}

}  // namespace panda::compiler
