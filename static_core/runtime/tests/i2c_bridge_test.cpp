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

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays)

namespace panda::test {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::string g_gCallResult;

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
        g_gCallResult = "";
    }

    ~InterpreterToCompiledCodeBridgeTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(InterpreterToCompiledCodeBridgeTest);
    NO_MOVE_SEMANTIC(InterpreterToCompiledCodeBridgeTest);

    uint16_t *MakeShorty(const std::initializer_list<TypeId> &shorty)
    {
        constexpr size_t ELEM_SIZE = 4;
        constexpr size_t ELEM_COUNT = std::numeric_limits<uint16_t>::digits / ELEM_SIZE;

        uint16_t val = 0;
        uint32_t count = 0;
        for (const TypeId &typeId : shorty) {
            if (count == ELEM_COUNT) {
                shorty_.push_back(val);
                val = 0;
                count = 0;
            }
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            val |= static_cast<uint8_t>(typeId) << ELEM_SIZE * count;
            ++count;
        }
        if (count == ELEM_COUNT) {
            shorty_.push_back(val);
            val = 0;
        }
        shorty_.push_back(val);
        return shorty_.data();
    }

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MTManagedThread *thread_ {};

private:
    std::vector<uint16_t> shorty_;
};

// Test interpreter -> compiled code bridge

template <typename Arg>
std::string ArgsToString(const Arg &arg)
{
    std::ostringstream out;
    if constexpr (std::is_integral_v<Arg>) {
        out << std::hex << "0x" << arg;
    } else {
        out << arg;
    }
    return out.str();
}

template <typename Arg, typename... Args>
std::string ArgsToString(const Arg &a0, Args... args)
{
    std::ostringstream out;
    out << ArgsToString(a0) << ", " << ArgsToString(args...);
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
    g_gCallResult = PrintFunc("void", __FUNCTION__, method);
}

template <bool IS_DYNAMIC = false>
static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
{
    return panda::CreateFrameWithSize(Frame::GetActualSize<IS_DYNAMIC>(nregs), nregs, method, prev);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeVoidNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidNoArg", &callee));

    uint8_t insn2[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn2, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidNoArg", &callee));

    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidNoArg", &callee));

    FreeFrame(frame);
}

static void InstanceVoidNoArg(Method *method, ObjectHeader *thisHeader)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, thisHeader);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeInstanceVoidNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), 0, 1, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(InstanceVoidNoArg));
    Frame *frame = CreateFrame(1, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);

    ObjectHeader *obj1 = panda::mem::AllocateNullifiedPayloadString(5);
    frameHandler.GetAccAsVReg().SetReference(obj1);
    ObjectHeader *obj2 = panda::mem::AllocateNullifiedPayloadString(4);
    frameHandler.GetVReg(0).SetReference(obj2);

    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidNoArg", &callee, obj2));

    uint8_t insn2[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn2, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidNoArg", &callee, obj1));

    g_gCallResult = "";
    int64_t args[] = {static_cast<int64_t>(ToUintPtr(obj2))};
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidNoArg", &callee, obj2));

    FreeFrame(frame);
}

static uint8_t ByteNoArg(Method *method)
{
    g_gCallResult = PrintFunc("uint8_t", __FUNCTION__, method);
    return static_cast<uint8_t>(5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeByteNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::U8});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ByteNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("uint8_t", "ByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), static_cast<uint8_t>(5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    uint8_t insnAcc[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insnAcc, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("uint8_t", "ByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), static_cast<uint8_t>(5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(static_cast<int32_t>(res), static_cast<uint8_t>(5));
    EXPECT_EQ(g_gCallResult, PrintFunc("uint8_t", "ByteNoArg", &callee));

    FreeFrame(frame);
}

