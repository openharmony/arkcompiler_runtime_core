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
#ifndef PANDA_VERIFICATION_UTIL_INT_SET_H
#define PANDA_VERIFICATION_UTIL_INT_SET_H

#include "bit_vector.h"

namespace panda::verifier {

/**
 * @brief A set implementation for integral types which automatically switches between representations
 * (e.g. std::unordered_set or a sorted vector for small sets, bitvector for large sets).
 *
 * @tparam T Element type
 * @tparam THRESHOLD threshold for switching between representations
 */
template <typename T, size_t THRESHOLD = 256>
class IntSet {
public:
    IntSet() : repr_ {MMakePandaUnique<SmallRepr>()} {};

    IntSet(const IntSet &other) : repr_ {other.repr_->Clone()} {};
    IntSet &operator=(const IntSet &other)
    {
        repr_ = other.repr_->Clone();
        return *this;
    }

    IntSet(IntSet &&) noexcept = default;
    IntSet &operator=(IntSet &&) noexcept = default;

    ~IntSet() = default;

    bool Contains(T x) const
    {
        return repr_->Contains(x);
    }

    size_t Size() const
    {
        return repr_->Size();
    }

    void Insert(T x)
    {
        repr_->Insert(x);
        if (UNLIKELY(repr_->Type() == ReprType::SMALL && Size() >= THRESHOLD)) {
            MoveToLargeRepr();
        }
    }

    template <bool known_to_be_sorted = false, typename Iter>
    void Insert(Iter begin, Iter end)
    {
        switch (repr_->Type()) {
            case ReprType::SMALL:
                AsSmallRepr().template InsertManyImpl<known_to_be_sorted>(begin, end);
                if (UNLIKELY(Size() >= THRESHOLD)) {
                    MoveToLargeRepr();
                    if (begin != end) {
                        // if we get here, repr is large now and there are remaining elements
                        AsLargeRepr().template InsertManyImpl<known_to_be_sorted>(std::move(begin), std::move(end));
                    }
                }
                return;
            case ReprType::LARGE:
                AsLargeRepr().template InsertManyImpl<known_to_be_sorted>(std::move(begin), std::move(end));
                return;
            default:
                UNREACHABLE();
        }
    }

    Index<T> TheOnlyElement() const
    {
        return SwitchOnRepr(
            [](const SmallRepr &repr) -> Index<T> {
                if (repr.Size() == 1) {
                    return repr.repr_[0];
                }
                return {};
            },
            []([[maybe_unused]] const LargeRepr &repr) { return Index<T> {}; });
    }

    template <size_t THRESHOLD2>
    IntSet<T, THRESHOLD> operator&(const IntSet<T, THRESHOLD2> &other) const
    {
        return other.SwitchOnRepr([this](const auto &other_repr) { return repr_->Intersect(other_repr); });
    }

    template <size_t THRESHOLD2>
    IntSet<T, THRESHOLD> &operator&=(const IntSet<T, THRESHOLD2> &other)
    {
        // TODO if this case is checked separately, consider switching on all 4 options
        if (repr_->Type() == ReprType::LARGE && other.repr_->Type() == ReprType::SMALL) {
            *this = other & *this;
        } else {
            bool change_repr =
                other.SwitchOnRepr([this](const auto &other_repr) { return repr_->IntersectInPlace(other_repr); });
            if (change_repr) {
                MoveToSmallRepr();
            }
        }
        return *this;
    }

    template <size_t THRESHOLD2>
    IntSet<T, THRESHOLD> operator|(const IntSet<T, THRESHOLD2> &other) const
    {
        return other.SwitchOnRepr([this](const auto &other_repr) { return repr_->Union(other_repr); });
    }

    template <size_t THRESHOLD2>
    IntSet<T, THRESHOLD> &operator|=(const IntSet<T, THRESHOLD2> &other)
    {
        if (other.repr_->Type() == ReprType::SMALL) {
            const auto &other_repr = other.AsSmallRepr().repr_;
            Insert<true>(other_repr.cbegin(), other_repr.cend());
        } else {
            if (repr_->Type() == ReprType::SMALL) {
                *this = other | *this;
            } else {
                AsLargeRepr().UnionInPlace(other.AsLargeRepr());
            }
        }
        return *this;
    }

