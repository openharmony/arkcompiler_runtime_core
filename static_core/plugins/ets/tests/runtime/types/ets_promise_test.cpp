/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

#include "ets_coroutine.h"
#include "ets_execution_context.h"
#include "ets_handle_scope.h"
#include "ets_platform_types.h"

#include "types/ets_class.h"
#include "types/ets_promise.h"
#include "types/ets_promise_ref.h"
#include "types/ets_string.h"
#include "plugins/ets/runtime/intrinsics/helpers/intrinsic_promise_impl.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "plugins/ets/runtime/types/ets_promise_async_stack_snapshot_queue.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets::test {

class EtsPromiseAsyncStackSnapshotQueueTestAccessor {
public:
    static EtsObjectArray *GetSnapshotQueue(EtsExecutionContext *executionCtx, EtsPromise *promise)
    {
        return promise->GetAsyncStackSnapshotQueue(executionCtx);
    }

    static void SetSnapshotQueue(EtsExecutionContext *executionCtx, EtsPromise *promise, EtsObjectArray *queue)
    {
        promise->SetAsyncStackSnapshotQueue(executionCtx, queue);
    }
};

template <typename T>
static void SubmitCallbackWithSnapshot(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                                       EtsHandle<T> &callback, JobWorkerThreadDomain workerDomain,
                                       EtsAsyncStackSnapshot *snapshot)
{
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    promise->SubmitCallback(executionCtx, callback->AsObject(), workerDomain);
    if (snapshot == nullptr) {
        return;
    }

    auto index = static_cast<uint32_t>(promise->GetQueueSize() - 1);
    auto callbackQueueLength = promise->GetCallbackQueue(executionCtx)->GetLength();
    EtsPromiseAsyncStackSnapshotQueue::Append(executionCtx, promise, snapshotHandle, index, callbackQueueLength);
}

class EtsPromiseTest : public testing::Test {
public:
    EtsPromiseTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        vm_ = coroutine->GetPandaVM();
    }

    ~EtsPromiseTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsPromiseTest);
    NO_MOVE_SEMANTIC(EtsPromiseTest);

    static std::vector<MirrorFieldInfo> GetPromiseMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsPromise, value_, "value"),
            MIRROR_FIELD_INFO(EtsPromise, mutex_, "mutex"),
            MIRROR_FIELD_INFO(EtsPromise, event_, "event"),
            MIRROR_FIELD_INFO(EtsPromise, callbackQueue_, "callbackQueue"),
            MIRROR_FIELD_INFO(EtsPromise, workerDomainQueue_, "workerDomainQueue"),
            MIRROR_FIELD_INFO(EtsPromise, asyncStackSnapshotQueue_, "asyncStackSnapshotQueue"),
            MIRROR_FIELD_INFO(EtsPromise, interopObject_, "interopObject"),
            MIRROR_FIELD_INFO(EtsPromise, linkedPromise_, "linkedPromise"),
            MIRROR_FIELD_INFO(EtsPromise, queueSize_, "queueSize"),
            MIRROR_FIELD_INFO(EtsPromise, state_, "state"),
            MIRROR_FIELD_INFO(EtsPromise, handled_, "handled"),
        };
    }

    static std::vector<MirrorFieldInfo> GetPromiseRefMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsPromiseRef, target_, "target")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class EtsPromiseQueueAllocationFailureTest : public testing::Test {
public:
    EtsPromiseQueueAllocationFailureTest() : options_(CreateOptions()) {}

    ~EtsPromiseQueueAllocationFailureTest() override
    {
        if (Runtime::GetCurrent() != nullptr) {
            Runtime::Destroy();
        }
    }

    NO_COPY_SEMANTIC(EtsPromiseQueueAllocationFailureTest);
    NO_MOVE_SEMANTIC(EtsPromiseQueueAllocationFailureTest);

protected:
    static RuntimeOptions CreateOptions()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        options.SetGcTriggerType("debug-never");
        constexpr uint64_t heapSizeLimit = 16U * 1024U * 1024U;  // NOLINT(readability-identifier-naming)
        options.SetHeapSizeLimit(heapSizeLimit);

        auto *stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});
        return options;
    }

    RuntimeOptions options_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// Check both EtsPromise and ark::Class<Promise> has the same number of fields
