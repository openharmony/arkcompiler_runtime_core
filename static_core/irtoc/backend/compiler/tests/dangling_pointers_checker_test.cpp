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

#include "compiler/tests/unit_test.h"
#include "irtoc/backend/compiler/dangling_pointers_checker.h"
#include "irtoc/backend/function.h"
#include "irtoc/backend/irtoc_runtime.h"

namespace panda::compiler {
class DanglingPointersCheckerTest : public GraphTest {};

class RelocationHandlerTest : public panda::irtoc::Function {
public:
    void MakeGraphImpl() override {};
    const char *GetName() const override
    {
        return "Test";
    };
    void SetTestExternalFunctions(std::initializer_list<std::string> funcs)
    {
        this->SetExternalFunctions(funcs);
    }
};

// NOLINTBEGIN(readability-magic-numbers)

// Correct load accumulator from frame with one instruction:
//    correct_acc_load  := LoadI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
//
// Correct store into frame with instruction:
//    correct_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test0)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::LoadI).Inputs(0).ref().Imm(frame_acc_offset);
            INST(6, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from frame with two instructions:
//    acc_ptr           := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
//    correct_acc_load  := LoadI(acc_ptr).Imm(frame_acc_offset).ref
//
// Correct store into frame with instruction:
//    correct_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test1)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10, 10).i64();
        CONSTANT(11, 15).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::AddI).Inputs(0).Imm(frame_acc_offset).ptr();
            INST(6, Opcode::LoadI).Inputs(5).ref().Imm(0);
            INST(8, Opcode::StoreI).ref().Inputs(0, 6).Imm(frame_acc_offset);
            INST(7, Opcode::Mul).Inputs(10, 11).i64();

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).Inputs({{compiler::DataType::INT64, 7}}).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Incorrect load accumulator from fake frame with two instructions:
//    fake_acc_ptr       := AddI(fake_frame).Imm(frame_acc_offset).ptr
//    incorrect_acc_load := LoadI(fake_acc_ptr).Imm(0).ref
//
// Correct store into frame with instruction:
//    correct_acc_store  := StoreI(LiveIn(frame).ptr, LiveIn(acc).ptr).Imm(frame_acc_offset)
TEST_F(DanglingPointersCheckerTest, test2)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).i64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::AddI).Inputs(1).Imm(frame_acc_offset).ptr();
            INST(6, Opcode::LoadI).Inputs(5).ref().Imm(0);
            INST(8, Opcode::StoreI).u64().Inputs(0, 1).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Incorrect load accumulator from fake frame with two instructions:
//    fake_acc_ptr        := AddI(fake_frame).Imm(frame_acc_offset).ptr
//    incorrect_acc_load  := LoadI(fake_acc_ptr).Imm(0).ref
//
// Incorrect store fake_acc_load into frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, fake_acc_load).Imm(frame_acc_offset)
TEST_F(DanglingPointersCheckerTest, test3)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).i64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::AddI).Inputs(1).Imm(frame_acc_offset).ptr();
            INST(6, Opcode::LoadI).Inputs(5).ref().Imm(0);
            INST(8, Opcode::StoreI).ref().Inputs(0, 6).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Incorrect load accumulator from fake frame with two instructions:
//    fake_acc_ptr        := AddI(fake_frame).Imm(frame_acc_offset).ptr
//    incorrect_acc_load  := LoadI(fake_acc_ptr).Imm(0).ref
//
// Incorrect store fake_acc_load into frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, fake_acc_load).Imm(frame_acc_offset)
//
// But acc type is integer: LiveIn(acc).u64; so the check is not performed
TEST_F(DanglingPointersCheckerTest, test4)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).i64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(10, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::AddI).Inputs(10).Imm(frame_acc_offset).ptr();
            INST(6, Opcode::LoadI).Inputs(5).ref().Imm(0);
            INST(8, Opcode::StoreI).ref().Inputs(0, 6).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from last frame definition with two instructions:
//    last_frame_def    := LoadI(LiveIn(frame).ptr).Imm(GetPrevFrameOffset()).ptr
//    acc_ptr           := AddI(last_frame_def).Imm(frame_acc_offset).ptr
//    correct_acc_load  := LoadI(acc_ptr).Imm(0).ref
//
// Correct store into frame with instruction:
//    correct_acc_store := StoreI(last_frame_def, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test5)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10, 10).i64();
        CONSTANT(11, 15).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::LoadI).Inputs(0).ptr().Imm(Frame::GetPrevFrameOffset());
            INST(5, Opcode::AddI).Inputs(8).Imm(frame_acc_offset).ptr();
            INST(6, Opcode::LoadI).Inputs(5).ref().Imm(0);
            INST(7, Opcode::Mul).Inputs(10, 11).i64();
            INST(9, Opcode::StoreI).ref().Inputs(8, 6).Imm(frame_acc_offset);

            // store tag
            INST(31, Opcode::StoreI).u64().Inputs(5, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).Inputs({{compiler::DataType::INT64, 7}}).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from last frame definition with one instructions:
