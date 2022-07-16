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
#ifndef PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_
#define PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_

#include "libpandabase/mem/mem.h"

namespace panda::os::unix::mem_hooks {

class PandaHooks {
public:
    static void Initialize();
    static void Enable();

    static void Disable();

    static size_t GetAllocViaStandard() noexcept
    {
        return alloc_via_standard;
    }

    class AddrRange {
    public:
        AddrRange() = default;
        ~AddrRange() = default;
        DEFAULT_COPY_SEMANTIC(AddrRange);
        DEFAULT_MOVE_SEMANTIC(AddrRange);

        AddrRange(uintptr_t base, size_t size) : base_(base), size_(size) {}

        bool Contains(uintptr_t addr) const
        {
            return base_ <= addr && addr < base_ + size_;
        }

    private:
        uintptr_t base_ {0};
        size_t size_ {0};
    };

private:
    static constexpr size_t MAX_ALLOC_VIA_STANDARD = 4_MB;

    static void SaveMemHooks();

    static void SetMemHooks();
    static bool ShouldCountAllocation(const void *caller);

    static void *(*volatile old_malloc_hook)(size_t, const void *);
    static void *(*volatile old_memalign_hook)(size_t, size_t, const void *);
    static void (*volatile old_free_hook)(void *, const void *);

    static void *MallocHook(size_t size, const void *caller);
    static void *MemalignHook(size_t alignment, size_t size, const void *caller);
    static void FreeHook(void *ptr, const void *caller);

    static size_t alloc_via_standard;
    static AddrRange ignore_code_range;
};

}  // namespace panda::os::unix::mem_hooks

namespace panda::os::mem_hooks {
using PandaHooks = panda::os::unix::mem_hooks::PandaHooks;
}  // namespace panda::os::mem_hooks

#endif  // PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_
