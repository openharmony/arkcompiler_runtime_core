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
#ifndef LIBPANDAFILE_TYPE_HELPER_H_
#define LIBPANDAFILE_TYPE_HELPER_H_

#include "file_items.h"

#include "libpandabase/utils/span.h"

namespace ark::panda_file {

inline ark::panda_file::Type GetEffectiveType(const panda_file::Type &type)
{
    return type;
}

inline bool IsArrayDescriptor(const uint8_t *descriptor)
{
    Span<const uint8_t> sp(descriptor, 1);
    return sp[0] == '[';
}

inline std::string GetDummyClassName()
{
    return "$UNION_FIELD_DUMMY_CLASS";
}

inline bool IsDummyClassName(const std::string &name)
{
    return name == GetDummyClassName();
}
}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_TYPE_HELPER_H_
