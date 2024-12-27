/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H
#define PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"
#include "runtime/mem/gc/gc_trigger.h"

namespace ark::ets::interop::js {

/**
 * Cross-reference garbage collector.
 * Implements logic to collect cross-references between JS and STS in SharedReferenceStorage.
 */
class XGC final : public mem::GCTrigger {
public:
    NO_COPY_SEMANTIC(XGC);
    NO_MOVE_SEMANTIC(XGC);
    ~XGC() override = default;

    /**
     * Create instance of XGC if it was not created before. Runtime should be existed before the XGC creation
     * @param mainCoro main coroutine
     * @return true if the instance successfully created, false - otherwise
     */
    [[nodiscard]] PANDA_PUBLIC_API static bool Create(EtsCoroutine *mainCoro);

    /// @return current instance of XGC
    static XGC *GetInstance();

    /**
     * Destroy the current instance of XGC if the instance is existed. Runtime should be existed before the XGC
     * destruction
     * @return true if the instance successfully destroyed, false - otherwise
     */
    PANDA_PUBLIC_API static bool Destroy();

    /**
     * Check trigger condition and post XGC task to gc queue and
     * trigger XMark for all related JS virtual machines if needed
     * @param gc GC using in the current VM
     */
    void TriggerGcIfNeeded(mem::GC *gc) override;

    /**
     * Post XGC task to gc queue and trigger XMark for all related JS virtual machines
     * @param gc GC using in the current VM
     */
    void Trigger(mem::GC *gc);

    /// @return XGC trigger type value
    mem::GCTriggerType GetType() const override
    {
        return mem::GCTriggerType::XGC;
    }

    /// GCListener specific public methods ///

    void GCStarted(const GCTask &task, size_t heapSize) override;
    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;
    void GCPhaseStarted(mem::GCPhase phase) override;
    void GCPhaseFinished(mem::GCPhase phase) override;

private:
    // For allocation of XGC with the private constructor by internal allocator
    friend mem::Allocator;
    XGC(PandaEtsVM *vm, STSVMInterfaceImpl *stsVmIface, ets_proxy::SharedReferenceStorage *storage);
    static XGC *instance_;

    /// @return new target threshold storage size for XGC trigger
    size_t ComputeNewSize();

    /// External specific fields ///

    PandaEtsVM *vm_ {nullptr};
    ets_proxy::SharedReferenceStorage *storage_ {nullptr};
    STSVMInterfaceImpl *stsVmIface_ {nullptr};

    /// XGC current state specific fields ///

    std::atomic_bool isXGcInProgress_ {false};
    bool remarkFinished_ {false};  // GUARDED_BY(mutatorLock)

    /// Trigger specific fields ///

    size_t beforeGCStorageSize_ {0U};
    const size_t minimalThreasholdSize_ {0U};
    // We can load a value of the variable from several threads, so need to use atomic
    std::atomic<size_t> targetThreasholdSize_ {0U};
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H
