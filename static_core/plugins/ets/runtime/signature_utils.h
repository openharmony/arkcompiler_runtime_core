/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_SIGNATURE_UTILS_H
#define PANDA_PLUGINS_ETS_RUNTIME_SIGNATURE_UTILS_H

#include <string_view>
#include <optional>

#include "libarkbase/macros.h"

namespace ark::ets::signature {

template <typename StringT, char TARGET_SEPARATOR>
std::optional<StringT> NormalizePackageSeparators(const std::string_view signature, size_t start, size_t end)
{
    if (UNLIKELY(start >= end || end > signature.size())) {
        return std::nullopt;
    }

    StringT result(signature);

    bool findColon = false;
    for (size_t i = start; i < end; i++) {
        if (result[i] == ':') {
            if (UNLIKELY(findColon)) {
                return std::nullopt;  // nested packages are not allowed.
            }
            findColon = true;
            result[i] = TARGET_SEPARATOR;
        }
    }

    return result;
}

}  // namespace ark::ets::signature

#endif  // PANDA_PLUGINS_ETS_RUNTIME_SIGNATURE_UTILS_H
