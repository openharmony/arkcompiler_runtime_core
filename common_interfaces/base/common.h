/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef COMMON_INTERFACES_BASE_COMMON_H
#define COMMON_INTERFACES_BASE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <iostream>

// Windows platform will define ERROR, cause compiler error
#ifdef ERROR
#undef ERROR
#endif

namespace panda {
#ifndef PANDA_TARGET_WINDOWS
#define PUBLIC_API __attribute__((visibility ("default")))
#else
#define PUBLIC_API __declspec(dllexport)
#endif

#define NO_INLINE_CC __attribute__((noinline))

#define LIKELY_CC(exp) (__builtin_expect((exp) != 0, true))
#define UNLIKELY_CC(exp) (__builtin_expect((exp) != 0, false))

#define UNREACHABLE_CC()                                             \
    do {                                                             \
        std::cerr << "This line should be unreachable" << std::endl; \
        std::abort();                                                \
        __builtin_unreachable();                                     \
    } while (0)

#define CHECK_CC(expr) \
    do { \
        if (UNLIKELY_CC(!(expr))) { \
            std::cerr << "CHECK FAILED: " << #expr; \
            std::cerr << "          IN: " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl; \
            std::abort(); \
        } \
    } while (0)

#if defined(NDEBUG)
#define ALWAYS_INLINE_CC __attribute__((always_inline))
#define DCHECK_CC(x)
#else
#define ALWAYS_INLINE_CC
#define DCHECK_CC(x) CHECK_CC(x)
#endif

#if defined(__cplusplus)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_CTOR_CC(TypeName) \
    TypeName(const TypeName&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_OPERATOR_CC(TypeName) \
    void operator=(const TypeName&) = delete

// Disabling copy ctor and copy assignment operator.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_SEMANTIC_CC(TypeName) \
    NO_COPY_CTOR_CC(TypeName);        \
    NO_COPY_OPERATOR_CC(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_CTOR_CC(TypeName)                   \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName&&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_OPERATOR_CC(TypeName)               \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName& operator=(TypeName&&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_SEMANTIC_CC(TypeName) \
    NO_MOVE_CTOR_CC(TypeName);        \
    NO_MOVE_OPERATOR_CC(TypeName)

#endif  // defined(__cplusplus)

enum class PUBLIC_API LOG_LEVEL : uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
    FOLLOW = 100, // if hilog enabled follow hilog, otherwise use INFO level
};

/*
 * @struct HeapParam
 * @brief Data structure for Arkcommon heap configuration parameters,\n
 * including the heap size, region size at runtime, and etc.
 */
struct HeapParam {
    /*
     * The reference value of region size, measured in KB, default to 64 KB, must be in range [4KB, 64KB].
     * It will be set to default value if assigned with 0.
     */
    size_t regionSize;

    /*
     * The maximum size of arkcommon heap, measured in KB, default to 256 * 1024 KB, must >= 4MB.
     * It will be set to default value if assigned with 0.
     */
    size_t heapSize;

    /*
     * Threshold used to determine whether a region is exempted (i.e., will not be forwarded).
     * If the percentage of live objects in a region is greater than this value, this region will not be exempted.
     * Default to 0.8, must be in range (0, 1].
     * It will be set to default value if assigned with 0.
     */
    double exemptionThreshold;

    /*
     * A hint to guide collector to release physical memory to OS.
     * heap utilization = heap-used-memory / total-heap-memory.
     * During each gc, collector determines how much memory should be cached,
     * and let the heap utilization be close to this value.
     * Default to 0.80, must be in range (0, 1].
     * It will be set to default value if assigned with 0.
     */
    double heapUtilization;

    /*
     * The ratio to expand heap after each GC.
     * GC is probably triggered more often if this value is set to an improperly small number.
     * Default to 0.15, must > 0.
     * It will be set to default value if assigned with 0.
     */
    double heapGrowth;

    /*
     * The rate of allocating memory from heap.
     * this value is the lower bound of the real allocation rate.
     * allocator maybe wait some time if this value is set with an improperly small number.
     * Mesured in MB/s, default to 10240 MB/s, must be > 0 MB/s.
     * It will be set to default value if assigned with 0.
     */
    double allocationRate;

    /*
     * The maximum wait time when allocating memory from heap.
     * The latter alloction will wait a number of time if the two alloction interval is less than the wait time.
     * The real wait time is the minimum of allocationWaitTime and the wait time calculated from real alloction rate.
     * Measured in ns, default to 1000 ns, must > 0 ns.
     * It will be set to default value if assigned with 0.
     */
    size_t allocationWaitTime;
};

/*
 * @struct GCParam
 * @brief Data structure for Arkcommon garbage collection configuration parameters,\n
 * including the garbage ratio, garbage collection interval and etc.
 */
struct GCParam {
    /*
     * GC will be triggered when heap allocated size is greater than this threshold.
     * Measured in KB, must be > 0.
     */
    size_t gcThreshold;

    /*
     * The threshold used to determine whether to collect from-space during GC.
     * The from-space will be collected if the percentage of the garbage in from space is greater than this threshold.
     * default to 0.5, must be in range [0.1, 1.0].
     */
    double garbageThreshold;

    /*
     * Minimum interval each GC request will be responded. If two adjacent GC requests with
     * interval less than this value, the latter one is ignored.
     * Measured in ns, default to 150 ms, must be > 0 ms.
     * It will be set default value if the value is 0.
     */
    uint64_t gcInterval;

    /*
     * Minimum interval each backup GC request will be responded.
     * Backup GC will be triggered if there is no GC during this interval.
     * Measured in ns, default to 240 s, must be > 0 s.
     * It will be set default value if the value is 0.
     */
    uint64_t backupGCInterval;

    /*
     * Parameters for adjusting the number of GC threads.
     * The number of gc threads is ((the hardware concurrency / this value) - 1).
     * default to 8, must be > 0.
     * It will be set default value if the value is 0.
     */
    uint32_t gcThreads;
};

/*
 * @struct RuntimeParam
 * @brief Data structure for Arkcommon runtime parameters,\n
 * including the config information of heap, garbage collection, thread and log.
 */
struct RuntimeParam {
    struct HeapParam heapParam;
    struct GCParam gcParam;
};
}  // panda

#endif  // COMMON_INTERFACES_BASE_COMMON_H
