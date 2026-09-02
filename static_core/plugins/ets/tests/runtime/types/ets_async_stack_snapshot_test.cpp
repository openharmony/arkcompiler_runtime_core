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

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <utility>
#include <vector>

#include "ets_coroutine.h"
#include "plugins/ets/runtime/dfx/ets_async_stack_snapshot_handle.h"
#include "plugins/ets/runtime/dfx/static_async_stack_snapshot_manager.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_promise_async_stack_snapshot_queue.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"
#include "runtime/execution/job.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/include/runtime.h"
#include "runtime/include/tooling/debug_interface.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "runtime/execution/job_worker_group.h"

namespace ark::ets::test {

class EtsAsyncStackSnapshotTest : public testing::Test {
public:
    EtsAsyncStackSnapshotTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto *stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        vm_ = EtsCoroutine::GetCurrent()->GetPandaVM();
        Runtime::GetCurrent()->SetDebugMode(true);
        vm_->SetAsyncDebuggerMaxAsyncDepth(4U);
    }

    ~EtsAsyncStackSnapshotTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsAsyncStackSnapshotTest);
    NO_MOVE_SEMANTIC(EtsAsyncStackSnapshotTest);

    static std::vector<MirrorFieldInfo> GetSourceIdentityMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsAsyncStackSourceIdentity, pandaFile_, "pandaFile"),
            MIRROR_FIELD_INFO(EtsAsyncStackSourceIdentity, methodId_, "methodId"),
            MIRROR_FIELD_INFO(EtsAsyncStackSourceIdentity, bytecodeOffset_, "bytecodeOffset"),
            MIRROR_FIELD_INFO(EtsAsyncStackSourceIdentity, nativePc_, "nativePc"),
        };
    }

    static std::vector<MirrorFieldInfo> GetFrameMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsAsyncStackFrame, functionName_, "functionName"),
            MIRROR_FIELD_INFO(EtsAsyncStackFrame, sourceIdentity_, "sourceIdentity"),
        };
    }

    static std::vector<MirrorFieldInfo> GetSegmentMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsAsyncStackSegment, description_, "description"),
            MIRROR_FIELD_INFO(EtsAsyncStackSegment, generation_, "generation"),
            MIRROR_FIELD_INFO(EtsAsyncStackSegment, frames_, "frames"),
        };
    }

    static std::vector<MirrorFieldInfo> GetSnapshotMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsAsyncStackSnapshot, generation_, "generation"),
            MIRROR_FIELD_INFO(EtsAsyncStackSnapshot, segments_, "segments"),
        };
    }

