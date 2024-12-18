/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_GUARD_UTIL_ASSERT_UTIL_H
#define PANDA_GUARD_UTIL_ASSERT_UTIL_H

#include "macros.h"
#include "utils/logger.h"

#define PANDA_GUARD_ASSERT_PRINT(cond, message)  \
    do {                                         \
        if (UNLIKELY((cond))) {                  \
            /* CC-OFFNXT(G.PRE.02) string arg */ \
            LOG(ERROR, PANDAGUARD) << message;   \
            /* CC-OFFNXT(G.PRE.02) string arg */ \
            std::cerr << message << std::endl;   \
            std::abort();                        \
        }                                        \
    } while (0)

#define PANDA_GUARD_ABORT_PRINT(message)     \
    do {                                     \
        /* CC-OFFNXT(G.PRE.02) string arg */ \
        LOG(ERROR, PANDAGUARD) << message;   \
        /* CC-OFFNXT(G.PRE.02) string arg */ \
        std::cerr << message << std::endl;   \
        std::abort();                        \
    } while (0)

#endif  // PANDA_GUARD_UTIL_ASSERT_UTIL_H
