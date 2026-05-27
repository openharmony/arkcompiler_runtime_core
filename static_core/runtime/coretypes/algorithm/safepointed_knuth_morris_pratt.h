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

#ifndef PANDA_RUNTIME_CORETYPES_ALGORITHM_SAFEPOINTED_KNUTH_MORRIS_PRATT
#define PANDA_RUNTIME_CORETYPES_ALGORITHM_SAFEPOINTED_KNUTH_MORRIS_PRATT

#include "include/mem/panda_containers.h"
#include "libarkbase/utils/span.h"
#include "runtime/coretypes/algorithm/knuth_morris_pratt.h"
#include "runtime/coretypes/algorithm/safepoint_provider.h"
#include "runtime/coretypes/algorithm/string_handle_view.h"
#include "runtime/include/coretypes/string_flatten.h"
#include "runtime/include/managed_thread.h"
#include <limits>
#include <type_traits>

namespace ark::coretypes::algo {

namespace detail::kmp {

// Performance measurement on the device demonstrates that CMP_BETWEEN_SAFEPOINTS_THRESHOLD
// comparisons of chars during the searching are done in 0.045-0.1 ms.
static constexpr size_t CMP_BETWEEN_SAFEPOINTS_THRESHOLD = 11000L;
// Performance measurement on the device demonstrates that GPF_BETWEEN_SAFEPOINTS_THRESHOLD
// comparisons in the prefix function computing are done in 0.07-0.1 ms.
static constexpr size_t GPF_BETWEEN_SAFEPOINTS_THRESHOLD = 8000L;
// Performance measurement on the device demonstrates that LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD
// new value for the prefix function retrievings are done in 0.03-0.1 ms.
static constexpr size_t LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD = 10000L;
// Performance measurement on the device demonstrates that the subject in length less or equals
// to MAX_ALLOWED_FOR_FAST_SUBJECT_LEN may be processed in 0.09-0.1 ms.
static constexpr int32_t MAX_ALLOWED_FOR_FAST_SUBJECT_LEN = 7000L;
// Performance measurement on the device demonstrates that the pattern in length less or equals
// to MAX_ALLOWED_FOR_FAST_PATTERN_LEN may be processed (including the prefix function computing)
// in 0.09-0.1 ms.
static constexpr uint32_t MAX_ALLOWED_FOR_FAST_PATTERN_LEN = 5000L;

template <typename S, typename P>
class SafepointedKmpStringMatcher final {
    static_assert(std::is_integral_v<S> && std::is_integral_v<P>,
                  "KMP handles only strings of chars of integral types");

public:
    SafepointedKmpStringMatcher(StringHandleView pattern, ManagedThread *mThread)
        : pattern_(pattern), mThread_(mThread), prefixFun_(ComputePrefixFun(pattern))
    {
        // it is expected that the ManagedThread instance used to put safepoints in the IndexOf and LastIndexOf
        // methods is the same as used to put safepoints in the ComputePrefixFun function and that is stored in the
        // mThread_ member.
    }

    int32_t IndexOf(StringHandleView subject, int32_t pos = 0) const
    {
        auto [subjectSp, patternSp] = GetSpans<S, P>(subject, pattern_);
        ASSERT(pos >= 0);
        ASSERT(subjectSp.size() >= pos + patternSp.size());
        const auto sp = SafepointProvider(mThread_, subject, pattern_);
        int32_t patternSize = ToI32(patternSp.size());
        int32_t subjectSize = ToI32(subjectSp.size());
        int32_t q = 0;
        auto searcher = IndexSearcher(sp, prefixFun_.begin(), subjectSp, patternSp, pos);
        int32_t batches = (subjectSize - pos) / CMP_BETWEEN_SAFEPOINTS_THRESHOLD;
        int32_t i = pos;
        int32_t e = 0;

        for (int32_t n = 0; n < batches; ++n) {
            for (i = pos + n * CMP_BETWEEN_SAFEPOINTS_THRESHOLD, e = pos + (n + 1) * CMP_BETWEEN_SAFEPOINTS_THRESHOLD;
                 i < e; ++i) {
                auto ret = searcher.template TestCharsAt<false>(i, q);
                if (ret != -1) {
                    return ret;
                }
            }
            sp.PutSafepoint(subjectSp, patternSp);
        }
        ASSERT((subjectSize + q) - (patternSize + i) < static_cast<int32_t>(CMP_BETWEEN_SAFEPOINTS_THRESHOLD));
        for (; patternSize + i <= subjectSize + q; ++i) {
            auto ret = searcher.template TestCharsAt<false>(i, q);
            if (ret != -1) {
                return ret;
            }
        }
        return -1;
    }

