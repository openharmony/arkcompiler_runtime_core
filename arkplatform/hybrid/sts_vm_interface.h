/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_STS_VM_INTERFACE_H
#define PANDA_STS_VM_INTERFACE_H

#include "hybrid/vm_interface.h"
#include <cstdint>

namespace arkplatform {

class STSVMInterface : public VMInterface {
public:
    STSVMInterface() = default;

    STSVMInterface(const STSVMInterface &) = delete;
    void operator=(const STSVMInterface &) = delete;

    STSVMInterface(STSVMInterface &&) = delete;
    STSVMInterface &operator=(STSVMInterface &&) = delete;

    ~STSVMInterface() override = default;

    VMInterfaceType GetVMType() const override
    {
        return STSVMInterface::VMInterfaceType::ETS_VM_IFACE;
    }

    virtual void MarkFromObject(void* obj) = 0;
};

} // namespace arkplatform

#endif // PANDA_STS_VM_INTERFACE_H