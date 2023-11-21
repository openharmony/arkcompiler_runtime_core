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

#include "llvm_aot_builder.h"

namespace panda::compiler {

std::unordered_map<std::string, size_t> LLVMAotBuilder::GetSectionsAddresses(const std::string &cmdline,
                                                                             const std::string &file_name)
{
    switch (arch_) {
        case Arch::AARCH32:
            return GetSectionsAddressesImpl<Arch::AARCH32>(cmdline, file_name);
        case Arch::AARCH64:
            return GetSectionsAddressesImpl<Arch::AARCH64>(cmdline, file_name);
        case Arch::X86:
            return GetSectionsAddressesImpl<Arch::X86>(cmdline, file_name);
        case Arch::X86_64:
            return GetSectionsAddressesImpl<Arch::X86_64>(cmdline, file_name);
        default:
            LOG(ERROR, COMPILER) << "LLVMAotBuilder: Unsupported arch";
            UNREACHABLE();
    }
}

template <Arch ARCH>
std::unordered_map<std::string, size_t> LLVMAotBuilder::GetSectionsAddressesImpl(const std::string &cmdline,
                                                                                 const std::string &file_name)
{
    ElfBuilder<ARCH> builder;
    // string_table_ is the only field modified by PrepareElfBuilder not idempotently
    auto old_string_table_size = string_table_.size();

    PrepareElfBuilder(builder, cmdline, file_name);
    builder.Build(file_name);

    string_table_.resize(old_string_table_size);

    auto text_section = builder.GetTextSection();
    auto ro_data_sections = builder.GetRoDataSections();
    auto aot_section = builder.GetAotSection();
    auto got_section = builder.GetGotSection();

    static constexpr auto FIRST_ENTRYPOINT_OFFSET =
        static_cast<int32_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(ARCH);

    std::unordered_map<std::string, size_t> section_addresses {
        {text_section->GetName(), text_section->GetOffset()},
        {aot_section->GetName(), aot_section->GetOffset()},
        // At runtime the .text section is placed right after the .aot_got section
        // without any padding, so we must use the first entrypoint address as a start
        // of the .aot_got section.
        {got_section->GetName(), text_section->GetOffset() - FIRST_ENTRYPOINT_OFFSET}};
    for (auto section : *ro_data_sections) {
        section_addresses.emplace(section->GetName(), section->GetOffset());
    }
    return section_addresses;
}

}  // namespace panda::compiler