static int8_t SignedByteNoArg(Method *method)
{
    g_gCallResult = PrintFunc("int8_t", __FUNCTION__, method);
    return static_cast<int8_t>(-5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeSignedByteNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I8});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(SignedByteNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("int8_t", "SignedByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), static_cast<int8_t>(-5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    uint8_t insnAcc[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insnAcc, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("int8_t", "SignedByteNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), static_cast<int8_t>(-5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(static_cast<int32_t>(res), static_cast<int8_t>(-5));
    EXPECT_EQ(g_gCallResult, PrintFunc("int8_t", "SignedByteNoArg", &callee));

    FreeFrame(frame);
}

static bool BoolNoArg(Method *method)
{
    g_gCallResult = PrintFunc("bool", __FUNCTION__, method);
    return true;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeBoolNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::U1});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(BoolNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("bool", "BoolNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), true);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(static_cast<int32_t>(res), true);
    EXPECT_EQ(g_gCallResult, PrintFunc("bool", "BoolNoArg", &callee));

    FreeFrame(frame);
}

static uint16_t ShortNoArg(Method *method)
{
    g_gCallResult = PrintFunc("uint16_t", __FUNCTION__, method);
    return static_cast<uint16_t>(5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeShortNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::U16});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ShortNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("uint16_t", "ShortNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), static_cast<uint16_t>(5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(static_cast<int32_t>(res), static_cast<uint16_t>(5));
    EXPECT_EQ(g_gCallResult, PrintFunc("uint16_t", "ShortNoArg", &callee));

    FreeFrame(frame);
}

static int16_t SignedShortNoArg(Method *method)
{
    g_gCallResult = PrintFunc("int16_t", __FUNCTION__, method);
    return static_cast<int16_t>(-5);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeSignedShortNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I16});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(SignedShortNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    EXPECT_EQ(g_gCallResult, PrintFunc("int16_t", "SignedShortNoArg", &callee));
    EXPECT_EQ(frame->GetAcc().Get(), static_cast<int16_t>(-5));
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    EXPECT_EQ(static_cast<int32_t>(res), static_cast<int16_t>(-5));
    EXPECT_EQ(g_gCallResult, PrintFunc("int16_t", "SignedShortNoArg", &callee));

    FreeFrame(frame);
}

static int32_t IntNoArg(Method *method)
{
    g_gCallResult = PrintFunc("int32_t", __FUNCTION__, method);
    return 5;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeIntNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(IntNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("int32_t", "IntNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().Get(), 5);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(res, 5);
    ASSERT_EQ(g_gCallResult, PrintFunc("int32_t", "IntNoArg", &callee));

    FreeFrame(frame);
}

static int64_t LongNoArg(Method *method)
{
    g_gCallResult = PrintFunc("int64_t", __FUNCTION__, method);
    return 8;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeLongNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::I64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(LongNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("int64_t", "LongNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().Get(), 8);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(res, 8);
    ASSERT_EQ(g_gCallResult, PrintFunc("int64_t", "LongNoArg", &callee));

    FreeFrame(frame);
}

static double DoubleNoArg(Method *method)
{
    g_gCallResult = PrintFunc("double", __FUNCTION__, method);
    return 3.0;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeDoubleNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(DoubleNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("double", "DoubleNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().GetDouble(), 3.0);
    EXPECT_EQ(frame->GetAcc().GetTag(), 0);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(bit_cast<double>(res), 3.0);
    ASSERT_EQ(g_gCallResult, PrintFunc("double", "DoubleNoArg", &callee));

    FreeFrame(frame);
}

static ObjectHeader *ObjNoArg(Method *method)
{
    g_gCallResult = PrintFunc("Object", __FUNCTION__, method);
    return nullptr;
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeObjNoArg)
{
    uint16_t *shorty = MakeShorty({TypeId::REFERENCE});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(ObjNoArg));
    Frame *frame = CreateFrame(0, nullptr, nullptr);
    uint8_t insn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};

    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(insn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("Object", "ObjNoArg", &callee));
    ASSERT_EQ(frame->GetAcc().GetReference(), nullptr);
    EXPECT_EQ(frame->GetAcc().GetTag(), 1);

    g_gCallResult = "";
    uint64_t res = InvokeCompiledCodeWithArgArray(nullptr, frame, &callee, thread_);
    ASSERT_EQ(reinterpret_cast<ObjectHeader *>(res), nullptr);
    ASSERT_EQ(g_gCallResult, PrintFunc("Object", "ObjNoArg", &callee));

    FreeFrame(frame);
}

static void VoidInt(Method *method, int32_t a0)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeInt)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidInt));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    frame->GetVReg(1).Set(5);

    uint8_t callShortInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x01, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callShortInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidInt", &callee, 5));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x01, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidInt", &callee, 5));

    g_gCallResult = "";
    int64_t arg = 5;
    InvokeCompiledCodeWithArgArray(&arg, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidInt", &callee, 5));

    frame->GetVReg(1).Set(0);
    frame->GetAcc().SetValue(5);
    uint8_t callAccInsn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x00, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidInt", &callee, 5));

    FreeFrame(frame);
}

