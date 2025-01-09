/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include <array>
#include <node_api.h>
#include "hybrid/ecma_vm_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js {

class TestEcmaVMInterface : public arkplatform::EcmaVMInterface {
public:
    void MarkFromObject(void *obj) override
    {
        markFromObjectCalled_ = true;
        if (obj != expectedJsObject_) {
            std::stringstream err;
            err << "MarkFromObject called with " << obj << ", but expected " << expectedJsObject_;
            errors_.push_back(err.str());
            return;
        }
    }

    void SetExpectedJsObject(void *obj)
    {
        expectedJsObject_ = obj;
    }

    std::vector<std::string> GetErrors()
    {
        if (!markFromObjectCalled_) {
            std::stringstream err;
            err << "MarkFromObject was not called";
            errors_.insert(errors_.begin(), err.str());
        }
        return errors_;
    }

private:
    std::vector<std::string> errors_;
    void *expectedJsObject_ = nullptr;
    bool markFromObjectCalled_ = false;
};

static TestEcmaVMInterface g_ecmaVMIface;

class TestGCListener : public mem::GCListener {
public:
    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
    {
        if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
            std::stringstream err;
            err << "Expected GC cause CROSSREF_CAUSE, but got " << task.reason;
            errorMessages_.push_back(err.str());
            return;
        }

        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 1U) {
            std::stringstream err;
            err << "Expected one xrefs, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
        bool sts2jsRef = false;
        xrefStorage->VisitRoots([this, xrefStorage, &sts2jsRef](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            if (!obj->HasInteropIndex()) {
                std::stringstream err;
                err << "Found object " << obj << " in ShredReferenceStorage without interop index";
                errorMessages_.push_back(err.str());
                return;
            }
            auto *xref = xrefStorage->GetReference(obj);
            if (xref->HasJSFlag()) {
                if (sts2jsRef) {
                    std::stringstream err;
                    err << "Found second object " << obj << " with 'js' flag";
                    errorMessages_.push_back(err.str());
                    return;
                }
                sts2jsRef = true;
                g_ecmaVMIface.SetExpectedJsObject(xref->GetJsRef());
            }
        });
    }

    void GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            auto *xref = xrefStorage->GetReference(obj);
            if (!xref->IsMarked()) {
                std::stringstream err;
                err << "Found not marked xref";
                errorMessages_.push_back(err.str());
                return;
            }
        });
    }

    const std::vector<std::string> &GetErrors() const
    {
        return errorMessages_;
    }

private:
    std::vector<std::string> errorMessages_;
};

static TestGCListener g_gcListener;
}  // namespace ark::ets::interop::js

#include "test_module.h"
