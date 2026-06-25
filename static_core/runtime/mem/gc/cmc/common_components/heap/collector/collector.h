/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <unordered_set>
#include <vector>

#include "common_components/base/globals.h"
#include "common_interfaces/objects/base_object.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/heap/collector/gc_stats.h"
#include "common_interfaces/thread/mutator.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"
#include "common_interfaces/heap/visitor.h"

#include "libarkbase/utils/logger.h"

namespace ark::common_vm {
enum CollectorType {
    NO_COLLECTOR = 0,  // No Collector
    PROXY_COLLECTOR,   // Proxy of Collector
    COPY_COLLECTOR,    // Regional-Copying GC
    SMOOTH_COLLECTOR,  // wgc
    COLLECTOR_TYPE_COUNT,
};

// Central garbage identification algorithm.
class Collector {
public:
    Collector() = default;
    virtual ~Collector() = default;

    // Initializer and finalizer.
    virtual void Init() = 0;
    virtual void Fini() {}
    const char *GetCollectorName() const;

    virtual void FixObjectRefFields(BaseObject *) const {}

    virtual void RunGarbageCollection(uint64_t, ark::GCTask &) = 0;

    virtual GCStats &GetGCStats()
    {
        LOG(FATAL, COMMON) << "Unresolved fatal";
        UNREACHABLE();
    }

    virtual BaseObject *ForwardObject(BaseObject *) = 0;

    virtual bool ShouldIgnoreRequest(GCRequest &quest) = 0;
    virtual bool IsFromObject(BaseObject *) const = 0;
    virtual bool IsUnmovableFromObject(BaseObject *) const = 0;
    virtual BaseObject *FindToVersion(BaseObject *obj) const = 0;

    virtual bool TryUpdateRefField(BaseObject *, RefField<> &, BaseObject *&) const = 0;
    virtual bool TryForwardRefField(BaseObject *, RefField<> &, BaseObject *&) const = 0;

    BaseObject *FindLatestVersion(BaseObject *obj) const
    {
        if (obj == nullptr) {
            return nullptr;
        }

        auto to = FindToVersion(obj);
        if (to != nullptr) {
            return to;
        }
        return obj;
    };

protected:
    CollectorType collectorType_ = CollectorType::NO_COLLECTOR;

private:
    ark::os::memory::RWLock vmIfacesLock_;

    friend class VerifyIterator;
    virtual void VisitRootsI(const RefFieldVisitor &visitor) = 0;
    virtual void VisitSTWRootsI(const RefFieldVisitor &visitor) = 0;
    virtual void VisitConcurrentRootsI(const RefFieldVisitor &visitor) = 0;
    virtual void VisitWeakRootsI(const mem::WeakRefFieldVisitor &visitor) = 0;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H
