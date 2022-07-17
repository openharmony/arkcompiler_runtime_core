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

#ifndef PANDA_VERIFIER_UTIL_TAGGED_INDEX_HPP_
#define PANDA_VERIFIER_UTIL_TAGGED_INDEX_HPP_

#include "libpandabase/macros.h"
#include "libpandabase/utils/bit_utils.h"

#include "verification/util/index.h"
#include "verification/util/hash.h"

#include <limits>
#include <tuple>
#include <type_traits>

#include <iostream>

namespace panda::verifier {

template <typename... Tag>
class TagPack {
public:
    static constexpr size_t Bits = 0ULL;
    static constexpr size_t Quantity = 0ULL;
    static constexpr size_t TagShift = 0ULL;
    static constexpr size_t TagBits = 0ULL;

    static constexpr auto GetTagShift()
    {
        return std::tuple<> {};
    }
    static constexpr auto GetTagBits()
    {
        return std::tuple<> {};
    }
    template <typename, size_t>
    static constexpr auto GetTagMask()
    {
        return std::tuple<> {};
    }
    static constexpr auto GetTagHandler()
    {
        return std::tuple<> {};
    }
    template <typename Int, const size_t Shift>
    static constexpr void SetTags([[maybe_unused]] Int &val)
    {
    }
};

template <typename T, typename... Tag>
class TagPack<T, Tag...> : private TagPack<Tag...> {
    using Base = TagPack<Tag...>;

public:
    static constexpr size_t Bits = Base::Bits + T::Bits;
    static constexpr size_t Quantity = Base::Quantity + 1ULL;
    static constexpr size_t TagShift = Base::Bits;
    static constexpr size_t TagBits = T::Bits;

    static constexpr auto GetTagShift()
    {
        return std::tuple_cat(std::tuple<size_t> {TagShift}, Base::GetTagShift());
    }
    static constexpr auto GetTagBits()
    {
        return std::tuple_cat(std::tuple<size_t> {TagBits}, Base::GetTagBits());
    }
    template <typename Int, const size_t Shift>
    static constexpr auto GetMask()
    {
        using UInt = std::make_unsigned_t<Int>;
        UInt mask = ((static_cast<UInt>(1) << TagBits) - static_cast<UInt>(1)) << TagShift;
        return mask << Shift;
    }
    template <typename Int, const size_t Shift>
    static constexpr auto GetTagMask()
    {
        using UInt = std::make_unsigned_t<Int>;
        return std::tuple_cat(std::tuple<UInt> {GetMask<Int, Shift>()}, Base::template GetTagMask<Int, Shift>());
    }
    static constexpr auto GetTagHandler()
    {
        return std::tuple_cat(std::tuple<T> {}, Base::GetTagHandler());
    }

    template <typename Int, const size_t Shift>
    static constexpr void SetTags(const typename T::type &tag, const typename Tag::type &... tags, Int &val)
    {
        using UInt = std::make_unsigned_t<Int>;
        auto uint_val = static_cast<UInt>(val);
        auto mask = GetMask<Int, Shift>();
        uint_val &= ~mask;
        auto tag_val = static_cast<UInt>(tag);
        tag_val <<= TagShift + Shift;
        tag_val &= mask;
        uint_val |= tag_val;
        val = static_cast<Int>(uint_val);
        Base::template SetTags<Int, Shift>(std::forward<const typename Tag::type>(tags)..., val);
    }
};

template <typename...>
class TaggedIndexHelper0;

template <typename... Tags, typename Int>
class TaggedIndexHelper0<Int, TagPack<Tags...>> {
    using AllTags = TagPack<Tags...>;
    using UInt = std::make_unsigned_t<Int>;
    static constexpr size_t UIntBits = sizeof(UInt) * 8ULL;
    static constexpr size_t AllTagBits = AllTags::Bits;
    static constexpr size_t TagQuantity = AllTags::Quantity;
    static constexpr size_t IntBits = UIntBits - AllTagBits - 1ULL;
    static constexpr UInt ALL_TAG_MASK = ((static_cast<UInt>(1) << AllTagBits) - static_cast<UInt>(1)) << IntBits;
    static constexpr UInt VALIDITY_BIT = (static_cast<UInt>(1) << (UIntBits - static_cast<size_t>(1)));
    static constexpr UInt ALL_TAG_AND_VALIDITY_MASK = ALL_TAG_MASK & VALIDITY_BIT;
    static constexpr UInt VALUE_MASK = (static_cast<UInt>(1) << IntBits) - static_cast<UInt>(1);
    static constexpr size_t VALUE_SIGN_BIT = (static_cast<UInt>(1) << (IntBits - static_cast<size_t>(1)));
    static constexpr UInt MAX_VALUE = VALUE_MASK;
    static constexpr UInt INVALID = static_cast<UInt>(0);