// and at the same offsets
TEST_F(EtsPromiseTest, PromiseMemoryLayout)
{
    EtsClass *promiseClass = PlatformTypes(vm_)->corePromise;
    MirrorFieldInfo::CompareMemberOffsets(promiseClass, GetPromiseMembers());
}

TEST_F(EtsPromiseTest, PromiseRefMemoryLayout)
{
    EtsClass *promiseRefClass = PlatformTypes(vm_)->corePromiseRef;
    MirrorFieldInfo::CompareMemberOffsets(promiseRefClass, GetPromiseRefMembers());
}

namespace {

struct QueuedPromise {
    EtsPromise *promise = nullptr;
    EtsObjectArray *callbackQueue = nullptr;
    EtsIntArray *workerDomainQueue = nullptr;
};

struct QueueFailureCalibration {
    size_t successfulFillerCount = 0U;
    size_t fillerAllocationStep = 0U;
    size_t freeMemoryAtFailure = 0U;
};

QueuedPromise CreateQueuedPromise(EtsExecutionContext *executionCtx)
{
    constexpr uint32_t initialQueueLength = 1U;  // NOLINT(readability-identifier-naming)

    EtsHandleScope scope(executionCtx);
    QueuedPromise queued;
    queued.promise = EtsPromise::Create(executionCtx);
    EXPECT_NE(queued.promise, nullptr);
    if (queued.promise == nullptr) {
        return {};
    }
    EtsHandle<EtsPromise> promiseHandle(executionCtx, queued.promise);

    auto *callback = EtsString::CreateFromMUtf8("callback");
    EXPECT_NE(callback, nullptr);
    if (callback == nullptr) {
        return {};
    }
    EtsHandle<EtsString> callbackHandle(executionCtx, callback);

    queued.callbackQueue = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, initialQueueLength);
    EXPECT_NE(queued.callbackQueue, nullptr);
    queued.workerDomainQueue = EtsIntArray::Create(initialQueueLength);
    EXPECT_NE(queued.workerDomainQueue, nullptr);
    if (queued.callbackQueue == nullptr || queued.workerDomainQueue == nullptr) {
        return {};
    }

    promiseHandle->SetCallbackQueue(executionCtx, queued.callbackQueue);
    promiseHandle->SetWorkerDomainQueue(executionCtx, queued.workerDomainQueue);
    {
        EtsMutex::LockHolder lock(promiseHandle);
        SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN, nullptr);
    }
    return queued;
}

QueueFailureCalibration CalibrateQueueAllocationFailure(const RuntimeOptions &options)
{
    QueueFailureCalibration calibration;
    EXPECT_TRUE(Runtime::Create(options));
    if (Runtime::GetCurrent() == nullptr) {
        return calibration;
    }

    constexpr uint32_t fillerLength = 3U;                                  // NOLINT(readability-identifier-naming)
    constexpr size_t maxFillerCount = std::numeric_limits<size_t>::max();  // NOLINT(readability-identifier-naming)
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        auto queued = CreateQueuedPromise(executionCtx);
        EXPECT_NE(queued.promise, nullptr);
        if (queued.promise == nullptr) {
            Runtime::Destroy();
            return calibration;
        }

        auto *heapManager = EtsCoroutine::GetCurrent()->GetPandaVM()->GetHeapManager();
        EXPECT_NE(heapManager, nullptr);
        if (heapManager == nullptr) {
            Runtime::Destroy();
            return calibration;
        }

        const auto freeBeforeFirstFiller = heapManager->GetFreeMemory();
        auto *firstFiller = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, fillerLength);
        EXPECT_NE(firstFiller, nullptr);
        if (firstFiller == nullptr) {
            executionCtx->GetMT()->ClearException();
            Runtime::Destroy();
            return calibration;
        }
        calibration.successfulFillerCount = 1U;
        calibration.fillerAllocationStep = freeBeforeFirstFiller - heapManager->GetFreeMemory();

        std::vector<EtsObjectArray *> fillers {firstFiller};
        for (size_t fillerCount = 1; fillerCount < maxFillerCount; ++fillerCount) {
            auto *filler = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, fillerLength);
            if (filler == nullptr) {
                calibration.freeMemoryAtFailure = heapManager->GetFreeMemory();
                break;
            }
            fillers.push_back(filler);
            ++calibration.successfulFillerCount;
        }
        executionCtx->GetMT()->ClearException();
    }
    Runtime::Destroy();
    return calibration;
}

}  // namespace

