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
#include <cstring>
#include <string>

#include "runtime/include/runtime.h"
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
#include "runtime/tests/test_utils.h"

#include "runtime/mem/object_helpers.h"

namespace panda::mem {
class GCTestLog : public testing::TestWithParam<const char *> {
public:
    NO_MOVE_SEMANTIC(GCTestLog);
    NO_COPY_SEMANTIC(GCTestLog);

    GCTestLog() = default;

    ~GCTestLog() override
    {
        [[maybe_unused]] bool success = Runtime::Destroy();
        ASSERT(success);
        Logger::Destroy();
    }

    // NOLINTBEGIN(readability-magic-numbers)
    void SetupRuntime(const std::string &gc_type, bool small_heap_and_young_spaces = false,
                      size_t promotion_region_alive_rate = 100) const
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
        options.SetG1PromotionRegionAliveRate(promotion_region_alive_rate);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        if (small_heap_and_young_spaces) {
            options.SetYoungSpaceSize(2_MB);
            options.SetHeapSizeLimit(15_MB);
        }
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);
    }
    // NOLINTEND(readability-magic-numbers)

    size_t GetGCCounter(GC *gc)
    {
        return gc->gc_counter_;
    }

    void CounterLogTest()
    {
        SetupRuntime(GetParam());

        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        const unsigned iterations = 100;

        ASSERT(GetGCCounter(gc) == 0);

        for (size_t i = 1; i < iterations; i++) {
            testing::internal::CaptureStderr();
            task.Run(*gc);
            expected_log_ = '[' + std::to_string(GetGCCounter(gc)) + ']';
            log_ = testing::internal::GetCapturedStderr();
            ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n"
                                                                   << expected_log_ << "\nLog:\n"
                                                                   << log_;
            ASSERT(GetGCCounter(gc) == i);
            task.reason = static_cast<GCTaskCause>(i % number_of_gc_causes_ == 0 ? i % number_of_gc_causes_ + 1
                                                                                 : i % number_of_gc_causes_);
        }
    }

    void FullLogTest()
    {
        auto gc_type = GetParam();
        bool is_stw = strcmp(gc_type, "stw") == 0;

        SetupRuntime(gc_type);

        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);

        testing::internal::CaptureStderr();
        task.reason = GCTaskCause::YOUNG_GC_CAUSE;
        task.Run(*gc);
        expected_log_ = is_stw ? "[FULL (Young)]" : "[YOUNG (Young)]";
        log_ = testing::internal::GetCapturedStderr();
        ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n" << expected_log_ << "\nLog:\n" << log_;

        testing::internal::CaptureStderr();
        task.reason = GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
        task.Run(*gc);
        expected_log_ = is_stw ? "[FULL (Threshold)]" : "[TENURED (Threshold)]";
        log_ = testing::internal::GetCapturedStderr();
        ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n" << expected_log_ << "\nLog:\n" << log_;

        testing::internal::CaptureStderr();
        task.reason = GCTaskCause::OOM_CAUSE;
        task.Run(*gc);
        expected_log_ = "[FULL (OOM)]";
        log_ = testing::internal::GetCapturedStderr();
        ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n" << expected_log_ << "\nLog:\n" << log_;
    }

    // GCCollectionType order is important
    static_assert(GCCollectionType::NONE < GCCollectionType::YOUNG);
    static_assert(GCCollectionType::YOUNG < GCCollectionType::MIXED);
    static_assert(GCCollectionType::MIXED < GCCollectionType::TENURED);
    static_assert(GCCollectionType::TENURED < GCCollectionType::FULL);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string expected_log_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string log_;

private:
    const size_t number_of_gc_causes_ = 8;
};

TEST_P(GCTestLog, FullLogTest)
{
    FullLogTest();
}

TEST_F(GCTestLog, GenGCYoungCauseFullCollectionLogTest)
{
    SetupRuntime("gen-gc", true);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    {
        ScopedManagedCodeThread s(thread);
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        ObjectAllocator object_allocator;

        uint32_t garbage_rate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
        // NOLINTNEXTLINE(readability-magic-numbers)
        size_t string_len = garbage_rate * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        VMHandle<coretypes::Array> arrays[3];
        {
            arrays[0] =
                VMHandle<coretypes::Array>(thread, object_allocator.AllocArray(2, ClassRoot::ARRAY_STRING, false));
            arrays[0]->Set(0, object_allocator.AllocString(string_len));
        }
    }

    GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
    testing::internal::CaptureStderr();
    task.Run(*gc);
    expected_log_ = "[FULL (Young)]";
    log_ = testing::internal::GetCapturedStderr();
    ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n" << expected_log_ << "\nLog:\n" << log_;
}

TEST_F(GCTestLog, G1GCMixedCollectionLogTest)
{
    SetupRuntime("g1-gc");

    uint32_t garbage_rate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t big_string_len = garbage_rate * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t big_string_len1 = (garbage_rate + 1) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t big_string_len2 = (garbage_rate + 2) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    size_t small_len = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::String);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    ObjectAllocator object_allocator;

    testing::internal::CaptureStderr();

    VMHandle<coretypes::Array> holder;
    VMHandle<ObjectHeader> young;
    holder = VMHandle<coretypes::Array>(thread, object_allocator.AllocArray(4, ClassRoot::ARRAY_STRING, false));
    holder->Set(0, object_allocator.AllocString(big_string_len));
    holder->Set(1, object_allocator.AllocString(big_string_len1));
    holder->Set(2, object_allocator.AllocString(big_string_len2));
    holder->Set(3, object_allocator.AllocString(small_len));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    expected_log_ = "[YOUNG (Young)]";
    log_ = testing::internal::GetCapturedStderr();
    ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n" << expected_log_ << "\nLog:\n" << log_;
    testing::internal::CaptureStderr();

    VMHandle<ObjectHeader> current;
    current = VMHandle<ObjectHeader>(thread, object_allocator.AllocArray(small_len, ClassRoot::ARRAY_U8, false));

    holder->Set(0, static_cast<ObjectHeader *>(nullptr));
    holder->Set(1, static_cast<ObjectHeader *>(nullptr));
    holder->Set(2, static_cast<ObjectHeader *>(nullptr));
    holder->Set(3, static_cast<ObjectHeader *>(nullptr));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }

    young = VMHandle<ObjectHeader>(thread, object_allocator.AllocObjectInYoung());
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task2(GCTaskCause::YOUNG_GC_CAUSE);
        task2.Run(*gc);
    }
    expected_log_ = "[MIXED (Young)]";
    log_ = testing::internal::GetCapturedStderr();
    ASSERT_NE(log_.find(expected_log_), std::string::npos) << "Expected:\n" << expected_log_ << "\nLog:\n" << log_;
}

TEST_P(GCTestLog, CounterLogTest)
{
    CounterLogTest();
}

INSTANTIATE_TEST_SUITE_P(GCTestLogOnDiffGCs, GCTestLog, ::testing::ValuesIn(TESTED_GC));
}  // namespace panda::mem