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

#include "runtime/mem/gc/g1/xgc-extension-data.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"

namespace ark::ets::interop::js {

XGC *XGC::instance_ = nullptr;

XGC::XGC(ets_proxy::SharedReferenceStorage *storage) : storage_(storage) {}

bool XGC::Create()
{
    if (instance_ != nullptr) {
        return false;
    }
    auto *ctx = InteropCtx::Current();
    auto *stsVmIface = ctx->GetSTSVMInterface();
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    instance_ = new XGC(storage);
    if (stsVmIface == nullptr) {
        // JS VM is not ArkJS.
        // NOTE(audovichenko): remove this later
        return true;
    }
    instance_->stsVmIface_ = reinterpret_cast<STSVMInterfaceImpl *>(stsVmIface);
    auto xobjHandler = [storage, stsVmIface](ObjectHeader *obj) {
        auto *etsObj = EtsObject::FromCoreType(obj);
        if (!etsObj->HasInteropIndex()) {
            return;
        }
        // NOTE(audovichenko): Handle multithreading issue.
        ets_proxy::SharedReference::Iterator it(storage->GetReference(etsObj));
        ets_proxy::SharedReference::Iterator end;
        do {
            if (it->HasJSFlag() && it->MarkIfNotMarked()) {
                arkplatform::EcmaVMInterface *ecmaIface = it->GetCtx()->GetEcmaVMInterface();
                ecmaIface->MarkFromObject(it->GetJsRef());
            }
            ++it;
        } while (it != end);
        reinterpret_cast<STSVMInterfaceImpl *>(stsVmIface)->NotifyWaiters();
    };
    auto *coro = EtsCoroutine::GetCurrent();
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *gc = coro->GetPandaVM()->GetGC();
    // NOTE(audovichenko): Don't like to use extension data.
    gc->SetExtensionData(allocator->New<mem::XGCExtensionData>(xobjHandler));
    gc->AddListener(instance_);
    return true;
}

void XGC::GCStarted(const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
        return;
    }
    auto *coro = EtsCoroutine::GetCurrent();
    coro->GetPandaVM()->RemoveRootProvider(InteropCtx::Current()->GetSharedRefStorage());
    stsVmIface_->StartXGCBarrier();
    isXGcInProgress_ = true;
    remarkFinished_ = false;
}

void XGC::GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc, [[maybe_unused]] size_t heapSize)
{
    if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
        return;
    }
    auto *coro = EtsCoroutine::GetCurrent();
    coro->GetPandaVM()->AddRootProvider(InteropCtx::Current()->GetSharedRefStorage());
    isXGcInProgress_ = false;
    if (remarkFinished_) {
        // XGC was not interrupted
        // NOTE(audovichenko): Sweep SharedReferenceStorage. It could be done:
        // * on the common STW
        // * in the main thread.
        // To do it on the main thread we should start a coro on the main worker
        // or post async job using libuv.
    }
    stsVmIface_->FinishXGCBarrier();
}

void XGC::GCPhaseStarted(mem::GCPhase phase)
{
    if (!isXGcInProgress_) {
        return;
    }
    switch (phase) {
        case mem::GCPhase::GC_PHASE_INITIAL_MARK: {
            storage_->UnmarkAll();
            break;
        }
        case mem::GCPhase::GC_PHASE_REMARK: {
            stsVmIface_->RemarkStartBarrier();
            break;
        }
        default: {
            break;
        }
    }
}

void XGC::GCPhaseFinished(mem::GCPhase phase)
{
    if (!isXGcInProgress_) {
        return;
    }
    switch (phase) {
        case mem::GCPhase::GC_PHASE_MARK: {
            stsVmIface_->WaitForConcurrentMark(nullptr);
            break;
        }
        case mem::GCPhase::GC_PHASE_REMARK: {
            stsVmIface_->WaitForRemark(nullptr);
            remarkFinished_ = true;
            break;
        }
        default: {
            break;
        }
    }
}

}  // namespace ark::ets::interop::js
