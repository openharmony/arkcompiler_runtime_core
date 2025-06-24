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
#ifndef RUNTIME_INCLUDE_TAIHE_MAP_HPP_
#define RUNTIME_INCLUDE_TAIHE_MAP_HPP_
// NOLINTBEGIN

#include <taihe/common.hpp>

#include <utility>

#define MAP_GROWTH_FACTOR 2

namespace taihe {
template <typename K, typename V>
struct map_view;

template <typename K, typename V>
struct map;

template <typename K, typename V>
struct map_view {
public:
    using item_t = std::pair<K const, V>;

    void reserve(std::size_t cap) const
    {
        if (cap == 0) {
            return;
        }
        node_t **bucket = reinterpret_cast<node_t **>(calloc(cap, sizeof(node_t *)));
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            node_t *current = m_handle->bucket[i];
            while (current) {
                node_t *next = current->next;
                std::size_t index = std::hash<K>()(current->item.first) % cap;
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

    bool empty() const noexcept
    {
        return m_handle->size == 0;
    }

    std::size_t capacity() const noexcept
    {
        return m_handle->cap;
    }

    void clear() const
    {
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            while (m_handle->bucket[i]) {
                node_t *next = m_handle->bucket[i]->next;
                delete m_handle->bucket[i];
                m_handle->bucket[i] = next;
            }
        }
        m_handle->size = 0;
    }

    template <bool cover = false, typename... Args>
    std::pair<item_t *, bool> emplace(as_param_t<K> key, Args &&...args) const
    {
        std::size_t index = std::hash<K>()(key) % m_handle->cap;
        node_t **current_ptr = &m_handle->bucket[index];
        while (*current_ptr) {
            if ((*current_ptr)->item.first == key) {
                if (cover) {
                    node_t *replaced = new node_t {
                        .item = {key, V {std::forward<Args>(args)...}},
                        .next = (*current_ptr)->next,
                    };
                    node_t *current = *current_ptr;
                    *current_ptr = replaced;
                    delete current;
                }
                return {&(*current_ptr)->item, false};
            }
            current_ptr = &(*current_ptr)->next;
        }
        node_t *node = new node_t {
            .item = {key, V {std::forward<Args>(args)...}},
            .next = m_handle->bucket[index],
        };
        m_handle->bucket[index] = node;
        m_handle->size++;
        std::size_t required_cap = m_handle->size;
        if (required_cap >= m_handle->cap) {
            reserve(required_cap * MAP_GROWTH_FACTOR);
        }
        return {&node->item, true};
    }

    item_t *find_item(as_param_t<K> key) const
    {
        std::size_t index = std::hash<K>()(key) % m_handle->cap;
        node_t *current = m_handle->bucket[index];
        while (current) {
            if (current->item.first == key) {
                return &current->item;
            }
            current = current->next;
        }
        return nullptr;
    }

    // TODO: Change the return type to item_t *
    V *find(as_param_t<K> key) const
    {
        item_t *item = find_item(key);
        if (item) {
            return &item->second;
        }
        return nullptr;
    }

    bool erase(as_param_t<K> key) const
    {
        std::size_t index = std::hash<K>()(key) % m_handle->cap;
        node_t **current_ptr = &m_handle->bucket[index];
        while (*current_ptr) {
            if ((*current_ptr)->item.first == key) {
                node_t *current = *current_ptr;
                *current_ptr = (*current_ptr)->next;
                delete current;
                m_handle->size--;
                return true;
            }
            current_ptr = &(*current_ptr)->next;
        }
        return false;
    }

    struct node_t {
        item_t item;
        node_t *next;
    };

    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = item_t;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        iterator(node_t **bucket, node_t *current, std::size_t index, std::size_t cap)
            : bucket(bucket), current(current), index(index), cap(cap)
        {
        }

        reference operator*() const
        {
            return current->item;
        }

        pointer operator->() const
        {
            return &current->item;
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
            iterator ocp = *this;
            ++(*this);
            return ocp;
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
        node_t **bucket;
        node_t *current;
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

    using const_iterator = iterator;

    const_iterator cbegin() const
    {
        return begin();
    }

    const_iterator cend() const
    {
        return end();
    }

    template <typename Visitor>
    void accept(Visitor &&visitor) const
    {
        for (std::size_t i = 0; i < m_handle->cap; i++) {
            node_t *current = m_handle->bucket[i];
            while (current) {
                visitor(current->item);
                current = current->next;
            }
        }
    }

private:
    struct handle_t {
        TRefCount count;
        std::size_t cap;
        node_t **bucket;
        std::size_t size;
    } *m_handle;

    explicit map_view(handle_t *handle) : m_handle(handle) {}

    friend struct map<K, V>;

    friend struct std::hash<map<K, V>>;

    friend bool operator==(map_view lhs, map_view rhs)
    {
        return lhs.m_handle == rhs.m_handle;
    }
};

template <typename K, typename V>
struct map : map_view<K, V> {
    using typename map_view<K, V>::node_t;
    using typename map_view<K, V>::handle_t;
    using map_view<K, V>::m_handle;

    explicit map(std::size_t cap = 16) : map(reinterpret_cast<handle_t *>(calloc(1, sizeof(handle_t))))
    {
        node_t **bucket = reinterpret_cast<node_t **>(calloc(cap, sizeof(node_t *)));
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
    explicit map(handle_t *handle) : map_view<K, V>(handle) {}
};

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

template <typename K, typename V>
struct std::hash<taihe::map<K, V>> {
    std::size_t operator()(taihe::map_view<K, V> val) const noexcept
    {
        return reinterpret_cast<std::size_t>(val.m_handle);
    }
};

#ifdef MAP_GROWTH_FACTOR
#undef MAP_GROWTH_FACTOR
#endif
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_MAP_HPP_