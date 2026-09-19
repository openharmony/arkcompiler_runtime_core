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

#include <array>

#include "plugins/ets/runtime/coldreload/coldreload.h"

namespace ark::ets::coldreload {

namespace {

// Names of the `Error` values, in ordinal order; kept in sync with the enum by the assert below.
constexpr std::array<const char *, 6> ERROR_NAMES = {
    "NONE", "INVALID_INPUT_ARG", "PATCH_OPEN_FAILED", "AOT_LOADED", "CLASSES_ALREADY_LOADED", "INTERNAL",
};

}  // namespace

const char *GetErrorString(Error err)
{
    static_assert(ERROR_NAMES.size() == static_cast<size_t>(Error::INTERNAL) + 1,
                  "every Error value needs a name in ERROR_NAMES");
    const auto idx = static_cast<size_t>(err);
    return idx < ERROR_NAMES.size() ? ERROR_NAMES[idx] : "UNKNOWN";
}

}  // namespace ark::ets::coldreload
