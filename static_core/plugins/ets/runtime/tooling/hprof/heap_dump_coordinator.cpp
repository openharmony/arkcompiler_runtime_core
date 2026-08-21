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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/tooling/hprof/heap_dump_coordinator.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/tooling/hprof/session/common_writer.h"
#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "plugins/ets/runtime/tooling/hprof/static_dump.h"

#if defined(PANDA_JS_ETS_HYBRID_MODE)
#include "hybrid/sts_vm_interface.h"
#endif

#include <cerrno>
#include <csignal>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <utility>
#if defined(__linux__)
#include <sys/wait.h>
#endif

namespace ark::tooling::hprof {

static constexpr uint8_t DUMP_SUCCESS = 0;
static constexpr uint8_t DUMP_FORK_FAILED = 1;
static constexpr uint8_t DUMP_FAILED_TO_WAIT = 2;
static constexpr uint8_t DUMP_WAIT_TIMEOUT = 3;

namespace {

#if defined(PANDA_JS_ETS_HYBRID_MODE)
bool HasXRefMapping(const std::unordered_multimap<uint64_t, uint64_t> &mappings, uint64_t source, uint64_t target)
{
    auto [begin, end] = mappings.equal_range(source);
    for (auto it = begin; it != end; ++it) {
        if (it->second == target) {
            return true;
        }
    }
    return false;
}
#endif

}  // namespace

std::atomic<bool> HeapDumpCoordinator::oomDumpTriggered_ {false};

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
HeapDumpCoordinator &HeapDumpCoordinator::GetInstance()
{
    static HeapDumpCoordinator instance;
    return instance;
}

void HeapDumpCoordinator::ResetOOMDumpStateForTest()
{
    // Atomic with relaxed order reason: tests reset the flag only when no dump attempt is running.
    oomDumpTriggered_.store(false, std::memory_order_relaxed);
}

bool HeapDumpCoordinator::TryBeginOOMDump()
{
    bool expected = false;
    return oomDumpTriggered_.compare_exchange_strong(expected, true);
}

bool HeapDumpCoordinator::Dump(const DumpRequest &request, ark::PandaVM *pandaVm,
                               std::unique_ptr<AbstractDumper> dynamicDumper, arkplatform::STSVMInterface *stsInterface,
                               bool dumpStaticHeap)
{
    auto staticDumper = dumpStaticHeap ? StaticDump::Create(pandaVm, &stringPool_, &objectIdMap_, request) : nullptr;
    if (dumpStaticHeap && staticDumper == nullptr) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Static dumper creation failed";
        return false;
    }
    DumpParticipants participants {std::move(dynamicDumper), std::move(staticDumper), stsInterface};
    return DumpBinarySeparate(request, std::move(participants));
}

// ---------------------------------------------------------------------------
// DumpBinarySeparate - separate-files mode
// ---------------------------------------------------------------------------
bool HeapDumpCoordinator::DumpBinarySeparate(const DumpRequest &request, DumpParticipants participants)
{
    os::memory::LockHolder dumpLock(dumpMutex_);

    if (participants.dynamicDumper == nullptr && participants.staticDumper == nullptr) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Dump skipped: no participant";
        return false;
    }
    PrepareSharedState();
    TriggerGC(request, participants);
    PrepareParticipants(participants);
    XRefSnapshot xrefs = CollectXRefs(request, participants);
    bool success = false;
    if (request.policy.executionMode == DumpExecutionMode::IN_PROCESS) {
        success = ExecuteSeparateDumpParallel(participants, xrefs);
    } else {
        success = ExecuteDumpWithFork(request, participants, xrefs);
    }
    participants.staticDumper.reset();
    participants.dynamicDumper.reset();
    FreezeSharedState();
    return success;
}

// ---------------------------------------------------------------------------
// WaitChildProcess - wait for forked child with timeout, kill on expiry
// ---------------------------------------------------------------------------
#if defined(__linux__)
constexpr unsigned int WAIT_STATUS_SIGNAL_MASK = 0x7FU;
constexpr unsigned int WAIT_STATUS_EXIT_CODE_MASK = 0xFF00U;
constexpr unsigned int WAIT_STATUS_EXIT_CODE_SHIFT = 8U;