//    last_frame_def      := LoadI(LiveIn(frame).ptr).Imm(GetPrevFrameOffset()).ptr
//    correct_acc_load    := LoadI(last_frame_def).Imm(frame_acc_offset).ref
//
// Incorrect store into old frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test6)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);
        INST(10, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::LoadI).Inputs(2).ptr().Imm(ManagedThread::GetFrameOffset());
            INST(5, Opcode::LoadI).Inputs(8).Imm(frame_acc_offset).ref();
            INST(9, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 10).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from last frame definition with one instructions:
//    last_frame_def      := LoadI(LiveIn(frame).ptr).Imm(GetPrevFrameOffset()).ptr
//    correct_acc_load    := LoadI(last_frame_def).Imm(frame_acc_offset).ref
//
// Incorrect store into old frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test7)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10, 10).i64();
        CONSTANT(11, 15).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::LoadI).Inputs(0).ptr().Imm(Frame::GetPrevFrameOffset());
            INST(5, Opcode::AddI).Inputs(8).Imm(frame_acc_offset).ptr();
            INST(6, Opcode::LoadI).Inputs(5).ref().Imm(0);
            INST(7, Opcode::Mul).Inputs(10, 11).i64();
            INST(9, Opcode::StoreI).ref().Inputs(0, 6).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).Inputs({{compiler::DataType::INT64, 7}}).ptr();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Use old accumulator after Call
