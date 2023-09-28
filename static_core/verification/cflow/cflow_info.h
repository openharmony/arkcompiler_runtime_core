/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_VERIFICATION_CFLOW_INFO_HPP_
#define PANDA_VERIFICATION_CFLOW_INFO_HPP_

#include "macros.h"
#include "verification/verification_status.h"

#include "runtime/include/class.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"

#include <cstdint>
#include <optional>

namespace panda::verifier {
class Job;

enum class InstructionType { NORMAL, JUMP, COND_JUMP, RETURN, THROW };

class CflowMethodInfo {
public:
    enum Flag : uint8_t { INSTRUCTION = 1, EXCEPTION_SOURCE = 2, EXCEPTION_HANDLER = 4, JUMP_TARGET = 8 };

    CflowMethodInfo() = delete;
    CflowMethodInfo(uint8_t const *addr_start, size_t code_size)
        : addr_start_ {addr_start},
          addr_end_ {addr_start + code_size}  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    {
        flags_.resize(code_size);
    }
    ~CflowMethodInfo() = default;
    NO_COPY_SEMANTIC(CflowMethodInfo);
    NO_MOVE_SEMANTIC(CflowMethodInfo);

    uint8_t const *GetAddrStart() const
    {
        return addr_start_;
    }

    uint8_t const *GetAddrEnd() const
    {
        return addr_end_;
    }

    bool IsAddrValid(uint8_t const *addr) const
    {
        return addr_start_ <= addr && addr < addr_end_;
    }

    bool IsFlagSet(uint8_t const *addr, Flag flag) const
    {
        ASSERT(addr >= addr_start_);
        ASSERT(addr < addr_end_);
        return ((flags_[addr - addr_start_] & flag) != 0);
    }

    void SetFlag(uint8_t const *addr, Flag flag)
    {
        ASSERT(addr >= addr_start_);
        ASSERT(addr < addr_end_);
        flags_[addr - addr_start_] |= flag;
    }

    void ClearFlag(uint8_t const *addr, Flag flag)
    {
        ASSERT(addr >= addr_start_);
        ASSERT(addr < addr_end_);
        flags_[addr - addr_start_] &= static_cast<uint8_t>(~flag);
    }

    PandaVector<uint8_t const *> const *GetHandlerStartAddresses() const
    {
        return &handler_start_addresses_;
    }

private:
    uint8_t const *addr_start_;
    uint8_t const *addr_end_;
    PandaVector<uint8_t> flags_;
    PandaVector<uint8_t const *> handler_start_addresses_;

    VerificationStatus FillCodeMaps(Method const *method);
    VerificationStatus ProcessCatchBlocks(Method const *method);

    friend PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(Method const *method);
};

PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(Method const *method);
}  // namespace panda::verifier

#endif  // !PANDA_VERIFICATION_CFLOW_INFO_HPP_