    /// Returns a lambda repeated calls to which return ordered values of the intersection
    template <size_t THRESHOLD2>
    auto LazyIntersect(const IntSet<T, THRESHOLD2> &other) const
    {
        auto &&stream1 = AsStream();
        auto &&stream2 = other.AsStream();
        return [val1 = stream1(), val2 = stream2(), stream1 = std::move(stream1),
                stream2 = std::move(stream2)]() mutable -> Index<T> {
            while (val1.IsValid() && val2.IsValid()) {
                if (val1 < val2) {
                    val1 = stream1();
                } else if (val1 > val2) {
                    val2 = stream2();
                } else {
                    auto tmp = val1;
                    val1 = stream1();
                    val2 = stream2();
                    return tmp;
                }
            }
            return {};
        };
    }

    template <typename Handler>
    bool ForAll(Handler &&handler) const
    {
        return SwitchOnRepr(
            [handler = std::forward<Handler>(handler)](const auto &repr) { return repr.ForAll(handler); });
    }

    std::function<Index<T>()> AsStream() const
    {
        return SwitchOnRepr([](const auto &repr) { return std::function<Index<T>()>(repr.AsStream()); });
    }

    template <size_t THRESHOLD2>
    bool operator==(const IntSet<T, THRESHOLD2> &rhs) const
    {
        ReprType lhs_type = repr_->Type();
        ReprType rhs_type = rhs.repr_->Type();
        if (lhs_type == ReprType::SMALL && rhs_type == ReprType::SMALL) {
            return AsSmallRepr().repr_ == rhs.AsSmallRepr().repr_;
        } else if (lhs_type == ReprType::LARGE && rhs_type == ReprType::LARGE) {
            return AsLargeRepr().repr_ == rhs.AsLargeRepr().repr_;
        } else {
            if (Size() != rhs.Size()) {
                return false;
            }
            auto lhs_stream {AsStream()};
            auto rhs_stream {rhs.AsStream()};
            auto lhs_val {lhs_stream()};
            auto rhs_val {rhs_stream()};
            while (lhs_val.IsValid() && rhs_val.IsValid()) {
                if (lhs_val != rhs_val) {
                    return false;
                }
                lhs_val = lhs_stream();
                rhs_val = rhs_stream();
            }
            return lhs_val == rhs_val;
        }
    }

    template <size_t THRESHOLD2>
    bool operator!=(const IntSet<T, THRESHOLD2> &rhs) const
    {
        return !(*this == rhs);
    }

private:
    enum class ReprType { SMALL, LARGE };

    class SmallRepr;
    class LargeRepr;

    class Repr {
    public:
        virtual ReprType Type() const = 0;
        virtual bool Contains(T x) const = 0;
        virtual void Insert(T x) = 0;
        virtual size_t Size() const = 0;
        virtual IntSet<T, THRESHOLD> Intersect(const SmallRepr &other) const = 0;
        virtual IntSet<T, THRESHOLD> Intersect(const LargeRepr &other) const = 0;
        // returns true if repr should be changed (from Large to Small)
        virtual bool IntersectInPlace(const SmallRepr &other) = 0;
        virtual bool IntersectInPlace(const LargeRepr &other) = 0;
        virtual IntSet<T, THRESHOLD> Union(const SmallRepr &other) const = 0;
        virtual IntSet<T, THRESHOLD> Union(const LargeRepr &other) const = 0;
        virtual MPandaUniquePtr<Repr> Clone() const = 0;
        virtual ~Repr() = default;
    };

    class SmallRepr final : public Repr {
    public:
        SmallRepr() = default;
        SmallRepr(MPandaVector<T> set) : repr_ {set} {};

        ReprType Type() const override
        {
            return ReprType::SMALL;
        }

        bool Contains(T x) const override
        {
            return std::binary_search(repr_.begin(), repr_.end(), x);
        }

        void Insert(T x) override
        {
            Insert(x, 0);
        }

        size_t Size() const override
        {
            return repr_.size();
        }

        IntSet<T, THRESHOLD> Intersect(const SmallRepr &other) const override
        {
            if (other.Size() < Size()) {
                return other.Intersect(*this);
            } else {
                MPandaVector<T> result;
                std::set_intersection(repr_.begin(), repr_.end(), other.repr_.begin(), other.repr_.end(),
                                      std::back_inserter(result));
                return {result};
            }
        }

        IntSet<T, THRESHOLD> Intersect(const LargeRepr &other) const override
        {
            MPandaVector<T> result;
            for (T value : repr_) {
                if (other.Contains(value)) {
                    result.push_back(value);
                }
            }
            return {result};
        }

