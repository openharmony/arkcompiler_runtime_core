/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_BASE_COMMON_H
#define COMMON_RUNTIME_COMMON_INTERFACES_BASE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "libarkbase/macros.h"

// Windows platform will define ERROR, cause compiler error
#ifdef ERROR
#undef ERROR
#endif
namespace common_vm {

#if defined(NDEBUG)
// CC-OFFNXT(G.PRE.02) code readability, standard log macro approach
#define DCHECK(x)
#else
// CC-OFFNXT(G.PRE.02) code readability, standard log macro approach
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DCHECK(x) CHECK(x)
#endif

// NOLINTNEXTLINE(readability-identifier-naming)
enum class LOG_LEVEL : uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
    FOLLOW = 100,  // if hilog enabled follow hilog, otherwise use INFO level
};

static constexpr size_t BITS_PER_BYTE = 8;
}  // namespace common_vm

#endif  // COMMON_RUNTIME_COMMON_INTERFACES_BASE_COMMON_H
