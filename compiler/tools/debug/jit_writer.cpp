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

#include "jit_writer.h"

#ifdef PANDA_COMPILER_CFI

namespace panda::compiler {
class JitCodeDataProvider : public ElfSectionDataProvider {
public:
    explicit JitCodeDataProvider(JitDebugWriter *jit_debug_writer) : jit_debug_writer_(jit_debug_writer) {}

    void FillData(Span<uint8_t> stream, size_t stream_begin) const override
    {
        const size_t code_offset = CodeInfo::GetCodeOffset(jit_debug_writer_->GetArch());
        CodePrefix prefix;
        size_t curr_pos = stream_begin;
        for (size_t i = 0; i < jit_debug_writer_->methods_.size(); i++) {
            auto &method = jit_debug_writer_->methods_[i];
            auto &method_header = jit_debug_writer_->method_headers_[i];
            prefix.code_size = method.GetCode().size();
            prefix.code_info_offset = code_offset + RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT);
            prefix.code_info_size = method.GetCodeInfo().size();
            // Prefix
            curr_pos = stream_begin + method_header.code_offset;
            const char *data = reinterpret_cast<char *>(&prefix);
            CopyToSpan(stream, data, sizeof(prefix), curr_pos);
            curr_pos += sizeof(prefix);

            // Code
            curr_pos += code_offset - sizeof(prefix);
            data = reinterpret_cast<const char *>(method.GetCode().data());
            CopyToSpan(stream, data, method.GetCode().size(), curr_pos);
            curr_pos += method.GetCode().size();

            // CodeInfo
            curr_pos += RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT) - method.GetCode().size();
            data = reinterpret_cast<const char *>(method.GetCodeInfo().data());
            CopyToSpan(stream, data, method.GetCodeInfo().size(), curr_pos);
        }
    }

    size_t GetDataSize() const override
    {
        return jit_debug_writer_->current_code_size_;
    }

private:
    JitDebugWriter *jit_debug_writer_;
};

void JitDebugWriter::Start()
{
    auto &file_header = file_headers_.emplace_back();
    file_header.classes_offset = class_headers_.size();
    file_header.file_checksum = 0;
    file_header.file_offset = 0;
    file_header.file_name_str = AddString("jit_code");
    file_header.methods_offset = method_headers_.size();
}

void JitDebugWriter::End()
{
    ASSERT(!file_headers_.empty());
    auto &file_header = file_headers_.back();
    file_header.classes_count = class_headers_.size() - file_header.classes_offset;
    if (file_header.classes_count == 0) {
        file_headers_.pop_back();
        return;
    }
    file_header.methods_count = method_headers_.size() - file_header.methods_offset;
    // We should keep class headers sorted, since AOT manager uses binary search to find classes.
    std::sort(class_headers_.begin() + file_header.classes_offset, class_headers_.end(),
              [](const auto &a, const auto &b) { return a.class_id < b.class_id; });
}

bool JitDebugWriter::Write()
{
    switch (GetArch()) {
#ifdef PANDA_TARGET_64
        case Arch::AARCH64:
            return WriteImpl<Arch::AARCH64>();
        case Arch::X86_64:
            return WriteImpl<Arch::X86_64>();
#endif
        case Arch::AARCH32:
            return WriteImpl<Arch::AARCH32>();
        default:
            LOG(ERROR, COMPILER) << "JitDebug: Unsupported arch";
            return false;
    }
}

template <Arch arch>
bool JitDebugWriter::WriteImpl()
{
    ElfBuilder<arch, true> builder;

    // In gdb you may use '(gdb) info functions jitted' for find all jit-entry
    builder.SetCodeName("(jitted) " + method_name_);

    JitCodeDataProvider code_provider(this);
    builder.GetTextSection()->SetDataProvider(&code_provider);

    builder.SetFrameData(GetFrameData());
    builder.Build("jitted_code");

    auto elf_size {builder.GetFileSize()};
    auto mem_range {code_allocator_->AllocateCodeUnprotected(elf_size)};
    auto elf_data {reinterpret_cast<uint8_t *>(mem_range.GetData())};
    if (elf_data == nullptr) {
        return false;
    }

    builder.HackAddressesForJit(elf_data);

    elf_ = {elf_data, elf_size};
    builder.Write(elf_);
    code_allocator_->ProtectCode(mem_range);

    code_ = builder.GetTextSectionData();
    return true;
}
}  // namespace panda::compiler

#endif