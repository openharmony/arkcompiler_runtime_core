/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

namespace ark::abc2program {

static std::string_view g_globalTypeName = "_GLOBAL";

static const char ARRAY_TYPE_POSTFIX = ']';

bool AbcFileUtils::IsSystemTypeName(const std::string &typeName)
{
    bool isArrayType = typeName.back() == ARRAY_TYPE_POSTFIX;
    bool isGlobal = typeName == g_globalTypeName;

    return isArrayType || isGlobal;
}

std::string AbcFileUtils::GetFileNameByAbsolutePath(const std::string &absolutePath)
{
    LOG(DEBUG, ABC2PROGRAM) << "Absolute path is : " << absolutePath;
    size_t pos = absolutePath.find_last_of(ark::os::file::File::GetPathDelim());
    ASSERT(pos != std::string::npos);
    std::string fileName = absolutePath.substr(pos + 1);
    return fileName;
}

}  // namespace ark::abc2program
