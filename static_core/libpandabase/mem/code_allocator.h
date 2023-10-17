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

#ifndef PANDA_CODEALLOCATOR_H
#define PANDA_CODEALLOCATOR_H

#include "utils/arena_containers.h"
#include "os/mem.h"
#include "os/mutex.h"

namespace panda {

class BaseMemStats;

class CodeAllocator {
public:
    PANDA_PUBLIC_API explicit CodeAllocator(BaseMemStats *mem_stats);
    PANDA_PUBLIC_API ~CodeAllocator();
    NO_COPY_SEMANTIC(CodeAllocator);
    NO_MOVE_SEMANTIC(CodeAllocator);

    /**
     * @brief Allocates @param size bytes, copies @param codeBuff to allocated memory and make this memory executable
     * @param size
     * @param codeBuff
     * @return
     */
    [[nodiscard]] PANDA_PUBLIC_API void *AllocateCode(size_t size, const void *code_buff);

    /**
     * @brief Allocates @param size bytes of non-protected memory
     * @param size to be allocated
     * @return MapRange of the allocated code
     */
    [[nodiscard]] PANDA_PUBLIC_API os::mem::MapRange<std::byte> AllocateCodeUnprotected(size_t size);

    /**
     * Make memory \mem_range executable
     * @param mem_range
     */
    PANDA_PUBLIC_API static void ProtectCode(os::mem::MapRange<std::byte> mem_range);

    /// Fast check if the given program counter belongs to JIT code
    PANDA_PUBLIC_API bool InAllocatedCodeRange(const void *pc);

private:
    void CodeRangeUpdate(void *ptr, size_t size);

private:
    static const Alignment PAGE_LOG_ALIGN;

    // TODO(dtrubenkov): Remove when some CodeCache space will be implemented, currently used for avoid memleak noise
    ArenaAllocator arena_allocator_;
    BaseMemStats *mem_stats_;
    os::memory::RWLock code_range_lock_;
    void *code_range_start_ {nullptr};
    void *code_range_end_ {nullptr};
};

}  // namespace panda

#endif  // PANDA_CODEALLOCATOR_H