static void LogChildProcessResult(pid_t childPid, int status)
{
    auto statusValue = static_cast<unsigned int>(status);
    auto signal = statusValue & WAIT_STATUS_SIGNAL_MASK;
    if (signal == 0U) {
        auto exitStatus = (statusValue & WAIT_STATUS_EXIT_CODE_MASK) >> WAIT_STATUS_EXIT_CODE_SHIFT;
        if (exitStatus != 0) {
            LOG(ERROR, RUNTIME) << "[HybDump][Sta] Child process exited: child=" << childPid
                                << ", status=" << exitStatus;
        }
        return;
    }

    if (signal < WAIT_STATUS_SIGNAL_MASK) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Child process terminated: child=" << childPid << ", signal=" << signal;
        return;
    }

    LOG(ERROR, RUNTIME) << "[HybDump][Sta] Child process returned unexpected status: child=" << childPid
                        << ", status=" << status;
}

static void WaitChildProcess(pid_t childPid, const std::function<void(uint8_t)> &callback)
{
    constexpr int DUMP_TIMEOUT_SECONDS = 300;
    constexpr int POLL_INTERVAL_MICROSECONDS = 100000;
    time_t deadline = time(nullptr) + DUMP_TIMEOUT_SECONDS;

    while (time(nullptr) <= deadline) {
        int status = 0;
        pid_t p = waitpid(childPid, &status, WNOHANG);
        if (p < 0) {
            int waitError = errno;
            if (waitError == EINTR) {
                continue;
            }
            LOG(ERROR, RUNTIME) << "[HybDump][Sta] Child process wait failed: child=" << childPid
                                << ", errno=" << waitError;
            if (callback) {
                callback(DUMP_FAILED_TO_WAIT);
            }
            return;
        }
        if (p == childPid) {
            LogChildProcessResult(childPid, status);
            if (callback) {
                callback(DUMP_SUCCESS);
            }
            return;
        }
        usleep(POLL_INTERVAL_MICROSECONDS);
    }

    LOG(ERROR, RUNTIME) << "[HybDump][Sta] Child process timed out: child=" << childPid
                        << ", timeout=" << DUMP_TIMEOUT_SECONDS << "s";
    // kill is used to terminate a forked child process that has hung for 300s;
    // this is a safety mechanism, not arbitrary process termination.
    // Only called on our own child PID.
    kill(childPid, SIGKILL);
    waitpid(childPid, nullptr, 0);
    if (callback) {
        callback(DUMP_WAIT_TIMEOUT);
    }
}
#endif  // defined(__linux__)

void HeapDumpCoordinator::PrepareSharedState()
{
    stringPool_.Unfreeze();
    // Do not reset objectIdMap_ here. StaticDump drives per-round liveness
    // through PrepareRound(), MarkLive(), and PruneDead(), so surviving objects
    // retain their node IDs across dumps.
    objectIdMap_.Unfreeze();
}

void HeapDumpCoordinator::FreezeSharedState()
{
    stringPool_.Freeze();
    objectIdMap_.Freeze();
}

void HeapDumpCoordinator::TriggerGC(const DumpRequest &request, DumpParticipants &participants)
{
    if (!request.policy.triggerGC) {
        return;
    }

    AbstractDumper *dynamicDumper = participants.dynamicDumper.get();
    AbstractDumper *staticDumper = participants.staticDumper.get();
#if defined(PANDA_JS_ETS_HYBRID_MODE)
    bool isHybrid = participants.stsInterface != nullptr && dynamicDumper != nullptr && staticDumper != nullptr;
    if (isHybrid && participants.stsInterface->TriggerXGCAndWait()) {
        dynamicDumper->CompleteCrossRuntimeGC();
    }
#endif
    if (dynamicDumper != nullptr) {
        dynamicDumper->TriggerGC();
    }
    if (staticDumper != nullptr) {
        staticDumper->TriggerGC();
    }
}

void HeapDumpCoordinator::PrepareParticipants(DumpParticipants &participants)
{
    if (participants.dynamicDumper != nullptr) {
        participants.dynamicDumper->PrepareSession();
    }
    if (participants.staticDumper != nullptr) {
        participants.staticDumper->PrepareSession();
    }
}

