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

#include <cstdint>

#include "runtime/compiler.h"
#include "runtime/interpreter/frame.h"
#include "runtime/tests/test_utils.h"

namespace panda::test {

class FrameTest : public testing::Test {
public:
    FrameTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~FrameTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

protected:
    panda::MTManagedThread *thread_;
};

template <bool is_dynamic = false>
Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
{
    uint32_t ext_sz = EmptyExtFrameDataSize;
    void *mem = aligned_alloc(8, panda::Frame::GetAllocSize(Frame::GetActualSize<is_dynamic>(nregs), ext_sz));
    return new (Frame::FromExt(mem, ext_sz)) panda::Frame(mem, method, prev, nregs);
}

void FreeFrame(Frame *f)
{
    std::free(f->GetExt());
}

TEST_F(FrameTest, Test)
{
    Frame *f = panda::test::CreateFrame(2, nullptr, nullptr);
    auto frame_handler = StaticFrameHandler(f);
    frame_handler.GetVReg(0).SetReference(nullptr);
    EXPECT_TRUE(frame_handler.GetVReg(0).HasObject());
    frame_handler.GetVReg(0).SetPrimitive(0);
    EXPECT_FALSE(frame_handler.GetVReg(0).HasObject());

    int64_t v64 = 0x1122334455667788;
    frame_handler.GetVReg(0).SetPrimitive(v64);
    EXPECT_EQ(frame_handler.GetVReg(0).GetLong(), v64);
    EXPECT_EQ(frame_handler.GetVReg(0).GetAs<int64_t>(), v64);

    frame_handler.GetVReg(1).MovePrimitive(frame_handler.GetVReg(0));
    EXPECT_FALSE(frame_handler.GetVReg(0).HasObject());
    EXPECT_EQ(frame_handler.GetVReg(0).Get(), static_cast<int32_t>(v64));

    frame_handler.GetVReg(1).MovePrimitive(frame_handler.GetVReg(0));
    EXPECT_FALSE(frame_handler.GetVReg(0).HasObject());
    EXPECT_EQ(frame_handler.GetVReg(0).GetLong(), v64);

    ObjectHeader *ptr = panda::mem::AllocateNullifiedPayloadString(15);
    frame_handler.GetVReg(0).SetReference(ptr);
    frame_handler.GetVReg(1).MoveReference(frame_handler.GetVReg(0));
    EXPECT_TRUE(frame_handler.GetVReg(0).HasObject());
    EXPECT_EQ(frame_handler.GetVReg(0).GetReference(), ptr);

    int32_t v32 = 0x11223344;
    frame_handler.GetVReg(0).SetPrimitive(v32);
    EXPECT_EQ(frame_handler.GetVReg(0).Get(), v32);
    EXPECT_EQ(frame_handler.GetVReg(0).GetAs<int32_t>(), v32);

    int16_t v16 = 0x1122;
    frame_handler.GetVReg(0).SetPrimitive(v16);
    EXPECT_EQ(frame_handler.GetVReg(0).Get(), v16);
    EXPECT_EQ(frame_handler.GetVReg(0).GetAs<int32_t>(), v16);

    int8_t v8 = 0x11;
    frame_handler.GetVReg(0).SetPrimitive(v8);
    EXPECT_EQ(frame_handler.GetVReg(0).Get(), v8);
    EXPECT_EQ(frame_handler.GetVReg(0).GetAs<int32_t>(), v8);

    float f32 = 123.5;
    frame_handler.GetVReg(0).SetPrimitive(f32);
    EXPECT_EQ(frame_handler.GetVReg(0).GetFloat(), f32);
    EXPECT_EQ(frame_handler.GetVReg(0).GetAs<float>(), f32);

    double f64 = 456.7;
    frame_handler.GetVReg(0).SetPrimitive(f64);
    EXPECT_EQ(frame_handler.GetVReg(0).GetDouble(), f64);
    EXPECT_EQ(frame_handler.GetVReg(0).GetAs<double>(), f64);

    panda::test::FreeFrame(f);
}

}  // namespace panda::test
