/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "runtime/arch/asm_support.h"
#include "libpandabase/utils/arch.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/native_pointer.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/method.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/include/thread.h"
#include "plugins_defines.h"

namespace ark {

// CC-OFFNXT(C_RULE_ID_MACRODEFINE_ENDWITH_SEMICOLON) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_VALUE(name, value) static_assert((name) == (value));
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_VALUE_WITH_TYPE(name, value, type) static_assert((name) == (value));
#include "asm_defines/asm_defines.def"

// Frame doesn't have aligned storage, so check its offset manually
#ifdef PANDA_TARGET_64
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(FRAME_METHOD_OFFSET == 8);
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(FRAME_PREV_FRAME_OFFSET == 0);
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(FRAME_SLOT_OFFSET == 80);
#endif

extern "C" ManagedThread *GetCurrentManagedThread()
{
    return ManagedThread::GetCurrent();
}

extern "C" void AsmUnreachable()
{
    UNREACHABLE();
}

#if !defined(PANDA_TARGET_ARM64)
extern "C" void OsrEntryAfterCFrame([[maybe_unused]] Frame *frame, [[maybe_unused]] uintptr_t loopHeadBc,
                                    [[maybe_unused]] const void *osrCode, [[maybe_unused]] size_t frameSize)
{
    UNREACHABLE();
}
extern "C" void OsrEntryAfterIFrame([[maybe_unused]] Frame *frame, [[maybe_unused]] uintptr_t loopHeadBc,
                                    [[maybe_unused]] const void *osrCode, [[maybe_unused]] size_t frameSize)
{
    UNREACHABLE();
}
extern "C" void OsrEntryTopFrame([[maybe_unused]] Frame *frame, [[maybe_unused]] uintptr_t loopHeadBc,
                                 [[maybe_unused]] const void *osrCode, [[maybe_unused]] size_t frameSize)
{
    UNREACHABLE();
}
#endif  // !defined(PANDA_TARGET_ARM64)

}  // namespace ark
