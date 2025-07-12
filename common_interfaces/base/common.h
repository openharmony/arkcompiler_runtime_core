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
#define PUBLIC_API __attribute__((visibility("default")))
#else
#define PUBLIC_API __declspec(dllexport)
#endif

#define NO_INLINE_CC __attribute__((noinline))

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIKELY_CC(exp) (__builtin_expect((exp) != 0, true))  // CC-OFF(G.PRE.02-CPP) design decision
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNLIKELY_CC(exp) (__builtin_expect((exp) != 0, false))  // CC-OFF(G.PRE.02-CPP) design decision

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREACHABLE_CC()                                             \
    do {                                                             \
        std::cerr << "This line should be unreachable" << std::endl; \
        std::abort();                                                \
        __builtin_unreachable();                                     \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_CC(expr)                                                                                        \
    do {                                                                                                      \
        if (UNLIKELY_CC(!(expr))) {                                                                           \
            std::cerr << "CHECK FAILED: " << #expr;                                                           \
            std::cerr << "          IN: " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl; \
            std::abort();                                                                                     \
        }                                                                                                     \
    } while (0)

#if defined(NDEBUG)
#define ALWAYS_INLINE_CC __attribute__((always_inline))
#define DCHECK_CC(x)
#else
#define ALWAYS_INLINE_CC
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DCHECK_CC(x) CHECK_CC(x)  // CC-OFF(G.PRE.02-CPP)
#endif

#if defined(__cplusplus)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_CTOR_CC(TypeName) TypeName(const TypeName &) = delete  // CC-OFF(G.PRE.02-CPP) design decision

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_OPERATOR_CC(TypeName) void operator=(const TypeName &) = delete  // CC-OFF(G.PRE.02-CPP) design decision

// Disabling copy ctor and copy assignment operator.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_SEMANTIC_CC(TypeName) \
    NO_COPY_CTOR_CC(TypeName);        \
    NO_COPY_OPERATOR_CC(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_CTOR_CC(TypeName)                \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName &&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_OPERATOR_CC(TypeName)            \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(TypeName &&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_SEMANTIC_CC(TypeName) \
    NO_MOVE_CTOR_CC(TypeName);        \
    NO_MOVE_OPERATOR_CC(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_MOVE_CTOR_CC(TypeName)           \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName &&) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_MOVE_OPERATOR_CC(TypeName)       \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(TypeName &&) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_MOVE_SEMANTIC_CC(TypeName) \
    DEFAULT_MOVE_CTOR_CC(TypeName);        \
    DEFAULT_MOVE_OPERATOR_CC(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_COPY_CTOR_CC(TypeName) TypeName(const TypeName &) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_COPY_OPERATOR_CC(TypeName)       \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(const TypeName &) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_COPY_SEMANTIC_CC(TypeName) \
    DEFAULT_COPY_CTOR_CC(TypeName);        \
    DEFAULT_COPY_OPERATOR_CC(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_NOEXCEPT_MOVE_CTOR_CC(TypeName)  \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName &&) noexcept = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_NOEXCEPT_MOVE_OPERATOR_CC(TypeName) \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */    \
    TypeName &operator=(TypeName &&) noexcept = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_NOEXCEPT_MOVE_SEMANTIC_CC(TypeName) \
    DEFAULT_NOEXCEPT_MOVE_CTOR_CC(TypeName);        \
    DEFAULT_NOEXCEPT_MOVE_OPERATOR_CC(TypeName)

#endif  // defined(__cplusplus)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABORT_AND_UNREACHABLE_COMMON()                               \
    do {                                                             \
        std::cerr << "This line should be unreachable" << std::endl; \
        std::abort();                                                \
        __builtin_unreachable();                                     \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MEMBER_OFFSET_COMMON(T, F) offsetof(T, F)

#if !defined(NDEBUG)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_FAIL_COMMON(expr)                                                                \
    do {                                                                                        \
        /* CC-OFFNXT(G.PRE.02) code readability */                                              \
        std::cerr << "ASSERT_FAILED: " << (expr) << std::endl; /* NOLINT(misc-static-assert) */ \
        __builtin_unreachable();                                                                \
    } while (0)

// CC-OFFNXT(G.PRE.06) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_OP_COMMON(lhs, op, rhs)                                                                         \
    do {                                                                                                       \
        auto __lhs = lhs;                                                                                      \
        auto __rhs = rhs;                                                                                      \
        if (UNLIKELY_CC(!(__lhs op __rhs))) {                                                                  \
            std::cerr << "CHECK FAILED: " << #lhs << " " #op " " #rhs << std::endl;                            \
            std::cerr << "      VALUES: " << __lhs << " " #op " " << __rhs << std::endl;                       \
            std::cerr << "          IN: " << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION__ << std::endl; \
            panda::PrintStack(std::cerr);                                                                      \
            std::abort();                                                                                      \
        }                                                                                                      \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LE_COMMON(lhs, rhs) ASSERT_OP_COMMON(lhs, <=, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LT_COMMON(lhs, rhs) ASSERT_OP_COMMON(lhs, <, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GE_COMMON(lhs, rhs) ASSERT_OP_COMMON(lhs, >=, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GT_COMMON(lhs, rhs) ASSERT_OP_COMMON(lhs, >, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_EQ_COMMON(lhs, rhs) ASSERT_OP_COMMON(lhs, ==, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_NE_COMMON(lhs, rhs) ASSERT_OP_COMMON(lhs, !=, rhs)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_COMMON(cond)        \
    if (UNLIKELY_CC(!(cond))) {    \
        ASSERT_FAIL_COMMON(#cond); \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_DO_COMMON(cond, func)                          \
    do {                                                      \
        /* CC-OFFNXT(G.PRE.02) code readability */            \
        if (auto cond_val = cond; UNLIKELY_CC(!(cond_val))) { \
            func;                                             \
            ASSERT_FAIL_COMMON(#cond);                        \
        }                                                     \
    } while (0)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_PRINT_COMMON(cond, message)                    \
    do {                                                      \
        /* CC-OFFNXT(G.PRE.02) code readability */            \
        if (auto cond_val = cond; UNLIKELY_CC(!(cond_val))) { \
            /* CC-OFFNXT(G.PRE.02) code readability */        \
            std::cerr << (message) << std::endl;              \
            ASSERT_FAIL_COMMON(#cond);                        \
        }                                                     \
    } while (0)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_RETURN_COMMON(cond) assert(cond)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREACHABLE_COMMON()                                                                            \
    do {                                                                                                \
        /* CC-OFFNXT(G.PRE.02) code readability */                                                      \
        ASSERT_PRINT_COMMON(false, "This line should be unreachable"); /* NOLINT(misc-static-assert) */ \
        __builtin_unreachable();                                                                        \
    } while (0)
#else                                                            // NDEBUG
#define ASSERT_COMMON(cond) static_cast<void>(0)                 // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_DO_COMMON(cond, func) static_cast<void>(0)        // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_PRINT_COMMON(cond, message) static_cast<void>(0)  // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_RETURN_COMMON(cond) static_cast<void>(cond)       // NOLINT(cppcoreguidelines-macro-usage)
#define UNREACHABLE_COMMON() ABORT_AND_UNREACHABLE_COMMON()      // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_OP_COMMON(lhs, op, rhs)                           // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LE_COMMON(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LT_COMMON(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GE_COMMON(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GT_COMMON(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_EQ_COMMON(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_NE_COMMON(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#endif                                                           // !NDEBUG

#ifdef __clang__
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FIELD_UNUSED_COMMON __attribute__((__unused__))
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FIELD_UNUSED_COMMON
#endif

// NOLINTNEXTLINE(readability-identifier-naming)
enum class LOG_LEVEL : uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
    FOLLOW = 100,  // if hilog enabled follow hilog, otherwise use INFO level
};

namespace common {
static constexpr size_t BITS_PER_BYTE = 8;
}  // namespace common
}  // namespace panda

#endif  // COMMON_INTERFACES_BASE_COMMON_H
