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

#include <mem/mem.h>
#include "os/file.h"

namespace panda::os::unix::native_stack {

bool ReadOsFile(const std::string &file_name, std::string *result)
{
    panda::os::unix::file::File cmdfile = panda::os::file::Open(file_name, panda::os::file::Mode::READONLY);
    panda::os::file::FileHolder fholder(cmdfile);
    constexpr size_t BUFF_SIZE = 8_KB;
    std::vector<char> buffer(BUFF_SIZE);
    auto res = cmdfile.Read(&buffer[0], buffer.size());
    if (res) {
        result->append(&buffer[0], res.Value());
        return true;
    }
    return false;
}

bool WriterOsFile(const void *buffer, size_t count, int fd)
{
    panda::os::unix::file::File myfile(fd);
    panda::os::file::FileHolder fholder(myfile);
    return myfile.WriteAll(buffer, count);
}

}  // namespace panda::os::unix::native_stack
