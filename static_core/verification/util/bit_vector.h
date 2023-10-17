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

#ifndef PANDA_VERIFIER_BIT_VECTOR_HPP_
#define PANDA_VERIFIER_BIT_VECTOR_HPP_

#include "utils/bit_utils.h"
#include "function_traits.h"
#include "panda_or_std.h"
#include "macros.h"
#include "index.h"

#include <utility>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <tuple>
#include <iostream>

namespace panda::verifier {

#ifdef PANDA_TARGET_64
using Word = uint64_t;
#else
using Word = uint32_t;
#endif

template <typename GetFunc>
class ConstBits {
public:
    explicit ConstBits(GetFunc &&f) : get_f_ {std::move(f)} {}
    ~ConstBits() = default;
    ConstBits() = delete;
    ConstBits(const ConstBits & /* unused */) = delete;
    ConstBits(ConstBits && /* unused */) = default;
    ConstBits &operator=(const ConstBits & /* unused */) = delete;
    ConstBits &operator=(ConstBits && /* unused */) = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Word() const
    {
        return get_f_();
    }

    template <typename Rhs>
    bool operator==(const Rhs &rhs) const
    {
        return get_f_() == rhs.get_f_();
    }

private:
    GetFunc get_f_;
    template <typename A>
    friend class ConstBits;
};

template <typename GetFunc, typename SetFunc>
class Bits : public ConstBits<GetFunc> {
public:
    Bits(GetFunc &&get, SetFunc &&set) : ConstBits<GetFunc>(std::move(get)), set_f_ {std::move(set)} {}
    ~Bits() = default;
    Bits() = delete;
    Bits(const Bits &) = delete;
    Bits(Bits &&) = default;
    Bits &operator=(const Bits &rhs)
    {
        set_f_(rhs);
        return *this;
    }
    Bits &operator=(Bits &&) = default;

    Bits &operator=(Word val)
    {
        set_f_(val);
        return *this;
    }

private:
    SetFunc set_f_;
};

class BitVector {
    using Allocator = MPandaAllocator<Word>;
    static constexpr size_t BITS_IN_WORD = sizeof(Word) * 8;
    static constexpr size_t BITS_IN_INT = sizeof(int) * 8;
    size_t MaxBitIdx() const
    {
        return size_ - 1;
    }
    static constexpr size_t POS_SHIFT = panda::Ctz(BITS_IN_WORD);
    static constexpr size_t POS_MASK = BITS_IN_WORD - 1;
    static constexpr Word MAX_WORD = std::numeric_limits<Word>::max();

    static Word MaskForIndex(size_t idx)
    {
        return static_cast<Word>(1) << idx;
    }

    static Word MaskUpToIndex(size_t idx)
    {
        return idx >= BITS_IN_WORD ? MAX_WORD : ((static_cast<Word>(1) << idx) - 1);
    }

    class Bit {
    public:
        Bit(BitVector &bit_vector, size_t index) : bit_vector_ {bit_vector}, index_ {index} {};

        // NOLINTNEXTLINE(google-explicit-constructor)
        operator bool() const
        {
            return const_cast<const BitVector &>(bit_vector_)[index_];
        }

        Bit &operator=(bool value)
        {
            if (value) {
                bit_vector_.Set(index_);
            } else {
                bit_vector_.Clr(index_);
            }
            return *this;
        }

    private:
        BitVector &bit_vector_;
        size_t index_;
    };

    static constexpr size_t SizeInBitsFromSizeInWords(size_t size)
    {
        return size << POS_SHIFT;
    }

    static constexpr size_t SizeInWordsFromSizeInBits(size_t size)
    {
        return (size + POS_MASK) >> POS_SHIFT;
    }

