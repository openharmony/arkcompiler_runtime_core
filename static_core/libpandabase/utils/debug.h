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

#ifndef PANDA_DEBUG_H
#define PANDA_DEBUG_H

namespace ark::debug {

[[noreturn]] __attribute__((visibility("default"))) void AssertionFail(const char *expr, const char *file,
                                                                       unsigned line, const char *function);

#if defined(PANDA_TARGET_MOBILE) || defined(PANDA_TARGET_WINDOWS) || defined(PANDA_TARGET_OHOS)
inline void PrintStackTrace([[maybe_unused]] int skip = 1) {}
#else
void PrintStackTrace(int skip = 1);
#endif

}  // namespace ark::debug

#endif  // PANDA_DEBUG_H
