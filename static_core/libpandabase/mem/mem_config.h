/*
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

#ifndef PANDA_RUNTIME_MEM_MEM_CONFIG_H
#define PANDA_RUNTIME_MEM_MEM_CONFIG_H

#include "macros.h"
#include "libpandabase/mem/mem.h"
#include "utils/asan_interface.h"

#include <cstddef>

namespace panda::mem {

/// class for global memory parameters
class MemConfig {
public:
    static void Initialize(size_t object_pool_size, size_t internal_size, size_t compiler_size, size_t code_size,
                           size_t frames_size, size_t stacks_size, size_t initial_object_pool_size)
    {
        ASSERT(!is_initialized_);
        initial_heap_size_limit_ = initial_object_pool_size;
        heap_size_limit_ = object_pool_size;
        internal_memory_size_limit_ = internal_size;
        compiler_memory_size_limit_ = compiler_size;
        code_cache_size_limit_ = code_size;
        frames_memory_size_limit_ = frames_size;
        native_stacks_memory_size_limit_ = stacks_size;
        is_initialized_ = true;
    }

    static void Initialize(size_t object_pool_size, size_t internal_size, size_t compiler_size, size_t code_size,
                           size_t frames_size, size_t stacks_size)
    {
        Initialize(object_pool_size, internal_size, compiler_size, code_size, frames_size, stacks_size,
                   object_pool_size);
    }

    static void Finalize()
    {
        is_initialized_ = false;
        heap_size_limit_ = 0;
        internal_memory_size_limit_ = 0;
        code_cache_size_limit_ = 0;
    }

    static size_t GetInitialHeapSizeLimit()
    {
        ASSERT(is_initialized_);
        return initial_heap_size_limit_;
    }

    static size_t GetHeapSizeLimit()
    {
        ASSERT(is_initialized_);
        return heap_size_limit_;
    }

    static size_t GetInternalMemorySizeLimit()
    {
        ASSERT(is_initialized_);
        return internal_memory_size_limit_;
    }

    static size_t GetCodeCacheSizeLimit()
    {
        ASSERT(is_initialized_);
        return code_cache_size_limit_;
    }

    static size_t GetCompilerMemorySizeLimit()
    {
        ASSERT(is_initialized_);
        return compiler_memory_size_limit_;
    }

    static size_t GetFramesMemorySizeLimit()
    {
        ASSERT(is_initialized_);
        return frames_memory_size_limit_;
    }

    static size_t GetNativeStacksMemorySizeLimit()
    {
        ASSERT(is_initialized_);
        return native_stacks_memory_size_limit_;
    }

    MemConfig() = delete;

    ~MemConfig() = delete;

    NO_COPY_SEMANTIC(MemConfig);
    NO_MOVE_SEMANTIC(MemConfig);

private:
    PANDA_PUBLIC_API static bool is_initialized_;
    PANDA_PUBLIC_API static size_t initial_heap_size_limit_;          // Initial heap size
    PANDA_PUBLIC_API static size_t heap_size_limit_;                  // Max heap size
    PANDA_PUBLIC_API static size_t internal_memory_size_limit_;       // Max internal memory used by the VM
    PANDA_PUBLIC_API static size_t code_cache_size_limit_;            // The limit for compiled code size.
    PANDA_PUBLIC_API static size_t compiler_memory_size_limit_;       // Max memory used by compiler
    PANDA_PUBLIC_API static size_t frames_memory_size_limit_;         // Max memory used for frames
    PANDA_PUBLIC_API static size_t native_stacks_memory_size_limit_;  // Limit for manually (i.e. not by OS means on
                                                                      // thread creation) allocated native stacks
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_MEM_CONFIG_H
