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

#include <iostream>
#include <fstream>
#include <string>

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_writer.h"

namespace panda::tooling::sampler {

void StreamWriter::WriteSample(const SampleInfo &sample) const
{
    ASSERT(write_stream_ptr_ != nullptr);
    ASSERT(sample.stack_info.managed_stack_size <= SampleInfo::StackInfo::MAX_STACK_DEPTH);

    static_assert(sizeof(sample.thread_info.thread_id) == sizeof(uint32_t));
    static_assert(sizeof(sample.thread_info.thread_status) == sizeof(uint32_t));
    static_assert(sizeof(sample.stack_info.managed_stack_size) == sizeof(uintptr_t));

    write_stream_ptr_->write(reinterpret_cast<const char *>(&sample.thread_info.thread_id),
                             sizeof(sample.thread_info.thread_id));
    write_stream_ptr_->write(reinterpret_cast<const char *>(&sample.thread_info.thread_status),
                             sizeof(sample.thread_info.thread_status));
    write_stream_ptr_->write(reinterpret_cast<const char *>(&sample.stack_info.managed_stack_size),
                             sizeof(sample.stack_info.managed_stack_size));
    write_stream_ptr_->write(reinterpret_cast<const char *>(sample.stack_info.managed_stack.data()),
                             sample.stack_info.managed_stack_size * sizeof(SampleInfo::ManagedStackFrameId));
}

void StreamWriter::WriteModule(const FileInfo &module_info)
{
    ASSERT(write_stream_ptr_ != nullptr);
    static_assert(sizeof(MODULE_INDICATOR_VALUE) == sizeof(uintptr_t));
    static_assert(sizeof(module_info.ptr) == sizeof(uintptr_t));
    static_assert(sizeof(module_info.checksum) == sizeof(uint32_t));
    static_assert(sizeof(module_info.pathname.length()) == sizeof(uintptr_t));

    if (module_info.pathname.empty()) {
        return;
    }
    size_t str_size = module_info.pathname.length();

    write_stream_ptr_->write(reinterpret_cast<const char *>(&MODULE_INDICATOR_VALUE), sizeof(MODULE_INDICATOR_VALUE));
    write_stream_ptr_->write(reinterpret_cast<const char *>(&module_info.ptr), sizeof(module_info.ptr));
    write_stream_ptr_->write(reinterpret_cast<const char *>(&module_info.checksum), sizeof(module_info.checksum));
    write_stream_ptr_->write(reinterpret_cast<const char *>(&str_size), sizeof(module_info.pathname.length()));
    write_stream_ptr_->write(module_info.pathname.data(), module_info.pathname.length() * sizeof(char));

    written_modules_.insert(module_info);
}

}  // namespace panda::tooling::sampler
