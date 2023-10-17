/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H
#define PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H

#include "runtime/compiler.h"
#include "cross_values.h"
#include "plugins/ets/runtime/ets_vm.h"
namespace panda::ets {

class EtsRuntimeInterface : public PandaRuntimeInterface {
public:
    /**********************************************************************************/
    /// Object information
    ClassPtr GetClass(MethodPtr method, IdType id) const override;
    size_t GetTlsPromiseClassPointerOffset(Arch arch) const override
    {
        return panda::cross_values::GetEtsCoroutinePromiseClassOffset(arch);
    }
    InteropCallKind GetInteropCallKind(MethodPtr method_ptr) const override;
    char *GetFuncPropName(MethodPtr method_ptr, uint32_t str_id) const override;
    uint64_t GetFuncPropNameOffset(MethodPtr method_ptr, uint32_t str_id) const override;

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/ets_interop_runtime_interface-inl.h"
#endif
};
}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H