protected:
    EtsExecutionContext *GetExecutionContext() const
    {
        return EtsExecutionContext::GetCurrent();
    }

    EtsAsyncStackSnapshot *CreateSnapshotWithSegments(size_t segmentCount, EtsLong generation)
    {
        auto *executionCtx = GetExecutionContext();
        EtsHandleScope scope(executionCtx);
        auto *segments = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSegment, segmentCount);
        EXPECT_NE(segments, nullptr);
        EtsHandle<EtsObjectArray> segmentsHandle(executionCtx, segments);
        for (size_t idx = 0; idx < segmentCount; ++idx) {
            auto *segment = EtsAsyncStackSegment::Create(executionCtx);
            EXPECT_NE(segment, nullptr);
            auto *frames = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackFrame, 0U);
            EXPECT_NE(frames, nullptr);
            EtsHandle<EtsObjectArray> framesHandle(executionCtx, frames);
            segment->SetDescription(executionCtx, EtsString::CreateFromMUtf8("promise.then"));
            segment->SetGeneration(generation);
            segment->SetFrames(executionCtx, framesHandle.GetPtr());
            segmentsHandle->Set(static_cast<uint32_t>(idx), segment);
        }

        auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
        EXPECT_NE(snapshot, nullptr);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
        snapshotHandle->SetGeneration(generation);
        snapshotHandle->SetSegments(executionCtx, segmentsHandle.GetPtr());
        return snapshotHandle.GetPtr();
    }

    EtsAsyncStackSnapshot *CreateSnapshotForNativeView()
    {
        auto *executionCtx = GetExecutionContext();
        EtsHandleScope scope(executionCtx);

        auto *sourceIdentity = EtsAsyncStackSourceIdentity::Create(executionCtx);
        EXPECT_NE(sourceIdentity, nullptr);
        EtsHandle<EtsAsyncStackSourceIdentity> sourceIdentityHandle(executionCtx, sourceIdentity);
        constexpr EtsLong sourceMethodId = 0x100000001LL;  // NOLINT(readability-identifier-naming)
        constexpr uint32_t sourceBytecodeOffset = 12U;     // NOLINT(readability-identifier-naming)
        constexpr uint32_t sourceNativePc = 34U;           // NOLINT(readability-identifier-naming)
        sourceIdentityHandle->SetMethodId(sourceMethodId);
        sourceIdentityHandle->SetBytecodeOffset(sourceBytecodeOffset);
        sourceIdentityHandle->SetNativePc(sourceNativePc);
        sourceIdentityHandle->SetPandaFile(executionCtx, EtsString::CreateFromMUtf8("modules/main.abc"));

        auto *frame = EtsAsyncStackFrame::Create(executionCtx);
        EXPECT_NE(frame, nullptr);
        EtsHandle<EtsAsyncStackFrame> frameHandle(executionCtx, frame);
        frameHandle->SetFunctionName(executionCtx, EtsString::CreateFromMUtf8("Main.run"));
        frameHandle->SetSourceIdentity(executionCtx, sourceIdentityHandle.GetPtr());

        auto *frames = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackFrame, 1U);
        EXPECT_NE(frames, nullptr);
        EtsHandle<EtsObjectArray> framesHandle(executionCtx, frames);
        framesHandle->Set(0U, frameHandle.GetPtr());

        auto *segment = EtsAsyncStackSegment::Create(executionCtx);
        EXPECT_NE(segment, nullptr);
        EtsHandle<EtsAsyncStackSegment> segmentHandle(executionCtx, segment);
        segmentHandle->SetDescription(executionCtx, EtsString::CreateFromMUtf8("promise.then"));
        segmentHandle->SetGeneration(7U);
        segmentHandle->SetFrames(executionCtx, framesHandle.GetPtr());

        auto *segments = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSegment, 1U);
        EXPECT_NE(segments, nullptr);
        EtsHandle<EtsObjectArray> segmentsHandle(executionCtx, segments);
        segmentsHandle->Set(0U, segmentHandle.GetPtr());

        auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
        EXPECT_NE(snapshot, nullptr);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
        snapshotHandle->SetGeneration(7U);
        snapshotHandle->SetSegments(executionCtx, segmentsHandle.GetPtr());
        return snapshotHandle.GetPtr();
    }

    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class EtsAsyncStackSnapshotMovingGcTest : public testing::Test {
public:
    EtsAsyncStackSnapshotMovingGcTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("g1-gc");
        options.SetLoadRuntimes({"ets"});
        options.SetUseTlabForAllocations(false);
        options.SetGcTriggerType("debug");
        options.SetGcDebugTriggerStart(std::numeric_limits<int>::max());
        options.SetExplicitConcurrentGcEnabled(false);

        auto *stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        vm_ = EtsCoroutine::GetCurrent()->GetPandaVM();
    }

    ~EtsAsyncStackSnapshotMovingGcTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsAsyncStackSnapshotMovingGcTest);
    NO_MOVE_SEMANTIC(EtsAsyncStackSnapshotMovingGcTest);

protected:
    EtsExecutionContext *GetExecutionContext() const
    {
        return EtsExecutionContext::GetCurrent();
    }

    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class EtsAsyncStackSnapshotHandleFailureTest : public testing::Test {
public:
    static constexpr uint32_t maxGlobalRefSize = 3000U;  // NOLINT(readability-identifier-naming)

    EtsAsyncStackSnapshotHandleFailureTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        options.SetMaxGlobalRefSize(maxGlobalRefSize);

        auto *stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        vm_ = EtsCoroutine::GetCurrent()->GetPandaVM();
    }

    ~EtsAsyncStackSnapshotHandleFailureTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsAsyncStackSnapshotHandleFailureTest);
    NO_MOVE_SEMANTIC(EtsAsyncStackSnapshotHandleFailureTest);

