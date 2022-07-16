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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "libpandafile/bytecode_emitter.h"
#include "libpandafile/bytecode_instruction.h"
#include "libpandafile/file_items.h"
#include "libpandafile/value.h"
#include "runtime/bridge/bridge.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/interpreter/frame.h"
#include "runtime/tests/test_utils.h"

using TypeId = panda::panda_file::Type::TypeId;
using Opcode = panda::BytecodeInstruction::Opcode;

namespace panda::test {

static std::string g_call_result;

class InterpreterToCompiledCodeBridgeTest : public testing::Test {
public:
    InterpreterToCompiledCodeBridgeTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);

        thread_ = MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
        g_call_result = "";
    }

    ~InterpreterToCompiledCodeBridgeTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    uint16_t *MakeShorty(const std::initializer_list<TypeId> &shorty)
    {
        constexpr size_t ELEM_SIZE = 4;
        constexpr size_t ELEM_COUNT = std::numeric_limits<uint16_t>::digits / ELEM_SIZE;

        uint16_t val = 0;
        uint32_t count = 0;
        for (auto it = shorty.begin(); it != shorty.end(); ++it) {
            if (count == ELEM_COUNT) {
                shorty_.push_back(val);
                val = 0;
                count = 0;
            }
            val |= static_cast<uint8_t>(*it) << ELEM_SIZE * count;
            ++count;
        }
        if (count == ELEM_COUNT) {
            shorty_.push_back(val);
            val = 0;
            count = 0;
        }
        shorty_.push_back(val);
        return shorty_.data();
    }

protected:
    MTManagedThread *thread_;
    std::vector<uint16_t> shorty_;
};

// Test interpreter -> compiled code bridge

template <typename Arg>
std::string ArgsToString(const Arg &arg)
{
    std::ostringstream out;
    out << arg;
    return out.str();
}

template <typename Arg, typename... Args>
std::string ArgsToString(const Arg &a0, Args... args)
{
    std::ostringstream out;
    out << a0 << ", " << ArgsToString(args...);
    return out.str();
}

template <typename... Args>
std::string PrintFunc(const char *ret, const char *name, Args... args)
{
    std::ostringstream out;
    out << ret << " " << name << "(" << ArgsToString(args...) << ")";
    return out.str();
}

static void VoidNoArg(Method *method)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method);
}

template <bool is_dynamic = false>
static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
{
    return panda::CreateFrameWithSize(Frame::GetActualSize<is_dynamic>(nregs), nregs, method, prev);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeVoidNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidNoArg", &callee));

    uint8_t insn2[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn2, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidNoArg", &callee));

    g_call_result = "";
    InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidNoArg", &callee));

    FreeFrame(frame);
}

static void InstanceVoidNoArg(Method *method, ObjectHeader *this_)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, this_);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeInstanceVoidNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), 0, 1, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(InstanceVoidNoArg));
    Frame *frame = CreateFrame(1, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);

    ObjectHeader *obj1 = panda::mem::AllocateNullifiedPayloadString(5);
    frame_handler.GetAccAsVReg().SetReference(obj1);
    ObjectHeader *obj2 = panda::mem::AllocateNullifiedPayloadString(4);
    frame_handler.GetVReg(0).SetReference(obj2);

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidNoArg", &callee, obj2));

    uint8_t insn2[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn2, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidNoArg", &callee, obj1));

    g_call_result = "";
    int64_t args[] = {static_cast<int64_t>(ToUintPtr(obj2))};
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidNoArg", &callee, obj2));

    FreeFrame(frame);
}

static uint8_t ByteNoArg(Method *method)
{
    g_call_result = PrintFunc("uint8_t", __FUNCTION__, method);
    return uint8_t(5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeByteNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::U8});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ByteNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("uint8_t", "ByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), uint8_t(5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    uint8_t insn_acc[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn_acc, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("uint8_t", "ByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), uint8_t(5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(int32_t(res.value), uint8_t(5));
    EXPECT_EQ(res.tag, 0UL);
    EXPECT_EQ(g_call_result, PrintFunc("uint8_t", "ByteNoArg", &callee));

    FreeFrame(frame);
}