TEST_F(EtsPromiseQueueAllocationFailureTest, PartialQueueAllocationFailureDoesNotCommit)
{
    constexpr uint32_t fillerLength = 3U;  // NOLINT(readability-identifier-naming)
    const auto calibration = CalibrateQueueAllocationFailure(options_);
    ASSERT_GT(calibration.successfulFillerCount, 2U);
    ASSERT_GT(calibration.fillerAllocationStep, 0U);

    ASSERT_TRUE(Runtime::Create(options_));
    auto *vm = EtsCoroutine::GetCurrent()->GetPandaVM();
    auto *heapManager = vm->GetHeapManager();
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        auto queued = CreateQueuedPromise(executionCtx);
        ASSERT_NE(queued.promise, nullptr);
        ASSERT_NE(queued.callbackQueue, nullptr);
        ASSERT_NE(queued.workerDomainQueue, nullptr);
        EtsHandle<EtsPromise> promiseHandle(executionCtx, queued.promise);

        std::vector<EtsObjectArray *> fillers;
        while (heapManager->GetFreeMemory() >= calibration.freeMemoryAtFailure + calibration.fillerAllocationStep) {
            auto *filler = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, fillerLength);
            if (filler == nullptr) {
                ASSERT_TRUE(executionCtx->GetMT()->HasPendingException());
                executionCtx->GetMT()->ClearException();
                break;
            }
            fillers.push_back(filler);
        }

        const auto freeBeforeGrowth = heapManager->GetFreeMemory();
        ASSERT_LE(freeBeforeGrowth, calibration.freeMemoryAtFailure + calibration.fillerAllocationStep);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            ark::ets::intrinsics::helpers::EnsurePromiseCapacity(executionCtx, promiseHandle);
        }

        ASSERT_TRUE(executionCtx->GetMT()->HasPendingException());
        ASSERT_EQ(promiseHandle->GetCallbackQueue(executionCtx), queued.callbackQueue);
        ASSERT_EQ(promiseHandle->GetWorkerDomainQueue(executionCtx), queued.workerDomainQueue);
        ASSERT_EQ(EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr()),
                  nullptr);
        ASSERT_EQ(promiseHandle->GetQueueSize(), 1);
        executionCtx->GetMT()->ClearException();
    }
    Runtime::Destroy();
}

TEST_F(EtsPromiseTest, CapacityGrowthDoesNotAllocateSnapshotQueueWhenCaptureIsDisabled)
{
    vm_->SetAsyncDebuggerMaxAsyncDepth(0U);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsPromise> promiseHandle(executionCtx, EtsPromise::Create(executionCtx));
        ASSERT_NE(promiseHandle.GetPtr(), nullptr);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            ark::ets::intrinsics::helpers::EnsurePromiseCapacity(executionCtx, promiseHandle);
        }
        ASSERT_FALSE(executionCtx->GetMT()->HasPendingException());
        ASSERT_NE(promiseHandle->GetCallbackQueue(executionCtx), nullptr);
        ASSERT_NE(promiseHandle->GetWorkerDomainQueue(executionCtx), nullptr);
        ASSERT_EQ(EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr()),
                  nullptr);
    }
}

