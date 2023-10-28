/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "created_object_file.h"

#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>

#define DEBUG_TYPE "created-object-file"

namespace panda::llvmbackend {

CreatedObjectFile::CreatedObjectFile(std::unique_ptr<llvm::MemoryBuffer> object_file_buffer,
                                     std::unique_ptr<llvm::object::ObjectFile> object_file)
    : buffer_(std::move(object_file_buffer)), object_file_(std::move(object_file))
{
    if (object_file_ != nullptr) {
        // Generally, we need this index for Irtoc to collect used registers.
        // In other cases, it seems a single scan is pretty enough.
        for (auto section : object_file_->sections()) {
            section_index_[cantFail(section.getName())] = section;
        }
    }
}

llvm::Expected<std::unique_ptr<CreatedObjectFile>> CreatedObjectFile::CopyOf(
    llvm::MemoryBufferRef object_file_buffer, const ObjectFilePostProcessor &object_file_post_processor)
{
    auto copy = llvm::MemoryBuffer::getMemBufferCopy(object_file_buffer.getBuffer());
    auto elf = llvm::object::ObjectFile::createObjectFile(copy->getMemBufferRef());
    if (!elf) {
        return elf.takeError();
    }
    auto file = cantFail(std::move(elf));
    object_file_post_processor(file.get());
    return std::make_unique<CreatedObjectFile>(std::move(copy), std::move(file));
}

llvm::object::ObjectFile *CreatedObjectFile::GetObjectFile() const
{
    return object_file_.get();
}

bool CreatedObjectFile::HasSection(const std::string &name) const
{
    return section_index_.find(name) != section_index_.end();
}

std::vector<uint8_t> SectionReference::ContentToVector()
{
    std::vector<uint8_t> vector;
    assert(GetSize() != 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(GetMemory(), GetMemory() + GetSize(), std::back_inserter(vector));
    return vector;
}

SectionReference CreatedObjectFile::GetSection(const std::string &name) const
{
    LLVM_DEBUG(llvm::dbgs() << "Getting section = '" << name << "'\n");
    assert(section_index_.find(name) != section_index_.end() && "Attempt to access an unknown section");
    const auto &section = section_index_.at(name);
    auto contents = cantFail(section.getContents());
    auto memory = reinterpret_cast<const uint8_t *>(contents.data());
    return SectionReference {memory, contents.size(), name, section.getAlignment()};
}

SectionReference CreatedObjectFile::GetSectionByFunctionName(const std::string &full_function_name) const
{
    auto section_reference = GetSection(".text." + full_function_name + ".");
    LLVM_DEBUG(llvm::dbgs() << "Got section by function = " << full_function_name
                            << " section's size = " << section_reference.GetSize() << "\n");
    return section_reference;
}

void CreatedObjectFile::WriteTo(std::string_view output) const
{
    std::error_code error_code;
    llvm::raw_fd_ostream dest(output, error_code, llvm::sys::fs::FA_Write);
    static constexpr bool GEN_CRASH_DIAG = false;
    if (error_code) {
        report_fatal_error(llvm::Twine("Unable to open file = '") + output + "' to dump object file, error_code = " +
                               llvm::Twine(error_code.value()) + ", message = " + error_code.message(),
                           GEN_CRASH_DIAG);
    }
    llvm::StringRef data;
    if (object_file_ != nullptr) {
        data = object_file_->getData();
    }
    LLVM_DEBUG(llvm::dbgs() << "Writing object file, size =  " << data.size() << " to " << output << "\n");
    dest << data;
    if (dest.has_error()) {
        auto error = dest.error();
        report_fatal_error(llvm::Twine("Unable to write object file to '") + output +
                               ", error_code = " + llvm::Twine(error.value()) + ", message = " + error.message(),
                           GEN_CRASH_DIAG);
    }
}
size_t CreatedObjectFile::Size() const
{
    assert(object_file_ != nullptr && "Attempt to get size of empty CreatedObjectFile");
    return object_file_->getData().size();
}

}  // namespace panda::llvmbackend
