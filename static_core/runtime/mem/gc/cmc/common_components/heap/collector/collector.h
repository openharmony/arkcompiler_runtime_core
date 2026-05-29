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
#include <set>
#include <vector>

#include "common_interfaces/objects/base_object.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/heap/collector/gc_stats.h"
#include "common_interfaces/thread/mutator.h"
#include "common_interfaces/base/runtime_param.h"
#include "common_interfaces/base_runtime.h"

namespace ark::common_vm {
// Central garbage identification algorithm.
class Collector {
public:
    Collector();
    virtual ~Collector() = default;

    static const char *GetGCPhaseName(GCPhase phase);

    // Initializer and finalizer.
    virtual void Init(const RuntimeParam &param) = 0;
    virtual void Fini() {}
    const char *GetCollectorName() const;

    // This pure virtual function implements the trigger of GC.
    // reason: Reason for GC.
    // async:  Trigger from unsafe context, e.g., holding a lock, in the middle of an allocation.
    //         In order to prevent deadlocks, async trigger only add one async gc task and will not block.
    void RequestGC(GCReason reason, bool async, GCType gcType, bool explicitRequest = false);

    virtual GCPhase GetGCPhase() const = 0;

    virtual void SetGCPhase(const GCPhase phase) = 0;

    virtual void FixObjectRefFields(BaseObject *) const {}

    virtual void RunGarbageCollection(uint64_t, GCReason, GCType) = 0;

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

    virtual void AddGCListener(GCListener *listener) = 0;
    virtual void RemoveGCListener(GCListener *listener) = 0;
    virtual void NotifyGCListeners(std::function<void(GCListener *)> notifier) = 0;

protected:
    virtual void RequestGCInternal(GCReason, bool, GCType, bool explicitRequest)
    {
        LOG(FATAL, COMMON) << "Unresolved fatal";
        UNREACHABLE();
    }
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H
