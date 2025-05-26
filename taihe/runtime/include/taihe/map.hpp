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
#ifndef TAIHE_MAP_HPP
#define TAIHE_MAP_HPP

#include <taihe/common.hpp>

#include <utility>

#define MAP_GROWTH_FACTOR 2

// This file is used as a standard library and needs to be easy to use.
// The rule that single-parameter constructors need to be explicit does not apply.
// NOLINTBEGIN
namespace taihe {
template <typename K, typename V>
struct map_view;

template <typename K, typename V>
struct map;

template <typename K, typename V>
struct map_view {
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

    template <bool cover = false, typename... Args>
    V *emplace(as_param_t<K> key, Args &&...args) const
    {
        std::size_t index = taihe::hash(key) % m_handle->cap;
        item_t *current = m_handle->bucket[index];
        while (current) {
            if (taihe::same(current->key, key)) {
                if (cover) {
                    current->val = V {std::forward<Args>(args)...};
                }
                return &current->val;
            }
            current = current->next;
        }
        item_t *item = new item_t {
            .key = key,
            .val = V {std::forward<Args>(args)...},
            .next = m_handle->bucket[index],
        };
        m_handle->bucket[index] = item;
        m_handle->size++;
        std::size_t required_cap = m_handle->size;
        if (required_cap >= m_handle->cap) {
            reserve(required_cap * MAP_GROWTH_FACTOR);
        }
        return &item->val;
    }

    V *find(as_param_t<K> key) const
    {
        std::size_t index = taihe::hash(key) % m_handle->cap;
        item_t *current = m_handle->bucket[index];
        while (current) {
            if (taihe::same(current->key, key)) {
                return &current->val;
            }
            current = current->next;
        }
        return nullptr;
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
                visitor(current->key, current->val);
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
                visitor(current->key, current->val);
                current = current->next;
            }
        }
    }

    struct item_t {
        K key;
        V val;
        item_t *next;
    };

    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<K const &, V &>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        iterator(item_t **bucket, item_t *current, std::size_t index, std::size_t cap)
            : bucket(bucket), current(current), index(index), cap(cap)
        {
        }

        value_type operator*() const
        {
            return {current->key, current->val};
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

    struct const_iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<K const &, V const &>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        const_iterator(item_t **bucket, item_t *current, std::size_t index, std::size_t cap)
            : bucket(bucket), current(current), index(index), cap(cap)
        {
        }

        value_type operator*() const
        {
            return {current->key, current->val};
        }

        const_iterator &operator++()
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

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const_iterator const &other) const
        {
            return current == other.current;
        }

        bool operator!=(const_iterator const &other) const
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

    const_iterator cbegin() const
    {
        std::size_t index = 0;
        while (index < m_handle->cap && !m_handle->bucket[index]) {
            ++index;
        }
        return const_iterator(m_handle->bucket, (index < m_handle->cap) ? m_handle->bucket[index] : nullptr, index,
                              m_handle->cap);
    }

    const_iterator cend() const
    {
        return const_iterator(m_handle->bucket, nullptr, m_handle->cap, m_handle->cap);
    }

private:
    struct data_t {
        TRefCount count;
        std::size_t cap;
        item_t **bucket;
        std::size_t size;
    } *m_handle;

    explicit map_view(data_t *handle) : m_handle(handle) {}

    friend struct map<K, V>;

    friend bool taihe::same_impl(adl_helper_t, map_view lhs, map_view rhs);
    friend std::size_t taihe::hash_impl(adl_helper_t, map_view val);
};

template <typename K, typename V>
struct map : map_view<K, V> {
    using typename map_view<K, V>::item_t;
    using typename map_view<K, V>::data_t;
    using map_view<K, V>::m_handle;

    explicit map(std::size_t cap = 16) : map(reinterpret_cast<data_t *>(calloc(1, sizeof(data_t))))
    {
        item_t **bucket = reinterpret_cast<item_t **>(calloc(cap, sizeof(item_t *)));
        tref_set(&m_handle->count, 1);
        m_handle->cap = cap;
        m_handle->bucket = bucket;
        m_handle->size = 0;
    }

    map(map<K, V> &&other) noexcept : map(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    map(map<K, V> const &other) : map(other.m_handle)
    {
        if (m_handle) {
            tref_inc(&m_handle->count);
        }
    }

    map(map_view<K, V> const &other) : map(other.m_handle)
    {
        if (m_handle) {
            tref_inc(&m_handle->count);
        }
    }

    map &operator=(map other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    ~map()
    {
        if (m_handle && tref_dec(&m_handle->count)) {
            this->clear();
            free(m_handle->bucket);
            free(m_handle);
        }
    }

private:
    explicit map(data_t *handle) : map_view<K, V>(handle) {}
};

template <typename K, typename V>
inline bool same_impl(adl_helper_t, map_view<K, V> lhs, map_view<K, V> rhs)
{
    return lhs.m_handle == rhs.m_handle;
}

template <typename K, typename V>
inline std::size_t hash_impl(adl_helper_t, map_view<K, V> val)
{
    return reinterpret_cast<std::size_t>(val.m_handle);
}

template <typename K, typename V>
struct as_abi<map<K, V>> {
    using type = void *;
};

template <typename K, typename V>
struct as_abi<map_view<K, V>> {
    using type = void *;
};

template <typename K, typename V>
struct as_param<map<K, V>> {
    using type = map_view<K, V>;
};
}  // namespace taihe
// NOLINTEND

#ifdef MAP_GROWTH_FACTOR
#undef MAP_GROWTH_FACTOR
#endif // MAP_GROWTH_FACTOR

#endif // TAIHE_MAP_HPP