static int8_t SignedByteNoArg(Method *method)
{
    g_call_result = PrintFunc("int8_t", __FUNCTION__, method);
    return int8_t(-5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeSignedByteNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I8});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(SignedByteNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("int8_t", "SignedByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), int8_t(-5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    uint8_t insn_acc[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn_acc, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("int8_t", "SignedByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), int8_t(-5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(int32_t(res.value), int8_t(-5));
    EXPECT_EQ(res.tag, 0UL);
    EXPECT_EQ(g_call_result, PrintFunc("int8_t", "SignedByteNoArg", &callee));

    FreeFrame(frame);
}

static bool BoolNoArg(Method *method)
{
    g_call_result = PrintFunc("bool", __FUNCTION__, method);
    return true;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeBoolNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::U1});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(BoolNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("bool", "BoolNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), true);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(int32_t(res.value), true);
    EXPECT_EQ(res.tag, 0UL);
    EXPECT_EQ(g_call_result, PrintFunc("bool", "BoolNoArg", &callee));

    FreeFrame(frame);
}

static uint16_t ShortNoArg(Method *method)
{
    g_call_result = PrintFunc("uint16_t", __FUNCTION__, method);
    return uint16_t(5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeShortNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::U16});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ShortNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("uint16_t", "ShortNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), uint16_t(5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(int32_t(res.value), uint16_t(5));
    EXPECT_EQ(res.tag, 0UL);
    EXPECT_EQ(g_call_result, PrintFunc("uint16_t", "ShortNoArg", &callee));

    FreeFrame(frame);
}

static int16_t SignedShortNoArg(Method *method)
{
    g_call_result = PrintFunc("int16_t", __FUNCTION__, method);
    return int16_t(-5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeSignedShortNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I16});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(SignedShortNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_call_result, PrintFunc("int16_t", "SignedShortNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), int16_t(-5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(int32_t(res.value), int16_t(-5));
    EXPECT_EQ(res.tag, 0UL);
    EXPECT_EQ(g_call_result, PrintFunc("int16_t", "SignedShortNoArg", &callee));

    FreeFrame(frame);
}

static int32_t IntNoArg(Method *method)
{
    g_call_result = PrintFunc("int32_t", __FUNCTION__, method);
    return 5;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeIntNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(IntNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("int32_t", "IntNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().Get(), 5);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(res.value, 5);
    EXPECT_EQ(res.tag, 0UL);
    ASSERT_EQ(g_call_result, PrintFunc("int32_t", "IntNoArg", &callee));

    FreeFrame(frame);
}

static int64_t LongNoArg(Method *method)
{
    g_call_result = PrintFunc("int64_t", __FUNCTION__, method);
    return 8;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeLongNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(LongNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("int64_t", "LongNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().Get(), 8);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(res.value, 8);
    EXPECT_EQ(res.tag, 0UL);
    ASSERT_EQ(g_call_result, PrintFunc("int64_t", "LongNoArg", &callee));

    FreeFrame(frame);
}

static double DoubleNoArg(Method *method)
{
    g_call_result = PrintFunc("double", __FUNCTION__, method);
    return 3.0;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeDoubleNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(DoubleNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("double", "DoubleNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().GetDouble(), 3.0);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(bit_cast<double>(res.value), 3.0);
    EXPECT_EQ(res.tag, 0UL);
    ASSERT_EQ(g_call_result, PrintFunc("double", "DoubleNoArg", &callee));

    FreeFrame(frame);
}

static ObjectHeader *ObjNoArg(Method *method)
{
    g_call_result = PrintFunc("Object", __FUNCTION__, method);
    return nullptr;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeObjNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::REFERENCE});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ObjNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("Object", "ObjNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().GetReference(), nullptr);
    EXPECT_EQ(frame->GetAcc().GetTag(), 1);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(reinterpret_cast<ObjectHeader *>(res.value), nullptr);
    EXPECT_EQ(res.tag, 1UL);
    ASSERT_EQ(g_call_result, PrintFunc("Object", "ObjNoArg", &callee));

    FreeFrame(frame);
}

static DecodedTaggedValue VRegNoArg(Method *method)
{
    g_call_result = PrintFunc("vreg", __FUNCTION__, method);
    return DecodedTaggedValue(5, 7);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeVRegNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VRegNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_call_result = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("vreg", "VRegNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().GetValue(), 5);
    ASSERT_EQ(frame->GetAcc().GetTag(), 7);

    g_call_result = "";
    DecodedTaggedValue res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("vreg", "VRegNoArg", &callee));
    ASSERT_EQ(res.value, 5);
    ASSERT_EQ(res.tag, 7);

    FreeFrame(frame);
}

static void VoidInt(Method *method, int32_t a0)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeInt)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidInt));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    frame->GetVReg(1).Set(5);

    uint8_t call_short_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x01, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_short_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidInt", &callee, 5));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x01, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidInt", &callee, 5));

    g_call_result = "";
    int64_t arg = 5;
    InvokeCompiledCodeWithArgArray(&arg, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidInt", &callee, 5));

    frame->GetVReg(1).Set(0);
    frame->GetAcc().SetValue(5);
    uint8_t call_acc_insn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidInt", &callee, 5));

    FreeFrame(frame);
}

static void InstanceVoidInt(Method *method, ObjectHeader *this_, int32_t a0)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, this_, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeInstanceInt)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), 0, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(InstanceVoidInt));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    ObjectHeader *obj = panda::mem::AllocateNullifiedPayloadString(1);
    frame_handler.GetVReg(0).SetReference(obj);
    frame_handler.GetVReg(1).Set(5);

    uint8_t call_short_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x10, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_short_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    g_call_result = "";
    int64_t args[] = {static_cast<int64_t>(ToUintPtr(obj)), 5};
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    frame_handler.GetVReg(1).Set(0);
    frame_handler.GetAcc().SetValue(5);
    uint8_t call_acc_insn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x10, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    FreeFrame(frame);
}