protected:
    static AsyncStackSnapshotHandlePtr CreateHandleWithAllocationFailureForTest(PandaEtsVM *vm,
                                                                                EtsAsyncStackSnapshot *snapshot)
    {
        return EtsAsyncStackSnapshotHandle::CreateWithAllocationFailureForTest(vm, snapshot);
    }

    EtsExecutionContext *GetExecutionContext() const
    {
        return EtsExecutionContext::GetCurrent();
    }

    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(EtsAsyncStackSnapshotTest, ManagedSnapshotMemoryLayout)
{
    auto *executionCtx = GetExecutionContext();
    MirrorFieldInfo::CompareMemberOffsets(PlatformTypes(executionCtx)->coreAsyncStackSourceIdentity,
                                          GetSourceIdentityMembers());
    MirrorFieldInfo::CompareMemberOffsets(PlatformTypes(executionCtx)->coreAsyncStackFrame, GetFrameMembers());
    MirrorFieldInfo::CompareMemberOffsets(PlatformTypes(executionCtx)->coreAsyncStackSegment, GetSegmentMembers());
    MirrorFieldInfo::CompareMemberOffsets(PlatformTypes(executionCtx)->coreAsyncStackSnapshot, GetSnapshotMembers());
}

TEST_F(EtsAsyncStackSnapshotTest, SourceIdentityPreservesMethodIdWidth)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *sourceIdentity = EtsAsyncStackSourceIdentity::Create(executionCtx);
    ASSERT_NE(sourceIdentity, nullptr);

    constexpr EtsLong HIGH_METHOD_ID = 0x100000000LL;
    sourceIdentity->SetMethodId(HIGH_METHOD_ID);
    ASSERT_EQ(sourceIdentity->GetMethodId(), HIGH_METHOD_ID);
}

TEST_F(EtsAsyncStackSnapshotTest, HandleCloneRetainsSameManagedSnapshot)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *snapshot = CreateSnapshotWithSegments(1U, 1U);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);

    auto handle = EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr());
    ASSERT_NE(handle, nullptr);
    auto clone = handle->Clone();
    ASSERT_NE(clone, nullptr);
    ASSERT_EQ(static_cast<EtsAsyncStackSnapshot *>(clone->GetOpaqueSnapshot()), snapshotHandle.GetPtr());
}

TEST_F(EtsAsyncStackSnapshotTest, HandleCreatesNativeSnapshotView)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);

    auto *snapshot = CreateSnapshotForNativeView();
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);

    auto handle = EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr());
    ASSERT_NE(handle, nullptr);
    auto view = handle->CreateSnapshotView();
    ASSERT_NE(view, nullptr);
    ASSERT_EQ(view->generation, 7U);
    ASSERT_EQ(view->segments.size(), 1U);
    ASSERT_EQ(view->segments[0].description, "promise.then");
    ASSERT_EQ(view->segments[0].generation, 7U);
    ASSERT_EQ(view->segments[0].frames.size(), 1U);
    ASSERT_EQ(view->segments[0].frames[0].functionName, "Main.run");
    ASSERT_EQ(view->segments[0].frames[0].pandaFile, "modules/main.abc");
    ASSERT_EQ(view->segments[0].frames[0].methodId, 0x100000001ULL);
    ASSERT_EQ(view->segments[0].frames[0].bytecodeOffset, 12U);
    ASSERT_EQ(view->segments[0].frames[0].nativePc, 34U);
}

