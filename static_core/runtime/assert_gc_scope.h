/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ASSERT_GC_SCOPE_H
#define PANDA_RUNTIME_ASSERT_GC_SCOPE_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"

#include <atomic>

namespace ark {

/**
 * @brief GC call guard (RAII).
 *
 * A runtime developer operating on the native heap can guard a code region that is expected
 * not to trigger a garbage collection by instantiating an AssertGCScopeT object on the stack:
 *
 *     void NativeFunc() {
 *         AssertGCScopeT gcScope;
 *         // ... expected not to run GC here ...
 *     }
 *
 * While such an object is alive, running a garbage collection inside the guarded region is
 * forbidden: GC::RunPhases checks AssertGCScopeT::IsAllowed() and aborts the process with a
 * FATAL error if a GC is executed inside a guarded region. This lets developers catch dangerous
 * "raw native pointer held across a GC" bugs early, in any build (debug or release), simply by
 * using the guard where it is needed - no runtime option or debug/release distinction is involved.
 *
 * The guard is reentrant: nested guarded regions are supported (the ref-counter returns to zero
 * only when the outermost region exits).
 */
class AssertGCScopeT {
public:
    AssertGCScopeT()
    {
        Enter();
    }

    ~AssertGCScopeT()
    {
        Exit();
    }

    /// @brief Enter the guarded region: disallow GC until the matching Exit() is called.
    PANDA_PUBLIC_API static void Enter();

    /// @brief Exit the guarded region: re-enable GC for the matching Enter().
    PANDA_PUBLIC_API static void Exit();

    /// @brief true if running a garbage collection is allowed in the current scope
    PANDA_PUBLIC_API static bool IsAllowed();

    NO_COPY_SEMANTIC(AssertGCScopeT);
    NO_MOVE_SEMANTIC(AssertGCScopeT);

private:
    static std::atomic<int> gcFlag_;
};

using DisallowGarbageCollection = AssertGCScopeT;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISALLOW_GARBAGE_COLLECTION [[maybe_unused]] ::ark::DisallowGarbageCollection no_gc_scope_guard

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DCHECK_ALLOW_GARBAGE_COLLECTION                                                           \
    do {                                                                                          \
        if (UNLIKELY(!::ark::AssertGCScopeT::IsAllowed())) {                                      \
            LOG(FATAL, GC) << "Executing garbage collection inside a DISALLOW_GARBAGE_COLLECTION" \
                              " region is forbidden.";                                            \
        }                                                                                         \
    } while (false)

}  // namespace ark

#endif  // PANDA_RUNTIME_ASSERT_GC_SCOPE_H