static void VoidVReg(Method *method, int64_t value)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeVReg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidVReg));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(1).Set(5);

    uint8_t call_short_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x01, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_short_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidVReg", &callee, 5));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x01, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidVReg", &callee, 5));

    g_call_result = "";
    int64_t arg[] = {5, 8};
    InvokeCompiledCodeWithArgArray(arg, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidVReg", &callee, 5));

    frame_handler.GetVReg(1).Set(0);
    frame_handler.GetAcc().SetValue(5);
    uint8_t call_acc_short[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x01, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_short, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidVReg", &callee, 5));
    FreeFrame(frame);
}

static void VoidIntVReg(Method *method, int32_t a0, int64_t value)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeIntVReg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidIntVReg));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(2);
    frame_handler.GetVReg(1).Set(5);

    uint8_t call_short_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x10, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_short_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    g_call_result = "";
    int64_t arg[] = {2, 5, 8};
    InvokeCompiledCodeWithArgArray(arg, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    frame_handler.GetAcc().SetValue(5);
    frame_handler.GetVReg(1).Set(0);
    uint8_t call_acc_short_insn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x10, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_short_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    FreeFrame(frame);
}

// arm max number of register parameters
static void Void3Int(Method *method, int32_t a0, int32_t a1, int32_t a2)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke3Int)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void3Int));
    Frame *frame = CreateFrame(3, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetAcc().SetValue(0);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);

    uint8_t call_insn[] = {static_cast<uint8_t>(Opcode::CALL_V4_V4_V4_V4_ID16), 0x10, 0x02, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void3Int", &callee, 1, 2, 3));

    // callee(acc, v1, v2)
    uint8_t call_acc_insn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_V4_V4_V4_IMM4_ID16), 0x21, 0x00, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void3Int", &callee, 0, 2, 3));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void3Int", &callee, 1, 2, 3));

    int64_t args[] = {1, 2, 3};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void3Int", &callee, 1, 2, 3));

    FreeFrame(frame);
}

