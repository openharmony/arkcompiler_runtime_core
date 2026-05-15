/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <sstream>

#include "gtest/gtest.h"
#include "include/coretypes/class.h"
#include "include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/handle_scope-inl.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/mem.h"

namespace ark::test {

extern "C" void *AllocFrameStackOverflow(ManagedThread *current);

class StackOverflowOomTest : public testing::Test {
public:
    StackOverflowOomTest()
    {
        RuntimeOptions options;
        options.SetHeapSizeLimit(TEST_HEAP_LIMIT);
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");

        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);
        ark::Runtime::Create(options);
    }

    ~StackOverflowOomTest() override
    {
        Runtime::Destroy();
    }

    void SetUp() override
    {
        thread_ = MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        thread_->ManagedCodeEnd();
    }

protected:
    void FillHeapUntilOom()
    {
        ASSERT_NE(thread_, nullptr);
        ASSERT_FALSE(thread_->HasPendingException());
        ObjectHeader *obj;

        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread_);

        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();

        Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                           ->GetClass(ctx.GetOutOfMemoryErrorClassDescriptor());
        do {
            obj = ObjectHeader::Create(klass);
            heapObjects_.push_back(obj);
        } while (obj != nullptr);

        ASSERT_TRUE(thread_->HasPendingException());
        thread_->ClearException();
    }

    std::vector<ObjectHeader *> heapObjects_;  // NOLINT(misc-non-private-member-variables-in-classes)
    MTManagedThread *thread_ = nullptr;        // NOLINT(misc-non-private-member-variables-in-classes)
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t TEST_HEAP_LIMIT = 8U * 1024U * 1024U;

    NO_MOVE_SEMANTIC(StackOverflowOomTest);
    NO_COPY_SEMANTIC(StackOverflowOomTest);
};

TEST_F(StackOverflowOomTest, FrameAllocationOverflowOnFullHeapEndsWithOom)
{
    FillHeapUntilOom();
    ASSERT_FALSE(thread_->HasPendingException());

    void *frame = AllocFrameStackOverflow(thread_);
    ASSERT_EQ(frame, nullptr);
    ASSERT_TRUE(thread_->HasPendingException());
    ASSERT_TRUE(thread_->GetException()->IsInstanceOf(thread_->GetVM()->GetOOMErrorObject()->ClassAddr<Class>()));

    thread_->ClearException();
}

}  // namespace ark::test
