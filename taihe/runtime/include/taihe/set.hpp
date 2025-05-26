/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef TAIHE_SET_HPP
#define TAIHE_SET_HPP

#include <taihe/common.hpp>

#include <utility>

#define SET_GROWTH_FACTOR 2

// This file is used as a standard library and needs to be easy to use.
// The rule that single-parameter constructors need to be explicit does not apply.
// NOLINTBEGIN
namespace taihe {
template <typename K>
struct set_view;

template <typename K>
struct set;

template <typename K>
struct set_view {
    void reserve(std::size_t cap) const
    {
        if (cap == 0) {
            return;
        }
        item_t **bucket = reinterpret_cast<item_t **>(calloc(cap, sizeof(item_t *)));
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            item_t *current = m_handle->bucket[i];
            while (current) {
                item_t *next = current->next;
                std::size_t index = taihe::hash(current->key) % cap;
                current->next = bucket[index];
                bucket[index] = current;
                current = next;
            }
        }
        free(m_handle->bucket);
        m_handle->cap = cap;
        m_handle->bucket = bucket;
    }

    std::size_t size() const noexcept
    {
        return m_handle->size;
    }

    std::size_t capacity() const noexcept
    {
        return m_handle->cap;
    }

    void clear() const
    {
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            while (m_handle->bucket[i]) {
                item_t *next = m_handle->bucket[i]->next;
                delete m_handle->bucket[i];
                m_handle->bucket[i] = next;
            }
        }
        m_handle->size = 0;
    }

    bool emplace(as_param_t<K> key) const
    {
        std::size_t index = taihe::hash(key) % m_handle->cap;
        item_t *current = m_handle->bucket[index];
        while (current) {
            if (taihe::same(current->key, key)) {
                return false;
            }
            current = current->next;
        }
        item_t *item = new item_t {
            .key = key,
            .next = m_handle->bucket[index],
        };
        m_handle->bucket[index] = item;
        m_handle->size++;
        std::size_t required_cap = m_handle->size;
        if (required_cap >= m_handle->cap) {
            reserve(required_cap * SET_GROWTH_FACTOR);
        }
        return true;
    }

    bool find(as_param_t<K> key) const
    {
        std::size_t index = taihe::hash(key) % m_handle->cap;
        item_t *current = m_handle->bucket[index];
        while (current) {
            if (taihe::same(current->key, key)) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool erase(as_param_t<K> key) const
    {
        std::size_t index = taihe::hash(key) % m_handle->cap;
        item_t **current_ptr = &m_handle->bucket[index];
        while (*current_ptr) {
            if (taihe::same((*current_ptr)->key, key)) {
                item_t *current = *current_ptr;
                *current_ptr = (*current_ptr)->next;
                delete current;
                m_handle->size--;
                return true;
            } else {
                current_ptr = &(*current_ptr)->next;
            }
        }
        return false;
    }

    template <typename Visitor>
    void accept(Visitor &&visitor)
    {
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            item_t *current = m_handle->bucket[i];
            while (current) {
                visitor(current->key);
                current = current->next;
            }
        }
    }

    template <typename Visitor>
    void accept(Visitor &&visitor) const
    {
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            item_t *current = m_handle->bucket[i];
            while (current) {
                visitor(current->key);
                current = current->next;
            }
        }
    }

    struct item_t {
        K key;
        item_t *next;
    };

    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = K const &;
        using difference_type = std::ptrdiff_t;
        using pointer = K const *;
        using reference = K const &;

        iterator(item_t **bucket, item_t *current, std::size_t index, std::size_t cap)
            : bucket(bucket), current(current), index(index), cap(cap)
        {
        }

        value_type operator*() const
        {
            return current->key;
        }

        iterator &operator++()
        {
            if (current->next) {
                current = current->next;
            } else {
                ++index;
                while (index < cap && !bucket[index]) {
                    ++index;
                }
                current = (index < cap) ? bucket[index] : nullptr;
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(iterator const &other) const
        {
            return current == other.current;
        }

        bool operator!=(iterator const &other) const
        {
            return !(*this == other);
        }

    private:
        item_t **bucket;
        item_t *current;
        std::size_t index;
        std::size_t cap;
    };

    iterator begin() const
    {
        std::size_t index = 0;
        while (index < m_handle->cap && !m_handle->bucket[index]) {
            ++index;
        }
        return iterator(m_handle->bucket, (index < m_handle->cap) ? m_handle->bucket[index] : nullptr, index,
                        m_handle->cap);
    }

    iterator end() const
    {
        return iterator(m_handle->bucket, nullptr, m_handle->cap, m_handle->cap);
    }

    iterator cbegin() const
    {
        return begin();
    }

    iterator cend() const
    {
        return end();
    }

private:
    struct data_t {
        TRefCount count;
        std::size_t cap;
        item_t **bucket;
        std::size_t size;
    } *m_handle;

    explicit set_view(data_t *handle) : m_handle(handle) {}

    friend struct set<K>;

    friend bool taihe::same_impl(adl_helper_t, set_view lhs, set_view rhs);
    friend std::size_t taihe::hash_impl(adl_helper_t, set_view val);
};

template <typename K>
struct set : set_view<K> {
    using typename set_view<K>::item_t;
    using typename set_view<K>::data_t;
    using set_view<K>::m_handle;

    explicit set(std::size_t cap = 16) : set(reinterpret_cast<data_t *>(calloc(1, sizeof(data_t))))
    {
        item_t **bucket = reinterpret_cast<item_t **>(calloc(cap, sizeof(item_t *)));
        tref_set(&m_handle->count, 1);
        m_handle->cap = cap;
        m_handle->bucket = bucket;
        m_handle->size = 0;
    }

    set(set<K> &&other) noexcept : set(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    set(set<K> const &other) : set(other.m_handle)
    {
        if (m_handle) {
            tref_inc(&m_handle->count);
        }
    }

    set(set_view<K> const &other) : set(other.m_handle)
    {
        if (m_handle) {
            tref_inc(&m_handle->count);
        }
    }

    set &operator=(set other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    ~set()
    {
        if (m_handle && tref_dec(&m_handle->count)) {
            this->clear();
            free(m_handle->bucket);
            free(m_handle);
        }
    }

private:
    explicit set(data_t *handle) : set_view<K>(handle) {}
};

template <typename K>
inline bool same_impl(adl_helper_t, set_view<K> lhs, set_view<K> rhs)
{
    return lhs.m_handle == rhs.m_handle;
}

template <typename K>
inline std::size_t hash_impl(adl_helper_t, set_view<K> val)
{
    return reinterpret_cast<std::size_t>(val.m_handle);
}

template <typename K>
struct as_abi<set<K>> {
    using type = void *;
};

template <typename K>
struct as_abi<set_view<K>> {
    using type = void *;
};

template <typename K>
struct as_param<set<K>> {
    using type = set_view<K>;
};
}  // namespace taihe
// NOLINTEND

#ifdef SET_GROWTH_FACTOR
#undef SET_GROWTH_FACTOR
#endif // SET_GROWTH_FACTOR

#endif // TAIHE_SET_HPP
