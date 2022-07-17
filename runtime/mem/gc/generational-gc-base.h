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

#ifndef RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_H
#define RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_H

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/include/mem/allocator.h"

namespace panda::mem {
namespace test {
class MemStatsGenGCTest;
}  // namespace test
/**
 * Base class for generational GC
 */
template <class LanguageConfig>
class GenerationalGC : public GCLang<LanguageConfig> {
public:
    using ConcurrentMarkPredicateT = std::function<bool()>;
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    CardTable *GetCardTable() override
    {
        return card_table_.get();
    }

protected:
    GenerationalGC(ObjectAllocatorBase *object_allocator, const GCSettings &settings)
        : GCLang<LanguageConfig>(object_allocator, settings)
    {
    }
    virtual bool ShouldRunTenuredGC(const GCTask &task);

    void DisableTenuredGC()
    {
        major_period_ = DISABLED_MAJOR_PERIOD;  // Disable tenured GC temporarily.
    }

    void RestoreTenuredGC()
    {
        major_period_ = DEFAULT_MAJOR_PERIOD;
    }

    ALWAYS_INLINE size_t GetMajorPeriod() const
    {
        return major_period_;
    }

    void PostForkCallback() override
    {
        GenerationalGC<LanguageConfig>::RestoreTenuredGC();
    }

    void WaitForGC(GCTask task) override;

    /**
     * Concurrently marking all objects
     * @param objects_stack
     */
    template <typename Marker>
    NO_THREAD_SAFETY_ANALYSIS void ConcurrentMark(Marker *marker, GCMarkingStackType *objects_stack,
                                                  CardTableVisitFlag visit_card_table_roots,
                                                  const ConcurrentMarkPredicateT &pred,
                                                  const ReferenceCheckPredicateT &ref_pred,
                                                  const MemRangeChecker &mem_range_checker);

    /**
     * Marking all objects on pause
     * @param objects_stack
     */
    template <typename Marker>
    NO_THREAD_SAFETY_ANALYSIS void OnPauseMark(Marker *marker, GCMarkingStackType *objects_stack,
                                               CardTableVisitFlag visit_card_table_roots,
                                               const ConcurrentMarkPredicateT &pred,
                                               const ReferenceCheckPredicateT &ref_pred,
                                               const MemRangeChecker &mem_range_checker);

    template <typename Marker>
    NO_THREAD_SAFETY_ANALYSIS void MarkImpl(Marker *marker, GCMarkingStackType *objects_stack,
                                            CardTableVisitFlag visit_card_table_roots,
                                            const ConcurrentMarkPredicateT &pred,
                                            const ReferenceCheckPredicateT &ref_pred,
                                            const MemRangeChecker &mem_range_checker);

    /**
     * Mark all objects in stack recursively for Full GC.
     */
    template <typename Marker>
    void MarkStack(Marker *marker, GCMarkingStackType *stack, const ReferenceCheckPredicateT &ref_pred,
                   const GC::MarkPredicate &markPredicate);

    /**
     * Mark all objects in stack recursively for Full GC.
     */
    template <typename Marker>
    void MarkStackCond(Marker *marker, GCMarkingStackType *stack, const ConcurrentMarkPredicateT &pred,
                       const ReferenceCheckPredicateT &ref_pred, const GC::MarkPredicate &markPredicate);

    /**
     * Update statistics in MemStats. Required initialized mem_stats_ field.
     * @param bytes_in_heap_before - bytes in heap before the GC
     * @param update_tenured_stats - if true, we will update tenured moved and tenured deleted memstats too
     * @param record_allocation_for_moved_objects - if true, we will record allocation for all moved objects (young and
     *  tenured)
     */
    void UpdateMemStats(size_t bytes_in_heap_before, bool update_tenured_stats = false,
                        bool record_allocation_for_moved_objects = false);

    class MemStats {
    public:
        template <bool atomic = false>
        ALWAYS_INLINE void RecordCountFreedYoung(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&young_free_object_count_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                young_free_object_count_ += count;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordSizeFreedYoung(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&young_free_object_size_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                young_free_object_size_ += size;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordCountMovedYoung(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&young_move_object_count_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                young_move_object_count_ += count;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordSizeMovedYoung(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&young_move_object_size_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                young_move_object_size_ += size;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordCountMovedTenured(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&tenured_move_object_count_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenured_move_object_count_ += count;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordSizeMovedTenured(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&tenured_move_object_size_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenured_move_object_size_ += size;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordCountFreedTenured(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&tenured_free_object_count_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenured_free_object_count_ += count;
            }
        }

        template <bool atomic = false>
        ALWAYS_INLINE void RecordSizeFreedTenured(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&tenured_free_object_size_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenured_free_object_size_ += size;
            }
        }

        void Reset()
        {
            young_free_object_count_ = 0U;
            young_free_object_size_ = 0U;
            young_move_object_count_ = 0U;
            young_move_object_size_ = 0U;
            tenured_free_object_count_ = 0U;
            tenured_free_object_size_ = 0U;
            tenured_move_object_count_ = 0U;
            tenured_move_object_size_ = 0U;
        }

        PandaString Dump();

        ALWAYS_INLINE size_t GetCountFreedYoung()
        {
            return young_free_object_count_;
        }

        ALWAYS_INLINE size_t GetSizeFreedYoung()
        {
            return young_free_object_size_;
        }

        ALWAYS_INLINE size_t GetCountMovedYoung()
        {
            return young_move_object_count_;
        }

        ALWAYS_INLINE size_t GetSizeMovedYoung()
        {
            return young_move_object_size_;
        }

        ALWAYS_INLINE size_t GetCountFreedTenured()
        {
            return tenured_free_object_count_;
        }

        ALWAYS_INLINE size_t GetSizeFreedTenured()
        {
            return tenured_free_object_size_;
        }

        ALWAYS_INLINE size_t GetCountMovedTenured()
        {
            return tenured_move_object_count_;
        }

        ALWAYS_INLINE size_t GetSizeMovedTenured()
        {
            return tenured_move_object_size_;
        }

    private:
        uint32_t young_free_object_count_ {0U};
        uint64_t young_free_object_size_ {0U};
        uint32_t young_move_object_count_ {0U};
        uint64_t young_move_object_size_ {0U};
        uint32_t tenured_free_object_count_ {0U};
        uint64_t tenured_free_object_size_ {0U};
        uint32_t tenured_move_object_count_ {0U};
        uint64_t tenured_move_object_size_ {0U};

        friend class GenerationalGC;
        friend class test::MemStatsGenGCTest;
    };

    ALWAYS_INLINE ObjectAllocatorGenBase *GetObjectGenAllocator()
    {
        return static_cast<ObjectAllocatorGenBase *>(this->GetObjectAllocator());
    }

    /**
     * Sweeps string table from about to become dangled pointers to young generation
     */
    void SweepStringTableYoung(const std::function<bool(ObjectHeader *)> &young_checker);

    void CreateCardTable(InternalAllocatorPtr internal_allocator_ptr, uintptr_t min_address, size_t size);

    MemStats mem_stats_;  // NOLINT(misc-non-private-member-variables-in-classes)
private:
    static constexpr size_t DEFAULT_MAJOR_PERIOD = 3;
    static constexpr size_t DISABLED_MAJOR_PERIOD = 65535;
    size_t major_period_ {DEFAULT_MAJOR_PERIOD};
    PandaUniquePtr<CardTable> card_table_ {nullptr};
    friend class test::MemStatsGenGCTest;
};

}  // namespace panda::mem
#endif  // RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_H