HeapDumpCoordinator::XRefSnapshot HeapDumpCoordinator::CollectXRefs(const DumpRequest &request,
                                                                    const DumpParticipants &participants)
{
    XRefSnapshot xrefs;
#if defined(PANDA_JS_ETS_HYBRID_MODE)
    if (request.reason == common::dump::DumpReason::STATIC_OOM) {
        LOG(INFO, RUNTIME) << "[HybDump][Sta] XRef skipped: static OOM";
        return xrefs;
    }

    AbstractDumper *dynamicDumper = participants.dynamicDumper.get();
    if (participants.stsInterface == nullptr || dynamicDumper == nullptr || participants.staticDumper == nullptr) {
        return xrefs;
    }

    uintptr_t ecmaVM = reinterpret_cast<uintptr_t>(dynamicDumper->GetCurrentVM());
    if (request.policy.scope != DumpScope::PROCESS && ecmaVM == 0) {
        return xrefs;
    }

    xrefs.enabled = true;
    uintptr_t vmFilter = request.policy.scope == DumpScope::PROCESS ? 0 : ecmaVM;
    participants.stsInterface->GetXRefMaps(vmFilter, xrefs.jsToEts, xrefs.etsToJs);
#else
    (void)request;
    (void)participants;
#endif
    return xrefs;
}

// ---------------------------------------------------------------------------
// Private: Execution paths
// ---------------------------------------------------------------------------
bool HeapDumpCoordinator::ExecuteDumpWithFork(const DumpRequest &request, DumpParticipants &participants,
                                              const XRefSnapshot &xrefs)
{
#if defined(__linux__)
    // faultloggerd verifies the request PID against the socket peer PID. Open
    // each output in the parent and inherit it across fork so the file remains
    // associated with the process that initiated the dump.
    if ((participants.dynamicDumper != nullptr && !participants.dynamicDumper->AcquireOutput()) ||
        (participants.staticDumper != nullptr && !participants.staticDumper->AcquireOutput())) {
        return false;
    }

    pid_t childPid = fork();
    if (childPid < 0) {
        int forkError = errno;
        // fork() can fail under memory pressure (notably the OOM path); fall back to
        // in-process so the dump isn't lost. No child was created, so the suspended
        // threads and prepared dumpers still match the in-process dump entry.
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Fork failed, falling back to in-process dump: errno=" << forkError
                              << ", error=" << strerror(forkError);
        bool success = ExecuteSeparateDumpParallel(participants, xrefs);
        if (request.completionCallback) {
            request.completionCallback(success ? DUMP_SUCCESS : DUMP_FORK_FAILED);
        }
        return success;
    }

    if (childPid == 0) {
        // Child: run dump, clean up, exit
        (void)os::thread::SetThreadName(os::thread::GetNativeHandle(), "binary_dump_process");
        if (participants.dynamicDumper != nullptr) {
            participants.dynamicDumper->PrepareForkChild();
        }
        // The dump must use the inherited VM snapshot and therefore cannot
        // exec. Keep all participant work on the thread that called fork;
        // creating worker threads here can enter copied synchronization state
        // whose owner threads disappeared at fork.
        ExecuteSeparateDumpSequential(participants, xrefs);
        // Participant destruction completes output cleanup in the child.
        // Runtime-specific scopes must be fork-aware and only restore runtime
        // state in the process that created them.
        participants.staticDumper.reset();
        participants.dynamicDumper.reset();
        _exit(0);
    }

    // Parent: spawn detached wait thread, resume and return immediately
    std::thread waitThread(WaitChildProcess, childPid, request.completionCallback);
    waitThread.detach();
    return true;
#else
    // fork() is unavailable on non-Linux platforms (e.g. Windows/mingw); fall
    // back to in-process execution.
    return ExecuteSeparateDumpParallel(participants, xrefs);
#endif
}

// ---------------------------------------------------------------------------
// Private: Core parallel execution
// ---------------------------------------------------------------------------
bool HeapDumpCoordinator::ExecuteSeparateDumpParallel(DumpParticipants &participants, const XRefSnapshot &xrefs)
{
    AbstractDumper *dynamicDumper = participants.dynamicDumper.get();
    AbstractDumper *staticDumper = participants.staticDumper.get();

    // Each dumper owns its complete side lifecycle. Dynamic runs on a worker
    // thread while static runs on this thread so heap traversal overlaps.
    DumpResult dynamicResult;
    std::thread dynamicThread;
    if (dynamicDumper != nullptr) {
        dynamicThread = std::thread([dynamicDumper, &dynamicResult]() { dynamicResult = dynamicDumper->Dump(); });
    } else {
        dynamicResult.success = true;
    }

    DumpResult staticResult = staticDumper == nullptr ? DumpResult {{0, 0}, true} : staticDumper->Dump();

    // XRef reads the dynamic entry-id map populated during the dynamic dump.
    if (dynamicThread.joinable()) {
        dynamicThread.join();
    }

    return CompleteSeparateDump(participants, dynamicResult, staticResult, xrefs);
}