static void InstanceVoidInt(Method *method, ObjectHeader *thisHeader, int32_t a0)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, thisHeader, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeInstanceInt)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), 0, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(InstanceVoidInt));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    ObjectHeader *obj = panda::mem::AllocateNullifiedPayloadString(1);
    frameHandler.GetVReg(0).SetReference(obj);
    frameHandler.GetVReg(1).Set(5);

    uint8_t callShortInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x10, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callShortInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    g_gCallResult = "";
    int64_t args[] = {static_cast<int64_t>(ToUintPtr(obj)), 5};
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    frameHandler.GetVReg(1).Set(0);
    frameHandler.GetAcc().SetValue(5);
    uint8_t callAccInsn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x10, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "InstanceVoidInt", &callee, obj, 5));

    FreeFrame(frame);
}

static void VoidVReg(Method *method, int64_t value)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeVReg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidVReg));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(1).Set(5);

    uint8_t callShortInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x01, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callShortInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidVReg", &callee, 5));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x01, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidVReg", &callee, 5));

    g_gCallResult = "";
    int64_t arg[] = {5, 8};
    InvokeCompiledCodeWithArgArray(arg, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidVReg", &callee, 5));

    frameHandler.GetVReg(1).Set(0);
    frameHandler.GetAcc().SetValue(5);
    uint8_t callAccShort[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x01, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccShort, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidVReg", &callee, 5));
    FreeFrame(frame);
}

static void VoidIntVReg(Method *method, int32_t a0, int64_t value)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeIntVReg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidIntVReg));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(2);
    frameHandler.GetVReg(1).Set(5);

    uint8_t callShortInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x10, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callShortInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    g_gCallResult = "";
    int64_t arg[] = {2, 5, 8};
    InvokeCompiledCodeWithArgArray(arg, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    frameHandler.GetAcc().SetValue(5);
    frameHandler.GetVReg(1).Set(0);
    uint8_t callAccShortInsn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_SHORT_V4_IMM4_ID16), 0x10, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccShortInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidIntVReg", &callee, 2, 5));

    FreeFrame(frame);
}

// arm max number of register parameters
static void Void3Int(Method *method, int32_t a0, int32_t a1, int32_t a2)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke3Int)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void3Int));
    Frame *frame = CreateFrame(3, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetAcc().SetValue(0);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);

    uint8_t callInsn[] = {static_cast<uint8_t>(Opcode::CALL_V4_V4_V4_V4_ID16), 0x10, 0x02, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void3Int", &callee, 1, 2, 3));

    // callee(acc, v1, v2)
    uint8_t callAccInsn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_V4_V4_V4_IMM4_ID16), 0x21, 0x00, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void3Int", &callee, 0, 2, 3));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void3Int", &callee, 1, 2, 3));

    int64_t args[] = {1, 2, 3};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void3Int", &callee, 1, 2, 3));

    FreeFrame(frame);
}

static void Void2IntLongInt(Method *method, int32_t a0, int32_t a1, int64_t a2, int32_t a3)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke2IntLongInt)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I64, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void2IntLongInt));
    Frame *frame = CreateFrame(4, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);

    uint8_t callInsn[] = {static_cast<uint8_t>(Opcode::CALL_V4_V4_V4_V4_ID16), 0x10, 0x32, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    int64_t args[] = {1, 2, 3, 4};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    frameHandler.GetVReg(2).Set(0);
    frameHandler.GetAcc().SetValue(3);
    uint8_t callAccInsn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_V4_V4_V4_IMM4_ID16), 0x10, 0x23, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2IntLongInt", &callee, 1, 2, 3, 4));

    FreeFrame(frame);
}