    int32_t LastIndexOf(StringHandleView subject, int32_t pos = std::numeric_limits<int32_t>::max()) const
    {
        auto [subjectSp, patternSp] = GetSpans<S, P>(subject, pattern_);
        ASSERT(pos >= 0);
        ASSERT(subjectSp.size() >= pos + patternSp.size());
        const auto sp = SafepointProvider(mThread_, subject, pattern_);
        int32_t patternSize = ToI32(patternSp.size());
        int32_t subjectSize = ToI32(subjectSp.size());
        int32_t idx = -1;
        int32_t q = 0;
        auto searcher = IndexSearcher(sp, prefixFun_.begin(), subjectSp, patternSp, pos);
        int32_t batches = (pos + patternSize) / CMP_BETWEEN_SAFEPOINTS_THRESHOLD;
        int32_t i = 0;
        int32_t e = 0;

        for (int32_t n = 0; n < batches; ++n) {
            for (i = n * CMP_BETWEEN_SAFEPOINTS_THRESHOLD, e = (n + 1) * CMP_BETWEEN_SAFEPOINTS_THRESHOLD; i < e; ++i) {
                auto ret = searcher.template TestCharsAt<true>(i, q);
                if (ret != -1) {
                    idx = ret;
                }
            }
            sp.PutSafepoint(subjectSp, patternSp);
        }
        ASSERT(std::min(pos + patternSize - i, (subjectSize + q) - (patternSize + i)) <
               static_cast<int32_t>(CMP_BETWEEN_SAFEPOINTS_THRESHOLD));
        for (; i - patternSize < pos && i + patternSize <= q + subjectSize; ++i) {
            auto ret = searcher.template TestCharsAt<true>(i, q);
            if (ret != -1) {
                idx = ret;
            }
        }
        ASSERT(idx <= pos);
        return idx;
    }

private:
    static int32_t ToI32(size_t size)
    {
        return static_cast<int32_t>(size);
    }

    PandaVector<int32_t> ComputePrefixFun(StringHandleView pattern) const
    {
        ASSERT(!pattern.IsEmpty());
        auto patternSp = GetSpan<P>(pattern);
        auto prefixFun = PandaVector<int32_t>();
        prefixFun.reserve(patternSp.size());
        const auto sp = SafepointProvider(mThread_, pattern);
        prefixFun.emplace_back(0);

        auto getPrefixSize = [&sp, &patternSp, &prefixFun, k = 0](auto i) mutable {
            auto spthreshold =
                LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD - (i - 1) % LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD;
            while (k > 0 && patternSp[k] != patternSp[i]) {
                size_t j = 0;
                for (; j < spthreshold && k > 0 && patternSp[k] != patternSp[i]; ++j) {
                    k = prefixFun[k - 1];
                }
                if (j == spthreshold) {
                    sp.PutSafepoint(patternSp);
                    spthreshold = LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD;
                }
            }
            if (patternSp[k] == patternSp[i]) {
                k++;
            }
            prefixFun.emplace_back(k);
        };

        size_t batches = (patternSp.size() - 1) / GPF_BETWEEN_SAFEPOINTS_THRESHOLD;
        size_t i = 1U;
        size_t e = 0;
        for (size_t l = 0; l < batches; ++l) {
            sp.PutSafepoint(patternSp);
            for (i = 1U + l * GPF_BETWEEN_SAFEPOINTS_THRESHOLD, e = 1U + (l + 1) * GPF_BETWEEN_SAFEPOINTS_THRESHOLD;
                 i < e; ++i) {
                getPrefixSize(i);
            }
        }
        ASSERT(patternSp.size() < GPF_BETWEEN_SAFEPOINTS_THRESHOLD + i);
        for (; i < patternSp.size(); i++) {
            getPrefixSize(i);
        }

        ASSERT(prefixFun.size() == patternSp.size());
        return prefixFun;
    }

    template <typename Sp, typename PfIt>
    class IndexSearcher final {
    public:
        IndexSearcher(const Sp &sp, PfIt prefixFun, Span<const S> &lhsSp, Span<const P> &rhsSp, int32_t pos)
            : prefixFun_(std::move(prefixFun)),
              sp_(sp),
              lhsSp_(lhsSp),
              rhsSp_(rhsSp),
              pos_(pos),
              patternSize_(rhsSp.size()),
              pch_(ToI32(rhsSp[0]))
        {
        }

        NO_COPY_SEMANTIC(IndexSearcher);
        NO_MOVE_SEMANTIC(IndexSearcher);

        ~IndexSearcher() = default;