TEST_F(EtsPromiseTest, SnapshotQueueUsesSpareCallbackCapacityLazily)
{
    vm_->SetAsyncDebuggerMaxAsyncDepth(4U);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsPromise> promiseHandle(executionCtx, EtsPromise::Create(executionCtx));
        ASSERT_NE(promiseHandle.GetPtr(), nullptr);

        constexpr uint32_t queueLength = 2U;  // NOLINT(readability-identifier-naming)
        auto *callbackQueue = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, queueLength);
        ASSERT_NE(callbackQueue, nullptr);
        auto *workerDomainQueue = EtsIntArray::Create(queueLength);
        ASSERT_NE(workerDomainQueue, nullptr);
        EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, callbackQueue);
        EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, workerDomainQueue);
        promiseHandle->SetCallbackQueue(executionCtx, callbackQueueHandle.GetPtr());
        promiseHandle->SetWorkerDomainQueue(executionCtx, workerDomainQueueHandle.GetPtr());

        auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
        ASSERT_NE(snapshot, nullptr);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
        auto *callback = EtsString::CreateFromMUtf8("callback");
        ASSERT_NE(callback, nullptr);
        EtsHandle<EtsString> callbackHandle(executionCtx, callback);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN,
                                       snapshotHandle.GetPtr());
        }

        ASSERT_FALSE(executionCtx->GetMT()->HasPendingException());
        ASSERT_EQ(promiseHandle->GetQueueSize(), 1);
        auto *snapshotQueue =
            EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr());
        ASSERT_NE(snapshotQueue, nullptr);
        ASSERT_EQ(snapshotQueue->GetLength(), queueLength);
        ASSERT_EQ(EtsAsyncStackSnapshot::FromEtsObject(snapshotQueue->Get(0)), snapshotHandle.GetPtr());
    }
}

static void SetUpPromiseQueues(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                               uint32_t callbackQueueLength, uint32_t snapshotQueueLength)
{
    auto *callbackQueue = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, callbackQueueLength);
    ASSERT_NE(callbackQueue, nullptr);
    auto *workerDomainQueue = EtsIntArray::Create(callbackQueueLength);
    ASSERT_NE(workerDomainQueue, nullptr);
    auto *snapshotQueue =
        EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSnapshot, snapshotQueueLength);
    ASSERT_NE(snapshotQueue, nullptr);
    EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, callbackQueue);
    EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, workerDomainQueue);
    EtsHandle<EtsObjectArray> snapshotQueueHandle(executionCtx, snapshotQueue);
    promise->SetCallbackQueue(executionCtx, callbackQueueHandle.GetPtr());
    promise->SetWorkerDomainQueue(executionCtx, workerDomainQueueHandle.GetPtr());
    EtsPromiseAsyncStackSnapshotQueueTestAccessor::SetSnapshotQueue(executionCtx, promise.GetPtr(),
                                                                    snapshotQueueHandle.GetPtr());
}

static std::vector<EtsObjectArray *> FillHeapForSnapshotQueueGrowth(EtsExecutionContext *executionCtx,
                                                                    uint32_t fillerLength)
{
    std::vector<EtsObjectArray *> fillers;
    constexpr size_t maxFillerCount = std::numeric_limits<size_t>::max();  // NOLINT(readability-identifier-naming)
    while (fillers.size() < maxFillerCount) {
        auto *filler = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, fillerLength);
        if (filler == nullptr) {
            EXPECT_TRUE(executionCtx->GetMT()->HasPendingException());
            executionCtx->GetMT()->ClearException();
            break;
        }
        fillers.push_back(filler);
    }
    return fillers;
}

TEST_F(EtsPromiseTest, SnapshotQueueGrowsWithCallbackQueueWhenCaptureIsDisabled)
{
    vm_->SetAsyncDebuggerMaxAsyncDepth(0U);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsPromise> promiseHandle(executionCtx, EtsPromise::Create(executionCtx));
        ASSERT_NE(promiseHandle.GetPtr(), nullptr);

        constexpr uint32_t callbackQueueLength = 2U;  // NOLINT(readability-identifier-naming)
        constexpr uint32_t snapshotQueueLength = 1U;  // NOLINT(readability-identifier-naming)
        SetUpPromiseQueues(executionCtx, promiseHandle, callbackQueueLength, snapshotQueueLength);

        auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
        ASSERT_NE(snapshot, nullptr);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
        auto *callback = EtsString::CreateFromMUtf8("callback");
        ASSERT_NE(callback, nullptr);
        EtsHandle<EtsString> callbackHandle(executionCtx, callback);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN,
                                       snapshotHandle.GetPtr());
        }

        ASSERT_EQ(promiseHandle->GetQueueSize(), 1U);
        auto *snapshotQueue =
            EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr());
        ASSERT_EQ(snapshotQueue->GetLength(), snapshotQueueLength);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            ark::ets::intrinsics::helpers::EnsurePromiseCapacity(executionCtx, promiseHandle);
        }

        auto *grownSnapshotQueue =
            EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr());
        ASSERT_NE(grownSnapshotQueue, nullptr);
        ASSERT_EQ(grownSnapshotQueue->GetLength(), callbackQueueLength);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN,
                                       nullptr);
        }

        ASSERT_EQ(promiseHandle->GetQueueSize(), 2U);
        ASSERT_EQ(grownSnapshotQueue->GetLength(), callbackQueueLength);
        ASSERT_EQ(EtsAsyncStackSnapshot::FromEtsObject(grownSnapshotQueue->Get(0)), snapshotHandle.GetPtr());
        ASSERT_EQ(grownSnapshotQueue->Get(1), nullptr);
    }
}