TEST_F(EtsAsyncStackSnapshotTest, LaunchFailureReleasesAsyncStackHandle)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);

    auto *storage = vm_->GetGlobalObjectStorage();
    const auto sizeBeforeHandle = storage->GetSize();
    auto handle = EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr());
    ASSERT_NE(handle, nullptr);
    ASSERT_EQ(storage->GetSize(), sizeBeforeHandle + 1U);

    auto *jobManager = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    ASSERT_NE(jobManager, nullptr);
    constexpr uint32_t exactWorkerId = 99U;  // NOLINT(readability-identifier-naming)
    auto groupId = JobWorkerThreadGroup::FromDomain(jobManager, JobWorkerThreadDomain::EXACT_ID, {exactWorkerId});
    auto callback = []([[maybe_unused]] void *data) {};
    auto *job = jobManager->CreateJob(PandaString {"async-stack-launch-failure"},
                                      Job::NativeEntrypointInfo {callback, nullptr});
    ASSERT_NE(job, nullptr);

    {
        LaunchParams launchParams {job->GetPriority(), groupId};
        launchParams.asyncDebuggerStack = std::move(handle);
        const auto launchResult = jobManager->Launch(job, launchParams);
        ASSERT_EQ(launchResult, LaunchResult::NO_SUITABLE_WORKER);
        jobManager->HandleLaunchResultManaged(launchResult);
        jobManager->DestroyJob(job);
    }
    executionCtx->GetMT()->ClearException();

    ASSERT_EQ(storage->GetSize(), sizeBeforeHandle);
}

TEST_F(EtsAsyncStackSnapshotHandleFailureTest, NativeAllocationFailureReleasesInsertedRoot)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);

    auto *storage = vm_->GetGlobalObjectStorage();
    const auto sizeBefore = storage->GetSize();
    auto handle = CreateHandleWithAllocationFailureForTest(vm_, snapshotHandle.GetPtr());

    ASSERT_EQ(handle, nullptr);
    ASSERT_EQ(storage->GetSize(), sizeBefore);
}

TEST_F(EtsAsyncStackSnapshotHandleFailureTest, GlobalRootInsertionFailureReturnsNull)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);

    auto *storage = vm_->GetGlobalObjectStorage();
    std::vector<ark::mem::Reference *> references;
    while (true) {
        auto *reference = storage->Add(snapshotHandle->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
        if (reference == nullptr) {
            break;
        }
        references.push_back(reference);
    }
    const auto fullSize = storage->GetSize();

    auto handle = EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr());
    ASSERT_EQ(handle, nullptr);
    ASSERT_EQ(storage->GetSize(), fullSize);

    for (auto *reference : references) {
        storage->Remove(reference);
    }
}

TEST_F(EtsAsyncStackSnapshotTest, CapturePhysicallyBoundsParentChain)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    vm_->SetAsyncDebuggerMaxAsyncDepth(3U);
    auto config = vm_->GetAsyncDebuggerConfig();
    ASSERT_TRUE(config.IsCaptureEnabled());

    auto job = ark::Job {ark::PandaString {},     1U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                         ark::Job::Type::MUTATOR, false};
    auto *previousJob = ark::Job::GetCurrent();
    ark::Job::SetCurrent(&job);

    EtsHandleScope scope(executionCtx);
    auto *parent = CreateSnapshotWithSegments(5U, static_cast<EtsLong>(config.generation));
    ASSERT_NE(parent, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> parentHandle(executionCtx, parent);
    job.SetAsyncDebuggerStack(EtsAsyncStackSnapshotHandle::Create(vm_, parentHandle.GetPtr()));

    auto *captured = StaticAsyncStackSnapshotManager::Capture(executionCtx, "promise.then", false);
    ark::Job::SetCurrent(previousJob);
    ASSERT_NE(captured, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> capturedHandle(executionCtx, captured);
    ASSERT_EQ(capturedHandle->GetGeneration(), static_cast<EtsLong>(config.generation));
    ASSERT_EQ(capturedHandle->GetSegments(executionCtx)->GetLength(), 3U);
}

TEST_F(EtsAsyncStackSnapshotTest, CaptureRequiresRuntimeDebugMode)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    vm_->SetAsyncDebuggerMaxAsyncDepth(4U);
    auto config = vm_->GetAsyncDebuggerConfig();
    ASSERT_TRUE(config.IsCaptureEnabled());

    auto job = ark::Job {ark::PandaString {},     1U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                         ark::Job::Type::MUTATOR, false};
    auto *previousJob = ark::Job::GetCurrent();
    ark::Job::SetCurrent(&job);

    auto *runtime = Runtime::GetCurrent();
    runtime->SetDebugMode(false);
    ASSERT_EQ(StaticAsyncStackSnapshotManager::Capture(executionCtx, "promise.then", false), nullptr);

    runtime->SetDebugMode(true);
    EtsHandleScope scope(executionCtx);
    auto *captured = StaticAsyncStackSnapshotManager::Capture(executionCtx, "promise.then", false);
    ark::Job::SetCurrent(previousJob);
    ASSERT_NE(captured, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> capturedHandle(executionCtx, captured);
    ASSERT_EQ(capturedHandle->GetGeneration(), static_cast<EtsLong>(config.generation));
}

TEST_F(EtsAsyncStackSnapshotTest, DebuggerAsyncStackRequiresRuntimeDebugMode)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    vm_->SetAsyncDebuggerMaxAsyncDepth(8U);

    auto *runtime = Runtime::GetCurrent();
    runtime->SetDebugMode(true);
    auto debugSession = runtime->StartDebugSession();
    auto &debugger = debugSession->GetDebugger();

    auto job = ark::Job {ark::PandaString {},     2U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                         ark::Job::Type::MUTATOR, false};
    auto *previousJob = ark::Job::GetCurrent();
    ark::Job::SetCurrent(&job);

    EtsHandleScope scope(executionCtx);
    auto *snapshot = CreateSnapshotWithSegments(1U, vm_->GetAsyncDebuggerConfig().generation);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    job.SetAsyncDebuggerStack(EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr()));
    ASSERT_NE(job.GetAsyncDebuggerStack(), nullptr);

    runtime->SetDebugMode(false);
    ASSERT_TRUE(debugger.SetAsyncCallStackDepth(4U).has_value());
    ASSERT_EQ(debugger.CreateCurrentAsyncStackSnapshotView(), nullptr);

    runtime->SetDebugMode(true);
    ASSERT_FALSE(debugger.SetAsyncCallStackDepth(4U).has_value());
    ASSERT_NE(debugger.CreateCurrentAsyncStackSnapshotView(), nullptr);
    ark::Job::SetCurrent(previousJob);
}

