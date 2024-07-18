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

#ifndef LIBPANDAFILE_FILE_INL_H_
#define LIBPANDAFILE_FILE_INL_H_

#include "helpers.h"
#include "file.h"
#include "utils/leb128.h"

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

}  // namespace ark::panda_file

#endif
