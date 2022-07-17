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

#ifndef _PANDA_TYPE_SET_HPP__
#define _PANDA_TYPE_SET_HPP__

#include "type_index.h"
#include "type_type.h"

#include "runtime/include/mem/panda_string.h"

namespace panda::verifier {
class TypeSystems;

class TypeSet {
public:
    TypeSet() = delete;

    template <typename... Types>
    explicit TypeSet(const Type &t, Types... types)
        : kind_ {t.GetTypeSystemKind()}, threadnum_ {t.GetThreadNum()}, Numbers_ {}
    {
        if (sizeof...(types) == 0) {
            Numbers_.Insert(t.Number());
        } else {
            std::array numbers_arr {t.Number(), types.Number()...};
            Numbers_.Insert(numbers_arr.begin(), numbers_arr.end());
        }
    }

    explicit TypeSet(TypeSystemKind kind, ThreadNum threadnum, IntSet<TypeNum> &&numbers = {})
        : kind_ {kind}, threadnum_ {threadnum}, Numbers_ {numbers} {};

    ~TypeSet() = default;

    void Insert(const Type &t)
    {
        ASSERT(t.GetTypeSystemKind() == kind_);
        ASSERT(t.GetThreadNum() == threadnum_);
        Numbers_.Insert(t.Number());
    }

    TypeSet &operator|(const Type &t)
    {
        Insert(t);
        return *this;
    }

    bool Contains(const Type &t) const
    {
        return t.GetTypeSystemKind() == kind_ && t.GetThreadNum() == threadnum_ && Numbers_.Contains(t.Number());
    }

    const Type &operator<<(const Type &st) const;

    const TypeSet &operator<<(const TypeSet &st) const;

    TypeSet operator&(const Type &rhs) const;

    TypeSet operator&(const TypeSet &rhs) const;

    size_t Size() const
    {
        return Numbers_.Size();
    }

    bool IsEmpty() const
    {
        return Size() == 0;
    }

    Type TheOnlyType() const
    {
        Index<TypeNum> the_only_number = Numbers_.TheOnlyElement();
        if (the_only_number.IsValid()) {
            return {kind_, threadnum_, *the_only_number};
        }
        return {};
    }

    template <typename Handler>
    bool ForAll(Handler &&handler) const
    {
        return Numbers_.ForAll([&](TypeNum num) { return handler(Type(kind_, threadnum_, num)); });
    }

    template <typename Handler>
    bool Exists(Handler &&handler) const
    {
        return !ForAll([handler {std::move(handler)}](Type t) { return !handler(t); });
    }

    template <typename TypeImageFunc>
    PandaString Image(TypeImageFunc type_img_func) const
    {
        PandaString result {"TypeSet{"};
        bool first = true;
        ForAll([&](const Type &type) {
            if (first) {
                first = false;
            } else {
                result += ", ";
            }
            result += type_img_func(type);
            return true;
        });
        result += "}";
        return result;
    }

    bool operator==(const TypeSet &rhs) const
    {
        return kind_ == rhs.kind_ && threadnum_ == rhs.threadnum_ && Numbers_ == rhs.Numbers_;
    }

    bool operator!=(const TypeSet &rhs) const
    {
        return !(*this == rhs);
    }

private:
    TypeSystemKind kind_;
    ThreadNum threadnum_;
    IntSet<TypeNum> Numbers_;
};
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_SET_HPP__