TEST_F(EtsAsyncStackSnapshotTest, CurrentViewRespectsReducedMaxAsyncDepth)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    vm_->SetAsyncDebuggerMaxAsyncDepth(8U);

    auto *runtime = Runtime::GetCurrent();
    runtime->SetDebugMode(true);
    auto debugSession = runtime->StartDebugSession();
    auto &debugger = debugSession->GetDebugger();

    auto job = ark::Job {ark::PandaString {},     2U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                         ark::Job::Type::MUTATOR, false};
    auto *previousJob = ark::Job::GetCurrent();
    ark::Job::SetCurrent(&job);

    EtsHandleScope scope(executionCtx);
    auto *snapshot = CreateSnapshotWithSegments(8U, 1U);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    job.SetAsyncDebuggerStack(EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr()));
    ASSERT_NE(job.GetAsyncDebuggerStack(), nullptr);

    vm_->SetAsyncDebuggerMaxAsyncDepth(2U);
    auto view = debugger.CreateCurrentAsyncStackSnapshotView();
    ark::Job::SetCurrent(previousJob);

    ASSERT_NE(view, nullptr);
    ASSERT_EQ(view->segments.size(), 2U);
}

TEST_F(EtsAsyncStackSnapshotTest, CurrentViewFiltersStaleGeneration)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    vm_->SetAsyncDebuggerMaxAsyncDepth(8U);

    auto *runtime = Runtime::GetCurrent();
    runtime->SetDebugMode(true);
    auto debugSession = runtime->StartDebugSession();
    auto &debugger = debugSession->GetDebugger();

    auto job = ark::Job {ark::PandaString {},     2U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                         ark::Job::Type::MUTATOR, false};
    auto *previousJob = ark::Job::GetCurrent();
    ark::Job::SetCurrent(&job);

    EtsHandleScope scope(executionCtx);
    auto *snapshot = CreateSnapshotWithSegments(1U, 1U);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    job.SetAsyncDebuggerStack(EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr()));
    ASSERT_NE(job.GetAsyncDebuggerStack(), nullptr);

    vm_->SetAsyncDebuggerEnabled(false);
    vm_->SetAsyncDebuggerEnabled(true);
    auto view = debugger.CreateCurrentAsyncStackSnapshotView();
    ark::Job::SetCurrent(previousJob);

    ASSERT_EQ(view, nullptr);
}

