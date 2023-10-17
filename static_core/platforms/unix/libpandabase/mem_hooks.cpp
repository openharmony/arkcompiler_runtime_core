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

#include <link.h>
#include <dlfcn.h>
#include <cstdlib>

#include "mem_hooks.h"
#include "libpandabase/utils/span.h"

namespace panda::os::unix::mem_hooks {

size_t PandaHooks::alloc_via_standard_ = 0;
bool PandaHooks::is_active_ = false;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
PandaHooks::AddrRange PandaHooks::ignore_code_range_;
void *(*PandaHooks::real_malloc_)(size_t) = nullptr;
void *(*PandaHooks::real_memalign_)(size_t, size_t) = nullptr;
void (*PandaHooks::real_free_)(void *) = nullptr;

int FindLibDwarfCodeRegion(dl_phdr_info *info, [[maybe_unused]] size_t size, void *data)
{
    auto arange = reinterpret_cast<PandaHooks::AddrRange *>(data);
    if (std::string_view(info->dlpi_name).find("libdwarf.so") != std::string_view::npos) {
        Span<const ElfW(Phdr)> phdr_list(info->dlpi_phdr, info->dlpi_phnum);
        for (ElfW(Phdr) phdr : phdr_list) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_X) != 0) {
                *arange = PandaHooks::AddrRange(info->dlpi_addr + phdr.p_vaddr, phdr.p_memsz);
                return 1;
            }
        }
    }
    return 0;
}

/* static */
void PandaHooks::Initialize()
{
    // libdwarf allocates a lot of memory using malloc internally.
    // Since this library is used only for debug purpose don't consider
    // malloc calls from this library.
    dl_iterate_phdr(FindLibDwarfCodeRegion, &PandaHooks::ignore_code_range_);
}

/* static */
void PandaHooks::SaveRealFunctions()
{
    real_malloc_ = reinterpret_cast<decltype(real_malloc_)>(
        dlsym(RTLD_NEXT, "malloc"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    ASSERT(real_malloc_ != nullptr);
    real_memalign_ = reinterpret_cast<decltype(real_memalign_)>(
        dlsym(RTLD_NEXT, "memalign"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    ASSERT(real_memalign_ != nullptr);
    real_free_ = reinterpret_cast<decltype(real_free_)>(
        dlsym(RTLD_NEXT, "free"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    ASSERT(real_free_ != nullptr);
}

/* static */
bool PandaHooks::ShouldCountAllocation(const void *caller)
{
    return !ignore_code_range_.Contains(ToUintPtr(caller));
}

/* static */
void *PandaHooks::MallocHook(size_t size, const void *caller)
{
    if (ShouldCountAllocation(caller)) {
        alloc_via_standard_ += size;
    }
    // tracking internal allocator is implemented by malloc, we would fail here with this option
#ifndef TRACK_INTERNAL_ALLOCATIONS
    if (alloc_via_standard_ > MAX_ALLOC_VIA_STANDARD) {
        std::cerr << "Too many usage of standard allocations" << std::endl;
        abort();
    }
#endif                                  // TRACK_INTERNAL_ALLOCATIONS
    void *result = real_malloc_(size);  // NOLINT(cppcoreguidelines-no-malloc)
    if (UNLIKELY(result == nullptr)) {
        std::cerr << "Malloc error" << std::endl;
        abort();
    }
    return result;
}

/* static */
void *PandaHooks::MemalignHook(size_t alignment, size_t size, const void *caller)
{
    if (ShouldCountAllocation(caller)) {
        alloc_via_standard_ += size;
    }
    // tracking internal allocator is implemented by malloc, we would fail here with this option
#ifndef TRACK_INTERNAL_ALLOCATIONS
    if (alloc_via_standard_ > MAX_ALLOC_VIA_STANDARD) {
        std::cerr << "Too many usage of standard allocations" << std::endl;
        abort();
    }
#endif  // TRACK_INTERNAL_ALLOCATIONS
    void *result = real_memalign_(alignment, size);
    if (UNLIKELY(result == nullptr)) {
        std::cerr << "Align error" << std::endl;
        abort();
    }
    return result;
}

/* static */
void PandaHooks::FreeHook(void *ptr, [[maybe_unused]] const void *caller)
{
    real_free_(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
}

/* static */
void PandaHooks::Enable()
{
#ifdef PANDA_USE_MEMORY_HOOKS
    is_active_ = true;
#endif  // PANDA_USE_MEMORY_HOOKS
}

/* static */
void PandaHooks::Disable()
{
#ifdef PANDA_USE_MEMORY_HOOKS
    is_active_ = false;
#endif  // PANDA_USE_MEMORY_HOOKS
}

}  // namespace panda::os::unix::mem_hooks

#ifdef PANDA_USE_MEMORY_HOOKS

// NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
void *malloc(size_t size) noexcept
{
    if (panda::os::mem_hooks::PandaHooks::real_malloc_ == nullptr) {
        panda::os::unix::mem_hooks::PandaHooks::SaveRealFunctions();
    }
    if (panda::os::mem_hooks::PandaHooks::IsActive()) {
        void *caller = __builtin_return_address(0);
        return panda::os::mem_hooks::PandaHooks::MallocHook(size, caller);
    }
    return panda::os::mem_hooks::PandaHooks::real_malloc_(size);
}

// NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
void *memalign(size_t alignment, size_t size) noexcept
{
    if (panda::os::mem_hooks::PandaHooks::real_memalign_ == nullptr) {
        panda::os::unix::mem_hooks::PandaHooks::SaveRealFunctions();
    }
    if (panda::os::mem_hooks::PandaHooks::IsActive()) {
        void *caller = __builtin_return_address(0);
        return panda::os::mem_hooks::PandaHooks::MemalignHook(alignment, size, caller);
    }
    return panda::os::mem_hooks::PandaHooks::real_memalign_(alignment, size);
}

// NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
void free(void *ptr) noexcept
{
    if (panda::os::mem_hooks::PandaHooks::real_free_ == nullptr) {
        panda::os::unix::mem_hooks::PandaHooks::SaveRealFunctions();
    }
    if (panda::os::mem_hooks::PandaHooks::IsActive()) {
        void *caller = __builtin_return_address(0);
        return panda::os::mem_hooks::PandaHooks::FreeHook(ptr, caller);
    }
    return panda::os::mem_hooks::PandaHooks::real_free_(ptr);
}

#endif  // PANDA_USE_MEMORY_HOOKS
