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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COPY_DATA_MANAGER_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COPY_DATA_MANAGER_H

#include "common_interfaces/base/common.h"
#include "common_components/base/immortal_wrapper.h"
#include "common_components/heap/heap.h"
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
#include <sys/mman.h>
#endif

#include "common_components/base/mem_utils.h"
#include "common_components/base/sys_call.h"
#include "common_components/heap/collector/region_bitmap.h"

#include "libarkbase/utils/logger.h"

#ifdef _WIN64
#include "common_components/base/atomic_spin_lock.h"
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif

namespace ark::common_vm {

class HeapBitmapManager {
    class Memory {
    public:
        struct Zone {
            enum ZoneType : size_t {
                BIT_MAP,
                ZONE_TYPE_CNT,
            };
            uintptr_t zoneStartAddress = 0;
            std::atomic<uintptr_t> zonePosition;
        };
        Memory() = default;
        void InitializeMemory(uintptr_t start, size_t sz, size_t unitCount)
        {
            startAddress_ = start;
            size_ = sz;
            InitZones(unitCount);
        }
        void InitZones(size_t unitCount)
        {
            uintptr_t start = startAddress_;
            allocZone_[Zone::ZoneType::BIT_MAP].zoneStartAddress = start;
            // Atomic with seq_cst order reason: initialization of zonePosition with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            allocZone_[Zone::ZoneType::BIT_MAP].zonePosition.store(start, std::memory_order_seq_cst);
#if defined(_WIN64)
            // Atomic with seq_cst order reason: data race with lastCommitEndAddr with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            lastCommitEndAddr[Zone::ZoneType::BIT_MAP].store(start, std::memory_order_seq_cst);
#endif
        }
        uintptr_t Allocate(Zone::ZoneType type, size_t sz)
        {
#if defined(_WIN64)
            allocSpinLock.Lock();
            // Atomic with acq_rel order reason: data race with zonePosition during concurrent allocation
            uintptr_t startAddr = allocZone_[type].zonePosition.fetch_add(sz, std::memory_order_acq_rel);
            uintptr_t endAddr = startAddr + sz;
            // Atomic with relaxed order reason: data race with lastCommitEndAddr with no synchronization or ordering
            // constraints imposed on other reads or writes
            uintptr_t lastAddr = lastCommitEndAddr[type].load(std::memory_order_relaxed);
            if (endAddr <= lastAddr) {
                allocSpinLock.Unlock();
                return startAddr;
            }
            size_t pageSize = RoundUp(sz, COMMON_PAGE_SIZE);
            LOG_IF(UNLIKELY(!VirtualAlloc(reinterpret_cast<void *>(lastAddr), pageSize, MEM_COMMIT, PAGE_READWRITE)),
                   ERROR, GC)
                << "VirtualAlloc commit failed in Allocate, errno: " << GetLastError();
            // Atomic with seq_cst order reason: data race with lastCommitEndAddr with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            lastCommitEndAddr[type].store(lastAddr + pageSize, std::memory_order_seq_cst);
            allocSpinLock.Unlock();
            return startAddr;
#else
            // Atomic with acq_rel order reason: data race with zonePosition during concurrent allocation
            auto address = allocZone_[type].zonePosition.fetch_add(sz, std::memory_order_acq_rel);
            MemorySet(address, sz, 0, sz);
            return address;
#endif
        }
        void ReleaseMemory()
        {
#if defined(_WIN64)
            LOG_IF(UNLIKELY(!VirtualFree(reinterpret_cast<void *>(startAddress_), size_, MEM_DECOMMIT)), ERROR, GC)
                << "VirtualFree failed in ReturnPage, errno: " << GetLastError();
#elif defined(__APPLE__)
            (void)madvise(reinterpret_cast<void *>(startAddress_), size_, MADV_DONTNEED);
#else
            LOG(DEBUG, GC) << "clear copy-data @[0x" << std::hex << startAddress_ << std::dec << "+" << size_ << ", 0x"
                           << std::hex << startAddress_ + size_ << std::dec << ")";
            if (madvise(reinterpret_cast<void *>(startAddress_), size_, MADV_DONTNEED) == 0) {
                LOG(DEBUG, GC) << "release copy-data @[0x" << std::hex << startAddress_ << std::dec << "+" << size_
                               << ", 0x" << std::hex << startAddress_ + size_ << std::dec << ")";
            }
#endif
            for (size_t i = 0; i < Zone::ZoneType::ZONE_TYPE_CNT; ++i) {
                // Atomic with seq_cst order reason: reset of zonePosition with requirement for sequentially consistent
                // order where threads observe all modifications in the same order
                allocZone_[i].zonePosition.store(allocZone_[i].zoneStartAddress, std::memory_order_seq_cst);
#if defined(_WIN64)
                // Atomic with seq_cst order reason: data race with lastCommitEndAddr with requirement for sequentially
                // consistent order where threads observe all modifications in the same order
                lastCommitEndAddr[i].store(allocZone_[i].zoneStartAddress, std::memory_order_seq_cst);
#endif
            }
        }

    private:
        Zone allocZone_[Zone::ZONE_TYPE_CNT];
        uintptr_t startAddress_ = 0;
        size_t size_ = 0;
#if defined(_WIN64)
        std::atomic<uintptr_t> lastCommitEndAddr[Zone::ZONE_TYPE_CNT];
        AtomicSpinLock allocSpinLock;
#endif
    };

public:
    HeapBitmapManager() = default;
    ~HeapBitmapManager()
    {
        CHECK(!initialized);
    }

    static HeapBitmapManager &GetHeapBitmapManager();

    void InitializeHeapBitmap();
    void DestroyHeapBitmap();

    void ClearHeapBitmap()
    {
        heapBitmap_[0].ReleaseMemory();
    }

    RegionBitmap *AllocateRegionBitmap(size_t regionSize)
    {
        uintptr_t addr =
            heapBitmap_[0].Allocate(Memory::Zone::ZoneType::BIT_MAP, RegionBitmap::GetRegionBitmapSize(regionSize));
        RegionBitmap *bitmap = reinterpret_cast<RegionBitmap *>(addr);
        CHECK(bitmap != nullptr);
        new (bitmap) RegionBitmap(regionSize);
        return bitmap;
    }

private:
    size_t GetHeapBitmapSize(size_t heapSize)
    {
        const size_t REGION_UNIT_SIZE = COMMON_PAGE_SIZE;  // must be equal to RegionDesc::UNIT_SIZE
        heapSize = RoundUp<size_t>(heapSize, REGION_UNIT_SIZE);
        size_t unitCnt = heapSize / REGION_UNIT_SIZE;
        regionUnitCount_ = unitCnt;
        // 64: 1 bit in bitmap marks 8bytes in region, i.e., 1 byte in bitmap marks 64 bytes in region.
        constexpr uint8_t bitMarksSize = 64;
        // 3 bitmaps for each region: markBitmap,resurrectBitmap, enqueueBitmap.
        constexpr uint8_t nBitmapTypes = 3;
        return unitCnt * (sizeof(RegionBitmap) + (REGION_UNIT_SIZE / bitMarksSize)) * nBitmapTypes;
    }

    Memory heapBitmap_[1];
    size_t regionUnitCount_ = 0;
    uintptr_t heapBitmapStart_ = 0;
    size_t allHeapBitmapSize_ = 0;
    bool initialized = false;
};
}  // namespace ark::common_vm
#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COPY_DATA_MANAGER_H
