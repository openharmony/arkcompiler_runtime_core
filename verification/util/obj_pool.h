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

#ifndef PANDA_VERIFIER_UTIL_OBJ_POOL_HPP_
#define PANDA_VERIFIER_UTIL_OBJ_POOL_HPP_

#include "macros.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace panda::verifier {

template <typename T, template <typename...> class Vector, typename InitializerType = void (*)(T &, std::size_t),
          typename CleanerType = void (*)(T &)>
class ObjPool {
public:
    class Accessor {
    public:
        Accessor() : idx {0}, pool {nullptr}, prev {nullptr}, next {nullptr} {}
        Accessor(std::size_t index, ObjPool *obj_pool) : idx {index}, pool {obj_pool}, prev {nullptr}, next {nullptr}
        {
            Insert();
        }
        Accessor(const Accessor &p) : idx {p.idx}, pool {p.pool}, prev {nullptr}, next {nullptr}
        {
            Insert();
        }
        Accessor(Accessor &&p) : idx {p.idx}, pool {p.pool}, prev {p.prev}, next {p.next}
        {
            p.Reset();
            Rebind();
        }
        Accessor &operator=(const Accessor &p)
        {
            Erase();
            Reset();
            idx = p.idx;
            pool = p.pool;
            Insert();
            return *this;
        }
        Accessor &operator=(Accessor &&p)
        {
            Erase();
            idx = p.idx;
            pool = p.pool;
            prev = p.prev;
            next = p.next;
            p.Reset();
            Rebind();
            return *this;
        }
        ~Accessor()
        {
            Erase();
        }
        T &operator*()
        {
            return pool->Storage[idx];
        }
        const T &operator*() const
        {
            return pool->Storage[idx];
        }
        operator bool() const
        {
            return pool != nullptr;
        }
        void Free()
        {
            Erase();
            Reset();
        }

    private:
        void Reset()
        {
            idx = 0;
            pool = nullptr;
            prev = nullptr;
            next = nullptr;
        }
        void Insert()
        {
            if (pool != nullptr) {
                next = pool->Accessors[idx];
                if (next != nullptr)
                    next->prev = this;
                pool->Accessors[idx] = this;
            }
        }
        void Erase()
        {
            if (pool != nullptr) {
                if (prev == nullptr) {
                    pool->Accessors[idx] = next;
                    if (pool->Accessors[idx] == nullptr) {
                        pool->Cleaner(pool->Storage[idx]);
                        pool->Free.push_back(idx);
                    }
                } else
                    prev->next = next;
                if (next != nullptr)
                    next->prev = prev;
            }
        }
        void Rebind()
        {
            if (pool != nullptr) {
                if (prev != nullptr)
                    prev->next = this;
                else
                    pool->Accessors[idx] = this;
                if (next != nullptr)
                    next->prev = this;
            }
        }

        std::size_t idx;
        ObjPool *pool;
        Accessor *prev;
        Accessor *next;

        friend class ObjPool;
    };

    ObjPool(InitializerType initializer, CleanerType cleaner) : Initializer {initializer}, Cleaner {cleaner} {}
    ObjPool(InitializerType initializer) : Initializer {initializer}, Cleaner {[](T &) { return; }} {}
    ObjPool() : Initializer {[](T &, std::size_t) { return; }}, Cleaner {[](T &) { return; }} {}

    ~ObjPool() = default;

    DEFAULT_MOVE_SEMANTIC(ObjPool);
    DEFAULT_COPY_SEMANTIC(ObjPool);

    Accessor New()
    {
        std::size_t idx;
        if (FreeCount() > 0) {
            idx = Free.back();
            Free.pop_back();
        } else {
            idx = Storage.size();
            Storage.emplace_back();
            Accessors.emplace_back(nullptr);
        }
        Initializer(Storage[idx], idx);
        return Accessor {idx, this};
    }

    std::size_t FreeCount() const
    {
        return Free.size();
    }

    std::size_t Count() const
    {
        return Storage.size();
    }

    std::size_t AccCount() const
    {
        size_t count = 0;
        for (auto el : Accessors) {
            Accessor *acc = el;
            while (acc != nullptr) {
                ++count;
                acc = acc->next;
            }
        }
        return count;
    }

    auto AllObjects()
    {
        thread_local size_t idx = 0;
        return [this]() -> std::optional<Accessor> {
            while (idx < Storage.size() && Accessors[idx] == nullptr) {
                ++idx;
            }
            if (idx >= Storage.size()) {
                idx = 0;
                return std::nullopt;
            }
            return {Accessor {idx++, this}};
        };
    }

    void ShrinkToFit()
    {
        size_t idx1 = 0;
        size_t idx2 = Storage.size() - 1;
        while (idx1 < idx2) {
            while ((idx1 < idx2) && (Accessors[idx1] != nullptr))
                ++idx1;
            while ((idx1 < idx2) && (Accessors[idx2] == nullptr))
                --idx2;
            if (idx1 < idx2) {
                Storage[idx1] = std::move(Storage[idx2]);
                Accessors[idx1] = Accessors[idx2];
                Accessor *acc = Accessors[idx1];
                while (acc != nullptr) {
                    acc->idx = idx1;
                    acc = acc->next;
                }
                --idx2;
            }
        }
        if (Accessors[idx1] != nullptr)
            ++idx1;
        Storage.resize(idx1);
        Storage.shrink_to_fit();
        Accessors.resize(idx1);
        Accessors.shrink_to_fit();
        Free.clear();
        Free.shrink_to_fit();
    }

private:
    InitializerType Initializer;
    CleanerType Cleaner;

    Vector<T> Storage;
    Vector<std::size_t> Free;
    Vector<Accessor *> Accessors;
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_UTIL_OBJ_POOL_HPP_
