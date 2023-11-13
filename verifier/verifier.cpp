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

#include "verifier.h"

#include "zlib.h"

#include "file.h"

namespace panda::verifier {

bool Verifier::VerifyChecksum(const std::string &filename)
{
    std::unique_ptr<const panda_file::File> file;
    auto file_to_verify = panda_file::File::Open(filename);
    file.swap(file_to_verify);

    if (file == nullptr) {
        return false;
    }

    uint32_t file_chksum = file->GetHeader()->checksum;

    uint32_t abc_offset = 12U; // the offset of file content after checksum
    ASSERT(file->GetHeader()->file_size > abc_offset);
    uint32_t cal_chksum = adler32(1, file->GetBase() + abc_offset, file->GetHeader()->file_size - abc_offset);
    return file_chksum == cal_chksum;
}

} // namespace panda::verifier
