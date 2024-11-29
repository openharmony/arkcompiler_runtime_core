/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_STD_CORE_INTL_COMMON_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_STD_CORE_INTL_COMMON_H

#include "plugins/ets/runtime/napi/ets_napi.h"
#include "unicode/unistr.h"
#include <string>

namespace ark::ets::stdlib::intl {

std::string EtsToStdStr(EtsEnv *env, ets_string etsStr);
icu::UnicodeString EtsToUnicodeStr(EtsEnv *env, ets_string etsStr);
}  // namespace ark::ets::stdlib::intl

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_STD_CORE_INTL_COMMON_H