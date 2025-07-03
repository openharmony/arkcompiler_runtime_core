/*
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

#include "debug.h"
#include "terminate.h"
#include "os/thread.h"
#include "os/stacktrace.h"
#include <cstring>
#include <iostream>
#include <iomanip>

namespace ark::debug {

[[noreturn]] void AssertionFail(const char *expr, const char *file, unsigned line, const char *function)
{
    int errnum = errno;
    std::cerr << "ASSERTION FAILED: " << expr << std::endl;
    std::cerr << "IN " << file << ":" << std::dec << line << ": " << function << std::endl;
    if (errnum != 0) {
        std::cerr << "ERRNO: " << errnum << " (" << std::strerror(errnum) << ")" << std::endl;
    }
    std::cerr << "Backtrace [tid=" << os::thread::GetCurrentThreadId() << "]:\n";
    PrintStack(std::cerr);
#ifdef FUZZING_EXIT_ON_FAILED_ASSERT
    ark::terminate::Terminate(file);
#else
    std::abort();
#endif
}

}  // namespace ark::debug
