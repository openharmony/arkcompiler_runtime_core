/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_INTERFACES_MEM_TLAB_H
#define COMMON_INTERFACES_MEM_TLAB_H

#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include "common_interfaces/base/common.h"

namespace common_vm {

class TLAB {
public:
    uintptr_t Alloc(size_t size)
    {
        DCHECK_CC(size > 0);
        DCHECK_CC(startAddr_ != 0);
        size_t limit = endAddr_;
        if (allocPtr_ + size <= limit) {
            uintptr_t addr = allocPtr_;
            allocPtr_ += size;
            return addr;
        } else {
            return 0;
        }
    }

    static constexpr size_t TLABStartAddrOffset()
    {
        return offsetof(TLAB, startAddr_);
    }

    static constexpr size_t TLABAllocPtrOffset()
    {
        return offsetof(TLAB, allocPtr_);
    }

    static constexpr size_t TLABEndAddrOffset()
    {
        return offsetof(TLAB, endAddr_);
    }

    uintptr_t startAddr_ = 0;
    uintptr_t endAddr_ = 0;
    uintptr_t allocPtr_ = 0;
};
}  // namespace common_vm
#endif  // COMMON_INTERFACES_MEM_TLAB_H