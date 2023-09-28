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
#include "libpandabase/macros.h"

namespace panda::os::unix::mem_hooks {

class PandaHooks {
public:
    PANDA_PUBLIC_API static void Initialize();
    PANDA_PUBLIC_API static void Enable();

    PANDA_PUBLIC_API static void Disable();

    static size_t GetAllocViaStandard() noexcept
    {
        return alloc_via_standard_;
    }

    static bool IsActive() noexcept
    {
        return is_active_;
    }

    static void *MallocHook(size_t size, const void *caller);
    static void *MemalignHook(size_t alignment, size_t size, const void *caller);
    static void FreeHook(void *ptr, const void *caller);

    static void SaveRealFunctions();
    static void *(*real_malloc_)(size_t);
    static void *(*real_memalign_)(size_t, size_t);
    static void (*real_free_)(void *);

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

    static bool ShouldCountAllocation(const void *caller);

    static size_t alloc_via_standard_;
    static AddrRange ignore_code_range_;
    static bool is_active_;
};

}  // namespace panda::os::unix::mem_hooks

namespace panda::os::mem_hooks {
using PandaHooks = panda::os::unix::mem_hooks::PandaHooks;
}  // namespace panda::os::mem_hooks

// Don't use for musl and mobile
// Sanitizers hook malloc functions, so we don't use memory hooks
#if !defined(__MUSL__) && !defined(PANDA_TARGET_ARM64) && !defined(PANDA_TARGET_ARM32) && \
    !defined(USE_ADDRESS_SANITIZER) && !defined(USE_THREAD_SANITIZER)
#define PANDA_USE_MEMORY_HOOKS
#endif

#ifdef PANDA_USE_MEMORY_HOOKS

void *malloc(size_t size) noexcept;                      // NOLINT(readability-redundant-declaration)
void *Memalign(size_t alignment, size_t size) noexcept;  // NOLINT(readability-redundant-declaration)
void free(void *ptr) noexcept;                           // NOLINT(readability-redundant-declaration)

#endif  // PANDA_USE_MEMORY_HOOKS

#endif  // PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_
