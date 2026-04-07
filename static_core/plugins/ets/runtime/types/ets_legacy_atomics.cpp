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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_LEGACY_ATOMICS_CPP
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_LEGACY_ATOMICS_CPP

#include <limits>
#include <type_traits>

#include "runtime/execution/job_execution_context.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/time.h"
#include "libarkbase/generated/coherency_line_size.h"

#include "runtime/include/thread_scopes.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/exceptions.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_worker_group.h"
#include "runtime/execution/job_events.h"
#include "runtime/execution/job_launch.h"

#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_legacy_atomics.h"
#include "plugins/ets/runtime/types/ets_arraybuffer-inl.h"

namespace ark::ets {

struct WorkerWaiter {
    os::memory::ConditionVariable condvar;
    PandaString name;
    bool isWaiting {true};
};

enum class WaiterStatus {
    WORKER,
    RESOLVED,
    PENDING,
    TIMEOUT_RESOLVED,
    TIMEOUT_PENDING,
    LOST,  // the target promise has been gc'ed
};

struct WaiterListNode {
    void *target;  // status == WORKER ? WorkerWaiter* : mem::Reference*
    mem::Reference *bufferRef;
    uint32_t index;
    uint32_t hash;
    WaiterStatus status;
};

struct alignas(ark::COHERENCY_LINE_SIZE) ContentionList {
    os::memory::Mutex mutex;
    std::list<WaiterListNode> waiterList GUARDED_BY(mutex);
};

struct EpInfo {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    ContentionList &bucket;
    std::list<WaiterListNode>::iterator nodeIterator;
    TimerEvent timerEvent;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    EpInfo(ContentionList &bucket, std::list<WaiterListNode>::iterator nodeIterator, JobManager *jobMan,
           ark::EventId id)
        : bucket(bucket), nodeIterator(nodeIterator), timerEvent(jobMan, id) {};
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::array<ContentionList, LegacyAtomics::CONTENTION_TABLE_SIZE> g_contentionTable;

static uint32_t HashFunc(EtsStdCoreArrayBuffer *buffer, uint32_t index)
{
    ASSERT(buffer != nullptr);
    uint32_t bufferHash = buffer->AsObject()->GetHashCode();
    uint32_t indexHash = std::hash<uint32_t> {}(index);
    uint32_t resultHash = bufferHash ^ indexHash;
    return resultHash;
}

static ContentionList &GetContentionListByHash(uint32_t hash)
{
    static_assert(ark::helpers::math::IsPowerOfTwo(LegacyAtomics::CONTENTION_TABLE_SIZE));
    return g_contentionTable[hash & (LegacyAtomics::CONTENTION_TABLE_SIZE - 1)];
}

template <typename T>
bool CheckEqual(EtsStdCoreArrayBuffer *buffer, uint32_t byteOffset, T value)
{
    static_assert(std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>);
    return value == buffer->GetElement<T>(0, byteOffset);
}

void ClearReferences(mem::GlobalObjectStorage *objectStorage, const std::list<WaiterListNode> &toRemoveList)
{
    ASSERT(objectStorage != nullptr);
    for (const auto &node : toRemoveList) {
        ASSERT(node.bufferRef != nullptr);
        objectStorage->Remove(node.bufferRef);
        if (node.status != WaiterStatus::WORKER) {
            objectStorage->Remove(static_cast<mem::Reference *>(node.target));
        }
    }
}

template <typename T>
static EtsString *DoWait(EtsStdCoreArrayBuffer *buffer, uint32_t index, T value, uint64_t timeout)
{
    static_assert(std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>);
    ASSERT_MANAGED_CODE();
    ASSERT(index <= static_cast<int64_t>(buffer->GetByteLength()) - sizeof(T));
    uint64_t untilTime = os::time::GetClockTimeInMilli() + timeout;
    bool hasTimeout = (timeout < std::numeric_limits<int32_t>::max());
    auto *mt = ManagedThread::GetCurrent();

    uint32_t hash = HashFunc(buffer, index);
    auto &bucket = GetContentionListByHash(hash);

    WorkerWaiter thisWaiter {};
    thisWaiter.name = JobExecutionContext::CastFromMutator(mt)->GetWorker()->GetName();

    mem::GlobalObjectStorage *objectStorage = mt->GetVM()->GetGlobalObjectStorage();
    auto *bufferRef = objectStorage->Add(buffer->AsObject()->GetCoreType(), mem::Reference::ObjectType::WEAK);
    if UNLIKELY (bufferRef == nullptr) {
        return nullptr;
    }

    std::list<WaiterListNode> preallocList {WaiterListNode {&thisWaiter, bufferRef, index, hash, WaiterStatus::WORKER}};
    bucket.mutex.Lock();
    if (!CheckEqual(buffer, index, value)) {
        bucket.mutex.Unlock();
        ClearReferences(objectStorage, preallocList);
        return EtsString::CreateFromMUtf8("not-equal");
    }

    auto nodeIterator = preallocList.begin();
    bucket.waiterList.splice(bucket.waiterList.end(), preallocList);
    bool timedOut = false;
    mt->NativeCodeBegin();
    LOG(DEBUG, COROUTINES) << "LegacyAtomicsWait: worker " << thisWaiter.name << " waits";
    if (hasTimeout) {
        do {
            timedOut = thisWaiter.condvar.TimedWait(&bucket.mutex, untilTime, 0, true);
        } while (thisWaiter.isWaiting && !timedOut);
    } else {
        do {
            thisWaiter.condvar.Wait(&bucket.mutex);
        } while (thisWaiter.isWaiting);
    }
    if UNLIKELY (timedOut && thisWaiter.isWaiting) {
        preallocList.splice(preallocList.end(), bucket.waiterList, nodeIterator);
        bucket.mutex.Unlock();
        mt->NativeCodeEnd();
        ClearReferences(objectStorage, preallocList);
        return EtsString::CreateFromMUtf8("timed-out");
    }
    bucket.mutex.Unlock();
    mt->NativeCodeEnd();
    return EtsString::CreateFromMUtf8("ok");
}

static void ResolvePromise(EtsHandle<EtsPromise> &hPromise, EtsExecutionContext *ctx, EtsString *obj)
{
    ASSERT(hPromise.GetPtr() != nullptr);
    ASSERT(ctx == EtsExecutionContext::GetCurrent());
    EtsHandle<EtsString> hobj(ctx, obj);
    EtsMutex::LockHolder lh(hPromise);
    if UNLIKELY (obj == nullptr) {
        hPromise->Resolve(ctx, nullptr);
    } else {
        hPromise->Resolve(ctx, hobj->AsObject());
    }
}

static EtsPromise *ResolvePromiseWithString(EtsPromise *promise, EtsExecutionContext *ctx, const char *data)
{
    ASSERT(ctx != nullptr);
    ASSERT_MANAGED_CODE();
    ASSERT(promise != nullptr);
    ASSERT(!promise->GetCoreType()->IsForwarded());

    EtsHandleScope hsc(ctx);
    EtsHandle<EtsPromise> hPromise(ctx, promise);
    ResolvePromise(hPromise, ctx, EtsString::CreateFromMUtf8(data));
    return hPromise.GetPtr();
}

static void RemovePromiseNodeResolvePromise(EtsExecutionContext *ctx, mem::GlobalObjectStorage *objectStorage,
                                            ContentionList &bucket, std::list<WaiterListNode>::iterator nodeIterator)
{
    std::list<WaiterListNode> toRemoveList;
    {
        os::memory::LockHolder lh(bucket.mutex);
        toRemoveList.splice(toRemoveList.end(), bucket.waiterList, nodeIterator);
    }
    if (nodeIterator->status == WaiterStatus::TIMEOUT_PENDING) {
        auto *promiseRef = static_cast<mem::Reference *>(nodeIterator->target);
        ASSERT(promiseRef != nullptr);

        auto *p = EtsPromise::FromCoreType(objectStorage->Get(promiseRef));
        if LIKELY (p != nullptr) {
            ResolvePromiseWithString(p, ctx, "timed-out");
        }
    }
    ClearReferences(objectStorage, toRemoveList);
}

static void DelayedResolve(ManagedThread *mt, ContentionList &bucket, std::list<WaiterListNode>::iterator nodeIterator)
{
    ASSERT(mt == ManagedThread::GetCurrent());
    mem::GlobalObjectStorage *objectStorage = mt->GetVM()->GetGlobalObjectStorage();
    auto *ctx = EtsExecutionContext::FromMT(mt);

    ScopedManagedCodeThread msc(mt);
    RemovePromiseNodeResolvePromise(ctx, objectStorage, bucket, nodeIterator);
}

/// @brief Returns false if something wrong happened and exception was raised
static bool LaunchTimerJob(ManagedThread *mt, ContentionList &bucket, std::list<WaiterListNode>::iterator nodeIterator,
                           uint64_t untilTime)
{
    ASSERT(mt == ManagedThread::GetCurrent());

    auto timerEntryPoint = [](void *params) {
        auto *timerMT = ManagedThread::GetCurrent();
        auto *args = static_cast<EpInfo *>(params);

        DelayedResolve(timerMT, args->bucket, args->nodeIterator);

        Runtime::GetCurrent()->GetInternalAllocator()->Delete(args);
    };

    auto *ctx = JobExecutionContext::CastFromMutator(mt);
    auto *jobMan = ctx->GetManager();
    // NOLINTNEXTLINE(readability-magic-numbers)
    uint64_t untilTimeMicro = untilTime * 1000U;

    auto *params = Runtime::GetCurrent()->GetInternalAllocator()->New<EpInfo>(bucket, nodeIterator, jobMan, 0);
    if UNLIKELY (params == nullptr) {
        ThrowRuntimeException("Internal error");
        return false;
    }
    params->timerEvent.SetExpirationTime(untilTimeMicro);
    ctx->GetWorker()->PostSchedulingTask(static_cast<int64_t>(untilTime) - os::time::GetClockTimeInMilli());

    auto epInfo = Job::NativeEntrypointInfo {timerEntryPoint, params};
    // NOLINTNEXTLINE(performance-move-const-arg)
    auto *job = jobMan->CreateJob("DelayedPromiseResolver", std::move(epInfo), EtsCoroutine::TIMER_CALLBACK,
                                  Job::Type::MUTATOR, true);

    auto wGroup = ark::JobWorkerThreadGroup::GenerateExactWorkerId(ctx->GetWorker()->GetId());
    auto launchRes = jobMan->Launch(job, ark::LaunchParams {job->GetPriority(), wGroup, &params->timerEvent});
    if UNLIKELY (launchRes != LaunchResult::OK) {
        ASSERT(mt->HasPendingException());
        ASSERT(launchRes != LaunchResult::NO_SUITABLE_WORKER);
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(params);
        jobMan->DestroyJob(job);
        return false;
    }
    return true;
}

template <typename T>
static EtsPromise *DoAsyncWait(EtsStdCoreArrayBuffer *buffer, uint32_t index, T value, uint64_t timeout)
{
    static_assert(std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>);
    ASSERT_MANAGED_CODE();
    ASSERT(index <= static_cast<int64_t>(buffer->GetByteLength()) - sizeof(T));
    bool hasTimeout = (timeout < std::numeric_limits<int32_t>::max());
    uint64_t untilTime = os::time::GetClockTimeInMilli() + timeout;
    auto *mt = ManagedThread::GetCurrent();
    auto *ctx = EtsExecutionContext::FromMT(mt);

    uint32_t hash = HashFunc(buffer, index);
    auto &bucket = GetContentionListByHash(hash);
    EtsHandleScope hsc(ctx);
    EtsHandle<EtsStdCoreArrayBuffer> hBuffer(ctx, buffer);

    mem::GlobalObjectStorage *objectStorage = mt->GetVM()->GetGlobalObjectStorage();
    auto *promise = EtsPromise::Create(ctx);
    if UNLIKELY (promise == nullptr) {
        ASSERT(mt->HasPendingException());
        return nullptr;
    }
    auto *bufferRef = objectStorage->Add(hBuffer->GetCoreType(), mem::Reference::ObjectType::WEAK);
    if UNLIKELY (bufferRef == nullptr) {
        ThrowRuntimeException("Internal error");
        return nullptr;
    }
    auto *promiseRef = objectStorage->Add(promise->GetCoreType(), mem::Reference::ObjectType::WEAK);
    if UNLIKELY (promiseRef == nullptr) {
        objectStorage->Remove(bufferRef);
        ThrowRuntimeException("Internal error");
        return nullptr;
    }
    std::list<WaiterListNode> preallocList {WaiterListNode {
        promiseRef, bufferRef, index, hash, hasTimeout ? WaiterStatus::TIMEOUT_PENDING : WaiterStatus::PENDING}};

    bucket.mutex.Lock();
    if (!CheckEqual(hBuffer.GetPtr(), index, value)) {
        bucket.mutex.Unlock();
        ClearReferences(objectStorage, preallocList);
        return ResolvePromiseWithString(promise, ctx, "not-equal");
    }

    auto nodeIterator = preallocList.begin();
    bucket.waiterList.splice(bucket.waiterList.end(), preallocList);

    if (!hasTimeout) {
        bucket.mutex.Unlock();
        return promise;
    }

    EtsHandle<EtsPromise> hPromise(ctx, promise);
    bool res = LaunchTimerJob(mt, bucket, nodeIterator, untilTime);
    if UNLIKELY (!res) {
        ASSERT(mt->HasPendingException());
        preallocList.splice(preallocList.end(), bucket.waiterList, nodeIterator);
        bucket.mutex.Unlock();
        ClearReferences(objectStorage, preallocList);
        return nullptr;
    }
    bucket.mutex.Unlock();
    return hPromise.GetPtr();
}

static void NotifySyncWaiter(const WaiterListNode &node)
{
    ASSERT(node.status == WaiterStatus::WORKER);
    auto *worker = static_cast<WorkerWaiter *>(node.target);
    ASSERT(worker->isWaiting);
    LOG(DEBUG, COROUTINES) << "LegacyAtomicsNotify: notifying worker " << worker->name;
    worker->isWaiting = false;
    worker->condvar.Signal();
}

static void NotifyAsyncWaiter(mem::GlobalObjectStorage *objectStorage, const WaiterListNode &node)
{
    ASSERT(node.status == WaiterStatus::RESOLVED);
    auto *promiseRef = static_cast<mem::Reference *>(node.target);
    auto *ctx = EtsExecutionContext::GetCurrent();

    auto *promise = EtsPromise::FromCoreType(objectStorage->Get(promiseRef));
    if (promise != nullptr) {
        EtsHandle<EtsPromise> hNodePromise(ctx, promise);
        auto *okStr = EtsString::CreateFromMUtf8("ok");
        if (UNLIKELY(okStr == nullptr)) {
            LOG(ERROR, COROUTINES) << "Legacy Atomics Notify: Failed to create string and cannot resolve promise";
            return;
        }
        ResolvePromise(hNodePromise, ctx, okStr);
    }
}

/// @brief HandleScope required
static void NotifyWaiters(mem::GlobalObjectStorage *objectStorage, const std::list<WaiterListNode> &toNotifyList)
{
    for (const auto &node : toNotifyList) {
        if (node.status == WaiterStatus::RESOLVED) {
            NotifyAsyncWaiter(objectStorage, node);
        } else {
            ASSERT(node.status == WaiterStatus::LOST || node.status == WaiterStatus::WORKER);
        }
    }
}

static int32_t DoNotify(EtsStdCoreArrayBuffer *buffer, uint32_t index, int32_t count)
{
    ASSERT_MANAGED_CODE();
    ASSERT(count >= 0);
    auto *ctx = EtsExecutionContext::GetCurrent();
    mem::GlobalObjectStorage *objectStorage = ctx->GetMT()->GetVM()->GetGlobalObjectStorage();

    uint32_t currentHash = HashFunc(buffer, index);
    auto &bucket = GetContentionListByHash(currentHash);

    EtsHandleScope sc(ctx);
    EtsHandle<EtsStdCoreArrayBuffer> hBuffer(ctx, buffer);
    std::list<WaiterListNode> toNotifyList;
    PandaSmallVector<mem::Reference *> beforeTimer;

    bucket.mutex.Lock();
    int32_t oldCount = count;
    for (auto nodeIterator = bucket.waiterList.begin(); count > 0 && nodeIterator != bucket.waiterList.end();) {
        auto currIter = nodeIterator++;
        if (currentHash != currIter->hash || index != currIter->index) {
            continue;
        }
        auto *nodeBuffer = EtsStdCoreArrayBuffer::FromCoreType(objectStorage->Get(currIter->bufferRef));
        if UNLIKELY (nodeBuffer == nullptr) {
            ASSERT(currIter->status != WaiterStatus::WORKER);  // sync waiter stores buffer on stack
            if (currIter->status != WaiterStatus::TIMEOUT_PENDING) {
                // no timeout
                currIter->status = WaiterStatus::LOST;
                toNotifyList.splice(toNotifyList.end(), bucket.waiterList, currIter);
            }  // else: there's a timeout that will erase the node for us
            continue;
        }
        if (currIter->status == WaiterStatus::TIMEOUT_RESOLVED ||
            !EtsReferenceEquals(ctx, hBuffer.GetPtr(), nodeBuffer)) {
            continue;
        }
        switch (currIter->status) {
            case WaiterStatus::TIMEOUT_PENDING:
                currIter->status = WaiterStatus::TIMEOUT_RESOLVED;
                // the node will be removed by the delayed job anyway, but we need to resolve promise now
                beforeTimer.push_back(static_cast<mem::Reference *>(std::exchange(currIter->target, nullptr)));
                break;
            case WaiterStatus::PENDING:
                currIter->status = WaiterStatus::RESOLVED;
                toNotifyList.splice(toNotifyList.end(), bucket.waiterList, currIter);
                break;
            case WaiterStatus::WORKER:
                NotifySyncWaiter(*currIter);
                toNotifyList.splice(toNotifyList.end(), bucket.waiterList, currIter);
                break;
            default:
                LOG(FATAL, COROUTINES) << "Legacy Atomics Notify: Invariant violation";
        }
        count--;
    }
    bucket.mutex.Unlock();
    for (auto *ref : beforeTimer) {
        auto *p = EtsPromise::FromCoreType(objectStorage->Get(ref));
        objectStorage->Remove(ref);
        if UNLIKELY (p == nullptr) {
            continue;
        }
        EtsHandle<EtsPromise> hNodePromise(ctx, p);
        ResolvePromise(hNodePromise, ctx, EtsString::CreateFromMUtf8("ok"));
    }
    NotifyWaiters(objectStorage, toNotifyList);
    ClearReferences(objectStorage, toNotifyList);
    return oldCount - count;
}

/*static*/
EtsString *LegacyAtomics::WaitInt32(EtsStdCoreArrayBuffer *buffer, int32_t index, int32_t value, int32_t timeout)
{
    ASSERT(index >= 0);
    ASSERT(timeout >= 0);
    auto uIndex = static_cast<uint32_t>(index);
    auto uTimeout = static_cast<uint64_t>(timeout);
    return DoWait(buffer, uIndex, value, uTimeout);
}

/*static*/
EtsString *LegacyAtomics::WaitInt64(EtsStdCoreArrayBuffer *buffer, int32_t index, int64_t value, int32_t timeout)
{
    ASSERT(index >= 0);
    ASSERT(timeout >= 0);
    auto uIndex = static_cast<uint32_t>(index);
    auto uTimeout = static_cast<uint64_t>(timeout);
    return DoWait(buffer, uIndex, value, uTimeout);
}

/*static*/
EtsPromise *LegacyAtomics::AsyncWaitInt32(EtsStdCoreArrayBuffer *buffer, int32_t index, int32_t value, int32_t timeout)
{
    ASSERT(index >= 0);
    ASSERT(timeout >= 0);
    auto uIndex = static_cast<uint32_t>(index);
    auto uTimeout = static_cast<uint64_t>(timeout);
    return DoAsyncWait(buffer, uIndex, value, uTimeout);
}

/*static*/
EtsPromise *LegacyAtomics::AsyncWaitInt64(EtsStdCoreArrayBuffer *buffer, int32_t index, int64_t value, int32_t timeout)
{
    ASSERT(index >= 0);
    ASSERT(timeout >= 0);
    auto uIndex = static_cast<uint32_t>(index);
    auto uTimeout = static_cast<uint64_t>(timeout);
    return DoAsyncWait(buffer, uIndex, value, uTimeout);
}

/*static*/
int32_t LegacyAtomics::Notify(EtsStdCoreArrayBuffer *buffer, int32_t index, int32_t count)
{
    ASSERT(index >= 0);
    auto uIndex = static_cast<uint32_t>(index);
    return DoNotify(buffer, uIndex, count);
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_LEGACY_ATOMICS_CPP