TEST_F(DanglingPointersCheckerTest, test8)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::LoadI).Inputs(0).ref().Imm(frame_acc_offset);
            INST(6, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);

            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(7, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// The correct graph. Not use old acc after call.
//
//          start_bb
//             |
//             V
//   ------------------------------------
//  | bb_2 | new_acc_load, new_acc_store |
//   ------------------------------------
//      |
//      V
//   --------------------    --------------------
//  | bb_3 | use_old_acc |  | bb_4 | use_old_acc |
//   --------------------    --------------------
//      |                      |
//      V                      V
//    -------------          ------
//   | bb_5 | Call |        | bb_8 |
//    -------------\        -------
//      |           |       A     |
//      V           V       |     V
//    ------         --------     ------
//   | bb_6 |       |  bb_7  |-->| bb_9 |--> end_bb
//    ------         --------     ------

TEST_F(DanglingPointersCheckerTest, test9)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10, 10);
        CONSTANT(11, 11);
        CONSTANT(12, 12);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::LoadI).Inputs(0).ref().Imm(frame_acc_offset);
            INST(5, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
            INST(19, Opcode::AddI).Inputs(11).Imm(frame_acc_offset).u64();
            INST(20, Opcode::Mul).Inputs(12, 19).u64();
            INST(22, Opcode::If).Inputs(19, 20).b().CC(CC_LE);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(6, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
        }
        BASIC_BLOCK(4, 8)
        {
            INST(7, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(17, Opcode::AddI).Inputs(11).Imm(frame_acc_offset).u64();
            INST(18, Opcode::Mul).Inputs(12, 17).u64();
            INST(21, Opcode::If).Inputs(17, 18).b().CC(CC_NE);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(8, Opcode::AddI).Inputs(10).Imm(frame_acc_offset).u64();
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(16, Opcode::AddI).Inputs(11).Imm(frame_acc_offset).u64();
            INST(9, Opcode::Mul).Inputs(12, 16).u64();
            INST(15, Opcode::If).Inputs(9, 16).b().CC(CC_EQ);
        }
        BASIC_BLOCK(8, 9)
        {
            INST(13, Opcode::AddI).Inputs(12).Imm(frame_acc_offset).u64();
        }

        BASIC_BLOCK(9, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// The incorrect graph. Use old acc after call.
//
//          start_bb
//             |
//             V
//   ------------------------------------
//  | bb_2 | new_acc_load, new_acc_store |
//   ------------------------------------
//      |
//      V
//   --------------------    --------------------
//  | bb_3 | use_old_acc |  | bb_4 | use_old_acc |
//   --------------------    --------------------
//      |                      |
//      V                      V
//    -------------          --------------------
//   | bb_5 | Call |        | bb_8 | use_old_acc |
//    -------------\        ---------------------
//      |           |       A     |
//      V           V       |     V
//    ------         --------     ------
//   | bb_6 |       |  bb_7  |-->| bb_9 |--> end_bb
//    ------         --------     ------

TEST_F(DanglingPointersCheckerTest, test10)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10, 10);
        CONSTANT(11, 11);
        CONSTANT(12, 12);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::LoadI).Inputs(0).ref().Imm(frame_acc_offset);
            INST(5, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
            INST(19, Opcode::AddI).Inputs(11).Imm(frame_acc_offset).u64();
            INST(20, Opcode::Mul).Inputs(12, 19).u64();
            INST(22, Opcode::If).Inputs(19, 20).b().CC(CC_LE);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(6, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
        }
        BASIC_BLOCK(4, 8)
        {
            INST(7, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            // store tag
            INST(30, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(31, Opcode::StoreI).u64().Inputs(30, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(17, Opcode::AddI).Inputs(11).Imm(frame_acc_offset).u64();
            INST(18, Opcode::Mul).Inputs(12, 17).u64();
            INST(21, Opcode::If).Inputs(17, 18).b().CC(CC_NE);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(8, Opcode::AddI).Inputs(10).Imm(frame_acc_offset).u64();
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(16, Opcode::AddI).Inputs(11).Imm(frame_acc_offset).u64();
            INST(9, Opcode::Mul).Inputs(12, 16).u64();
            INST(15, Opcode::If).Inputs(9, 16).b().CC(CC_EQ);
        }
        BASIC_BLOCK(8, 9)
        {
            INST(23, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
            INST(13, Opcode::AddI).Inputs(12).Imm(frame_acc_offset).u64();
        }

        BASIC_BLOCK(9, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// The correct graph. Use Phi to define acc that has been changed in one branch.
//
//          start_bb
//             |
//             V
//         -----------------------------
//        |           bb_2              |
//         -----------------------------
//         |                           |
//         V                           |
//   -----------------------------     |
//  | bb_3 | load_acc, change_acc |    |
//   -----------------------------     |
//                             |       |
//                             V       V
//               ------------------------------------------------
//              |      | last_acc_def = Phi(live_in, change_acc) |
//              | bb_4 | store(last_acc_def)                     |
//              |      | Call                                    |
//               ------------------------------------------------
//                                     |
//                                     V
//                                  end_bb

TEST_F(DanglingPointersCheckerTest, test11)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10, 10);
        CONSTANT(11, 11);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::AddI).Inputs(10).Imm(frame_acc_offset).u64();
            INST(4, Opcode::If).Inputs(3, 11).b().CC(CC_NE);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::LoadI).Inputs(0).ref().Imm(frame_acc_offset);
            INST(6, Opcode::AddI).Inputs(5).ptr().Imm(10);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::Phi).Inputs(1, 6).ptr();
            INST(8, Opcode::StoreI).ref().Inputs(0, 7).Imm(frame_acc_offset);

            // store tag
            INST(21, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(22, Opcode::StoreI).u64().Inputs(21, 2).Imm(acc_tag_offset);

            INST(9, Opcode::Call).TypeId(0).ptr();
            INST(20, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Use object after call:
//    frame_live_in := LiveIn(frame)
//    Call
//    frame_use := LoadI(frame_live_in).Imm(frame_acc_offset).i64
TEST_F(DanglingPointersCheckerTest, test12)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::StoreI).ref().Inputs(0, 1).Imm(frame_acc_offset);

            // store tag
            INST(8, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(9, Opcode::StoreI).u64().Inputs(8, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(5, Opcode::AddI).Inputs(1).ptr().Imm(frame_acc_offset);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Use primitive after call:
//    frame_live_in := LiveIn(frame)
//    primitive := LoadI(frame_live_in).Imm(offset).u64
//    Call
//    primitive_use := AddI(not_object).Imm(10).u64
TEST_F(DanglingPointersCheckerTest, test13)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::LoadI).Inputs(0).u64().Imm(10);
            INST(7, Opcode::StoreI).ref().Inputs(0, 1).Imm(frame_acc_offset);

            // store tag
            INST(8, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(9, Opcode::StoreI).u64().Inputs(8, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(5, Opcode::AddI).Inputs(6).u64().Imm(frame_acc_offset);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Use skipped pointer inst after call:
//    frame_live_in := LiveIn(frame)
//    pointer := AddI(frame_live_in).Imm(frame_acc_offset).ptr
//    Call
//    primitive := LoadI(pointer).Imm(0).u64
TEST_F(DanglingPointersCheckerTest, test14)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(7, Opcode::StoreI).ref().Inputs(0, 1).Imm(frame_acc_offset);

            // store tag
            INST(8, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(9, Opcode::StoreI).u64().Inputs(8, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(5, Opcode::LoadI).Inputs(6).u64().Imm(0);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Use ref inst after call:
//    pointer := AddI(acc_live_in).Imm(10).ptr
//    Call
//    primitive := LoadI(pointer).Imm(0).u64
TEST_F(DanglingPointersCheckerTest, test15)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2, -1)
        {
            INST(10, Opcode::LoadI).Inputs(0).ref().Imm(10);
            INST(6, Opcode::AddI).Inputs(1).ptr().Imm(10);
            INST(7, Opcode::StoreI).ref().Inputs(0, 1).Imm(frame_acc_offset);

            // store tag
            INST(8, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(9, Opcode::StoreI).u64().Inputs(8, 2).Imm(acc_tag_offset);

            INST(3, Opcode::Call).TypeId(0).ptr();
            INST(5, Opcode::LoadI).Inputs(10).u64().Imm(0);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from frame:
//    correct_acc_load      := LoadI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
//
// Correct accumulatore and tag store:
//    correct_acc_store     := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
//    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
//    correct_acc_tag_store := StoreI(acc_ptr, LiveIn(acc_tag).u64).Imm(acc_tag_offset).u64
TEST_F(DanglingPointersCheckerTest, test16)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::LoadI).Inputs(0).ref().Imm(frame_acc_offset);
            INST(5, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(6, Opcode::StoreI).ref().Inputs(0, 4).Imm(frame_acc_offset);
            INST(7, Opcode::StoreI).u64().Inputs(5, 2).Imm(acc_tag_offset);
            INST(8, Opcode::Call).TypeId(0).ptr();
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator and tag:
//    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
//    correct_acc_load      := LoadI(acc_ptr).Imm(frame_acc_offset).ref
//    correct_acc_tag_load  := LoadI(acc_ptr).Imm(acc_tag_offset).u64
//
// Correct accumulatore and tag store:
//    correct_acc_store     := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
//    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
//    correct_acc_tag_store := StoreI(acc_ptr, correct_acc_tag_load).Imm(acc_tag_offset).u64
TEST_F(DanglingPointersCheckerTest, test17)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(5, Opcode::LoadI).Inputs(4).ref().Imm(0);
            INST(6, Opcode::LoadI).Inputs(4).i64().Imm(acc_tag_offset);
            INST(7, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);
            INST(8, Opcode::StoreI).u64().Inputs(4, 6).Imm(acc_tag_offset);
            INST(9, Opcode::Call).TypeId(0).ptr();
            INST(10, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator and tag:
//    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
//    correct_acc_load      := LoadI(acc_ptr).Imm(frame_acc_offset).ref
//    acc_tag_ptr           := AddI(acc_ptr).Imm(acc_tag_offset).ref
//    correct_acc_tag_load  := LoadI(acc_tag_ptr).Imm(0).u64
//
// Correct accumulatore and tag store:
//    correct_acc_store     := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
//    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
//    correct_acc_tag_store := StoreI(acc_ptr, correct_acc_tag_load).Imm(acc_tag_offset).u64
TEST_F(DanglingPointersCheckerTest, test18)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(5, Opcode::LoadI).Inputs(4).ref().Imm(0);
            INST(6, Opcode::AddI).Inputs(4).ptr().Imm(acc_tag_offset);
            INST(7, Opcode::LoadI).Inputs(6).i64().Imm(0);
            INST(8, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);
            INST(9, Opcode::StoreI).u64().Inputs(4, 7).Imm(acc_tag_offset);
            INST(10, Opcode::Call).TypeId(0).ptr();
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// Correct load accumulator and tag:
//    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
//    correct_acc_load      := LoadI(acc_ptr).Imm(frame_acc_offset).ref
//    acc_tag_ptr           := AddI(acc_ptr).Imm(acc_tag_offset).ref
//    correct_acc_tag_load  := LoadI(acc_tag_ptr).Imm(0).u64
//
// Correct accumulatore and tag store:
//    correct_acc_store       := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
//    acc_ptr                 := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
//    incorrect_acc_tag_store := StoreI(acc_ptr, LiveIn(acc_tag).u64).Imm(acc_tag_offset).u64
TEST_F(DanglingPointersCheckerTest, test19)
{
    auto arch = panda::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frame_acc_offset = cross_values::GetFrameAccOffset(arch);
    auto acc_tag_offset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocation_handler;
    relocation_handler.SetTestExternalFunctions({*DanglingPointersChecker::target_funcs_.begin()});
    GetGraph()->SetRelocationHandler(&relocation_handler);
    GetGraph()->SetMethod(&relocation_handler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::AddI).Inputs(0).ptr().Imm(frame_acc_offset);
            INST(5, Opcode::LoadI).Inputs(4).ref().Imm(0);
            INST(6, Opcode::AddI).Inputs(4).ptr().Imm(acc_tag_offset);
            INST(7, Opcode::LoadI).Inputs(6).i64().Imm(0);
            INST(8, Opcode::StoreI).ref().Inputs(0, 5).Imm(frame_acc_offset);
            INST(9, Opcode::StoreI).u64().Inputs(4, 2).Imm(acc_tag_offset);
            INST(10, Opcode::Call).TypeId(0).ptr();
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<panda::compiler::DanglingPointersChecker>());
}

// NOLINTEND(readability-magic-numbers)
}  // namespace panda::compiler
