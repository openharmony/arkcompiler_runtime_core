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

#include <vector>

#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/method.h"
#include "runtime/tooling/debugger.h"
#include "runtime/tests/test_utils.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

namespace panda::debugger::test {

class DebuggerTest : public testing::Test {
public:
    DebuggerTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        // this test doesn't check GC logic - just to make test easier without any handles
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~DebuggerTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

protected:
    panda::MTManagedThread *thread_;
};

static ObjectHeader *ToPtr(uint64_t v)
{
    return reinterpret_cast<ObjectHeader *>(static_cast<object_pointer_type>(v));
}

static uint64_t FromPtr(ObjectHeader *ptr)
{
    return static_cast<object_pointer_type>(reinterpret_cast<uint64_t>(ptr));
}

template <bool is_dynamic = false>
static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
{
    uint32_t ext_sz = EmptyExtFrameDataSize;
    void *mem = aligned_alloc(8, Frame::GetAllocSize(Frame::GetActualSize<is_dynamic>(nregs), ext_sz));
    return new (Frame::FromExt(mem, ext_sz)) Frame(mem, method, prev, nregs);
}

static void FreeFrame(Frame *frame)
{
    std::free(frame->GetExt());
}

TEST_F(DebuggerTest, Frame)
{
    pandasm::Parser p;

    auto source = R"(
        .function void foo(i32 a0, i32 a1) {
            movi v0, 1
            movi v1, 2
            return.void
        }
    )";

    std::string src_filename = "src.pa";
    auto res = p.Parse(source, src_filename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    auto file_ptr = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(file_ptr != nullptr);

    PandaString descriptor;
    auto class_id = file_ptr->GetClassId(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*file_ptr, class_id);
    panda_file::File::EntityId method_id;
    panda_file::File::EntityId code_id;

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        method_id = mda.GetMethodId();
        ASSERT_TRUE(mda.GetCodeId());
        code_id = mda.GetCodeId().value();
    });

    panda_file::CodeDataAccessor code_data_accessor(*file_ptr, code_id);
    auto nargs = code_data_accessor.GetNumArgs();
    auto nregs = code_data_accessor.GetNumVregs();

    constexpr size_t BYTECODE_OFFSET = 0xeeff;

    Method method(nullptr, file_ptr.get(), method_id, code_id, 0, nargs, nullptr);
    panda::Frame *frame = test::CreateFrame(nregs + nargs, &method, nullptr);
    frame->SetBytecodeOffset(BYTECODE_OFFSET);

    struct VRegValue {
        uint64_t value {};
        bool is_ref {};
    };

    std::vector<VRegValue> regs {{0x1111111122222222, false},
                                 {FromPtr(panda::mem::AllocateNullifiedPayloadString(1)), true},
                                 {0x3333333344444444, false},
                                 {FromPtr(panda::mem::AllocateNullifiedPayloadString(1)), true}};

    auto frame_handler = StaticFrameHandler(frame);
    for (size_t i = 0; i < regs.size(); i++) {
        if (regs[i].is_ref) {
            frame_handler.GetVReg(i).SetReference(ToPtr(regs[i].value));
        } else {
            frame_handler.GetVReg(i).SetPrimitive(static_cast<int64_t>(regs[i].value));
        }
    }

    {
        VRegValue acc {0xaaaaaaaabbbbbbbb, false};
        frame->GetAccAsVReg().SetPrimitive(static_cast<int64_t>(acc.value));
        tooling::PtDebugFrame debug_frame(frame->GetMethod(), frame);

        EXPECT_EQ(debug_frame.GetVRegNum(), nregs);
        EXPECT_EQ(debug_frame.GetArgumentNum(), nargs);
        EXPECT_EQ(debug_frame.GetMethodId(), method_id);
        EXPECT_EQ(debug_frame.GetBytecodeOffset(), BYTECODE_OFFSET);
        EXPECT_EQ(debug_frame.GetAccumulator(), acc.value);

        for (size_t i = 0; i < debug_frame.GetVRegNum(); i++) {
            EXPECT_EQ(debug_frame.GetVReg(i), regs[i].value);
        }

        for (size_t i = 0; i < debug_frame.GetArgumentNum(); i++) {
            EXPECT_EQ(debug_frame.GetArgument(i), regs[i + nregs].value);
        }
    }

    {
        VRegValue acc {FromPtr(panda::mem::AllocateNullifiedPayloadString(1)), true};
        frame->GetAccAsVReg().SetReference(ToPtr(acc.value));
        tooling::PtDebugFrame debug_frame(frame->GetMethod(), frame);

        EXPECT_EQ(debug_frame.GetVRegNum(), nregs);
        EXPECT_EQ(debug_frame.GetArgumentNum(), nargs);
        EXPECT_EQ(debug_frame.GetMethodId(), method_id);
        EXPECT_EQ(debug_frame.GetBytecodeOffset(), BYTECODE_OFFSET);
        EXPECT_EQ(debug_frame.GetAccumulator(), acc.value);

        for (size_t i = 0; i < debug_frame.GetVRegNum(); i++) {
            EXPECT_EQ(debug_frame.GetVReg(i), regs[i].value);
        }

        for (size_t i = 0; i < debug_frame.GetArgumentNum(); i++) {
            EXPECT_EQ(debug_frame.GetArgument(i), regs[i + nregs].value);
        }
    }

    test::FreeFrame(frame);
}

}  // namespace panda::debugger::test
