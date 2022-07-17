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

#ifndef COMPILER_AOT_COMPILED_METHOD_H
#define COMPILER_AOT_COMPILED_METHOD_H

#include "utils/arch.h"
#include "utils/span.h"
#include "compiler/code_info/code_info.h"
#include "compiler/optimizer/code_generator/callconv.h"

#include <cstdint>
#include <vector>

namespace panda {
class Method;
}  // namespace panda

namespace panda::compiler {
class CompiledMethod {
public:
    CompiledMethod(Arch arch, Method *method) : arch_(arch), method_(method) {}
    NO_COPY_OPERATOR(CompiledMethod);
    DEFAULT_COPY_CTOR(CompiledMethod)
    DEFAULT_MOVE_SEMANTIC(CompiledMethod);
    ~CompiledMethod() = default;

    void SetCode(Span<const uint8_t> data)
    {
        code_.reserve(data.size());
        std::copy(data.begin(), data.end(), std::back_inserter(code_));
    }

    void SetCodeInfo(Span<const uint8_t> data)
    {
        code_info_.reserve(data.size());
        std::copy(data.begin(), data.end(), std::back_inserter(code_info_));
    }

    Method *GetMethod()
    {
        return method_;
    }

    const Method *GetMethod() const
    {
        return method_;
    }

    Span<const uint8_t> GetCode() const
    {
        return Span(code_);
    }

    Span<const uint8_t> GetCodeInfo() const
    {
        return Span(code_info_);
    }

    size_t GetOverallSize() const
    {
        return RoundUp(CodePrefix::STRUCT_SIZE, GetCodeAlignment(arch_)) + RoundUp(code_.size(), CodeInfo::ALIGNMENT) +
               RoundUp(code_info_.size(), CodeInfo::SIZE_ALIGNMENT);
    }

#ifdef PANDA_COMPILER_CFI
    CfiInfo &GetCfiInfo()
    {
        return cfi_info_;
    }

    const CfiInfo &GetCfiInfo() const
    {
        return cfi_info_;
    }

    void SetCfiInfo(const CfiInfo &cfi_info)
    {
        cfi_info_ = cfi_info;
    }
#endif

private:
    Arch arch_ {RUNTIME_ARCH};
    Method *method_ {nullptr};
    std::vector<uint8_t> code_;
    std::vector<uint8_t> code_info_;
#ifdef PANDA_COMPILER_CFI
    CfiInfo cfi_info_;
#endif
};
}  // namespace panda::compiler

#endif  // COMPILER_AOT_COMPILED_METHOD_H
