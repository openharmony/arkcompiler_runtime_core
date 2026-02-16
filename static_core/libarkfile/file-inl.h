/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef LIBPANDAFILE_FILE_INL_H_
#define LIBPANDAFILE_FILE_INL_H_

#include "libarkfile/helpers.h"
#include "libarkfile/file.h"
#include "libarkbase/utils/leb128.h"

namespace ark::panda_file {

inline File::StringData File::GetStringData(EntityId id) const
{
    StringData strData {};
    auto sp = GetSpanFromId(id);

    auto tagUtf16Length = panda_file::helpers::ReadULeb128(&sp);
    strData.utf16Length = tagUtf16Length >> 1U;
    strData.isAscii = static_cast<bool>(tagUtf16Length & 1U);
    strData.data = sp.data();

    return strData;
}

// CC-OFFNXT(G.FUD.06) switch-case
inline std::string StringDataToString(File::StringData sd)
{
    const char *data = utf::Mutf8AsCString(sd.data);
    std::string result;
    // * 1.2 because of character replacement in the below loop
    result.reserve(sd.utf16Length * 1.2);

    for (const char *p = data; *p; ++p) {
        char c = *p;
        switch (c) {
            case '\a':
                result.append("\\a");
                break;
            case '\b':
                result.append("\\b");
                break;
            case '\f':
                result.append("\\f");
                break;
            case '\n':
                result.append("\\n");
                break;
            case '\r':
                result.append("\\r");
                break;
            case '\t':
                result.append("\\t");
                break;
            case '\v':
                result.append("\\v");
                break;
            case '\'':
                result.append("\\'");
                break;
            case '\?':
                result.append("\\?");
                break;
            case '\\':
                result.append("\\\\");
                break;
            default:
                result.push_back(c);
                break;
        }
    }
    result.shrink_to_fit();
    return result;
}

}  // namespace ark::panda_file

#endif
