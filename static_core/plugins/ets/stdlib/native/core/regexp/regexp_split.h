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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_SPLIT_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_SPLIT_H

#include <ani.h>

namespace ark::ets::stdlib {

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array SplitNativeImpl(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                          ani_int patternSize, ani_int strSize, ani_long limit, ani_boolean unicodeMatching,
                          ani_boolean requiresUtf16Execution);

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_SPLIT_H