static void Void2IntLongInt(Method *method, int32_t a0, int32_t a1, int64_t a2, int32_t a3)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke2IntLongInt)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I64, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void2IntLongInt));
    Frame *frame = CreateFrame(4, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);

    uint8_t call_insn[] = {static_cast<uint8_t>(Opcode::CALL_V4_V4_V4_V4_ID16), 0x10, 0x32, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    int64_t args[] = {1, 2, 3, 4};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    frame_handler.GetVReg(2).Set(0);
    frame_handler.GetAcc().SetValue(3);
    uint8_t call_acc_insn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_V4_V4_V4_IMM4_ID16), 0x10, 0x23, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    FreeFrame(frame);
}

static void VoidLong(Method *method, int64_t a0)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeLong)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidLong));
    Frame *frame = CreateFrame(1, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(9);

    uint8_t call_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidLong", &callee, 9));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidLong", &callee, 9));

    int64_t args[] = {9};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidLong", &callee, 9));

    FreeFrame(frame);
}

static void VoidDouble(Method *method, double a0)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeDouble)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidDouble));
    Frame *frame = CreateFrame(1, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(4.0);

    uint8_t call_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidDouble", &callee, 4.0));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidDouble", &callee, 4.0));

    int64_t args[] = {bit_cast<int64_t>(4.0)};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "VoidDouble", &callee, 4.0));

    FreeFrame(frame);
}

static void Void4Int(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke4Int)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void4Int));
    Frame *frame = CreateFrame(4, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);

    uint8_t call_insn[] = {static_cast<uint8_t>(Opcode::CALL_V4_V4_V4_V4_ID16), 0x10, 0x32, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    int64_t args[] = {1, 2, 3, 4};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    frame_handler.GetVReg(3).Set(0);
    frame_handler.GetAcc().SetValue(4);
    uint8_t call_acc_insn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_V4_V4_V4_IMM4_ID16), 0x10, 0x32, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_acc_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    FreeFrame(frame);
}

static void Void2Long(Method *method, int64_t a0, int64_t a1)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke2Long)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I64, TypeId::I64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void2Long));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(3);
    frame_handler.GetVReg(1).Set(9);

    uint8_t call_insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x10, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2Long", &callee, 3, 9));

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2Long", &callee, 3, 9));

    int64_t args[] = {3, 9};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void2Long", &callee, 3, 9));

    FreeFrame(frame);
}

static void Void4IntDouble(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, double a4)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke4IntDouble)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void4IntDouble));
    Frame *frame = CreateFrame(5, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5.0);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void4IntDouble", &callee, 1, 2, 3, 4, 5.0));

    int64_t args[] = {1, 2, 3, 4, bit_cast<int64_t>(5.0)};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void4IntDouble", &callee, 1, 2, 3, 4, 5.0));

    FreeFrame(frame);
}

// aarch64 max number of register parameters
static void Void7Int(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke7Int)
{
    uint16_t *shorty = MakeShorty(
        {TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void7Int));
    Frame *frame = CreateFrame(7, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5);
    frame_handler.GetVReg(5).Set(6);
    frame_handler.GetVReg(6).Set(7);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void7Int", &callee, 1, 2, 3, 4, 5, 6, 7));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void7Int", &callee, 1, 2, 3, 4, 5, 6, 7));

    FreeFrame(frame);
}

