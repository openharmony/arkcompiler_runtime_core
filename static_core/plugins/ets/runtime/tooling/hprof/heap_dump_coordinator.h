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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_HEAP_DUMP_COORDINATOR_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_HEAP_DUMP_COORDINATOR_H

#include "libarkbase/os/mutex.h"
#include "profiler/heap_dump.h"
#include "plugins/ets/runtime/tooling/hprof/session/object_id_map.h"
#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"

#include <atomic>
#include <memory>
#include <unordered_map>

namespace ark {
class PandaVM;
}  // namespace ark

namespace arkplatform {
class STSVMInterface;
}  // namespace arkplatform

namespace ark::tooling::hprof {

class CommonWriter;

using common::dump::AbstractDumper;
using common::dump::DumpExecutionMode;
using common::dump::DumpRequest;
using common::dump::DumpResult;
using common::dump::DumpScope;
using common::dump::DumpStatistics;

/** Runtime participants owned by one dump invocation. */
struct DumpParticipants {
    std::unique_ptr<AbstractDumper> dynamicDumper;
    std::unique_ptr<AbstractDumper> staticDumper;
    arkplatform::STSVMInterface *stsInterface = nullptr;  // Not owned; valid for the session.
};

/**
 * Process-wide owner of static and hybrid heap dump sessions.
 *
 * Every new-format dump contains, or is coordinated by, the ETS runtime. The
 * dynamic runtime contributes a dumper through the common interface only in a
 * hybrid process; pure dynamic dumps continue to use their legacy path.
 */
class HeapDumpCoordinator {
public:
    static HeapDumpCoordinator &GetInstance();

    ObjectIdMap &GetObjectIdMap()
    {
        return objectIdMap_;
    }

    StringIdPool &GetStringIdPool()
    {
        return stringPool_;
    }

    static void ResetOOMDumpStateForTest();
    static bool TryBeginOOMDump();

    /**
     * Create the static participant when requested and execute one dump
     * session. Hybrid-only XGC and XRef operations use the STS VM interface.
     */
    bool Dump(const DumpRequest &request, ark::PandaVM *pandaVm, std::unique_ptr<AbstractDumper> dynamicDumper,
              arkplatform::STSVMInterface *stsInterface, bool dumpStaticHeap);

    /** Low-level entry used by focused coordinator tests. */
    bool DumpBinarySeparate(const DumpRequest &request, DumpParticipants participants);

private:
    using XRefMap = std::unordered_multimap<uint64_t, uint64_t>;

    struct XRefSnapshot {
        XRefMap jsToEts;
        XRefMap etsToJs;
        bool enabled = false;
    };

    HeapDumpCoordinator() = default;

    void PrepareSharedState();
    void FreezeSharedState();
    void TriggerGC(const DumpRequest &request, DumpParticipants &participants);
    void PrepareParticipants(DumpParticipants &participants);
    XRefSnapshot CollectXRefs(const DumpRequest &request, const DumpParticipants &participants);

    bool ExecuteDumpWithFork(const DumpRequest &request, DumpParticipants &participants, const XRefSnapshot &xrefs);
    bool ExecuteSeparateDumpParallel(DumpParticipants &participants, const XRefSnapshot &xrefs);
    bool ExecuteSeparateDumpSequential(DumpParticipants &participants, const XRefSnapshot &xrefs);
    bool CompleteSeparateDump(DumpParticipants &participants, const DumpResult &dynamicResult, DumpResult staticResult,
                              const XRefSnapshot &xrefs);

    void WriteXRefAndSummary(CommonWriter *writer, const DumpStatistics &statistics,
                             const DumpParticipants &participants, const XRefSnapshot &xrefs);
    void WriteXRefs(CommonWriter *writer, const DumpParticipants &participants, const XRefSnapshot &xrefs);

    StringIdPool stringPool_;
    ObjectIdMap objectIdMap_;

    static std::atomic<bool> oomDumpTriggered_;
    os::memory::Mutex dumpMutex_;
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_HEAP_DUMP_COORDINATOR_H
