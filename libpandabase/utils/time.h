/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_UTILS_TIME_H
#define LIBPANDABASE_UTILS_TIME_H

#include <cstdint>

#include "macros.h"

WEAK_FOR_LTO_START

namespace panda::time {

/**
 *  Return current time in milliseconds
 */
uint64_t GetCurrentTimeInMillis(bool need_system = false);

/**
 *  Return current time in nanoseconds
 */
uint64_t GetCurrentTimeInNanos(bool need_system = false);

}  // namespace panda::time

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_UTILS_TIME_H
