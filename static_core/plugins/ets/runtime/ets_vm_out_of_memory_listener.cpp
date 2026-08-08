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

#include "plugins/ets/runtime/ets_vm_out_of_memory_listener.h"

#include "profiler/heap_dump.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/time.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/include/oom_stats.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/tooling/hprof/heap_dump_coordinator.h"

#if defined(PANDA_JS_ETS_HYBRID_MODE)
#include "plugins/ets/runtime/interop_js/interop_context.h"
#endif

#include <new>
#include <unistd.h>
#include <utility>

namespace ark::ets {

namespace {

using common::dump::AbstractDumper;
using common::dump::DumpExecutionMode;
using common::dump::DumpReason;
using common::dump::DumpRequest;
using tooling::hprof::HeapDumpCoordinator;

DumpRequest CreateStaticOOMDumpRequest()
{
    DumpRequest request;
    request.reason = DumpReason::STATIC_OOM;
    request.policy.triggerGC = false;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.identity = {static_cast<int32_t>(getpid()), static_cast<int32_t>(ark::os::thread::GetCurrentThreadId()),
                        ark::time::GetCurrentTimeInMillis(true)};
    return request;
}

#if defined(PANDA_JS_ETS_HYBRID_MODE)
using common::dump::DumpScope;

struct DynamicParticipantSelection {
    std::unique_ptr<AbstractDumper> dumper;
    arkplatform::STSVMInterface *stsInterface = nullptr;
    DumpScope scope = DumpScope::VM;
    std::shared_ptr<void> sharedStateLease;
};

DynamicParticipantSelection SelectDynamicParticipant(PandaEtsVM *vm, const DumpRequest &request)
{
    DynamicParticipantSelection selection;
    selection.sharedStateLease = interop::js::InteropCtx::TryAcquireSharedEtsVmState();
    if (selection.sharedStateLease == nullptr) {
        return selection;
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *interopCtx = executionCtx == nullptr ? nullptr : interop::js::InteropCtx::Current(executionCtx);
    if (interopCtx != nullptr && interopCtx->GetECMAInterface() != nullptr) {
        selection.dumper = interopCtx->GetECMAInterface()->CreateHeapDumper(request);
        selection.stsInterface = interopCtx->GetSTSVMInterface();
    }
    if (selection.dumper != nullptr) {
        return selection;
    }

    auto processRequest = request;
    processRequest.policy.scope = DumpScope::PROCESS;
    auto *mainInteropCtx = interop::js::InteropCtx::GetMainInteropContext(vm);
    auto *mainEcmaInterface = mainInteropCtx == nullptr ? nullptr : mainInteropCtx->GetECMAInterface();
    if (mainEcmaInterface != nullptr) {
        selection.dumper = mainEcmaInterface->CreateHeapDumper(processRequest);
        selection.stsInterface = mainInteropCtx->GetSTSVMInterface();
    }
    selection.scope = DumpScope::PROCESS;
    return selection;
}
#endif

}  // namespace

EtsVmOutOfMemoryListener::EtsVmOutOfMemoryListener(PandaEtsVM *vm)
    : vm_(vm), oomDumpReserve_(new (std::nothrow) OOMDumpReserve)
{
    if (oomDumpReserve_ == nullptr) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Failed to allocate OOM dump reserve";
    }
}

void EtsVmOutOfMemoryListener::OutOfMemory(size_t size, SpaceType spaceType)
{
    mem::HeapManager *heapManager = vm_->GetHeapManager();
    auto *memStats = vm_->GetMemStats();
    const size_t activeMemory = (memStats != nullptr) ? memStats->GetFootprintHeap() : 0U;
    ark::oom_stats::OomNotifier::NotifyBeforeManagedOom(heapManager->GetMaxMemory(), activeMemory, size,
                                                        Runtime::GetCurrent()->GetProcessPackageName(), spaceType);

    TriggerOOMDump();
}

void EtsVmOutOfMemoryListener::TriggerOOMDump()
{
    auto &coordinator = HeapDumpCoordinator::GetInstance();

    if (!coordinator.TryBeginOOMDump()) {
        LOG(INFO, RUNTIME) << "[HybDump][Sta] OOM dump skipped: already triggered";
        return;
    }
    LOG(INFO, RUNTIME) << "[HybDump][Sta] OOM dump begin";

    // The OOM dump allocates native bookkeeping data. Releasing the one-shot
    // reserve here gives that work a small, deterministic amount of headroom.
    oomDumpReserve_.reset();

    auto request = CreateStaticOOMDumpRequest();
    std::unique_ptr<AbstractDumper> dynamicDumper;
    arkplatform::STSVMInterface *stsInterface = nullptr;

#if defined(PANDA_JS_ETS_HYBRID_MODE)
    auto dynamicSelection = SelectDynamicParticipant(vm_, request);
    request.policy.scope = dynamicSelection.scope;
    dynamicDumper = std::move(dynamicSelection.dumper);
    stsInterface = dynamicSelection.stsInterface;
#endif

#if defined(PANDA_JS_ETS_HYBRID_MODE)
    if (dynamicSelection.sharedStateLease != nullptr && dynamicDumper == nullptr) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Dynamic dumper unavailable, continuing with static dump";
    }
#endif
    (void)coordinator.Dump(request, vm_, std::move(dynamicDumper), stsInterface, true);
}
}  // namespace ark::ets