TEST_F(EtsAsyncStackSnapshotTest, GenerationMismatchStartsNewChain)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    auto job = ark::Job {ark::PandaString {},     2U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                         ark::Job::Type::MUTATOR, false};
    auto *previousJob = ark::Job::GetCurrent();
    ark::Job::SetCurrent(&job);

    EtsHandleScope scope(executionCtx);
    constexpr EtsLong PARENT_GENERATION = 999U;
    auto *parent = CreateSnapshotWithSegments(4U, PARENT_GENERATION);
    ASSERT_NE(parent, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> parentHandle(executionCtx, parent);
    job.SetAsyncDebuggerStack(EtsAsyncStackSnapshotHandle::Create(vm_, parentHandle.GetPtr()));

    auto *captured = StaticAsyncStackSnapshotManager::Capture(executionCtx, "promise.then", false);
    ark::Job::SetCurrent(previousJob);
    ASSERT_NE(captured, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> capturedHandle(executionCtx, captured);
    ASSERT_EQ(capturedHandle->GetSegments(executionCtx)->GetLength(), 1U);
}

TEST_F(EtsAsyncStackSnapshotTest, PendingPromiseQueueKeepsSnapshotReachable)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *promise = EtsPromise::Create(executionCtx);
    ASSERT_NE(promise, nullptr);
    EtsHandle<EtsPromise> promiseHandle(executionCtx, promise);
    auto *callback = EtsString::CreateFromMUtf8("callback");
    ASSERT_NE(callback, nullptr);
    EtsHandle<EtsString> callbackHandle(executionCtx, callback);
    auto *snapshot = CreateSnapshotWithSegments(1U, 1U);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);

    promiseHandle->SetCallbackQueue(executionCtx, EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, 1U));
    promiseHandle->SetWorkerDomainQueue(executionCtx, EtsIntArray::Create(1U));

    {
        EtsMutex::LockHolder lock(promiseHandle);
        promiseHandle->SubmitCallback(executionCtx, callbackHandle->AsObject(), JobWorkerThreadDomain::MAIN);
        EtsPromiseAsyncStackSnapshotQueue::Append(executionCtx, promiseHandle, snapshotHandle, 0U, 1U);
    }

    ASSERT_EQ(promiseHandle->GetQueueSize(), 1);
    auto handle = EtsPromiseAsyncStackSnapshotQueue::CreateHandleAt(executionCtx, promiseHandle, 0U);
    ASSERT_NE(handle, nullptr);
    ASSERT_EQ(static_cast<EtsAsyncStackSnapshot *>(handle->GetOpaqueSnapshot()), snapshotHandle.GetPtr());
}

TEST_F(EtsAsyncStackSnapshotMovingGcTest, HandleResolvesMovedSnapshotThroughGlobalRoot)
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *executionCtx = GetExecutionContext();
    EtsHandleScope scope(executionCtx);
    auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
    ASSERT_NE(snapshot, nullptr);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    snapshotHandle->SetGeneration(1U);

    auto handle = EtsAsyncStackSnapshotHandle::Create(vm_, snapshotHandle.GetPtr());
    ASSERT_NE(handle, nullptr);
    auto *snapshotBeforeMove = static_cast<EtsAsyncStackSnapshot *>(handle->GetOpaqueSnapshot());
    while (static_cast<EtsAsyncStackSnapshot *>(handle->GetOpaqueSnapshot()) == snapshotBeforeMove) {
        constexpr uint32_t garbageArrayLength = 1024U;  // NOLINT(readability-identifier-naming)
        auto *garbage = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, garbageArrayLength);
        ASSERT_NE(garbage, nullptr);
    }

    auto *snapshotAfterMove = static_cast<EtsAsyncStackSnapshot *>(handle->GetOpaqueSnapshot());
    ASSERT_NE(snapshotAfterMove, snapshotBeforeMove);
    auto clone = handle->Clone();
    ASSERT_NE(clone, nullptr);
    ASSERT_EQ(static_cast<EtsAsyncStackSnapshot *>(clone->GetOpaqueSnapshot()), snapshotAfterMove);
}

}  // namespace ark::ets::test