TEST_F(EtsPromiseTest, AppendGrowsShortSnapshotQueue)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsPromise> promiseHandle(executionCtx, EtsPromise::Create(executionCtx));
        ASSERT_NE(promiseHandle.GetPtr(), nullptr);

        constexpr uint32_t callbackQueueLength = 2U;  // NOLINT(readability-identifier-naming)
        constexpr uint32_t snapshotQueueLength = 1U;  // NOLINT(readability-identifier-naming)
        auto *callbackQueue = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, callbackQueueLength);
        ASSERT_NE(callbackQueue, nullptr);
        auto *workerDomainQueue = EtsIntArray::Create(callbackQueueLength);
        ASSERT_NE(workerDomainQueue, nullptr);
        auto *snapshotQueue =
            EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSnapshot, snapshotQueueLength);
        ASSERT_NE(snapshotQueue, nullptr);
        EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, callbackQueue);
        EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, workerDomainQueue);
        EtsHandle<EtsObjectArray> snapshotQueueHandle(executionCtx, snapshotQueue);
        promiseHandle->SetCallbackQueue(executionCtx, callbackQueueHandle.GetPtr());
        promiseHandle->SetWorkerDomainQueue(executionCtx, workerDomainQueueHandle.GetPtr());
        EtsPromiseAsyncStackSnapshotQueueTestAccessor::SetSnapshotQueue(executionCtx, promiseHandle.GetPtr(),
                                                                        snapshotQueueHandle.GetPtr());

        auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
        ASSERT_NE(snapshot, nullptr);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
        auto *callback = EtsString::CreateFromMUtf8("callback");
        ASSERT_NE(callback, nullptr);
        EtsHandle<EtsString> callbackHandle(executionCtx, callback);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN,
                                       nullptr);
        }

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN,
                                       snapshotHandle.GetPtr());
        }

        auto *grownSnapshotQueue =
            EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr());
        ASSERT_NE(grownSnapshotQueue, nullptr);
        ASSERT_EQ(grownSnapshotQueue->GetLength(), callbackQueueLength);
        ASSERT_EQ(grownSnapshotQueue->Get(0), nullptr);
        ASSERT_EQ(EtsAsyncStackSnapshot::FromEtsObject(grownSnapshotQueue->Get(1)), snapshotHandle.GetPtr());
    }
}

