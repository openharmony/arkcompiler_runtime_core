/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_TOOLING_HPROF_HEAP_DUMP_H
#define PANDA_RUNTIME_TOOLING_HPROF_HEAP_DUMP_H

#include "hybrid/hybrid_heap_snapshot_info.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/mem/gc/gc.h"

#include <functional>

namespace ark {
class Field;
class PandaVM;
}  // namespace ark

namespace ark::tooling::hprof {

/**
 * Provides static-side heap data extraction for hybrid heapdump.
 * Contains only runtime-agnostic code (no plugin/ETS dependencies).
 */
class HeapDump {
public:
    HeapDump() = default;
    ~HeapDump() = default;
    NO_COPY_SEMANTIC(HeapDump);
    NO_MOVE_SEMANTIC(HeapDump);

    static void ForceFullGC(PandaVM *vm);
    static std::vector<arkplatform::NodeInfo> GetAllEtsObjects(PandaVM *vm);
    static void IterateAllObjects(PandaVM *vm, const std::function<void(uint64_t)> &callback);
    using WeakEdgeChecker = std::function<bool(ObjectHeader *, const Field &)>;

    static void DumpReferences(uint64_t etsAddr, std::vector<arkplatform::EdgeInfo> &edges,
                               const WeakEdgeChecker &checker = nullptr);

    // Edge extraction helpers
    static void DumpObjectFields(ObjectHeader *object, std::vector<arkplatform::EdgeInfo> &edges,
                                 const WeakEdgeChecker &checker = nullptr);
    static void DumpArrayElements(ObjectHeader *object, Class *cls, std::vector<arkplatform::EdgeInfo> &edges);
    static void DumpClassStaticFields(ObjectHeader *object, std::vector<arkplatform::EdgeInfo> &edges,
                                      const WeakEdgeChecker &checker = nullptr);

    // Type helpers (public for use by plugin layer)
    static arkplatform::StaticNodeType MapToStaticNodeType(Class *cls);
    static std::string GetNodeName(ObjectHeader *object);
    static size_t GetObjectSize(ObjectHeader *object);
    static arkplatform::NodeInfo ObjectToNodeInfo(ObjectHeader *object);

private:
    static bool IsWeakReferentEdge(ObjectHeader *object, const Field &field, const WeakEdgeChecker &checker);
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_RUNTIME_TOOLING_HPROF_HEAP_DUMP_H
