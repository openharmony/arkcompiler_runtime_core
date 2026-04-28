/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ARK_COMM_RUNTIME_ALLOCATOR_PAGE_POOL_H
#define ARK_COMM_RUNTIME_ALLOCATOR_PAGE_POOL_H

#include <atomic>
#ifdef _WIN64
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif

#include "common_components/base/globals.h"
#include "common_components/base/sys_call.h"
#include "common_components/heap/allocator/treap.h"
#include "common_components/platform/os.h"
#include "securec.h"
#if defined(_WIN64) || defined(__APPLE__)
#include "common_components/base/mem_utils.h"
#endif

#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/logger.h"

namespace common_vm {
// a page pool maintain a pool of free pages, serve page allocation and free
class PagePool {
public:
    explicit PagePool(const char *name) : tag_(name) {}
    PagePool(PagePool const &) = delete;
    PagePool &operator=(const PagePool &) = delete;
    ~PagePool() {}
    void Init(uint32_t pageCount)
    {
        totalPageCount_ = pageCount;
        // Atomic with seq_cst order reason: initialization of atomic variable with requirement for sequentially
        // consistent order where threads observe all modifications in the same order
        smallPageUsed_.store(0, std::memory_order_seq_cst);
        usedZone_ = 0;
        size_t size = static_cast<size_t>(totalPageCount_) * COMMON_PAGE_SIZE;
        freePagesTree_.Init(totalPageCount_);
        base_ = MapMemory(size, tag_);
        totalSize_ = size;
    }
    void Fini()
    {
        freePagesTree_.Fini();
#ifdef _WIN64
        LOG_IF(UNLIKELY(!VirtualFree(base_, 0, MEM_RELEASE)), ERROR, MEMORYPOOL)
            << "VirtualFree failed in PagePool destruction, errno: " << GetLastError();
#else
        LOG_IF(UNLIKELY(munmap(base_, totalPageCount_ * COMMON_PAGE_SIZE) != EOK), ERROR, MEMORYPOOL)
            << "munmap failed in PagePool destruction, errno: " << errno;
#endif
    }

    uint8_t *GetPage(size_t bytes = COMMON_PAGE_SIZE)
    {
        uint32_t idx = 0;
        size_t count = (bytes + COMMON_PAGE_SIZE - 1) / COMMON_PAGE_SIZE;
        size_t pageSize = RoundUp(bytes, COMMON_PAGE_SIZE);
        LOG_IF(UNLIKELY(count >= std::numeric_limits<uint32_t>::max()), FATAL, MM_OBJECT_EVENTS)
            << "native memory out of memory!";
        {
            ark::os::memory::LockHolder lg(freePagesMutex_);
            if (freePagesTree_.TakeUnits(static_cast<uint32_t>(count), idx, false)) {
                auto *ret = base_ + static_cast<size_t>(idx) * COMMON_PAGE_SIZE;
#ifdef _WIN64
                LOG_IF(UNLIKELY(!VirtualAlloc(ret, pageSize, MEM_COMMIT, PAGE_READWRITE)), ERROR, MEMORYPOOL)
                    << "VirtualAlloc commit failed in GetPage, errno: " << GetLastError();
#endif
                return ret;
            }
            if ((usedZone_ + pageSize) <= totalSize_ && base_ != nullptr) {
                size_t current = usedZone_;
                usedZone_ += pageSize;
#ifdef _WIN64
                LOG_IF(UNLIKELY(!VirtualAlloc(base_ + current, pageSize, MEM_COMMIT, PAGE_READWRITE)), ERROR,
                       MEMORYPOOL)
                    << "VirtualAlloc commit failed in GetPage, errno: " << GetLastError();
#endif
                return base_ + current;
            }
        }
        return MapMemory(pageSize, tag_, true);
    }