static void Void7Int8Double(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                            int32_t a6, double d0, double d1, double d2, double d3, double d4, double d5, double d6,
                            double d7)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, d0, d1, d2, d3, d4, d5, d6, d7);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke7Int8Double)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64,
                                   TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void7Int8Double));
    Frame *frame = CreateFrame(15, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5);
    frame_handler.GetVReg(5).Set(6);
    frame_handler.GetVReg(6).Set(7);
    frame_handler.GetVReg(7).Set(8.0);
    frame_handler.GetVReg(8).Set(9.0);
    frame_handler.GetVReg(9).Set(10.0);
    frame_handler.GetVReg(10).Set(11.0);
    frame_handler.GetVReg(11).Set(12.0);
    frame_handler.GetVReg(12).Set(13.0);
    frame_handler.GetVReg(13).Set(14.0);
    frame_handler.GetVReg(14).Set(15.0);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void7Int8Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8.0, 9.0, 10.0, 11.0,
                                       12.0, 13.0, 14.0, 15.0));

    int64_t args[] = {1,
                      2,
                      3,
                      4,
                      5,
                      6,
                      7,
                      bit_cast<int64_t>(8.0),
                      bit_cast<int64_t>(9.0),
                      bit_cast<int64_t>(10.0),
                      bit_cast<int64_t>(11.0),
                      bit_cast<int64_t>(12.0),
                      bit_cast<int64_t>(13.0),
                      bit_cast<int64_t>(14.0),
                      bit_cast<int64_t>(15.0)};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void7Int8Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8.0, 9.0, 10.0, 11.0,
                                       12.0, 13.0, 14.0, 15.0));

    FreeFrame(frame);
}

static void Void8Int(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6,
                     int32_t a7)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, a7);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke8Int)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void8Int));
    Frame *frame = CreateFrame(8, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5);
    frame_handler.GetVReg(5).Set(6);
    frame_handler.GetVReg(6).Set(7);
    frame_handler.GetVReg(7).Set(8);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void8Int", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7, 8};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void8Int", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    FreeFrame(frame);
}

static void Void6IntVReg(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                         int64_t value)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke6IntVReg)
{
    uint16_t *shorty = MakeShorty(
        {TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void6IntVReg));
    Frame *frame = CreateFrame(8, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5);
    frame_handler.GetVReg(5).Set(6);
    frame_handler.GetVReg(6).Set(7);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void6IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7, 8};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void6IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7));

    FreeFrame(frame);
}

static void Void7IntVReg(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                         int32_t a6, int64_t value)
{
    g_call_result = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke7IntVReg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void7IntVReg));
    Frame *frame = CreateFrame(8, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5);
    frame_handler.GetVReg(5).Set(6);
    frame_handler.GetVReg(6).Set(7);
    frame_handler.GetVReg(7).Set(8);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void7IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void7IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    FreeFrame(frame);
}

static void Void8Int9Double(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                            int32_t a6, int32_t a7, double d0, double d1, double d2, double d3, double d4, double d5,
                            double d6, double d7, double d8)
{
    g_call_result =
        PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, a7, d0, d1, d2, d3, d4, d5, d6, d7, d8);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke8Int9Double)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::I32, TypeId::F64, TypeId::F64, TypeId::F64,
                                   TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void8Int9Double));
    Frame *frame = CreateFrame(17, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(frame);
    frame_handler.GetVReg(0).Set(1);
    frame_handler.GetVReg(1).Set(2);
    frame_handler.GetVReg(2).Set(3);
    frame_handler.GetVReg(3).Set(4);
    frame_handler.GetVReg(4).Set(5);
    frame_handler.GetVReg(5).Set(6);
    frame_handler.GetVReg(6).Set(7);
    frame_handler.GetVReg(7).Set(8);
    frame_handler.GetVReg(8).Set(9.0);
    frame_handler.GetVReg(9).Set(10.0);
    frame_handler.GetVReg(10).Set(11.0);
    frame_handler.GetVReg(11).Set(12.0);
    frame_handler.GetVReg(12).Set(13.0);
    frame_handler.GetVReg(13).Set(14.0);
    frame_handler.GetVReg(14).Set(15.0);
    frame_handler.GetVReg(15).Set(16.0);
    frame_handler.GetVReg(16).Set(17.0);

    uint8_t call_range_insn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_call_result = "";
    InterpreterToCompiledCodeBridge(call_range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void8Int9Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8, 9.0, 10.0, 11.0,
                                       12.0, 13.0, 14.0, 15.0, 16.0, 17.0));

    int64_t args[] = {1,
                      2,
                      3,
                      4,
                      5,
                      6,
                      7,
                      8,
                      bit_cast<int64_t>(9.0),
                      bit_cast<int64_t>(10.0),
                      bit_cast<int64_t>(11.0),
                      bit_cast<int64_t>(12.0),
                      bit_cast<int64_t>(13.0),
                      bit_cast<int64_t>(14.0),
                      bit_cast<int64_t>(15.0),
                      bit_cast<int64_t>(16.0),
                      bit_cast<int64_t>(17.0)};
    g_call_result = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("void", "Void8Int9Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8, 9.0, 10.0, 11.0,
                                       12.0, 13.0, 14.0, 15.0, 16.0, 17.0));

    FreeFrame(frame);
}

