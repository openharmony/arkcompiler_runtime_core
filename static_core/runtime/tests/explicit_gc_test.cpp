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

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/region_space.h"
#include "runtime/include/gc_task.h"
#include "runtime/mem/gc/g1/g1-gc.h"

#include "runtime/tests/test_utils.h"
#include "runtime/mem/object_helpers.h"

namespace panda::mem {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,-warnings-as-errors)
class ExplicitGC : public testing::Test {
public:
    ExplicitGC() = default;

    ~ExplicitGC() override
    {
        [[maybe_unused]] bool success = Runtime::Destroy();
        ASSERT(success);
        Logger::Destroy();
    }

    NO_COPY_SEMANTIC(ExplicitGC);
    NO_MOVE_SEMANTIC(ExplicitGC);

    void SetupRuntime(const std::string &gc_type, bool is_explicit_full) const
    {
        panda::Logger::ComponentMask component_mask;
        component_mask.set(Logger::Component::GC);

        Logger::InitializeStdLogging(Logger::Level::INFO, component_mask);
        EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::INFO, Logger::Component::GC));

        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType(gc_type);
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(is_explicit_full);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);
    }
};

TEST_F(ExplicitGC, TestG1GCPhases)
{
    SetupRuntime("g1-gc", true);

    uint32_t garbage_rate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
    // NOLINTNEXTLINE(readability-magic-numbers,-warnings-as-errors)
    size_t big_len = garbage_rate * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocator allocator;
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> holder;
    std::string expected_log;
    std::string log;

    holder = VMHandle<coretypes::Array>(thread, allocator.AllocArray(2, ClassRoot::ARRAY_STRING, false));
    holder->Set(0, allocator.AllocString(big_len));
    holder->Set(1, allocator.AllocString(big_len));

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run young
        expected_log = "[YOUNG (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expected_log), std::string::npos) << "Expected:\n" << expected_log << "\nLog:\n" << log;
    }

    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(1))->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(0))->HasFlag(RegionFlag::IS_OLD));

    holder->Set(0, static_cast<ObjectHeader *>(nullptr));
    holder->Set(1, static_cast<ObjectHeader *>(nullptr));

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task.Run(*gc);  // prepare for mix
        expected_log = "[TENURED (Threshold)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expected_log), std::string::npos) << "Expected:\n" << expected_log << "\nLog:\n" << log;
    }

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run mixed gc
        expected_log = "[MIXED (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expected_log), std::string::npos) << "Expected:\n" << expected_log << "\nLog:\n" << log;
    }
}

TEST_F(ExplicitGC, TestGenGCPhases)
{
    SetupRuntime("gen-gc", true);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    ObjectAllocator object_allocator;

    std::string expected_log;
    std::string log;

    VMHandle<ObjectHeader> obj;
    obj = VMHandle<ObjectHeader>(thread, object_allocator.AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run young
        expected_log = "[YOUNG (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expected_log), std::string::npos) << "Expected:\n" << expected_log << "\nLog:\n" << log;
    }
}

TEST_F(ExplicitGC, TestG1GCWithFullExplicit)
{
    SetupRuntime("g1-gc", false);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    ObjectAllocator object_allocator;

    std::string expected_log;
    std::string log;

    VMHandle<ObjectHeader> obj;
    obj = VMHandle<ObjectHeader>(thread, object_allocator.AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run full
        expected_log = "[FULL (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expected_log), std::string::npos) << "Expected:\n" << expected_log << "\nLog:\n" << log;
    }
}

TEST_F(ExplicitGC, TestGenGCWithFullExplicit)
{
    SetupRuntime("gen-gc", false);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    ObjectAllocator object_allocator;

    std::string expected_log;
    std::string log;

    VMHandle<ObjectHeader> obj;
    obj = VMHandle<ObjectHeader>(thread, object_allocator.AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run full
        expected_log = "[FULL (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expected_log), std::string::npos) << "Expected:\n" << expected_log << "\nLog:\n" << log;
    }
}

}  // namespace panda::mem