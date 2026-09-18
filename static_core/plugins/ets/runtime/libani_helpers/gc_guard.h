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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_GC_GUARD_H
#define PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_GC_GUARD_H

#include "libarkbase/macros.h"
#include "runtime/assert_gc_scope.h"

namespace arkts {

/**
 * @brief RAII guard that makes garbage collection fatal for the lifetime of the object.
 *
 * Use this only in runtime-owned native code, such as ArkUI or Web native implementations:
 *
 *     void NativeFunc()
 *     {
 *         GCGuard gcGuard;
 *         // A GC triggered here terminates the process with a FATAL error.
 *     }
 *
 * This is a libani_helpers C++ helper, not an ANI function-table API. Consequently, it does not
 * change the ani_env ABI or expose an API to application developers. Nested guards are supported.
 */
class PANDA_PUBLIC_API GCGuard final {
public:
    GCGuard()
    {
        ::ark::AssertGCScopeT::Enter();
    }

    ~GCGuard()
    {
        ::ark::AssertGCScopeT::Exit();
    }

    NO_COPY_SEMANTIC(GCGuard);
    NO_MOVE_SEMANTIC(GCGuard);
};

}  // namespace arkts

#endif  // PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_GC_GUARD_H
