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

#include "common_interfaces/objects/base_object.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/heap/collector/gc_stats.h"
#include "common_interfaces/thread/mutator.h"
#include "runtime/include/mem/panda_containers.h"
#include "common_interfaces/base_runtime.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::common_vm {
enum CollectorType {
    NO_COLLECTOR = 0,  // No Collector
    PROXY_COLLECTOR,   // Proxy of Collector
    COPY_COLLECTOR,    // Regional-Copying GC
    SMOOTH_COLLECTOR,  // wgc
    COLLECTOR_TYPE_COUNT,
};

class GCListener {
public:
    GCListener() = default;

    virtual void OnGCStart(GCReason reason, GCType type) = 0;
    virtual void OnGCFinish(GCReason reason, GCType type) = 0;
    virtual void OnGCPhaseStart(GCPhase phase) = 0;
    virtual void OnGCPhaseEnd(GCPhase phase) = 0;

    virtual ~GCListener() = default;

    NO_COPY_SEMANTIC(GCListener);
    NO_MOVE_SEMANTIC(GCListener);
};

// Central garbage identification algorithm.
class Collector {
public:
    Collector();
    virtual ~Collector();

    static const char *GetGCPhaseName(GCPhase phase);

    // Initializer and finalizer.
    virtual void Init() = 0;
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

    bool RegisterVM(VMInterface *vm);
    bool UnregisterVM(VMInterface *vm);
    void ForEachVM(std::function<void(VMInterface *)> action);

protected:
    virtual void RequestGCInternal(GCReason, bool, GCType, bool)
    {
        LOG(FATAL, COMMON) << "Unresolved fatal";
        UNREACHABLE();
    }

    CollectorType collectorType_ = CollectorType::NO_COLLECTOR;
    std::atomic<GCPhase> gcPhase_ = {GCPhase::GC_PHASE_IDLE};

private:
    PandaVector<GCListener *> gcListeners_;
    ark::os::memory::Mutex gcListenersLock_;

    std::unordered_set<VMInterface *> vmIfaces_;
    ark::os::memory::RWLock vmIfacesLock_;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H
