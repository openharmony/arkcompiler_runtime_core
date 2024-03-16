/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "abc_file_utils.h"
#include "os/file.h"

namespace panda::abc2program {

constexpr std::string_view GLOBAL_TYPE_NAME = "_GLOBAL";

const std::string ARRAY_TYPE_PREFIX = "[";

bool AbcFileUtils::IsSystemTypeName(const std::string &type_name)
{
    bool is_array_type = (type_name.find(ARRAY_TYPE_PREFIX) != std::string::npos);
    bool is_global = (type_name == GLOBAL_TYPE_NAME);
    return is_array_type || is_global;
}

std::string AbcFileUtils::GetFileNameByAbsolutePath(const std::string &absolute_path)
{
    size_t pos = absolute_path.find_last_of(panda::os::file::File::GetPathDelim());
    ASSERT(pos != std::string::npos);
    std::string file_name = absolute_path.substr(pos + 1);
    return file_name;
}

}  // namespace panda::abc2program