    void ReturnPage(uint8_t *page, size_t bytes = COMMON_PAGE_SIZE) noexcept
    {
        uint8_t *end = base_ + totalSize_;
        size_t num = (bytes + COMMON_PAGE_SIZE - 1) / COMMON_PAGE_SIZE;
        if (page < base_ || page >= end) {
#ifdef _WIN64
            LOG_IF(UNLIKELY(!VirtualFree(page, 0, MEM_RELEASE)), ERROR, MEMORYPOOL)
                << "VirtualFree failed in ReturnPage, errno: " << GetLastError();
#else
            LOG_IF(UNLIKELY(munmap(page, num * COMMON_PAGE_SIZE) != EOK), ERROR, MEMORYPOOL)
                << "munmap failed in ReturnPage, errno: " << errno;
#endif
            return;
        }
        LOG_IF(UNLIKELY(num >= std::numeric_limits<uint32_t>::max()), FATAL, MM_OBJECT_EVENTS)
            << "native memory out of memory!";
        uint32_t idx = static_cast<uint32_t>((page - base_) / COMMON_PAGE_SIZE);
#if defined(_WIN64)
        LOG_IF(UNLIKELY(!VirtualFree(page, num * COMMON_PAGE_SIZE, MEM_DECOMMIT)), ERROR, MEMORYPOOL)
            << "VirtualFree failed in ReturnPage, errno: " << GetLastError();
#elif defined(__APPLE__)
        MemorySet(reinterpret_cast<uintptr_t>(page), num * COMMON_PAGE_SIZE, 0, num * COMMON_PAGE_SIZE);
        (void)madvise(page, num * COMMON_PAGE_SIZE, MADV_DONTNEED);
#else
        (void)madvise(page, num * COMMON_PAGE_SIZE, MADV_DONTNEED);
#endif
        ark::os::memory::LockHolder lg(freePagesMutex_);
        LOG_IF(UNLIKELY(!freePagesTree_.MergeInsert(idx, static_cast<uint32_t>(num), false)), FATAL, MM_OBJECT_EVENTS)
            << "tid " << GetTid() << ": failed to return pages to freePagesTree [" << idx << "+" << num << ", "
            << (idx + num) << ")";
    }

    // return unused pages to os
    void Trim() const {}

    PANDA_PUBLIC_API static PagePool &Instance() noexcept;

protected:
    uint8_t *MapMemory(size_t size, const char *memName, bool isCommit = false) const
    {
#ifdef _WIN64
        void *result = VirtualAlloc(NULL, size, isCommit ? MEM_COMMIT : MEM_RESERVE, PAGE_READWRITE);
        if (result == NULL) {  // LCOV_EXCL_BR_LINE
            LOG(FATAL, COMMON) << "allocate create page failed! Out of Memory!";
            UNREACHABLE();
        }
        (void)memName;
#else
        void *result = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        LOG_IF(UNLIKELY(result == MAP_FAILED), FATAL, MM_OBJECT_EVENTS)
            << "allocate create page failed! Out of Memory!";
#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
        (void)madvise(result, size, MADV_NOHUGEPAGE);
#endif
        (void)isCommit;
#endif

#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
        COMMON_PRCTL(result, size, memName);
#endif
        os::PrctlSetVMA(result, size, (std::string("ArkTS Heap CMCGC PagePool ") + memName).c_str());
        return reinterpret_cast<uint8_t *>(result);
    }

    ark::os::memory::Mutex freePagesMutex_;
    Treap freePagesTree_;
    uint8_t *base_ = nullptr;  // start address of the mapped pages
    size_t totalSize_ = 0;     // total size of the mapped pages
    size_t usedZone_ = 0;      // used zone for native memory pool.
    const char *tag_ = nullptr;

private:
    std::atomic<uint32_t> smallPageUsed_ = {0};
    uint32_t totalPageCount_ = 0;
};
}  // namespace common_vm
#endif  // ARK_COMM_RUNTIME_ALLOCATOR_PAGE_POOL_H