        template <bool LAST = false>
        int32_t TestCharsAt(int32_t idx, int32_t &q)
        {
            int32_t sch = ToI32(lhsSp_[idx]);
            auto spthreshold =
                LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD - (idx - pos_) % LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD;
            while (q > 0 && sch != pch_) {
                size_t j = 0;
                for (; j < spthreshold && q > 0 && sch != pch_; ++j) {
                    q = *(prefixFun_ + q - 1);
                    pch_ = ToI32(rhsSp_[q]);
                }
                if (j == spthreshold) {
                    sp_.PutSafepoint(lhsSp_, rhsSp_);
                    spthreshold = LOWERING_Q_BETWEEN_SAFEPOINTS_THRESHOLD;
                }
            }
            if (sch == pch_) {
                q++;
                if (q < patternSize_) {
                    pch_ = ToI32(rhsSp_[q]);
                } else {
                    if constexpr (LAST) {
                        q = *(prefixFun_ + q - 1);
                        pch_ = ToI32(rhsSp_[q]);
                        return idx + 1 - patternSize_;
                    } else {
                        return idx + 1 - patternSize_;
                    }
                }
            }
            return -1;
        }

    private:
        PfIt prefixFun_;
        const Sp &sp_;
        Span<const S> &lhsSp_;
        Span<const P> &rhsSp_;
        int32_t pos_;
        int32_t patternSize_;
        int32_t pch_;
    };

    // It looks like a bug in clang-15, the deduction guide is needed only for this version of the compiler.
    // Also, there is a bug in gcc: false positive error reporting: 'error: deduction guide must be declared
    // at namespace scope.'
    //
    // [temp.deduct.guide] includes the sentence:
    // A deduction-guide shall be declared in the same scope as the corresponding class template and, for a member
    // class template, with the same access.
#if __clang__
    template <typename Sp, typename PfIt>
    IndexSearcher(const Sp &, PfIt, Span<const S> &, Span<const P> &, int32_t) -> IndexSearcher<Sp, PfIt>;
#endif

    StringHandleView pattern_;
    ManagedThread *mThread_;
    PandaVector<int32_t> prefixFun_;
};

template <typename S>
struct StrSpanAdapter;

template <>
struct StrSpanAdapter<const uint8_t> {
    template <typename Str>
    static Span<const uint8_t> ToSpan(const Str &str)
    {
        ASSERT(str.IsUtf8());
        return {str.GetDataUtf8(), str.GetLength()};
    }
};

template <>
struct StrSpanAdapter<const uint16_t> {
    template <typename Str>
    static Span<const uint16_t> ToSpan(const Str &str)
    {
        ASSERT(str.IsUtf16());
        return {str.GetDataUtf16(), str.GetLength()};
    }
};

template <typename S, typename Str>
Span<const S> GetSpan(const Str &str)
{
    return StrSpanAdapter<const S>::ToSpan(str);
}

template <typename S, typename P>
class FastKmpStringMatcher final {
public:
    template <typename Str>
    FastKmpStringMatcher(const Str &pattern, [[maybe_unused]] ManagedThread *mThread) : matcher_(GetSpan<P>(pattern))
    {
    }

    template <typename Str>
    int32_t IndexOf(const Str &subject, int32_t pos = std::numeric_limits<int32_t>::max()) const
    {
        return matcher_.IndexOf(GetSpan<S>(subject), pos);
    }

    template <typename Str>
    int32_t LastIndexOf(const Str &subject, int32_t pos = std::numeric_limits<int32_t>::max()) const
    {
        return matcher_.LastIndexOf(GetSpan<S>(subject), pos);
    }

private:
    KmpStringMatcher<P> matcher_;
};

// Str may be either ark::coretypes::String, or ark::coretyles::FlatStringInfo (for fast searching with putting no
// safepoints), or ark::coretypes::algo::StringHandleView (for searching with putting safepoints)
template <template <typename, typename> typename StringMatcher, typename Str>
int32_t IndexOf(const Str &subj, const Str &pattern, int32_t pos, ManagedThread *mThread)
{
    if (subj.IsUtf8() && pattern.IsUtf8()) {
        return StringMatcher<uint8_t, uint8_t> {pattern, mThread}.IndexOf(subj, pos);
    } else if (subj.IsUtf16() && pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return StringMatcher<uint16_t, uint16_t> {pattern, mThread}.IndexOf(subj, pos);
    } else if (pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return StringMatcher<uint8_t, uint16_t> {pattern, mThread}.IndexOf(subj, pos);
    } else {  // NOLINT(readability-else-after-return)
        return StringMatcher<uint16_t, uint8_t> {pattern, mThread}.IndexOf(subj, pos);
    }
}

// Str may be either ark::coretypes::String, or ark::coretyles::FlatStringInfo (for fast searching with putting no
// safepoints), or ark::coretypes::algo::StringHandleView (for searching with putting safepoints)
template <template <typename, typename> typename StringMatcher, typename Str>
int32_t LastIndexOf(const Str &subj, const Str &pattern, int32_t pos, ManagedThread *mThread)
{
    if (subj.IsUtf8() && pattern.IsUtf8()) {
        return StringMatcher<uint8_t, uint8_t> {pattern, mThread}.LastIndexOf(subj, pos);
    } else if (subj.IsUtf16() && pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return StringMatcher<uint16_t, uint16_t> {pattern, mThread}.LastIndexOf(subj, pos);
    } else if (pattern.IsUtf16()) {  // NOLINT(readability-else-after-return)
        return StringMatcher<uint8_t, uint16_t> {pattern, mThread}.LastIndexOf(subj, pos);
    } else {  // NOLINT(readability-else-after-return)
        return StringMatcher<uint16_t, uint8_t> {pattern, mThread}.LastIndexOf(subj, pos);
    }
}

template <typename Str>
int32_t FastIndexOf(const Str &subj, const Str &pattern, int32_t pos)
{
    return IndexOf<FastKmpStringMatcher>(subj, pattern, pos, nullptr);
}

template <typename Str>
int32_t FastLastIndexOf(const Str &subj, const Str &pattern, int32_t pos)
{
    return LastIndexOf<FastKmpStringMatcher>(subj, pattern, pos, nullptr);
}

inline int32_t IndexOf(String &subj, String &pattern, int pos, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, &subj);
    VMHandle<String> patternHandle(mThread, &pattern);
    return IndexOf<SafepointedKmpStringMatcher>(StringHandleView {subjHandle}, StringHandleView {patternHandle}, pos,
                                                mThread);
}

inline int32_t IndexOf(const FlatStringInfo &subj, const FlatStringInfo &pattern, int pos, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, subj.GetString());
    VMHandle<String> patternHandle(mThread, pattern.GetString());
    return IndexOf<SafepointedKmpStringMatcher>(
        StringHandleView {subjHandle, subj.GetLength(), subj.GetStartIndex()},
        StringHandleView {patternHandle, pattern.GetLength(), pattern.GetStartIndex()}, pos, mThread);
}

