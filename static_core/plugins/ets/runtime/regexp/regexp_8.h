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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_REGEXP_REGEXP_8_H
#define PANDA_PLUGINS_ETS_RUNTIME_REGEXP_REGEXP_8_H

#include "plugins/ets/runtime/regexp/regexp_executor.h"
#include "runtime/include/mem/panda_string.h"

namespace ark::ets {

using Pcre2Obj = void *;

class RegExp8 {
public:
    static Pcre2Obj CreatePcre2Object(const uint8_t *pattern, uint32_t flags, uint32_t extraFlags, const int len);
    static RegExpExecResult Execute(Pcre2Obj re, const uint8_t *str, const int len, const int startOffset);
    static void ExtractGroups(Pcre2Obj expression, int count, RegExpExecResult &result, void *data);
    static void FreePcre2Object(Pcre2Obj re);
};

}  // namespace ark::ets
#endif  // PANDA_PLUGINS_ETS_RUNTIME_REGEXP_REGEXP_8_H