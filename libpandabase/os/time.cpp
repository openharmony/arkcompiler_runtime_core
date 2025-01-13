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

#include "os/time.h"

#include <chrono>
#include <ctime>

#ifdef PANDA_TARGET_UNIX
#include "platforms/unix/libpandabase/time.h"
#elif PANDA_TARGET_WINDOWS
#include "platforms/windows/libpandabase/time.h"
#else
#error "Unsupported platform"
#endif  // PANDA_TARGET_UNIX

namespace panda::os::time {

/**
 *  Return thread CPU time in nanoseconds
 */
uint64_t GetClockTimeInThreadCpuTime()
{
    return GetClockTime<std::chrono::nanoseconds>(CLOCK_THREAD_CPUTIME_ID);
}

}  // namespace panda::os::time