    template <size_t tagnum>
    static constexpr size_t TagShift()
    {
        return IntBits + std::get<tagnum>(AllTags::GetTagShift());
    }
    template <size_t tagnum>
    static constexpr size_t TagBits()
    {
        return std::get<tagnum>(AllTags::GetTagBits());
    }
    template <size_t tagnum>
    static constexpr UInt TagMask()
    {
        return std::get<tagnum>(AllTags::template GetTagMask<UInt, IntBits>());
    }
    template <size_t tagnum>
    using TagHandler = std::tuple_element_t<tagnum, decltype(AllTags::GetTagHandler())>;

    void SetValid()
    {
        Value_ |= VALIDITY_BIT;
    }

public:
    TaggedIndexHelper0() = default;
    TaggedIndexHelper0(typename Tags::type... tags, Int idx)
    {
        AllTags::template SetTags<UInt, IntBits>(std::forward<typename Tags::type>(tags)..., Value_);
        SetValid();
        SetInt(idx);
    }
    void SetInt(Int val)
    {
        ASSERT(IsValid());  // tag should be set before value
        if constexpr (std::is_signed_v<Int>) {
            if (val < 0) {
                ASSERT(static_cast<UInt>(-val) <= MAX_VALUE >> static_cast<UInt>(1));
            } else {
                ASSERT(static_cast<UInt>(val) <= MAX_VALUE >> static_cast<UInt>(1));
            }
        } else {
            ASSERT(static_cast<UInt>(val) <= MAX_VALUE);
        }
        Value_ &= ~VALUE_MASK;
        Value_ |= (static_cast<UInt>(val) & VALUE_MASK);
    }
    TaggedIndexHelper0 &operator=(Int val)
    {
        SetInt(val);
        return *this;
    }
    template <size_t N, typename Tag>
    void SetTag(Tag tag)
    {
        ASSERT(N < TagQuantity);
        SetValid();
        Value_ &= ~TagMask<N>();
        Value_ |= static_cast<UInt>(TagHandler<N>::GetIndexFor(tag)) << TagShift<N>();
    }
    TaggedIndexHelper0(const TaggedIndexHelper0 &) = default;
    TaggedIndexHelper0(TaggedIndexHelper0 &&idx) : Value_ {idx.Value_}
    {
        idx.Invalidate();
    }
    TaggedIndexHelper0 &operator=(const TaggedIndexHelper0 &) = default;
    TaggedIndexHelper0 &operator=(TaggedIndexHelper0 &&idx)
    {
        Value_ = idx.Value_;
        idx.Invalidate();
        return *this;
    }
    ~TaggedIndexHelper0() = default;
    void Invalidate()
    {
        Value_ = INVALID;
    }
    bool IsValid() const
    {
        return (Value_ & VALIDITY_BIT) != 0;
    }
    template <size_t N>
    auto GetTag() const
    {
        ASSERT(IsValid());
        return TagHandler<N>::GetValueFor((Value_ & TagMask<N>()) >> TagShift<N>());
    }
    Int GetInt() const
    {
        ASSERT(IsValid());
        UInt val = Value_ & VALUE_MASK;
        Int ival;
        if constexpr (std::is_signed_v<Int>) {
            if (val & VALUE_SIGN_BIT) {
                val |= ALL_TAG_AND_VALIDITY_MASK;  // sign-extend
                ival = static_cast<Int>(val);
            } else {
                ival = static_cast<Int>(val);
            }
        } else {
            ival = static_cast<Int>(val);
        }
        return ival;
    }
    Index<Int> GetIndex() const
    {
        if (IsValid()) {
            return GetInt();
        }
        return {};
    }
    operator Index<Int>() const
    {
        return GetIndex();
    }
    template <const Int INV>
    Index<Int, INV> GetIndex() const
    {
        ASSERT(static_cast<UInt>(INV) > MAX_VALUE);
        if (IsValid()) {
            return GetInt();
        }
        return {};
    }
    template <const Int INV>
    operator Index<Int, INV>() const
    {
        return GetIndex<INV>();
    }
    operator Int() const
    {
        ASSERT(IsValid());
        return GetInt();
    }
    bool operator==(const TaggedIndexHelper0 rhs) const
    {
        ASSERT(IsValid());
        ASSERT(rhs.IsValid());
        return Value_ == rhs.Value_;
    }
    bool operator!=(const TaggedIndexHelper0 rhs) const
    {
        ASSERT(IsValid());
        ASSERT(rhs.IsValid());
        return Value_ != rhs.Value_;
    }

private:
    UInt Value_ {INVALID};
    template <typename T>
    friend struct std::hash;
};

struct First;
struct Second;

template <typename...>
class TaggedIndexSelectorH;

template <typename Int, typename... Tags>
class TaggedIndexSelectorH<First, Int, std::tuple<Tags...>> : public TaggedIndexHelper0<Int, TagPack<Tags...>> {
    using Base = TaggedIndexHelper0<Int, TagPack<Tags...>>;

public:
    TaggedIndexSelectorH() = default;
    TaggedIndexSelectorH(typename Tags::type... tags, Int &val) : Base {std::forward<typename Tags::type>(tags)..., val}
    {
    }

