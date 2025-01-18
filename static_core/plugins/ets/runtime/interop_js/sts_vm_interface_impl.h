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

#ifndef PLUGINS_ETS_RUNTIME_INTEROP_JS_STS_VM_INTERFACE_IMPL_H
#define PLUGINS_ETS_RUNTIME_INTEROP_JS_STS_VM_INTERFACE_IMPL_H

#include "hybrid/sts_vm_interface.h"
#include "libpandabase/macros.h"

namespace ark::ets::interop::js {
class STSVMInterfaceImpl final : public arkplatform::STSVMInterface {
public:
    NO_COPY_SEMANTIC(STSVMInterfaceImpl);
    NO_MOVE_SEMANTIC(STSVMInterfaceImpl);
    STSVMInterfaceImpl() = default;
    ~STSVMInterfaceImpl() override = default;

    void MarkFromObject(void *ref) override;
};

}  // namespace ark::ets::interop::js

#endif  // PLUGINS_ETS_RUNTIME_INTEROP_JS_STS_VM_INTERFACE_IMPL_H
