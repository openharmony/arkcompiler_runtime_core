/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_CORETYPES_ALGORITHM_NAIVE_INDEXOF
#define PANDA_RUNTIME_CORETYPES_ALGORITHM_NAIVE_INDEXOF

#include "runtime/coretypes/algorithm/safepoint_provider.h"
#include "runtime/coretypes/algorithm/string_handle_view.h"
#include "runtime/include/coretypes/string_flatten.h"

namespace ark::coretypes::algo {

namespace detail::naive {
// Performance measurement on the device demonstrates that CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD comparisons of the
// first char of the pattern with the chars from the subject are done in 0.05-0.07 ms with hikes up to 0.096 ms.
static constexpr int32_t CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD = 90000L;
// Performance measurement on the device demonstrates that CMP_REST_BETWEEN_SAFEPOINTS_THRESHOLD comparisons of the
// chars from the pattern with the chars from the subject are done in 0.06-0.1 ms.
static constexpr int32_t CMP_REST_BETWEEN_SAFEPOINTS_THRESHOLD = 80000L;
// Performance measurement on the device demonstrates that MAX_ALLOWED_FOR_FAST_LENS_MULTIPLY comparisons of the
// chars from the pattern with the chars from the subject are done in 0.08-0.09 ms.
static constexpr uint64_t MAX_ALLOWED_FOR_FAST_LENS_MULTIPLY = 120000L;

template <typename Str>
int32_t FastIndexOf(Str &subj, Str &pattern, int32_t pos, int32_t max)
{
    auto lhsCount = subj.GetLength();
    auto rhsCount = pattern.GetLength();
    if (pattern.IsUtf8() && subj.IsUtf8()) {
        common_vm::Span<const uint8_t> lhsSp(subj.GetDataUtf8(), lhsCount);
        common_vm::Span<const uint8_t> rhsSp(pattern.GetDataUtf8(), rhsCount);
        return ark::mem::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    } else if (pattern.IsUtf16() && subj.IsUtf16()) {  // NOLINT(readability-else-after-return)
        common_vm::Span<const uint16_t> lhsSp(subj.GetDataUtf16(), lhsCount);
        common_vm::Span<const uint16_t> rhsSp(pattern.GetDataUtf16(), rhsCount);
        return ark::mem::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    } else if (pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        common_vm::Span<const uint8_t> lhsSp(subj.GetDataUtf8(), lhsCount);
        common_vm::Span<const uint16_t> rhsSp(pattern.GetDataUtf16(), rhsCount);
        return ark::mem::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    } else {  // NOLINT(readability-else-after-return)
        common_vm::Span<const uint16_t> lhsSp(subj.GetDataUtf16(), lhsCount);
        common_vm::Span<const uint8_t> rhsSp(pattern.GetDataUtf8(), rhsCount);
        return ark::mem::BaseString::IndexOf(lhsSp, rhsSp, pos, max);
    }
}

template <typename Str>
int32_t FastLastIndexOf(Str &lhs, Str &rhs, int pos)
{
    auto lhsCount = lhs.GetLength();
    auto rhsCount = rhs.GetLength();
    ASSERT(lhsCount >= rhsCount);
    if (rhs.IsUtf8() && lhs.IsUtf8()) {
        common_vm::Span<const uint8_t> lhsSp(lhs.GetDataUtf8(), lhsCount);
        common_vm::Span<const uint8_t> rhsSp(rhs.GetDataUtf8(), rhsCount);
        return ark::mem::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    } else if (rhs.IsUtf16() && lhs.IsUtf16()) {  // NOLINT(readability-else-after-return)
        common_vm::Span<const uint16_t> lhsSp(lhs.GetDataUtf16(), lhsCount);
        common_vm::Span<const uint16_t> rhsSp(rhs.GetDataUtf16(), rhsCount);
        return ark::mem::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    } else if (rhs.IsUtf16()) {  // NOLINT(readability-else-after-return)
        common_vm::Span<const uint8_t> lhsSp(lhs.GetDataUtf8(), lhsCount);
        common_vm::Span<const uint16_t> rhsSp(rhs.GetDataUtf16(), rhsCount);
        return ark::mem::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    } else {  // NOLINT(readability-else-after-return)
        common_vm::Span<const uint16_t> lhsSp(lhs.GetDataUtf16(), lhsCount);
        common_vm::Span<const uint8_t> rhsSp(rhs.GetDataUtf8(), rhsCount);
        return ark::mem::BaseString::LastIndexOf(lhsSp, rhsSp, pos);
    }
}

template <typename S, typename P>
int32_t TestTheKey(int i, Span<const S> &lhs, int32_t first, Span<const P> &rhs, const DuoSafepointProvider &sp)
{
    if (static_cast<int32_t>(lhs[i]) != first) {
        return -1;
    }

    /* Found first character, now look at the rest of rhsSp */
    auto end = static_cast<int>(rhs.size());
    DCHECK(end > 0);
    int j = 1;
    int e = 0;
    int batches = (end - 1) / CMP_REST_BETWEEN_SAFEPOINTS_THRESHOLD;
    for (int l = 0; l < batches && static_cast<int32_t>(lhs[j + i]) == static_cast<int32_t>(rhs[j]); ++l) {
        sp.PutSafepoint(lhs, rhs);
        for (j = 1 + l * CMP_REST_BETWEEN_SAFEPOINTS_THRESHOLD, e = 1 + (l + 1) * CMP_REST_BETWEEN_SAFEPOINTS_THRESHOLD;
             j < e && static_cast<int32_t>(lhs[j + i]) == static_cast<int32_t>(rhs[j]); ++j) {
        }
    }
    sp.PutSafepoint(lhs, rhs);
    ASSERT(end - j < CMP_REST_BETWEEN_SAFEPOINTS_THRESHOLD ||
           static_cast<int32_t>(lhs[j + i]) != static_cast<int32_t>(rhs[j]));
    for (; j < end && static_cast<int32_t>(lhs[j + i]) == static_cast<int32_t>(rhs[j]); ++j) {
    }
    if (j == end) {
        /* Found whole string. */
        return i;
    }
    return -1;
}

template <typename S, typename P>
int32_t IndexOf(StringHandleView subj, StringHandleView pattern, int32_t pos, int32_t max, ManagedThread *mThread)
{
    auto [lhsSp, rhsSp] = GetSpans<S, P>(subj, pattern);
    const auto sp = SafepointProvider(mThread, subj, pattern);
    auto first = static_cast<int32_t>(rhsSp[0]);
    int batches = (max - pos) / CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD;
    int i = pos;
    int e = 0;
    for (int n = 0; n < batches; ++n) {
        for (i = pos + n * CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD,
            e = pos + (n + 1) * CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD;
             i < e; ++i) {
            auto ret = TestTheKey(i, lhsSp, first, rhsSp, sp);
            if (ret != -1) {
                return ret;
            }
        }
        sp.PutSafepoint(lhsSp, rhsSp);
    }
    ASSERT(max - pos - i < CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD);
    for (; i <= max; ++i) {
        auto ret = TestTheKey(i, lhsSp, first, rhsSp, sp);
        if (ret != -1) {
            return ret;
        }
    }
    return -1;
}

template <typename S, typename P>
int32_t LastIndexOf(StringHandleView subj, StringHandleView pattern, int32_t pos, ManagedThread *mThread)
{
    auto [lhsSp, rhsSp] = GetSpans<S, P>(subj, pattern);
    const auto sp = SafepointProvider(mThread, subj, pattern);
    auto first = static_cast<int32_t>(rhsSp[0]);
    int batches = pos / CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD;
    int i = pos;
    int e = 0;
    for (; i >= batches * CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD; --i) {
        auto ret = TestTheKey(i, lhsSp, first, rhsSp, sp);
        if (ret != -1) {
            return ret;
        }
    }
    sp.PutSafepoint(lhsSp, rhsSp);
    for (int n = batches; n > 0; --n) {
        for (i = n * CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD - 1, e = (n - 1) * CMP_FIRST_BETWEEN_SAFEPOINTS_THRESHOLD;
             i >= e; --i) {
            auto ret = TestTheKey(i, lhsSp, first, rhsSp, sp);
            if (ret != -1) {
                return ret;
            }
        }
        sp.PutSafepoint(lhsSp, rhsSp);
    }
    return -1;
}

inline int32_t IndexOf(StringHandleView subj, StringHandleView pattern, int32_t pos, int32_t max,
                       ManagedThread *mThread)
{
    if (subj.IsUtf8() && pattern.IsUtf8()) {
        return IndexOf<uint8_t, uint8_t>(subj, pattern, pos, max, mThread);
    } else if (subj.IsUtf16() && pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return IndexOf<uint16_t, uint16_t>(subj, pattern, pos, max, mThread);
    } else if (pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return IndexOf<uint8_t, uint16_t>(subj, pattern, pos, max, mThread);
    } else {  // NOLINT(readability-else-after-return)
        return IndexOf<uint16_t, uint8_t>(subj, pattern, pos, max, mThread);
    }
}

inline int32_t LastIndexOf(StringHandleView subj, StringHandleView pattern, int32_t pos, ManagedThread *mThread)
{
    if (subj.IsUtf8() && pattern.IsUtf8()) {
        return LastIndexOf<uint8_t, uint8_t>(subj, pattern, pos, mThread);
    } else if (subj.IsUtf16() && pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return LastIndexOf<uint16_t, uint16_t>(subj, pattern, pos, mThread);
    } else if (pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return LastIndexOf<uint8_t, uint16_t>(subj, pattern, pos, mThread);
    } else {  // NOLINT(readability-else-after-return)
        return LastIndexOf<uint16_t, uint8_t>(subj, pattern, pos, mThread);
    }
}

inline int32_t IndexOf(String &subj, String &pattern, int pos, int max, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, &subj);
    VMHandle<String> patternHandle(mThread, &pattern);
    return IndexOf(StringHandleView {subjHandle}, StringHandleView {patternHandle}, pos, max, mThread);
}

inline int32_t IndexOf(const FlatStringInfo &subj, const FlatStringInfo &pattern, int pos, int max,
                       ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, subj.GetString());
    VMHandle<String> patternHandle(mThread, pattern.GetString());
    return IndexOf({subjHandle, subj.GetLength(), subj.GetStartIndex()},
                   {patternHandle, pattern.GetLength(), pattern.GetStartIndex()}, pos, max, mThread);
}

