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

namespace ark::ets::interop::js {

/**
 * Cross-reference garbage collector.
 * Implements logic to collect cross-references between JS and STS in SharedReferenceStorage.
 */
class XGC final : public mem::GCListener {
public:
    NO_COPY_SEMANTIC(XGC);
    NO_MOVE_SEMANTIC(XGC);
    ~XGC() override = default;

    /**
     * Create instance of XGC if it was not created before. Runtime should be existed before the XGC creation
     * @param ctx main interop context
     * @return true if the instance successfully created, false - otherwise
     */
    [[nodiscard]] static bool Create();

    void GCStarted(const GCTask &task, size_t heapSize) override;
    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;
    void GCPhaseStarted(mem::GCPhase phase) override;
    void GCPhaseFinished(mem::GCPhase phase) override;

private:
    explicit XGC(ets_proxy::SharedReferenceStorage *storage);
    static XGC *instance_;

    ets_proxy::SharedReferenceStorage *storage_;
    bool isXGcInProgress_ = false;
    bool remarkFinished_ = false;
    STSVMInterfaceImpl *stsVmIface_ = nullptr;
};
}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H
