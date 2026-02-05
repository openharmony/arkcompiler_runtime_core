/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_ECMA_VM_INTERFACE_H
#define PANDA_ECMA_VM_INTERFACE_H

#include "hybrid/vm_interface.h"
#include <cstdint>
#include <functional>

namespace arkplatform {
using TaggedType = uint64_t;

class EcmaVMInterface : public VMInterface {
public:
    EcmaVMInterface() = default;

    EcmaVMInterface(const EcmaVMInterface &) = delete;
    void operator=(const EcmaVMInterface &) = delete;

    EcmaVMInterface(EcmaVMInterface &&) = delete;
    EcmaVMInterface &operator=(EcmaVMInterface &&) = delete;

    ~EcmaVMInterface() override = default;

    VMInterfaceType GetVMType() const override
    {
        return VMInterface::VMInterfaceType::ECMA_VM_IFACE;
    }

    virtual void NotifyXGCInterruption() {}

    virtual bool StartXRefMarking()
    {
        return true;
    }

    virtual TaggedType *GetTopFrameSPFromDynamic() const
    {
        return nullptr;
    }

    virtual bool ForEachDynamicFrame([[maybe_unused]] void *currFrameSP, [[maybe_unused]] void *toFrameSP,
                                     [[maybe_unused]] const std::function<void(const void *)> &cb) const
    {
        return false;
    }
};

}  // namespace arkplatform

#endif  // PANDA_VM_INTERFACE_H
