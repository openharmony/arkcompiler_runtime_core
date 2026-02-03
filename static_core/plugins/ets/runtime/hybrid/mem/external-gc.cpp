/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "libarkbase/utils/logger.h"
#include "runtime/mark_word.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/heap/heap_visitor.h"
#ifdef PANDA_JS_ETS_HYBRID_MODE
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#endif  // PANDA_JS_ETS_HYBRID_MODE

namespace ark::mem::ets {

void VisitVmRoots(const GCRootVisitor &visitor, PandaVM *vm)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.VisitNonHeapRoots(visitor, VisitGCRootFlags::ACCESS_ROOT_ALL);
    vm->VisitStringTable(visitor, VisitGCRootFlags::ACCESS_ROOT_ALL);
}

}  // namespace ark::mem::ets

namespace common {

static ark::PandaVM *GetPandaVM()
{
    auto *runtime = ark::Runtime::GetCurrent();
    if (runtime == nullptr) {
        return nullptr;
    }
    return runtime->GetPandaVM();
}

void VisitStaticRoots(const RefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }

    ark::GCRootVisitor rootVisitor = [&visitor](const ark::mem::GCRoot &gcRoot) {
        common::RefField<> refField {reinterpret_cast<common::BaseObject *>(gcRoot.GetObjectHeader())};
        visitor(refField);
    };
    ark::mem::ets::VisitVmRoots(rootVisitor, vm);
}

void RegisterStaticRootsProcessFunc()
{
    RegisterVisitStaticRootsHook(VisitStaticRoots);
}

}  // namespace common
