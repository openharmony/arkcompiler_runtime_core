/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef COMPILER_AOT_AOT_BUILDER_LLVM_AOT_BUILDER_H
#define COMPILER_AOT_AOT_BUILDER_LLVM_AOT_BUILDER_H

#include "aot_builder.h"

namespace panda::compiler {

class LLVMAotBuilder : public AotBuilder {
public:
    std::unordered_map<std::string, size_t> GetSectionsAddresses(const std::string &cmdline,
                                                                 const std::string &file_name);

    const std::vector<CompiledMethod> &GetMethods()
    {
        return methods_;
    }

    const std::vector<compiler::MethodHeader> &GetMethodHeaders()
    {
        return method_headers_;
    }

    /**
     * Put method header for a given method.
     * The method will be patched later (see AdjustMethod below).
     */
    void AddMethodHeader(Method *method, size_t method_index)
    {
        auto &method_header = method_headers_.emplace_back();
        methods_.emplace_back(arch_, method, method_index);
        method_header.method_id = method->GetFileId().GetOffset();
        method_header.code_offset = UNPATCHED_CODE_OFFSET;
        method_header.code_size = UNPATCHED_CODE_SIZE;
        current_bitmap_->SetBit(method_index);
    }

    /// Adjust a method's header according to the supplied method
    void AdjustMethodHeader(const CompiledMethod &method, size_t index)
    {
        MethodHeader &method_header = method_headers_[index];
        method_header.code_offset = current_code_size_;
        method_header.code_size = method.GetOverallSize();
        current_code_size_ += method_header.code_size;
        current_code_size_ = RoundUp(current_code_size_, GetCodeAlignment(arch_));
    }

    /// Adjust a method stored in this aot builder according to the supplied method
    void AdjustMethod(const CompiledMethod &method, size_t index)
    {
        ASSERT(methods_.at(index).GetMethod() == method.GetMethod());
        ASSERT(method_headers_.at(index).code_size == method.GetOverallSize());
        methods_.at(index).SetCode(method.GetCode());
        methods_.at(index).SetCodeInfo(method.GetCodeInfo());
    }

private:
    static constexpr auto UNPATCHED_CODE_OFFSET = std::numeric_limits<decltype(MethodHeader::code_offset)>::max();
    static constexpr auto UNPATCHED_CODE_SIZE = std::numeric_limits<decltype(MethodHeader::code_size)>::max();

    template <Arch ARCH>
    std::unordered_map<std::string, size_t> GetSectionsAddressesImpl(const std::string &cmdline,
                                                                     const std::string &file_name);
};
}  // namespace panda::compiler
#endif  // COMPILER_AOT_AOT_BUILDER_LLVM_AOT_BUILDER_H
