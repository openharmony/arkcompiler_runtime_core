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
#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_INL_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_INL_H

#include <iostream>
#include <string>

#include "libpandabase/utils/logger.h"

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_reader.h"
#include "runtime/tooling/sampler/sample_writer.h"

namespace panda::tooling::sampler {

// -------------------------------------------------
// ------------- Format reader ---------------------
// -------------------------------------------------

/*
 * Example for 64 bit architecture
 *
 *              Thread id   Thread status    Stack Size      Managed stack frame id
 * Sample row |___________|______________|________________|_____________------___________|
 *              32 bits      32 bits          64 bits       (128 * <stack size>) bits
 *
 *
 *              0xFF..FF    pointer   checksum   name size     module path (ASCII str)
 * Module row |__________|__________|__________|___________|_____________------___________|
 *              64 bits    64 bits    32 bits    64 bits       (8 * <name size>) bits
 */
inline SampleReader::SampleReader(const char *filename)
{
    {
        std::ifstream bin_file(filename, std::ios::binary | std::ios::ate);
        if (!bin_file) {
            LOG(FATAL, PROFILER) << "Unable to open file \"" << filename << "\"";
            UNREACHABLE();
        }
        std::streamsize buffer_size = bin_file.tellg();
        bin_file.seekg(0, std::ios::beg);

        buffer_.resize(buffer_size);

        if (!bin_file.read(buffer_.data(), buffer_size)) {
            LOG(FATAL, PROFILER) << "Unable to read sampler trace file";
            UNREACHABLE();
        }
        bin_file.close();
    }

    size_t buffer_counter = 0;
    while (buffer_counter < buffer_.size()) {
        if (ReadUintptrTBitMisaligned(&buffer_[buffer_counter]) == StreamWriter::MODULE_INDICATOR_VALUE) {
            // This entry is panda file
            size_t pf_name_size = ReadUintptrTBitMisaligned(&buffer_[buffer_counter + PANDA_FILE_NAME_SIZE_OFFSET]);
            size_t next_module_ptr_offset = PANDA_FILE_NAME_OFFSET + pf_name_size * sizeof(char);

            if (buffer_counter + next_module_ptr_offset > buffer_.size()) {
                LOG(ERROR, PROFILER) << "ark sampling profiler drop last module, because of invalid trace file";
                return;
            }

            module_row_ptrs_.push_back(&buffer_[buffer_counter]);
            buffer_counter += next_module_ptr_offset;
            continue;
        }

        // buffer_counter now is entry of a sample, stack size lies after thread_id
        size_t stack_size = ReadUintptrTBitMisaligned(&buffer_[buffer_counter + SAMPLE_STACK_SIZE_OFFSET]);
        if (stack_size > SampleInfo::StackInfo::MAX_STACK_DEPTH) {
            LOG(FATAL, PROFILER) << "ark sampling profiler trace file is invalid, stack_size > MAX_STACK_DEPTH";
            UNREACHABLE();
        }

        size_t next_sample_ptr_offset = SAMPLE_STACK_OFFSET + stack_size * sizeof(SampleInfo::ManagedStackFrameId);
        if (buffer_counter + next_sample_ptr_offset > buffer_.size()) {
            LOG(ERROR, PROFILER) << "ark sampling profiler drop last samples, because of invalid trace file";
            return;
        }

        sample_row_ptrs_.push_back(&buffer_[buffer_counter]);
        buffer_counter += next_sample_ptr_offset;
    }

    if (buffer_counter != buffer_.size()) {
        LOG(ERROR, PROFILER) << "ark sampling profiler trace file is invalid";
    }
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
inline bool SampleReader::GetNextSample(SampleInfo *sample_out)
{
    if (sample_row_ptrs_.size() <= sample_row_counter_) {
        return false;
    }
    const char *current_sample_ptr = sample_row_ptrs_[sample_row_counter_];
    sample_out->thread_info.thread_id = ReadUint32TBitMisaligned(&current_sample_ptr[SAMPLE_THREAD_ID_OFFSET]);
    sample_out->thread_info.thread_status = static_cast<SampleInfo::ThreadStatus>(
        ReadUint32TBitMisaligned(&current_sample_ptr[SAMPLE_THREAD_STATUS_OFFSET]));
    sample_out->stack_info.managed_stack_size =
        ReadUintptrTBitMisaligned(&current_sample_ptr[SAMPLE_STACK_SIZE_OFFSET]);

    ASSERT(sample_out->stack_info.managed_stack_size <= SampleInfo::StackInfo::MAX_STACK_DEPTH);
    memcpy(sample_out->stack_info.managed_stack.data(), current_sample_ptr + SAMPLE_STACK_OFFSET,
           sample_out->stack_info.managed_stack_size * sizeof(SampleInfo::ManagedStackFrameId));
    ++sample_row_counter_;
    return true;
}

inline bool SampleReader::GetNextModule(FileInfo *module_out)
{
    if (module_row_ptrs_.size() <= module_row_counter_) {
        return false;
    }

    module_out->pathname.clear();

    const char *current_module_ptr = module_row_ptrs_[module_row_counter_];
    module_out->ptr = ReadUintptrTBitMisaligned(&current_module_ptr[PANDA_FILE_POINTER_OFFSET]);
    module_out->checksum = ReadUint32TBitMisaligned(&current_module_ptr[PANDA_FILE_CHECKSUM_OFFSET]);
    size_t str_size = ReadUintptrTBitMisaligned(&current_module_ptr[PANDA_FILE_NAME_SIZE_OFFSET]);
    const char *str_ptr = &current_module_ptr[PANDA_FILE_NAME_OFFSET];
    module_out->pathname = std::string(str_ptr, str_size);

    ++module_row_counter_;
    return true;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace panda::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_INL_H
