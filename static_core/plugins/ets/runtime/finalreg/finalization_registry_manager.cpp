/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_worker_group.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"

namespace ark::ets {
void FinalizationRegistryManager::CleanupJobFinished()
{
    // Atomic with acq_rel order reason: other threads should see correct value
    [[maybe_unused]] uint32_t oldCnt = finRegCleanupJobsCount_.fetch_sub(1, std::memory_order_acq_rel);
    [[maybe_unused]] auto *executionCtx = EtsExecutionContext::GetCurrent();
    LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__
                        << "] FinReg cleanup job finished | Worker domain: " << GetExecCtxDomain(executionCtx)
                        << " | Worker id: "
                        << JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetWorker()->GetId();

    ASSERT(oldCnt > 0);
}

JobWorkerThreadDomain FinalizationRegistryManager::GetExecCtxDomain(EtsExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    auto *worker = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetWorker();
    if (worker->IsMainWorker()) {
        return JobWorkerThreadDomain::MAIN;
    }
    if (worker->InExclusiveMode()) {
        return JobWorkerThreadDomain::EXCLUSIVE;
    }
    return JobWorkerThreadDomain::GENERAL;
}

// NO_THREAD_SAFETY_ANALYSIS is used here because there is no need to lock `finalizationList_` in this method, since
// this method is not called during the execution of `StartCleanupCoroIfNeeded`.
NO_THREAD_SAFETY_ANALYSIS void FinalizationRegistryManager::Enqueue(EtsFinalizationRegistry *finReg)
{
    auto workerId = finReg->GetWorkerId();
    auto workerDomain = finReg->GetWorkerDomain();
    auto pred = [workerId, workerDomain](const EtsFinalizationRegistry *fr) {
        if (workerDomain != fr->GetWorkerDomain()) {
            return false;
        }
        if (workerDomain == JobWorkerThreadDomain::EXCLUSIVE) {
            return workerId == fr->GetWorkerId();
        }
        return true;
    };
    auto it = std::find_if(finalizationList_.begin(), finalizationList_.end(), pred);
    if (it == finalizationList_.end()) {
        LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__ << "] Adding new FinReg (" << finReg
                            << ") to finalization list | Worker domain: " << finReg->GetWorkerDomain()
                            << " | Worker id: " << finReg->GetWorkerId();
        finalizationList_.push_back(finReg);
    } else {
        EtsFinalizationRegistry *head = *it;
        finReg->SetNextFinReg(head);
        *it = finReg;
    }
}

// NO_THREAD_SAFETY_ANALYSIS is used here because there is no need to lock `finalizationList_` in this method, since
// this method is not called during the execution of `StartCleanupCoroIfNeeded`.
NO_THREAD_SAFETY_ANALYSIS void FinalizationRegistryManager::UpdateAndSweep(const ReferenceUpdater &updater)
{
    auto it = finalizationList_.begin();
    while (it != finalizationList_.end()) {
        EtsFinalizationRegistry *head = UpdateAndSweepFinRegChain(*it, updater);
        if (head == nullptr) {
            LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__ << "] Remove FinReg (" << *it
                                << ") from finalization list | Worker domain: " << (*it)->GetWorkerDomain()
                                << " | Worker id: " << (*it)->GetWorkerId();
            finalizationList_.erase(it++);
        } else {
            *it = head;
            ++it;
        }
    }
}

EtsFinalizationRegistry *FinalizationRegistryManager::UpdateAndSweepFinRegChain(EtsFinalizationRegistry *cur,
                                                                                const ReferenceUpdater &updater)
{
    EtsFinalizationRegistry *head = nullptr;
    EtsFinalizationRegistry *prev = nullptr;
    while (cur != nullptr) {
        auto *next = cur->GetNextFinReg();
        ObjectHeader *obj = cur->GetCoreType();
        if (updater(&obj) == ObjectStatus::ALIVE_OBJECT) {
            cur = EtsFinalizationRegistry::FromCoreType(obj);
            if (head == nullptr) {
                head = cur;
            } else {
                prev->SetNextFinReg(cur);
            }
            prev = cur;
        }
        cur = next;
    }
    return head;
}

bool FinalizationRegistryManager::CanCleanup(JobWorkerThread::Id currentId, JobWorkerThreadDomain currentDomain,
                                             JobWorkerThread::Id id, JobWorkerThreadDomain domain)
{
    if (domain == JobWorkerThreadDomain::MAIN) {
        return currentDomain == JobWorkerThreadDomain::MAIN;
    }
    if (domain == JobWorkerThreadDomain::GENERAL) {
        return currentDomain == JobWorkerThreadDomain::MAIN || currentDomain == JobWorkerThreadDomain::GENERAL;
    }
    if (domain == JobWorkerThreadDomain::EXCLUSIVE) {
        return (currentDomain == JobWorkerThreadDomain::EXCLUSIVE) && (currentId == id);
    }
    return false;
}

bool FinalizationRegistryManager::UpdateFinRegJobsCountAndCheckIfCleanupNeeded()
{
    // Atomic with acquire order reason: getting correct value
    uint32_t cnt = finRegCleanupJobsCount_.load(std::memory_order_acquire);
    uint32_t oldCnt = cnt;
    // Atomic with acq_rel order reason: sync for counter
    while (cnt < MAX_FINREG_CLEANUP_JOBS && !finRegCleanupJobsCount_.compare_exchange_weak(
                                                // CC-OFFNXT(G.FMT.06-CPP) project code style
                                                cnt, cnt + 1U, std::memory_order_acq_rel, std::memory_order_acquire)) {
        oldCnt = cnt;
    }
    LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__
                        << "] Check if we can to launch FinReg "
                           "cleanup. Max cleanup jobs : "
                        << MAX_FINREG_CLEANUP_JOBS << " | Current count of jobs running cleanup: " << oldCnt;
    return oldCnt < MAX_FINREG_CLEANUP_JOBS;
}

static JobWorkerThreadGroup::Id GetGroupId(JobManager *jobMan, JobWorkerThreadDomain domain, JobWorkerThread::Id id)
{
    if (domain == JobWorkerThreadDomain::MAIN) {
        return JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::MAIN);
    }
    if (domain == JobWorkerThreadDomain::EXCLUSIVE) {
        return JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::EXCLUSIVE, {id});
    }
    return JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::GENERAL);
}