    void Deallocate()
    {
        if (data_ != nullptr) {
            Allocator allocator;
            allocator.deallocate(data_, SizeInWords());
        }
        size_ = 0;
        data_ = nullptr;
    }

public:
    explicit BitVector(size_t sz) : size_ {sz}, data_ {Allocator().allocate(SizeInWords())}
    {
        Clr();
    }
    ~BitVector()
    {
        Deallocate();
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    BitVector(const BitVector &other)
    {
        CopyFrom(other);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    BitVector(BitVector &&other) noexcept
    {
        MoveFrom(std::move(other));
    }

    BitVector &operator=(const BitVector &rhs)
    {
        if (&rhs == this) {
            return *this;
        }
        Deallocate();
        CopyFrom(rhs);
        return *this;
    }

    BitVector &operator=(BitVector &&rhs) noexcept
    {
        Deallocate();
        MoveFrom(std::move(rhs));
        return *this;
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,hicpp-signed-bitwise)

    auto bits(size_t from, size_t to) const  // NOLINT(readability-identifier-naming)
    {
        ASSERT(data_ != nullptr);
        ASSERT(from <= to);
        ASSERT(to <= MaxBitIdx());
        ASSERT(to - from <= BITS_IN_WORD - 1);
        const Word mask = MaskUpToIndex(to - from + 1);
        const size_t pos_from = from >> POS_SHIFT;
        const size_t pos_to = to >> POS_SHIFT;
        const size_t idx_from = from & POS_MASK;
        return ConstBits([this, mask, pos_from, pos_to, idx_from]() -> Word {
            if (pos_from == pos_to) {
                return (data_[pos_from] >> idx_from) & mask;
            }
            Word data = (data_[pos_from] >> idx_from) | (data_[pos_to] << (BITS_IN_WORD - idx_from));
            return data & mask;
        });
    }

    auto bits(size_t from, size_t to)  // NOLINT(readability-identifier-naming)
    {
        ASSERT(data_ != nullptr);
        ASSERT(from <= to);
        ASSERT(to <= MaxBitIdx());
        ASSERT(to - from <= BITS_IN_WORD - 1);
        const Word mask = MaskUpToIndex(to - from + 1);
        const size_t pos_from = from >> POS_SHIFT;
        const size_t pos_to = to >> POS_SHIFT;
        const size_t idx_from = from & POS_MASK;
        return Bits(
            [this, mask, pos_from, pos_to, idx_from]() -> Word {
                if (pos_from == pos_to) {
                    return (data_[pos_from] >> idx_from) & mask;
                }
                Word data = (data_[pos_from] >> idx_from) | (data_[pos_to] << (BITS_IN_WORD - idx_from));
                return data & mask;
            },
            [this, mask, pos_from, pos_to, idx_from](Word val_incomming) {
                const Word val = val_incomming & mask;
                const auto low_mask = mask << idx_from;
                const auto low_val = val << idx_from;
                if (pos_from == pos_to) {
                    data_[pos_from] &= ~low_mask;
                    data_[pos_from] |= low_val;
                } else {
                    const auto high_shift = BITS_IN_WORD - idx_from;
                    const auto high_mask = mask >> high_shift;
                    const auto high_val = val >> high_shift;
                    data_[pos_from] &= ~low_mask;
                    data_[pos_from] |= low_val;
                    data_[pos_to] &= ~high_mask;
                    data_[pos_to] |= high_val;
                }
            });
    }

    bool operator[](size_t idx) const
    {
        ASSERT(idx < Size());
        return (data_[idx >> POS_SHIFT] & MaskForIndex(idx % BITS_IN_WORD)) != 0;
    }
    Bit operator[](size_t idx)
    {
        return {*this, idx};
    }

    void Clr()
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            data_[pos] = 0;
        }
    }
    void Set()
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            data_[pos] = MAX_WORD;
        }
    }
    void Invert()
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            data_[pos] = ~data_[pos];
        }
    }
    void Clr(size_t idx)
    {
        ASSERT(idx < Size());
        data_[idx >> POS_SHIFT] &= ~MaskForIndex(idx % BITS_IN_WORD);
    }
    void Set(size_t idx)
    {
        ASSERT(idx < Size());
        data_[idx >> POS_SHIFT] |= MaskForIndex(idx % BITS_IN_WORD);
    }
    void Invert(size_t idx)
    {
        operator[](idx) = !operator[](idx);
    }

    BitVector operator~() const
    {
        BitVector result {*this};
        result.Invert();
        return result;
    }

    bool operator==(const BitVector &rhs) const
    {
        if (Size() != rhs.Size()) {
            return false;
        }
        size_t last_word_partial_bits = Size() % BITS_IN_WORD;
        size_t num_full_words = SizeInWords() - ((last_word_partial_bits != 0) ? 1 : 0);
        for (size_t pos = 0; pos < num_full_words; pos++) {
            if (data_[pos] != rhs.data_[pos]) {
                return false;
            }
        }
        if (last_word_partial_bits != 0) {
            size_t last_word_start = Size() - last_word_partial_bits;
            return bits(last_word_start, Size() - 1) == rhs.bits(last_word_start, Size() - 1);
        }
        return true;
    }

    bool operator!=(const BitVector &rhs) const
    {
        return !(*this == rhs);
    }

    template <typename Handler>
    void Process(size_t from, size_t to, Handler handler)
    {
        ASSERT(data_ != nullptr);
        ASSERT(from <= to);
        ASSERT(to <= MaxBitIdx());
        const size_t pos_from = from >> POS_SHIFT;
        const size_t pos_to = to >> POS_SHIFT;
        const size_t idx_from = from & POS_MASK;
        const size_t idx_to = to & POS_MASK;
        auto process_word = [this, &handler](size_t pos) {
            const Word val = handler(data_[pos], BITS_IN_WORD);
            data_[pos] = val;
        };
        auto process_part = [this, &handler, &process_word](size_t pos, size_t idx_start, size_t idx_dest) {
            const auto len = idx_dest - idx_start + 1;
            if (len == BITS_IN_WORD) {
                process_word(pos);
            } else {
                const Word mask = MaskUpToIndex(len);
                const Word val = handler((data_[pos] >> idx_start) & mask, len) & mask;
                data_[pos] &= ~(mask << idx_start);
                data_[pos] |= val << idx_start;
            }
        };
        if (pos_from == pos_to) {
            process_part(pos_from, idx_from, idx_to);
        } else {
            process_part(pos_from, idx_from, BITS_IN_WORD - 1);
            for (size_t pos = pos_from + 1; pos < pos_to; ++pos) {
                process_word(pos);
            }
            process_part(pos_to, 0, idx_to);
        }
    }

    void Clr(size_t from, size_t to)
    {
        Process(from, to, [](auto, auto) { return static_cast<Word>(0); });
    }

    void Set(size_t from, size_t to)
    {
        Process(from, to, [](auto, auto) { return MAX_WORD; });
    }

    void Invert(size_t from, size_t to)
    {
        Process(from, to, [](auto val, auto) { return ~val; });
    }

    template <typename Handler>
    void Process(const BitVector &rhs,
                 Handler &&handler)  // every handler result bit must depend only from corresponding bit pair
    {
        size_t sz = std::min(Size(), rhs.Size());
        size_t words = SizeInWordsFromSizeInBits(sz);
        size_t lhs_words = SizeInWords();
        size_t pos = 0;
        for (; pos < words; ++pos) {
            data_[pos] = handler(data_[pos], rhs.data_[pos]);
        }
        if ((pos >= lhs_words) || ((handler(0U, 0U) == 0U) && (handler(1U, 0U) == 1U))) {
            return;
        }
        for (; pos < lhs_words; ++pos) {
            data_[pos] = handler(data_[pos], 0U);
        }
    }

    BitVector &operator&=(const BitVector &rhs)
    {
        Process(rhs, [](const auto l, const auto r) { return l & r; });
        return *this;
    }

    BitVector &operator|=(const BitVector &rhs)
    {
        if (Size() < rhs.Size()) {
            Resize(rhs.Size());
        }
        Process(rhs, [](const auto l, const auto r) { return l | r; });
        return *this;
    }

    BitVector &operator^=(const BitVector &rhs)
    {
        if (Size() < rhs.Size()) {
            Resize(rhs.Size());
        }
        Process(rhs, [](const auto l, const auto r) { return l ^ r; });
        return *this;
    }

    BitVector operator&(const BitVector &rhs) const
    {
        if (Size() > rhs.Size()) {
            return rhs & *this;
        }
        BitVector result {*this};
        result &= rhs;
        return result;
    }

    BitVector operator|(const BitVector &rhs) const
    {
        if (Size() < rhs.Size()) {
            return rhs | *this;
        }
        BitVector result {*this};
        result |= rhs;
        return result;
    }

    BitVector operator^(const BitVector &rhs) const
    {
        if (Size() < rhs.Size()) {
            return rhs ^ *this;
        }
        BitVector result {*this};
        result ^= rhs;
        return result;
    }

    template <typename Handler>
    void ForAllIdxVal(Handler handler) const
    {
        size_t last_word_partial_bits = Size() % BITS_IN_WORD;
        size_t num_full_words = SizeInWords() - (last_word_partial_bits ? 1 : 0);
        for (size_t pos = 0; pos < num_full_words; pos++) {
            Word val = data_[pos];
            if (!handler(pos * BITS_IN_WORD, val)) {
                return;
            }
        }
        if (last_word_partial_bits) {
            size_t last_word_start = Size() - last_word_partial_bits;
            Word val = bits(last_word_start, Size() - 1);
            handler(last_word_start, val);
        }
    }

    template <const int VAL, typename Handler>
    bool ForAllIdxOf(Handler handler) const
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            auto val = data_[pos];
            val = VAL ? val : ~val;
            size_t idx = pos << POS_SHIFT;
            while (val) {
                auto i = static_cast<size_t>(panda::Ctz(val));
                idx += i;
                if (idx >= Size()) {
                    return true;
                }
                if (!handler(idx)) {
                    return false;
                }
                ++idx;
                val >>= i;
                val >>= 1;
            }
        }
        return true;
    }

    template <const int VAL>
    auto LazyIndicesOf(size_t from = 0, size_t to = std::numeric_limits<size_t>::max()) const
    {
        size_t idx = from;
        size_t pos = from >> POS_SHIFT;
        auto val = (VAL ? data_[pos] : ~data_[pos]) >> (idx % BITS_IN_WORD);
        ++pos;
        to = std::min(size_ - 1, to);
        auto sz = SizeInWordsFromSizeInBits(to + 1);
        return [this, sz, to, pos, val, idx]() mutable -> Index<size_t> {
            while (true) {
                if (idx > to) {
                    return {};
                }
                if (val) {
                    auto i = static_cast<size_t>(panda::Ctz(val));
                    idx += i;
                    if (idx > to) {
                        return {};
                    }
                    val >>= i;
                    val >>= 1;
                    return idx++;
                }
                while (val == 0 && pos < sz) {
                    val = VAL ? data_[pos] : ~data_[pos];
                    ++pos;
                }
                idx = (pos - 1) << POS_SHIFT;
                if (pos >= sz && val == 0) {
                    return {};
                }
            }
        };
    }

    size_t SetBitsCount() const
    {
        size_t result = 0;

        size_t pos = 0;
        bool last_word_partially_filled = (Size() & POS_MASK) != 0;
        if (SizeInWords() > 0) {
            for (; pos < (SizeInWords() - (last_word_partially_filled ? 1 : 0)); ++pos) {
                result += static_cast<size_t>(panda::Popcount(data_[pos]));
            }
        }
        if (last_word_partially_filled) {
            const Word mask = MaskUpToIndex(Size() & POS_MASK);
            result += static_cast<size_t>(panda::Popcount(data_[pos] & mask));
        }
        return result;
    }

    template <typename Op, typename Binop, typename... Args>
    static size_t PowerOfOpThenFold(Op op, Binop binop, const Args &...args)
    {
        size_t result = 0;

        size_t sz = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.SizeInWords()...);
        size_t size = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.Size()...);
        size_t num_args = sizeof...(Args);
        auto get_processed_word = [&op, &binop, num_args, &args...](size_t idx) {
            size_t n = 0;
            auto unop = [&n, num_args, &op](Word val) { return op(val, n++, num_args); };
            return NAry {binop}(std::tuple<std::decay_t<decltype(args.data_[idx])>...> {unop(args.data_[idx])...});
        };

        size_t pos = 0;
        bool last_word_partially_filled = (size & POS_MASK) != 0;
        if (sz > 0) {
            for (; pos < (sz - (last_word_partially_filled ? 1 : 0)); ++pos) {
                auto val = get_processed_word(pos);
                result += static_cast<size_t>(panda::Popcount(val));
            }
        }
        if (last_word_partially_filled) {
            const Word mask = MaskUpToIndex(size & POS_MASK);
            result += static_cast<size_t>(panda::Popcount(get_processed_word(pos) & mask));
        }
        return result;
    }

    template <typename... Args>
    static size_t PowerOfAnd(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t, size_t) { return val; },
                                 [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    template <typename... Args>
    static size_t PowerOfOr(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t, size_t) { return val; },
                                 [](Word lhs, Word rhs) { return lhs | rhs; }, args...);
    }

    template <typename... Args>
    static size_t PowerOfXor(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t, size_t) { return val; },
                                 [](Word lhs, Word rhs) { return lhs ^ rhs; }, args...);
    }

    template <typename... Args>
    static size_t PowerOfAndNot(const Args &...args)
    {
        return PowerOfOpThenFold(
            [](Word val, size_t idx, size_t num_args) { return (idx < num_args - 1) ? val : ~val; },
            [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    size_t Size() const
    {
        return size_;
    }

    size_t SizeInWords() const
    {
        return SizeInWordsFromSizeInBits(size_);
    }

    void Resize(size_t sz)
    {
        if (sz == 0) {
            Deallocate();
        } else {
            size_t new_size_in_words = SizeInWordsFromSizeInBits(sz);
            size_t old_size_in_words = SizeInWordsFromSizeInBits(size_);
            if (old_size_in_words != new_size_in_words) {
                Allocator allocator;
                Word *new_data = allocator.allocate(new_size_in_words);
                ASSERT(new_data != nullptr);
                size_t pos = 0;
                for (; pos < std::min(old_size_in_words, new_size_in_words); ++pos) {
                    new_data[pos] = data_[pos];
                }
                for (; pos < new_size_in_words; ++pos) {
                    new_data[pos] = 0;
                }
                Deallocate();
                data_ = new_data;
            }
            size_ = sz;
        }
    }

    template <const int V, typename Op, typename BinOp, typename... Args>
    static auto LazyOpThenFoldThenIndicesOf(Op op, BinOp binop, const Args &...args)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace panda::verifier;
        size_t sz = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.SizeInWords()...);
        size_t size = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.Size()...);
        size_t num_args = sizeof...(Args);
        auto get_processed_word = [op, binop, num_args, &args...](size_t idx) {
            size_t n = 0;
            auto unop = [&n, num_args, &op](Word val) { return op(val, n++, num_args); };
            Word val = NAry {binop}(std::tuple<std::decay_t<decltype(args.data_[idx])>...> {unop(args.data_[idx])...});
            return V ? val : ~val;
        };
        size_t pos = 0;
        auto val = get_processed_word(pos++);
        size_t idx = 0;
        return [sz, size, pos, val, idx, get_processed_word]() mutable -> Index<size_t> {
            do {
                if (idx >= size) {
                    return {};
                }
                if (val) {
                    auto i = static_cast<size_t>(panda::Ctz(val));
                    idx += i;
                    if (idx >= size) {
                        return {};
                    }
                    val >>= i;
                    val >>= 1;
                    return idx++;
                }
                while (val == 0 && pos < sz) {
                    val = get_processed_word(pos++);
                }
                idx = (pos - 1) << POS_SHIFT;
                if (pos >= sz && val == 0) {
                    return {};
                }
            } while (true);
        };
    }

    template <const int V, typename... Args>
    static auto LazyAndThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>([](Word val, size_t, size_t) { return val; },
                                              [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    template <const int V, typename... Args>
    static auto LazyOrThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>([](Word val, size_t, size_t) { return val; },
                                              [](Word lhs, Word rhs) { return lhs | rhs; }, args...);
    }

    template <const int V, typename... Args>
    static auto LazyXorThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>([](Word val, size_t, size_t) { return val; },
                                              [](Word lhs, Word rhs) { return lhs ^ rhs; }, args...);
    }

    template <const int V, typename... Args>
    static auto LazyAndNotThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>(
            [](Word val, size_t idx, size_t num_args) {
                val = (idx < num_args - 1) ? val : ~val;
                return val;
            },
            [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,hicpp-signed-bitwise)

private:
    size_t size_;
    Word *data_ = nullptr;

    void CopyFrom(const BitVector &other)
    {
        size_ = other.size_;
        size_t size_in_words = other.SizeInWords();
        data_ = Allocator().allocate(size_in_words);
        std::copy_n(other.data_, size_in_words, data_);
    }

    void MoveFrom(BitVector &&other) noexcept
    {
        size_ = other.size_;
        data_ = other.data_;
        // don't rhs.Deallocate() as we stole its data_!
        other.size_ = 0;
        other.data_ = nullptr;
    }
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_BIT_VECTOR_HPP_
