/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"

#include "runtime/include/runtime.h"
#include "runtime/regexp/ecmascript/mem/dyn_chunk.h"

namespace panda::mem::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda::coretypes;

class DynBufferTest : public testing::Test {
public:
    DynBufferTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~DynBufferTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(DynBufferTest);
    NO_MOVE_SEMANTIC(DynBufferTest);

private:
    panda::MTManagedThread *thread_ {};
};

TEST_F(DynBufferTest, EmitAndGet)
{
    DynChunk dyn_chunk = DynChunk();
    // NOLINTNEXTLINE(readability-magic-numbers)
    dyn_chunk.EmitChar(65);
    // NOLINTNEXTLINE(readability-magic-numbers)
    dyn_chunk.EmitU16(66);
    // NOLINTNEXTLINE(readability-magic-numbers)
    dyn_chunk.EmitU32(67);
    ASSERT_EQ(dyn_chunk.GetSize(), 7);
    ASSERT_EQ(dyn_chunk.GetAllocatedSize(), DynChunk::ALLOCATE_MIN_SIZE);
    ASSERT_EQ(dyn_chunk.GetError(), false);
    dyn_chunk.Insert(1, 1);
    uint32_t val1 = dyn_chunk.GetU8(0);
    uint32_t val2 = dyn_chunk.GetU16(2);
    uint32_t val3 = dyn_chunk.GetU32(4);
    ASSERT_EQ(val1, 65);
    ASSERT_EQ(val2, 66);
    ASSERT_EQ(val3, 67);
}

TEST_F(DynBufferTest, EmitSelfAndGet)
{
    DynChunk dyn_chunk = DynChunk();
    // NOLINTNEXTLINE(readability-magic-numbers)
    dyn_chunk.EmitChar(65);
    dyn_chunk.EmitSelf(0, 1);
    ASSERT_EQ(dyn_chunk.GetSize(), 2);
    ASSERT_EQ(dyn_chunk.GetAllocatedSize(), DynChunk::ALLOCATE_MIN_SIZE);
    ASSERT_EQ(dyn_chunk.GetError(), false);
    uint32_t val1 = dyn_chunk.GetU8(0);
    uint32_t val2 = dyn_chunk.GetU8(1);
    ASSERT_EQ(val1, 65);
    ASSERT_EQ(val2, 65);
}

TEST_F(DynBufferTest, EmitStrAndGet)
{
    DynChunk dyn_chunk = DynChunk();
    dyn_chunk.EmitStr("abc");
    ASSERT_EQ(dyn_chunk.GetSize(), 4);
    ASSERT_EQ(dyn_chunk.GetAllocatedSize(), DynChunk::ALLOCATE_MIN_SIZE);
    ASSERT_EQ(dyn_chunk.GetError(), false);
    uint32_t val1 = dyn_chunk.GetU8(0);
    uint32_t val2 = dyn_chunk.GetU8(1);
    uint32_t val3 = dyn_chunk.GetU8(2);
    uint32_t val4 = dyn_chunk.GetU8(3);
    ASSERT_EQ(val1, 97);
    ASSERT_EQ(val2, 98);
    ASSERT_EQ(val3, 99);
    ASSERT_EQ(val4, 0);
}
}  // namespace panda::mem::test
