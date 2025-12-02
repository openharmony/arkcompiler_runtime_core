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

#ifndef ARK_GUARD_UTIL_ASSERT_UTIL_H
#define ARK_GUARD_UTIL_ASSERT_UTIL_H

#include "libarkbase/macros.h"

#include "error.h"

#define ARK_GUARD_ERROR_PRINT(errCode, desc, cause, solutions) \
    do {                                                       \
        ark::guard::Error error((errCode));                    \
        error.GetDescStream() << (desc);                       \
        error.GetCauseStream() << (cause);                     \
        error.GetSolutionsStream() << (solutions);             \
        error.Print();                                         \
    } while (0)

#define ARK_GUARD_ASSERT(cond, errCode, desc)             \
    do {                                                  \
        if (UNLIKELY((cond))) {                           \
            ARK_GUARD_ERROR_PRINT(errCode, desc, "", ""); \
            std::abort();                                 \
        }                                                 \
    } while (0)

#define ARK_GUARD_ABORT(errCode, desc)                \
    do {                                              \
        ARK_GUARD_ERROR_PRINT(errCode, desc, "", ""); \
        std::abort();                                 \
    } while (0)

#endif  // ARK_GUARD_UTIL_ASSERT_UTIL_H