static std::pair<EtsFinRegNode *, EtsFinRegNode *> CombineLists(EtsFinalizationRegistry *finReg)
{
    EtsFinRegNode *head = finReg->GetFinalizationQueueHead();
    EtsFinRegNode *tail = finReg->GetFinalizationQueueTail();
    finReg->SetFinalizationQueueHead(nullptr);
    finReg->SetFinalizationQueueTail(nullptr);
    auto *nextFinReg = finReg->GetNextFinReg();
    finReg->SetNextFinReg(nullptr);
    finReg = nextFinReg;
    while (finReg != nullptr) {
        EtsFinRegNode *newHead = finReg->GetFinalizationQueueHead();
        EtsFinRegNode *newTail = finReg->GetFinalizationQueueTail();
        finReg->SetFinalizationQueueHead(nullptr);
        finReg->SetFinalizationQueueTail(nullptr);
        tail->SetNext(newHead);
        tail = newTail;
        nextFinReg = finReg->GetNextFinReg();
        finReg->SetNextFinReg(nullptr);
        finReg = nextFinReg;
    }
    return std::make_pair(head, tail);
}

void FinalizationRegistryManager::LaunchCleanupJobIfNeeded(EtsExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    os::memory::LockHolder lock(cleanupMutex_);
    auto *jobExecCtx = JobExecutionContext::CastFromMutator(executionCtx->GetMT());
    auto *jobMan = jobExecCtx->GetManager();
    auto currentWorkerId = jobExecCtx->GetWorker()->GetId();
    auto currentWorkerDomain = GetExecCtxDomain(executionCtx);
    LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__
                        << "] GC finished, check if FinReg cleanup job needed | Worker domain: " << currentWorkerDomain
                        << " | Worker id: " << currentWorkerId << " | execution ctx id: " << jobExecCtx->GetId();
    // Check are there node that can be finalized by the current worker
    auto pred = [currentWorkerId, currentWorkerDomain](const EtsFinalizationRegistry *finReg) {
        return CanCleanup(currentWorkerId, currentWorkerDomain, finReg->GetWorkerId(), finReg->GetWorkerDomain());
    };
    auto it = std::find_if(finalizationList_.begin(), finalizationList_.end(), pred);
    if (it == finalizationList_.end()) {
        LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__ << "] Nothing to clean up";
        return;
    }
    auto [head, tail] = CombineLists(*it);
    auto toDelete = it;
    ++it;
    LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__ << "] FinReg deleted from finalization list: ("
                        << *toDelete << ")";
    finalizationList_.erase(toDelete);
    it = std::find_if(
        it, finalizationList_.end(), [currentWorkerId, currentWorkerDomain](const EtsFinalizationRegistry *finReg) {
            return CanCleanup(currentWorkerId, currentWorkerDomain, finReg->GetWorkerId(), finReg->GetWorkerDomain());
        });
    if (it != finalizationList_.end()) {
        auto [head2, tail2] = CombineLists(*it);
        LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__ << "] FinReg deleted from finalization list: (" << *it
                            << ")";
        finalizationList_.erase(it);
        tail->SetNext(head2);
        tail = tail2;
    }

    if (UpdateFinRegJobsCountAndCheckIfCleanupNeeded()) {
        ASSERT(head != nullptr);
        auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, jobMan);
        Method *cleanup = PlatformTypes(vm_)->coreFinalizationRegistryExecCleanup->GetPandaMethod();
        auto workerId = jobExecCtx->GetWorker()->GetId();
        auto workerDomain = GetExecCtxDomain(executionCtx);
        auto groupId = GetGroupId(jobMan, workerDomain, workerId);
        auto args = PandaVector<Value> {Value(EtsObject::ToCoreType(head))};
        LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__
                            << "] Launch FinReg cleanup job | Worker domain: " << workerDomain
                            << " | Worker id: " << workerId << " | execution ctx id: " << jobExecCtx->GetId();
        LOG(DEBUG, RUNTIME) << "[FinRegManager::" << __FUNCTION__
                            << "] FinReg cleanup in progress, count: "
                            // Atomic with relaxed order reason: no sync depends
                            << finRegCleanupJobsCount_.load(std::memory_order_relaxed);
        auto epInfo = Job::ManagedEntrypointInfo {event, cleanup, std::move(args)};
        auto *job = jobMan->CreateJob("FinReg cleanup", std::move(epInfo));
        auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId});
        if (UNLIKELY(launchResult == LaunchResult::RESOURCE_LIMIT_EXCEED)) {
            jobMan->DestroyJob(job);
            // Atomic with acq_rel order reason: sync context dependes
            finRegCleanupJobsCount_.fetch_sub(1, std::memory_order_acq_rel);
        }
    }
}
}  // namespace ark::ets