inline int32_t LastIndexOf(String &subj, String &pattern, int pos, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, &subj);
    VMHandle<String> patternHandle(mThread, &pattern);
    return LastIndexOf<SafepointedKmpStringMatcher>(StringHandleView {subjHandle}, StringHandleView {patternHandle},
                                                    pos, mThread);
}

inline int32_t LastIndexOf(const FlatStringInfo &subj, const FlatStringInfo &pattern, int pos, ManagedThread *mThread)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(mThread);
    VMHandle<String> subjHandle(mThread, subj.GetString());
    VMHandle<String> patternHandle(mThread, pattern.GetString());
    return LastIndexOf<SafepointedKmpStringMatcher>(
        StringHandleView {subjHandle, subj.GetLength(), subj.GetStartIndex()},
        StringHandleView {patternHandle, pattern.GetLength(), pattern.GetStartIndex()}, pos, mThread);
}

}  // namespace detail::kmp

template <typename Str>
int32_t KmpIndexOf(Str &subj, Str &pattern, ManagedThread *mThread, int pos)
{
    ASSERT(subj.GetLength() >= pattern.GetLength());
    ASSERT(pos >= 0);
    ASSERT(subj.GetLength() >= pos + pattern.GetLength());
    // we may call the fast implementation, with no safepoint inserting and re-reading of pointers to the subject
    // and pattern: there is no safepoints; therefore, the strings pointed by lhs and rhs cannot be moved to different
    // memory locations.
    if (subj.GetLength() - pos <= detail::kmp::MAX_ALLOWED_FOR_FAST_SUBJECT_LEN &&
        pattern.GetLength() <= detail::kmp::MAX_ALLOWED_FOR_FAST_PATTERN_LEN) {
        return detail::kmp::FastIndexOf(subj, pattern, pos);
    }
    return detail::kmp::IndexOf(subj, pattern, pos, mThread);
}

template <typename Str>
int32_t KmpLastIndexOf(Str &subj, Str &pattern, ManagedThread *mThread, int pos)
{
    ASSERT(subj.GetLength() >= pattern.GetLength());
    ASSERT(pos >= 0);
    ASSERT(subj.GetLength() >= pos + pattern.GetLength());
    // we may call the fast implementation, with no safepoint inserting and re-reading of pointers to the subject
    // and pattern: there is no safepoints; therefore, the strings pointed by lhs and rhs cannot be moved to different
    // memory locations.
    if (pos <= detail::kmp::MAX_ALLOWED_FOR_FAST_SUBJECT_LEN &&
        pattern.GetLength() <= detail::kmp::MAX_ALLOWED_FOR_FAST_PATTERN_LEN) {
        return detail::kmp::FastLastIndexOf(subj, pattern, pos);
    }
    return detail::kmp::LastIndexOf(subj, pattern, pos, mThread);
}

}  // namespace ark::coretypes::algo

#endif  // PANDA_RUNTIME_CORETYPES_ALGORITHM_SAFEPOINTED_KNUTH_MORRIS_PRATT