static void VoidLong(Method *method, int64_t a0)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeLong)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidLong));
    Frame *frame = CreateFrame(1, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(9);

    uint8_t callInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidLong", &callee, 9));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidLong", &callee, 9));

    int64_t args[] = {9};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidLong", &callee, 9));

    FreeFrame(frame);
}

static void VoidDouble(Method *method, double a0)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, InvokeDouble)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(VoidDouble));
    Frame *frame = CreateFrame(1, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(4.0);

    uint8_t callInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidDouble", &callee, 4.0));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidDouble", &callee, 4.0));

    int64_t args[] = {bit_cast<int64_t>(4.0)};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "VoidDouble", &callee, 4.0));

    FreeFrame(frame);
}

static void Void4Int(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke4Int)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void4Int));
    Frame *frame = CreateFrame(4, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);

    uint8_t callInsn[] = {static_cast<uint8_t>(Opcode::CALL_V4_V4_V4_V4_ID16), 0x10, 0x32, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    int64_t args[] = {1, 2, 3, 4};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    frameHandler.GetVReg(3).Set(0);
    frameHandler.GetAcc().SetValue(4);
    uint8_t callAccInsn[] = {static_cast<uint8_t>(Opcode::CALL_ACC_V4_V4_V4_IMM4_ID16), 0x10, 0x32, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callAccInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void4Int", &callee, 1, 2, 3, 4));

    FreeFrame(frame);
}

static void Void2Long(Method *method, int64_t a0, int64_t a1)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke2Long)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I64, TypeId::I64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void2Long));
    Frame *frame = CreateFrame(2, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(3);
    frameHandler.GetVReg(1).Set(9);

    uint8_t callInsn[] = {static_cast<uint8_t>(Opcode::CALL_SHORT_V4_V4_ID16), 0x10, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2Long", &callee, 3, 9));

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2Long", &callee, 3, 9));

    int64_t args[] = {3, 9};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void2Long", &callee, 3, 9));

    FreeFrame(frame);
}

static void Void4IntDouble(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, double a4)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke4IntDouble)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void4IntDouble));
    Frame *frame = CreateFrame(5, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5.0);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void4IntDouble", &callee, 1, 2, 3, 4, 5.0));

    int64_t args[] = {1, 2, 3, 4, bit_cast<int64_t>(5.0)};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void4IntDouble", &callee, 1, 2, 3, 4, 5.0));

    FreeFrame(frame);
}

// aarch64 max number of register parameters
static void Void7Int(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke7Int)
{
    uint16_t *shorty = MakeShorty(
        {TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void7Int));
    Frame *frame = CreateFrame(7, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5);
    frameHandler.GetVReg(5).Set(6);
    frameHandler.GetVReg(6).Set(7);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void7Int", &callee, 1, 2, 3, 4, 5, 6, 7));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void7Int", &callee, 1, 2, 3, 4, 5, 6, 7));

    FreeFrame(frame);
}

static void Void7Int8Double(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                            int32_t a6, double d0, double d1, double d2, double d3, double d4, double d5, double d6,
                            double d7)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, d0, d1, d2, d3, d4, d5, d6, d7);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke7Int8Double)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64,
                                   TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void7Int8Double));
    Frame *frame = CreateFrame(15, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5);
    frameHandler.GetVReg(5).Set(6);
    frameHandler.GetVReg(6).Set(7);
    frameHandler.GetVReg(7).Set(8.0);
    frameHandler.GetVReg(8).Set(9.0);
    frameHandler.GetVReg(9).Set(10.0);
    frameHandler.GetVReg(10).Set(11.0);
    frameHandler.GetVReg(11).Set(12.0);
    frameHandler.GetVReg(12).Set(13.0);
    frameHandler.GetVReg(13).Set(14.0);
    frameHandler.GetVReg(14).Set(15.0);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void7Int8Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8.0, 9.0, 10.0, 11.0,
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
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void7Int8Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8.0, 9.0, 10.0, 11.0,
                                       12.0, 13.0, 14.0, 15.0));

    FreeFrame(frame);
}

