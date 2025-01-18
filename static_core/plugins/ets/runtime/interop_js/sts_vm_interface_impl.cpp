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

#include "runtime/mem/gc/g1/g1-gc.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"

namespace ark::ets::interop::js {
void STSVMInterfaceImpl::MarkFromObject(void *ref)
{
    ASSERT(ref != nullptr);
    // NOTE(audovichenko): Find the corresponding ref
    auto *sharedRef = reinterpret_cast<ets_proxy::SharedReference *>(ref);
    if (sharedRef->MarkIfNotMarked()) {
        EtsObject *etsObj = sharedRef->GetEtsObject();
        auto *gc = reinterpret_cast<mem::G1GC<EtsLanguageConfig> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC());
        gc->MarkObjectRecursively(etsObj->GetCoreType());
    }
}
}  // namespace ark::ets::interop::js