TEST_F(EtsPromiseQueueAllocationFailureTest, SnapshotQueueGrowthFailureDoesNotBlockCallbackRegistration)
{
    ASSERT_TRUE(Runtime::Create(options_));
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsPromise> promiseHandle(executionCtx, EtsPromise::Create(executionCtx));
        ASSERT_NE(promiseHandle.GetPtr(), nullptr);

        constexpr uint32_t callbackQueueLength = 2U;  // NOLINT(readability-identifier-naming)
        constexpr uint32_t snapshotQueueLength = 1U;  // NOLINT(readability-identifier-naming)
        SetUpPromiseQueues(executionCtx, promiseHandle, callbackQueueLength, snapshotQueueLength);

        auto *firstCallback = EtsString::CreateFromMUtf8("first-callback");
        ASSERT_NE(firstCallback, nullptr);
        EtsHandle<EtsString> firstCallbackHandle(executionCtx, firstCallback);
        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, firstCallbackHandle, JobWorkerThreadDomain::MAIN,
                                       nullptr);
        }

        auto *snapshot = EtsAsyncStackSnapshot::Create(executionCtx);
        ASSERT_NE(snapshot, nullptr);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
        auto *secondCallback = EtsString::CreateFromMUtf8("second-callback");
        ASSERT_NE(secondCallback, nullptr);
        EtsHandle<EtsString> secondCallbackHandle(executionCtx, secondCallback);

        auto fillers = FillHeapForSnapshotQueueGrowth(executionCtx, callbackQueueLength);
        ASSERT_FALSE(fillers.empty());

        {
            EtsMutex::LockHolder lock(promiseHandle);
            ark::ets::intrinsics::helpers::EnsurePromiseCapacity(executionCtx, promiseHandle);
        }
        ASSERT_FALSE(executionCtx->GetMT()->HasPendingException());
        ASSERT_EQ(EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr()),
                  nullptr);

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, secondCallbackHandle, JobWorkerThreadDomain::MAIN,
                                       snapshotHandle.GetPtr());
        }
        ASSERT_FALSE(executionCtx->GetMT()->HasPendingException());
        ASSERT_EQ(promiseHandle->GetQueueSize(), 2U);
        ASSERT_EQ(promiseHandle->GetCallbackQueue(executionCtx)->Get(0), firstCallbackHandle->AsObject());
        ASSERT_EQ(promiseHandle->GetCallbackQueue(executionCtx)->Get(1), secondCallbackHandle->AsObject());
        ASSERT_EQ(promiseHandle->GetWorkerDomainQueue(executionCtx)->Get(1),
                  static_cast<int>(JobWorkerThreadDomain::MAIN));
    }
    Runtime::Destroy();
}

TEST_F(EtsPromiseQueueAllocationFailureTest, SnapshotQueueFailureDoesNotBlockCallbackRegistration)
{
    ASSERT_TRUE(Runtime::Create(options_));
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    {
        ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, EtsAsyncStackSnapshot::Create(executionCtx));
        ASSERT_NE(snapshotHandle.GetPtr(), nullptr);

        EtsHandle<EtsPromise> promiseHandle(executionCtx, EtsPromise::Create(executionCtx));
        ASSERT_NE(promiseHandle.GetPtr(), nullptr);
        constexpr uint32_t queueLength = 1U;  // NOLINT(readability-identifier-naming)
        auto *callbackQueue = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, queueLength);
        ASSERT_NE(callbackQueue, nullptr);
        auto *workerDomainQueue = EtsIntArray::Create(queueLength);
        ASSERT_NE(workerDomainQueue, nullptr);
        EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, callbackQueue);
        EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, workerDomainQueue);
        promiseHandle->SetCallbackQueue(executionCtx, callbackQueueHandle.GetPtr());
        promiseHandle->SetWorkerDomainQueue(executionCtx, workerDomainQueueHandle.GetPtr());

        auto *callback = EtsString::CreateFromMUtf8("callback");
        ASSERT_NE(callback, nullptr);
        EtsHandle<EtsString> callbackHandle(executionCtx, callback);

        std::vector<EtsObjectArray *> fillers;
        while (true) {
            auto *filler = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSnapshot, queueLength);
            if (filler == nullptr) {
                break;
            }
            fillers.push_back(filler);
        }
        ASSERT_TRUE(executionCtx->GetMT()->HasPendingException());
        executionCtx->GetMT()->ClearException();

        {
            EtsMutex::LockHolder lock(promiseHandle);
            SubmitCallbackWithSnapshot(executionCtx, promiseHandle, callbackHandle, JobWorkerThreadDomain::MAIN,
                                       snapshotHandle.GetPtr());
        }

        ASSERT_FALSE(executionCtx->GetMT()->HasPendingException());
        ASSERT_EQ(promiseHandle->GetQueueSize(), 1);
        ASSERT_EQ(callbackQueueHandle->Get(0), callbackHandle->AsObject());
        ASSERT_EQ(workerDomainQueueHandle->Get(0), static_cast<int>(JobWorkerThreadDomain::MAIN));
        ASSERT_EQ(EtsPromiseAsyncStackSnapshotQueueTestAccessor::GetSnapshotQueue(executionCtx, promiseHandle.GetPtr()),
                  nullptr);
    }
    Runtime::Destroy();
}
}  // namespace ark::ets::test