static void Void8Int(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6,
                     int32_t a7)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, a7);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke8Int)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::I32});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void8Int));
    Frame *frame = CreateFrame(8, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5);
    frameHandler.GetVReg(5).Set(6);
    frameHandler.GetVReg(6).Set(7);
    frameHandler.GetVReg(7).Set(8);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void8Int", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7, 8};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void8Int", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    FreeFrame(frame);
}

static void Void6IntVReg(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                         int64_t value)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke6IntVReg)
{
    uint16_t *shorty = MakeShorty(
        {TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void6IntVReg));
    Frame *frame = CreateFrame(8, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5);
    frameHandler.GetVReg(5).Set(6);
    frameHandler.GetVReg(6).Set(7);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void6IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7, 8};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void6IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7));

    FreeFrame(frame);
}

static void Void7IntVReg(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                         int32_t a6, int64_t value)
{
    g_gCallResult = PrintFunc("void", __FUNCTION__, method, a0, a1, a2, a3, a4, a5, a6, value);
}

TEST_F(InterpreterToCompiledCodeBridgeTest, Invoke7IntVReg)
{
    uint16_t *shorty = MakeShorty({TypeId::VOID, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                   TypeId::I32, TypeId::I32, TypeId::TAGGED});
    Method callee(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), ACC_STATIC, 0, shorty);
    callee.SetCompiledEntryPoint(reinterpret_cast<const void *>(Void7IntVReg));
    Frame *frame = CreateFrame(8, nullptr, nullptr);
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5);
    frameHandler.GetVReg(5).Set(6);
    frameHandler.GetVReg(6).Set(7);
    frameHandler.GetVReg(7).Set(8);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void7IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    int64_t args[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void7IntVReg", &callee, 1, 2, 3, 4, 5, 6, 7, 8));

    FreeFrame(frame);
}

static void Void8Int9Double(Method *method, int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5,
                            int32_t a6, int32_t a7, double d0, double d1, double d2, double d3, double d4, double d5,
                            double d6, double d7, double d8)
{
    g_gCallResult =
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
    auto frameHandler = StaticFrameHandler(frame);
    frameHandler.GetVReg(0).Set(1);
    frameHandler.GetVReg(1).Set(2);
    frameHandler.GetVReg(2).Set(3);
    frameHandler.GetVReg(3).Set(4);
    frameHandler.GetVReg(4).Set(5);
    frameHandler.GetVReg(5).Set(6);
    frameHandler.GetVReg(6).Set(7);
    frameHandler.GetVReg(7).Set(8);
    frameHandler.GetVReg(8).Set(9.0);
    frameHandler.GetVReg(9).Set(10.0);
    frameHandler.GetVReg(10).Set(11.0);
    frameHandler.GetVReg(11).Set(12.0);
    frameHandler.GetVReg(12).Set(13.0);
    frameHandler.GetVReg(13).Set(14.0);
    frameHandler.GetVReg(14).Set(15.0);
    frameHandler.GetVReg(15).Set(16.0);
    frameHandler.GetVReg(16).Set(17.0);

    uint8_t callRangeInsn[] = {static_cast<uint8_t>(Opcode::CALL_RANGE_V8_ID16), 0x00, 0, 0, 0, 0};
    g_gCallResult = "";
    InterpreterToCompiledCodeBridge(callRangeInsn, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void8Int9Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8, 9.0, 10.0, 11.0,
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
    g_gCallResult = "";
    InvokeCompiledCodeWithArgArray(args, frame, &callee, thread_);
    ASSERT_EQ(g_gCallResult, PrintFunc("void", "Void8Int9Double", &callee, 1, 2, 3, 4, 5, 6, 7, 8, 9.0, 10.0, 11.0,
                                       12.0, 13.0, 14.0, 15.0, 16.0, 17.0));

    FreeFrame(frame);
}

}  // namespace panda::test

// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays)