        bool IntersectInPlace(const SmallRepr &other) override
        {
            repr_.erase(
                std::remove_if(repr_.begin(), repr_.end(),
                               [&, other_iter = other.repr_.begin(), other_end = other.repr_.end()](T x) mutable {
                                   other_iter = std::lower_bound(other_iter, other_end, x);
                                   return other_iter == other_end || *other_iter != x;
                               }),
                repr_.end());
            return false;
        }

        bool IntersectInPlace(const LargeRepr &other) override
        {
            repr_.erase(std::remove_if(repr_.begin(), repr_.end(), [&](T x) { return !other.Contains(x); }),
                        repr_.end());
            return false;
        }

        IntSet<T, THRESHOLD> Union(const SmallRepr &other) const override
        {
            MPandaVector<T> result;
            std::set_union(repr_.begin(), repr_.end(), other.repr_.begin(), other.repr_.end(),
                           std::back_inserter(result));
            if (result.size() < THRESHOLD) {
                return result;
            } else {
                return VectorToBitVector(result);
            }
        }

        IntSet<T, THRESHOLD> Union(const LargeRepr &other) const override
        {
            return other.Union(*this);
        }

        MPandaUniquePtr<Repr> Clone() const override
        {
            return MMakePandaUnique<SmallRepr>(repr_);
        }

        T MaxElem() const
        {
            return *repr_.rbegin();
        }

        template <bool known_to_be_sorted, typename Iter>
        void InsertManyImpl(Iter &begin, const Iter &end)
        {
            size_t sz = Size();
            size_t lower_bound = 0;
            while (sz < THRESHOLD) {
                for (size_t i = sz; i < THRESHOLD; i++) {
                    if (begin == end) {
                        return;
                    }
                    if (known_to_be_sorted) {
                        lower_bound = Insert(*begin, lower_bound);
                    } else {
                        Insert(*begin, 0);
                    }
                    ++begin;
                }
                sz = Size();
            }
        }

        template <typename Handler>
        bool ForAll(Handler &&handler) const
        {
            for (T value : repr_) {
                if (!handler(value)) {
                    return false;
                }
            }
            return true;
        }

        auto AsStream() const
        {
            return [i = size_t(0), this]() mutable -> Index<T> {
                if (i < repr_.size()) {
                    return repr_[i++];
                } else {
                    return {};
                }
            };
        }

    private:
        size_t Insert(T x, size_t lower_bound)
        {
            auto iter = std::lower_bound(repr_.begin() + lower_bound, repr_.end(), x);
            auto new_lower_bound = static_cast<size_t>(iter - repr_.begin());
            if (iter == repr_.end()) {
                repr_.push_back(x);
            } else if (*iter != x) {
                repr_.insert(iter, x);
            }
            return new_lower_bound;
        }

        MPandaVector<T> repr_;
        friend class IntSet;
    };

    class LargeRepr final : public Repr {
    public:
        LargeRepr(BitVector set) : repr_ {set} {};

        ReprType Type() const override
        {
            return ReprType::LARGE;
        }

        bool Contains(T x) const override
        {
            return x < repr_.size() && repr_[x];
        }

        void Insert(T x) override
        {
            if (x >= repr_.size()) {
                // clang-tidy under GCC bug, static_cast<size_t>(x) * 3U / 2U is enough
                repr_.resize(std::max(static_cast<size_t>(x) * 3U / 2U, THRESHOLD));
            }
            repr_.Set(x);
        }

        size_t Size() const override
        {
            return repr_.SetBitsCount();
        }

        IntSet<T, THRESHOLD> Intersect(const SmallRepr &other) const override
        {
            return other.Intersect(*this);
        }

        IntSet<T, THRESHOLD> Intersect(const LargeRepr &other) const override
        {
            BitVector res = repr_ & other.repr_;
            if (res.SetBitsCount() >= THRESHOLD) {
                return res;
            } else {
                return BitVectorToVector(res);
            }
        }

        bool IntersectInPlace(const SmallRepr &other) override
        {
            if (other.Size() == 0) {
                repr_ = BitVector(0);
            } else {
                size_t other_bv_size = other.MaxElem() + 1;
                BitVector bv(other_bv_size);
                for (T x : other.repr_) {
                    bv.Set(x);
                }
                ResizeDownOnly(other_bv_size);
                repr_ &= bv;
            }
            return true;
        }

