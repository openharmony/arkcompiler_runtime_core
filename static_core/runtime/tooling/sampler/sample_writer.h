/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_WRITER_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_WRITER_H

#include <iostream>
#include <string>
#include <fstream>

#include "libpandabase/os/thread.h"

#include "runtime/tooling/sampler/sample_info.h"

#include <unordered_set>

namespace ark::tooling::sampler {

/*
 * =======================================================
 * =============  Sampler binary format ==================
 * =======================================================
 *
 * Writing with the fasters and more convenient format .aspt
 * Then it should be converted to flamegraph
 *
 * .aspt - ark sampling profiler trace file, binary format
 *
 * .aspt consists of 2 type information:
 *   - module row (panda file and its pointer)
 *   - sample row (sample information)
 *
 * module row for 64-bits:
 *   first 8 byte is 0xFFFFFFFF (to recognize that it's not a sample row)
 *   next 8 byte is pointer module
 *   next 8 byte is size of panda file name
 *   next bytes is panda file name in ASCII symbols
 *
 * sample row for 64-bits:
 *   first 4 bytes is thread id of thread from sample was obtained
 *   next 4 bytes is thread status of thread from sample was obtained
 *   next 8 bytes is stack size
 *   next bytes is stack frame
 *   one stack frame is panda file ptr and file id
 *
 * Example for 64-bit architecture:
 *
 *            Thread id   Thread status    Stack Size      Managed stack frame id
 * Sample row |___________|___________|________________|_____________------___________|
 *              32 bits      32 bits        64 bits       (128 * <stack size>) bits
 *
 *              0xFF..FF    pointer   checksum   name size     module path (ASCII str)
 * Module row |__________|__________|__________|___________|_____________------___________|
 *              64 bits    64 bits    32 bits    64 bits       (8 * <name size>) bits
 */
class StreamWriter final {
public:
    explicit StreamWriter(const char *filename)
    {
        /*
         * This class instance should be used only from one thread
         * It may lead to format invalidation
         * This class wasn't made thread safe for performance reason
         */
        writeStreamPtr_ = std::make_unique<std::ofstream>(filename, std::ios::binary);
        ASSERT(writeStreamPtr_ != nullptr);
    }

    ~StreamWriter()
    {
        writeStreamPtr_->flush();
        writeStreamPtr_->close();
    };

    PANDA_PUBLIC_API void WriteModule(const FileInfo &moduleInfo);
    PANDA_PUBLIC_API void WriteSample(const SampleInfo &sample) const;

    bool IsModuleWritten(const FileInfo &moduleInfo) const
    {
        return writtenModules_.find(moduleInfo) != writtenModules_.end();
    }

    NO_COPY_SEMANTIC(StreamWriter);
    NO_MOVE_SEMANTIC(StreamWriter);

    static constexpr uintptr_t MODULE_INDICATOR_VALUE = 0xFFFFFFFF;

private:
    std::unique_ptr<std::ofstream> writeStreamPtr_;
    std::unordered_set<FileInfo> writtenModules_;
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_WRITER_H