inline int32_t LastIndexOf(String &subj, String &pattern, int pos, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, &subj);
    VMHandle<String> patternHandle(mThread, &pattern);
    return LastIndexOf(StringHandleView {subjHandle}, StringHandleView {patternHandle}, pos, mThread);
}

inline int32_t LastIndexOf(const FlatStringInfo &subj, const FlatStringInfo &pattern, int pos, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, subj.GetString());
    VMHandle<String> patternHandle(mThread, pattern.GetString());
    return LastIndexOf({subjHandle, subj.GetLength(), subj.GetStartIndex()},
                       {patternHandle, pattern.GetLength(), pattern.GetStartIndex()}, pos, mThread);
}

}  // namespace detail::naive

template <typename Str>
int32_t NaiveIndexOf(Str &subj, Str &pattern, ManagedThread *mThread, int pos, int max)
{
    ASSERT(subj.GetLength() >= pattern.GetLength());
    ASSERT(pos >= 0);
    ASSERT(max >= pos);
    // We may call the fast implementation, with no safepoint inserting and re-reading of pointers to the subject
    // and pattern: there is no safepoints; therefore, the strings pointed by lhs and rhs cannot be moved to different
    // memory locations.
    // Multiplication must be in uint64_t to avoid overflow.
    if (static_cast<uint64_t>(max - pos + 1) * pattern.GetLength() <=
        detail::naive::MAX_ALLOWED_FOR_FAST_LENS_MULTIPLY) {
        return detail::naive::FastIndexOf(subj, pattern, pos, max);
    }
    return detail::naive::IndexOf(subj, pattern, pos, max, mThread);
}

template <typename Str>
int32_t NaiveLastIndexOf(Str &subj, Str &pattern, ManagedThread *mThread, int pos)
{
    ASSERT(subj.GetLength() >= pattern.GetLength());
    ASSERT(pos >= 0);
    ASSERT(subj.GetLength() >= pos + pattern.GetLength());
    // We may call the fast implementation, with no safepoint inserting and re-reading of pointers to the subject
    // and pattern: there is no safepoints; therefore, the strings pointed by lhs and rhs cannot be moved to different
    // memory locations.
    // Multiplication must be in uint64_t to avoid overflow.
    if ((pos + 1) * static_cast<uint64_t>(pattern.GetLength()) <= detail::naive::MAX_ALLOWED_FOR_FAST_LENS_MULTIPLY) {
        return detail::naive::FastLastIndexOf(subj, pattern, pos);
    }
    return detail::naive::LastIndexOf(subj, pattern, pos, mThread);
}

}  // namespace ark::coretypes::algo

#endif  // PANDA_RUNTIME_CORETYPES_ALGORITHM_NAIVE_INDEXOF
