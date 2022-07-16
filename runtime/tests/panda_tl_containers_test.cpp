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

#include "gtest/gtest.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/runslots_allocator-inl.h"

namespace panda::mem::test {

class PandaContainersTest : public testing::Test {
public:
    PandaContainersTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetLimitStandardAlloc(true);
        options.SetInternalAllocatorType("panda_allocators");
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    template <class T>
    size_t GetLocalObjects(T *allocator)
    {
        size_t num_objects = 0;
        allocator->IterateOverObjects([&num_objects]([[maybe_unused]] ObjectHeader *obj) { num_objects++; });
        return num_objects;
    }

    ~PandaContainersTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

protected:
    panda::MTManagedThread *thread_;
};

constexpr size_t MAX_SIZE = InternalAllocator<>::LocalSmallObjectAllocator::GetMaxSize();
TEST_F(PandaContainersTest, LocalTest)
{
    ASSERT(!Runtime::GetCurrent()->GetOptions().UseMallocForInternalAllocations());
    auto local_allocator = panda::ManagedThread::GetCurrent()->GetLocalInternalAllocator();
    ASSERT_EQ(GetLocalObjects(local_allocator), 0);
    {
        PandaVectorTL<uint8_t> vector_local;
        vector_local.push_back(0);
        // std::vector allocated memory block for it's elements via local_allocator
        ASSERT_EQ(GetLocalObjects(local_allocator), 1);
        for (size_t i = 1; i < MAX_SIZE; i++) {
            vector_local.push_back(i % MAX_SIZE);
            if (vector_local.capacity() <= MAX_SIZE) {
                // Threre is 1 memory block for all elements
                ASSERT_EQ(GetLocalObjects(local_allocator), 1);
            } else {
                // When vector size exceeds MAX_SIZE=256 bytes, we allocate it without thread-local runslots allocator
                ASSERT_EQ(GetLocalObjects(local_allocator), 0);
            }
        }
        // Simple check on data consistency
        for (size_t i = 0; i < MAX_SIZE; i++) {
            ASSERT_EQ(vector_local.at(i), i % MAX_SIZE);
        }
        vector_local.push_back(0);
        // When vector size exceeds MAX_SIZE=256 bytes, we allocate it without thread-local runslots allocator
        ASSERT_EQ(GetLocalObjects(local_allocator), 0);
    }
    ASSERT_EQ(GetLocalObjects(local_allocator), 0);
}

// Check that, when we use GLOBAL scope, there is no memory in thread-local allocator
TEST_F(PandaContainersTest, GlobalTest)
{
    auto local_allocator = panda::ManagedThread::GetCurrent()->GetLocalInternalAllocator();
    ASSERT_EQ(GetLocalObjects(local_allocator), 0);
    {
        PandaVector<uint8_t> vector_global;
        vector_global.push_back(0);
        ASSERT_EQ(GetLocalObjects(local_allocator), 0);
        for (size_t i = 1; i < MAX_SIZE; i++) {
            vector_global.push_back(i % MAX_SIZE);
            ASSERT_EQ(GetLocalObjects(local_allocator), 0);
        }

        for (size_t i = 0; i < MAX_SIZE; i++) {
            ASSERT_EQ(vector_global.at(i), i % MAX_SIZE);
        }
        vector_global.push_back(0);
        ASSERT_EQ(GetLocalObjects(local_allocator), 0);
    }
    ASSERT_EQ(GetLocalObjects(local_allocator), 0);
}

}  // namespace panda::mem::test