    ~TaggedIndexSelectorH() = default;
};

template <typename Int, typename... Tags>
class TaggedIndexSelectorH<Second, Int, std::tuple<Tags...>>
    : public TaggedIndexHelper0<size_t, TagPack<Tags..., Int>> {
    using Base = TaggedIndexHelper0<Int, TagPack<Tags..., Int>>;

public:
    TaggedIndexSelectorH() = default;
    TaggedIndexSelectorH(typename Tags::type... tags, size_t &val)
        : Base {std::forward<typename Tags::type>(tags)..., val}
    {
    }

    ~TaggedIndexSelectorH() = default;
};

template <typename Int, typename... Tags>
class TaggedIndexSelector : public TaggedIndexSelectorH<std::conditional_t<std::is_integral_v<Int>, First, Second>, Int,
                                                        std::tuple<Tags...>> {
    using Base =
        TaggedIndexSelectorH<std::conditional_t<std::is_integral_v<Int>, First, Second>, Int, std::tuple<Tags...>>;

public:
    TaggedIndexSelector() = default;
    TaggedIndexSelector(typename Tags::type... tags, Int &val) : Base {std::forward<typename Tags::type>(tags)..., val}
    {
    }

    ~TaggedIndexSelector() = default;
};

template <typename...>
class TaggedIndexHelper2;

template <typename... Tags, typename Int>
class TaggedIndexHelper2<std::tuple<Tags...>, std::tuple<Int>> {
public:
    using TagsInTuple = std::tuple<Tags...>;
    using IntType = Int;
};

template <typename... Ls, typename R, typename... Rs>
class TaggedIndexHelper2<std::tuple<Ls...>, std::tuple<R, Rs...>>
    : public TaggedIndexHelper2<std::tuple<Ls..., R>, std::tuple<Rs...>> {
};

template <typename... TagsAndInt>
class TaggedIndex
    : public TaggedIndexSelectorH<
          std::conditional_t<
              std::is_integral_v<typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType>, First,
              Second>,
          typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType,
          typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::TagsInTuple> {
    using Base = TaggedIndexSelectorH<
        std::conditional_t<
            std::is_integral_v<typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType>, First,
            Second>,
        typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType,
        typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::TagsInTuple>;

public:
    TaggedIndex() = default;
    template <typename... Args>
    TaggedIndex(Args &&... args) : Base {std::forward<Args>(args)...}
    {
    }

    ~TaggedIndex() = default;
};

}  // namespace panda::verifier

namespace std {

template <typename... TagsAndInt>
struct hash<panda::verifier::TaggedIndex<TagsAndInt...>> {
    size_t operator()(const panda::verifier::TaggedIndex<TagsAndInt...> &i) const noexcept
    {
        return panda::verifier::StdHash(i.Value_);
    }
};

}  // namespace std

#endif  // !PANDA_VERIFIER_UTIL_TAGGED_INDEX_HPP_