        bool IntersectInPlace(const LargeRepr &other) override
        {
            ResizeDownOnly(other.repr_.size());
            repr_ &= other.repr_;
            return Size() < THRESHOLD;
        }

        IntSet<T, THRESHOLD> Union(const SmallRepr &other) const override
        {
            IntSet<T, THRESHOLD> result {Clone()};
            result.Insert<true>(other.repr_.cbegin(), other.repr_.cend());
            return result;
        }

        IntSet<T, THRESHOLD> Union(const LargeRepr &other) const override
        {
            return {repr_ | other.repr_};
        }

        void UnionInPlace(const LargeRepr &other)
        {
            ResizeUpOnly(other.repr_.size());
            repr_ |= other.repr_;
        }

        MPandaUniquePtr<Repr> Clone() const override
        {
            return MMakePandaUnique<LargeRepr>(repr_);
        }

        template <bool known_to_be_sorted, typename Iter>
        void InsertManyImpl(Iter begin, Iter end)
        {
            while (begin != end) {
                Insert(*begin);
                ++begin;
            }
        }

        template <typename Handler>
        bool ForAll(Handler &&handler) const
        {
            return repr_.for_all_idx_of<1>(std::forward<Handler>(handler));
        }

        auto AsStream() const
        {
            return repr_.LazyIndicesOf<1>();
        }

    private:
        BitVector repr_;

        void ResizeDownOnly(size_t sz)
        {
            if (sz < repr_.size()) {
                repr_.resize(sz);
            }
        }

        void ResizeUpOnly(size_t sz)
        {
            if (sz > repr_.size()) {
                repr_.resize(sz);
            }
        }

        friend class IntSet;
    };

    friend class SmallRepr;
    friend class LargeRepr;

    MPandaUniquePtr<Repr> repr_;

    IntSet(MPandaVector<T> set) : repr_ {MMakePandaUnique<SmallRepr>(set)} {};
    IntSet(BitVector set) : repr_ {MMakePandaUnique<LargeRepr>(set)} {};
    IntSet(MPandaUniquePtr<Repr> &&repr) : repr_ {std::move(repr)} {};

    // unsafe!
    const SmallRepr &AsSmallRepr() const
    {
        return *static_cast<const SmallRepr *>(repr_.get());
    }

    const LargeRepr &AsLargeRepr() const
    {
        return *static_cast<const LargeRepr *>(repr_.get());
    }

    SmallRepr &AsSmallRepr()
    {
        return *static_cast<SmallRepr *>(repr_.get());
    }

    LargeRepr &AsLargeRepr()
    {
        return *static_cast<LargeRepr *>(repr_.get());
    }

    template <typename SmallCase, typename LargeCase>
    auto SwitchOnRepr(SmallCase &&smallCase, LargeCase &&largeCase) const
    {
        switch (repr_->Type()) {
            case ReprType::SMALL:
                return smallCase(AsSmallRepr());
            case ReprType::LARGE:
                return largeCase(AsLargeRepr());
            default:
                UNREACHABLE();
        }
    }

    template <typename CommonCase>
    auto SwitchOnRepr(CommonCase &&commonCase) const
    {
        return SwitchOnRepr(commonCase, commonCase);
    }

    void MoveToLargeRepr()
    {
        repr_ = MMakePandaUnique<LargeRepr>(VectorToBitVector(AsSmallRepr().repr_));
    }

    void MoveToSmallRepr()
    {
        repr_ = MMakePandaUnique<SmallRepr>(BitVectorToVector(AsLargeRepr().repr_));
    }

    static MPandaVector<T> BitVectorToVector(const BitVector &bv)
    {
        MPandaVector<T> res;
        bv.for_all_idx_of<1>([&](size_t idx) {
            res.push_back(idx);
            return true;
        });
        return res;
    }

    static BitVector VectorToBitVector(const MPandaVector<T> &vec)
    {
        BitVector bv(*vec.rbegin() * 3 / 2);
        for (T y : vec) {
            bv.Set(y);
        }
        return bv;
    }
};

template <typename T, size_t THRESHOLD>
std::ostream &operator<<(std::ostream &os, const IntSet<T, THRESHOLD> &set)
{
    os << "IntSet{";
    bool first = true;
    set.ForAll([&](T value) {
        if (first) {
            first = false;
        } else {
            os << " ";
        }
        os << value;
        return true;
    });
    os << "}";
    return os;
}

}  // namespace panda::verifier

#endif  // PANDA_VERIFICATION_UTIL_INT_SET_H
