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

#ifndef PANDA_LIBPANDABASE_OS_DARWIN_MEM_HOOKS_H
#define PANDA_LIBPANDABASE_OS_DARWIN_MEM_HOOKS_H

#include "libarkbase/mem/mem.h"
#include "libarkbase/macros.h"

namespace ark::os::darwin::mem_hooks {

class PandaHooks {
public:
    PANDA_PUBLIC_API static void Initialize();
    PANDA_PUBLIC_API static void Enable();
    PANDA_PUBLIC_API static void Disable();
};

}  // namespace ark::os::darwin::mem_hooks

namespace ark::os::mem_hooks {
using PandaHooks = ark::os::darwin::mem_hooks::PandaHooks;
}  // namespace ark::os::mem_hooks

#endif  // PANDA_LIBPANDABASE_OS_DARWIN_MEM_HOOKS_H