#if !defined(PANDA_TARGET_ARM32) && !defined(PANDA_TARGET_X86)
static TaggedValue NoArgDyn(Method *method, uint32_t num_args)
{
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args);
    return TaggedValue(77.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeNoArgDyn)
{
    Frame *frame = CreateFrame<true>(0, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_out(77.0);

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(NoArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_SHORT_IMM4_V4_V4_V4), 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "NoArgDyn", &callee, 0));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint8_t range_insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_RANGE_IMM16_V16), 0x00, 0x00, 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "NoArgDyn", &callee, 0));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(nullptr, 0, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "NoArgDyn", &callee, 0));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue OneArgDyn(Method *method, uint32_t num_args, int64_t arg0)
{
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0);
    return TaggedValue(24.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeOneArgDyn)
{
    Frame *frame = CreateFrame<true>(1, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(42.0);
    TaggedValue value_out(24.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(OneArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_SHORT_IMM4_V4_V4_V4), 0x01, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "OneArgDyn", &callee, 1, value_in0.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint8_t range_insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_RANGE_IMM16_V16), 0x01, 0x00, 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "OneArgDyn", &callee, 1, value_in0.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 1, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "OneArgDyn", &callee, 1, value_in0.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue OneVarArgDyn(Method *method, uint32_t num_args, ...)
{
    if (num_args != 1) {
        g_call_result = PrintFunc("any", __FUNCTION__, method, num_args);
        return TaggedValue(1.0);
    }

    va_list args;
    va_start(args, num_args);
    int64_t arg0 = va_arg(args, int64_t);
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0);
    va_end(args);
    return TaggedValue(-1.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeOneVarArgDyn)
{
    Frame *frame = CreateFrame<true>(1, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(42.0);
    TaggedValue value_out(-1.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(OneVarArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_SHORT_IMM4_V4_V4_V4), 0x01, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "OneVarArgDyn", &callee, 1, value_in0.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 1, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "OneVarArgDyn", &callee, 1, value_in0.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue TwoArgDyn(Method *method, uint32_t num_args, int64_t arg0, int64_t arg1)
{
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1);
    return TaggedValue(64.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeTwoArgDyn)
{
    Frame *frame = CreateFrame<true>(2, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(16.0);
    TaggedValue value_in1(32.0);
    TaggedValue value_out(64.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(TwoArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_SHORT_IMM4_V4_V4_V4), 0x02, 0x01};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "TwoArgDyn", &callee, 2, value_in0.GetRawData(), value_in1.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint8_t range_insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_RANGE_IMM16_V16), 0x02, 0x00, 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "TwoArgDyn", &callee, 2, value_in0.GetRawData(), value_in1.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 2, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "TwoArgDyn", &callee, 2, value_in0.GetRawData(), value_in1.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue TwoVarArgDyn(Method *method, uint32_t num_args, ...)
{
    if (num_args != 2) {
        g_call_result = PrintFunc("any", __FUNCTION__, method, num_args);
        return TaggedValue(2.0);
    }

    va_list args;
    va_start(args, num_args);
    int64_t arg0 = va_arg(args, int64_t);
    int64_t arg1 = va_arg(args, int64_t);
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1);
    va_end(args);
    return TaggedValue(-2.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeTwoVarArgDyn)
{
    Frame *frame = CreateFrame<true>(2, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(19.0);
    TaggedValue value_in1(84.0);
    TaggedValue value_out(-2.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(TwoVarArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_SHORT_IMM4_V4_V4_V4), 0x02, 0x01};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result,
              PrintFunc("any", "TwoVarArgDyn", &callee, 2, value_in0.GetRawData(), value_in1.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 2, frame, &callee, thread_);
    ASSERT_EQ(g_call_result,
              PrintFunc("any", "TwoVarArgDyn", &callee, 2, value_in0.GetRawData(), value_in1.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue ThreeArgDyn(Method *method, uint32_t num_args, int64_t arg0, int64_t arg1, int64_t arg2)
{
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1, arg2);
    return TaggedValue(1961.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeThreeArgDyn)
{
    Frame *frame = CreateFrame<true>(3, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(1147.0);
    TaggedValue value_in1(1703.0);
    TaggedValue value_in2(1938.0);
    TaggedValue value_out(1961.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());
    frame_handler.GetVReg(2).SetValue(value_in2.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ThreeArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_IMM4_V4_V4_V4_V4_V4), 0x03, 0x21, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "ThreeArgDyn", &callee, 3, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint8_t range_insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_RANGE_IMM16_V16), 0x03, 0x00, 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "ThreeArgDyn", &callee, 3, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData(), value_in2.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 3, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "ThreeArgDyn", &callee, 3, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue ThreeVarArgDyn(Method *method, uint32_t num_args, ...)
{
    if (num_args != 3) {
        g_call_result = PrintFunc("any", __FUNCTION__, method, num_args);
        return TaggedValue(3.0);
    }

    va_list args;
    va_start(args, num_args);
    int64_t arg0 = va_arg(args, int64_t);
    int64_t arg1 = va_arg(args, int64_t);
    int64_t arg2 = va_arg(args, int64_t);
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1, arg2);
    va_end(args);
    return TaggedValue(-3.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeThreeVarArgDyn)
{
    Frame *frame = CreateFrame<true>(3, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(1147.0);
    TaggedValue value_in1(1703.0);
    TaggedValue value_in2(1938.0);
    TaggedValue value_out(-3.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());
    frame_handler.GetVReg(2).SetValue(value_in2.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ThreeVarArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_IMM4_V4_V4_V4_V4_V4), 0x03, 0x21, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "ThreeVarArgDyn", &callee, 3, value_in0.GetRawData(),
                                       value_in1.GetRawData(), value_in2.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData(), value_in2.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 3, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "ThreeVarArgDyn", &callee, 3, value_in0.GetRawData(),
                                       value_in1.GetRawData(), value_in2.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue FourArgDyn(Method *method, uint32_t num_args, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3)
{
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1, arg2, arg3);
    return TaggedValue(3.14);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeFourArgDyn)
{
    Frame *frame = CreateFrame<true>(4, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(1.0);
    TaggedValue value_in1(2.0);
    TaggedValue value_in2(3.0);
    TaggedValue value_in3(4.0);
    TaggedValue value_out(3.14);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());
    frame_handler.GetVReg(2).SetValue(value_in2.GetRawData());
    frame_handler.GetVReg(3).SetValue(value_in3.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(FourArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_IMM4_V4_V4_V4_V4_V4), 0x04, 0x21, 0x03};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FourArgDyn", &callee, 4, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData(), value_in3.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint8_t range_insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_RANGE_IMM16_V16), 0x04, 0x00, 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FourArgDyn", &callee, 4, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData(), value_in3.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData(), value_in2.GetRawData(), value_in3.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 4, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FourArgDyn", &callee, 4, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData(), value_in3.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue FourVarArgDyn(Method *method, uint32_t num_args, ...)
{
    if (num_args != 4) {
        g_call_result = PrintFunc("any", __FUNCTION__, method, num_args);
        return TaggedValue(4.0);
    }

    va_list args;
    va_start(args, num_args);
    int64_t arg0 = va_arg(args, int64_t);
    int64_t arg1 = va_arg(args, int64_t);
    int64_t arg2 = va_arg(args, int64_t);
    int64_t arg3 = va_arg(args, int64_t);
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1, arg2, arg3);
    va_end(args);
    return TaggedValue(-4.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeFourVarArgDyn)
{
    Frame *frame = CreateFrame<true>(4, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(1.0);
    TaggedValue value_in1(2.0);
    TaggedValue value_in2(3.0);
    TaggedValue value_in3(4.0);
    TaggedValue value_out(-4.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());
    frame_handler.GetVReg(2).SetValue(value_in2.GetRawData());
    frame_handler.GetVReg(3).SetValue(value_in3.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(FourVarArgDyn));

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_IMM4_V4_V4_V4_V4_V4), 0x04, 0x21, 0x03};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FourVarArgDyn", &callee, 4, value_in0.GetRawData(),
                                       value_in1.GetRawData(), value_in2.GetRawData(), value_in3.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData(), value_in2.GetRawData(), value_in3.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 4, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FourVarArgDyn", &callee, 4, value_in0.GetRawData(),
                                       value_in1.GetRawData(), value_in2.GetRawData(), value_in3.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}

static TaggedValue FiveArgDyn(Method *method, uint32_t num_args, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3,
                              int64_t arg4)
{
    g_call_result = PrintFunc("any", __FUNCTION__, method, num_args, arg0, arg1, arg2, arg3, arg4);
    return TaggedValue(1515.0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeFiveArgDyn)
{
    Frame *frame = CreateFrame<true>(5, nullptr, nullptr);
    auto frame_handler = DynamicFrameHandler(frame);
    TaggedValue value_in0(1.0);
    TaggedValue value_in1(2.0);
    TaggedValue value_in2(3.0);
    TaggedValue value_in3(4.0);
    TaggedValue value_in4(5.0);
    TaggedValue value_out(1515.0);
    frame_handler.GetVReg(0).SetValue(value_in0.GetRawData());
    frame_handler.GetVReg(1).SetValue(value_in1.GetRawData());
    frame_handler.GetVReg(2).SetValue(value_in2.GetRawData());
    frame_handler.GetVReg(3).SetValue(value_in3.GetRawData());
    frame_handler.GetVReg(4).SetValue(value_in4.GetRawData());

    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, nullptr);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(FiveArgDyn));

    uint8_t range_insn[] = {static_cast<uint8_t>(Opcode::CALLI_DYN_RANGE_IMM16_V16), 0x05, 0x00, 0x00, 0x00};
    g_call_result = "";
    frame_handler.GetAcc().SetValue(0);
    InterpreterToCompiledCodeBridgeDyn(range_insn, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FiveArgDyn", &callee, 5, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData(), value_in3.GetRawData(), value_in4.GetRawData()));
    ASSERT_EQ(frame_handler.GetAcc().GetValue(), value_out.GetRawData());

    uint64_t args[] = {value_in0.GetRawData(), value_in1.GetRawData(), value_in2.GetRawData(), value_in3.GetRawData(),
                       value_in4.GetRawData()};
    g_call_result = "";
    uint64_t res = InvokeCompiledCodeWithArgArrayDyn(args, 5, frame, &callee, thread_);
    ASSERT_EQ(g_call_result, PrintFunc("any", "FiveArgDyn", &callee, 5, value_in0.GetRawData(), value_in1.GetRawData(),
                                       value_in2.GetRawData(), value_in3.GetRawData(), value_in4.GetRawData()));
    ASSERT_EQ(res, value_out.GetRawData());

    FreeFrame(frame);
}
#endif
}  // namespace panda::test