bool HeapDumpCoordinator::ExecuteSeparateDumpSequential(DumpParticipants &participants, const XRefSnapshot &xrefs)
{
    AbstractDumper *dynamicDumper = participants.dynamicDumper.get();
    AbstractDumper *staticDumper = participants.staticDumper.get();

    DumpResult dynamicResult = dynamicDumper == nullptr ? DumpResult {{0, 0}, true} : dynamicDumper->Dump();
    DumpResult staticResult = staticDumper == nullptr ? DumpResult {{0, 0}, true} : staticDumper->Dump();
    return CompleteSeparateDump(participants, dynamicResult, staticResult, xrefs);
}

bool HeapDumpCoordinator::CompleteSeparateDump(DumpParticipants &participants, const DumpResult &dynamicResult,
                                               DumpResult staticResult, const XRefSnapshot &xrefs)
{
    // Append XRef + heap-summary to the static stream. The static OutputStream
    // is owned by the static dumper and remains alive until participants are released.
    // A fresh CommonWriter appends XRef and summary before the stream is
    // flushed. File order is unchanged:
    // header -> pool -> records -> class_dump -> xref -> summary.
    if (staticResult.success && participants.staticDumper != nullptr) {
        auto *stream = static_cast<OutputStream *>(participants.staticDumper->GetOutput());
        if (stream != nullptr) {
            CommonWriter cw(stream);
            WriteXRefAndSummary(&cw, staticResult.statistics, participants, xrefs);
            staticResult.success = stream->Flush();
        }
    }
    return dynamicResult.success && staticResult.success;
}

// ---------------------------------------------------------------------------
// Private: Writer/summary helpers
// ---------------------------------------------------------------------------
void HeapDumpCoordinator::WriteXRefAndSummary(CommonWriter *writer, const DumpStatistics &statistics,
                                              const DumpParticipants &participants, const XRefSnapshot &xrefs)
{
    if (xrefs.enabled) {
        WriteXRefs(writer, participants, xrefs);
    }
    writer->WriteHeapSummary(statistics.objectCount, statistics.classCount, objectIdMap_.CountOf<Language::DYNAMIC>(),
                             objectIdMap_.CountOf<Language::STATIC>());
}

void HeapDumpCoordinator::WriteXRefs(CommonWriter *writer, const DumpParticipants &participants,
                                     const XRefSnapshot &xrefs)
{
#if defined(PANDA_JS_ETS_HYBRID_MODE)
    // Symmetric to the static side: etsAddr -> staNodeId via objectIdMap_, and
    // jsAddr -> dynNodeId via the dynamic dumper's entry-id map. The XRef record
    // thus carries two nodeIds (not a raw address), so the merger - which indexes
    // dynamic nodes by nodeId - can resolve both endpoints.
    AbstractDumper *dynamic = participants.dynamicDumper.get();
    writer->BeginRecord(TAG_XREF_EDGE);
    for (const auto &[jsAddr, etsAddr] : xrefs.jsToEts) {
        ObjectIdMap::NodeId etsNodeId = objectIdMap_.Find(etsAddr);
        if (etsNodeId == 0) {  // Unregistered static address - skip
            continue;
        }
        uint32_t jsNodeId = (dynamic != nullptr) ? dynamic->GetNodeId(jsAddr) : 0;
        if (jsNodeId == 0) {  // JS object not in the dynamic dump - skip
            continue;
        }
        if (HasXRefMapping(xrefs.etsToJs, etsAddr, jsAddr)) {
            writer->WriteXRefEdge(jsNodeId, etsNodeId, XREF_DIR_BIDIR);
        } else {
            writer->WriteXRefEdge(jsNodeId, etsNodeId, XREF_DIR_DYN_TO_STA);
        }
    }
    for (const auto &[etsAddr, jsAddr] : xrefs.etsToJs) {
        if (HasXRefMapping(xrefs.jsToEts, jsAddr, etsAddr)) {
            continue;
        }
        ObjectIdMap::NodeId etsNodeId = objectIdMap_.Find(etsAddr);
        if (etsNodeId == 0) {  // Unregistered static address - skip
            continue;
        }
        uint32_t jsNodeId = (dynamic != nullptr) ? dynamic->GetNodeId(jsAddr) : 0;
        if (jsNodeId == 0) {  // JS object not in the dynamic dump - skip
            continue;
        }
        writer->WriteXRefEdge(jsNodeId, etsNodeId, XREF_DIR_STA_TO_DYN);
    }
    writer->EndRecord();
#else
    (void)writer;
    (void)participants;
    (void)xrefs;
#endif
}
}  // namespace ark::tooling::hprof
