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
#ifndef TAIHE_STRING_HPP
#define TAIHE_STRING_HPP

#include <taihe/string.abi.h>
#include <taihe/common.hpp>

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

// This file is used as a standard library and needs to be easy to use.
// The rule that single-parameter constructors need to be explicit does not apply.
// NOLINTBEGIN
namespace taihe {
struct string_view;
struct string;

struct string_view {
    using value_type = char;
    using size_type = std::size_t;
    using const_reference = value_type const &;
    using const_pointer = value_type const *;
    using const_iterator = const_pointer;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit string_view(struct TString handle) : m_handle(handle) {}

    string_view(char const *value TH_NONNULL) : string_view(tstr_new_ref(value, strlen(value))) {}

    string_view(char const *value TH_NONNULL, size_type size) : string_view(tstr_new_ref(value, size)) {}

    string_view(std::initializer_list<char> value) : string_view(value.begin(), static_cast<uint32_t>(value.size())) {}

    string_view(std::string_view value) : string_view(value.data(), value.size()) {}

    string_view(std::string const &value) : string_view(value.data(), value.size()) {}

    operator std::string_view() const noexcept
    {
        return {tstr_buf(m_handle), tstr_len(m_handle)};
    }

    // methods
    const_reference operator[](size_type pos) const
    {
        if (pos >= size()) {
            TH_THROW(std::out_of_range, "Index out of range");
        }
        return tstr_buf(m_handle)[pos];
    }

    bool empty() const noexcept
    {
        return tstr_len(m_handle) == 0;
    }

    size_type size() const noexcept
    {
        return tstr_len(m_handle);
    }

    const_reference front() const
    {
        if (empty()) {
            TH_THROW(std::out_of_range, "Empty string");
        }
        return tstr_buf(m_handle)[0];
    }

    const_reference back() const
    {
        if (empty()) {
            TH_THROW(std::out_of_range, "Empty string");
        }
        return tstr_buf(m_handle)[size() - 1];
    }

    const_pointer c_str() const noexcept
    {
        return tstr_buf(m_handle);
    }

    const_pointer data() const noexcept
    {
        return tstr_buf(m_handle);
    }

    const_iterator begin() const noexcept
    {
        return tstr_buf(m_handle);
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    const_iterator end() const noexcept
    {
        return tstr_buf(m_handle) + tstr_len(m_handle);
    }

    const_iterator cend() const noexcept
    {
        return end();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    friend struct string;

    friend string concat(string_view left, string_view right);
    friend string_view substr(string_view sv, std::size_t pos, std::size_t len);
    friend string operator+(string_view left, string_view right);
    string_view substr(std::size_t pos, std::size_t len) const;

protected:
    struct TString m_handle;
};

struct string : public string_view {
    explicit string(struct TString handle) : string_view(handle) {}

    string(char const *value TH_NONNULL) : string(tstr_new(value, std::strlen(value))) {}

    string(char const *value TH_NONNULL, size_type size) : string(tstr_new(value, size)) {}

    string(std::initializer_list<char> value) : string(value.begin(), static_cast<uint32_t>(value.size())) {}

    string(std::string_view value) : string(value.data(), value.size()) {}

    string(std::string const &value) : string(value.data(), value.size()) {}

    // constructors
    string(string_view const &other) : string(tstr_dup(other.m_handle)) {}

    string(string const &other) : string(tstr_dup(other.m_handle)) {}

    string(string &&other) noexcept : string(other.m_handle)
    {
        other.m_handle.ptr = NULL;
    }

    // assignment
    string &operator=(string other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    // destructor
    ~string()
    {
        if (m_handle.ptr != NULL) {
            tstr_drop(m_handle);
        }
    }

    string &operator+=(string_view other);
};

inline string concat(string_view left, string_view right)
{
    return string(tstr_concat(left.m_handle, right.m_handle));
}

inline string operator+(string_view left, string_view right)
{
    return string(tstr_concat(left.m_handle, right.m_handle));
}

inline string &string::operator+=(string_view other)
{
    return *this = *this + other;
}

inline string_view substr(string_view sv, std::size_t pos, std::size_t len)
{
    return string_view(tstr_substr(sv.m_handle, pos, len));
}

inline string_view string_view::substr(std::size_t pos, std::size_t len) const
{
    return string_view(tstr_substr(this->m_handle, pos, len));
}

inline bool operator==(string_view lhs, string_view rhs)
{
    return std::string_view(lhs) == std::string_view(rhs);
}

inline bool operator!=(string_view lhs, string_view rhs)
{
    return std::string_view(lhs) != std::string_view(rhs);
}

inline bool operator<(string_view lhs, string_view rhs)
{
    return std::string_view(lhs) < std::string_view(rhs);
}

inline bool operator>(string_view lhs, string_view rhs)
{
    return std::string_view(lhs) > std::string_view(rhs);
}

inline bool operator<=(string_view lhs, string_view rhs)
{
    return std::string_view(lhs) <= std::string_view(rhs);
}

inline bool operator>=(string_view lhs, string_view rhs)
{
    return std::string_view(lhs) >= std::string_view(rhs);
}

inline std::ostream &operator<<(std::ostream &os, string_view sv)
{
    return os << std::string_view(sv);
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline string to_string(T value)
{
    char buffer[32];
    std::to_chars_result result = std::to_chars(std::begin(buffer), std::end(buffer), value);
    if (result.ec != std::errc {}) {
        TH_THROW(std::runtime_error, "Conversion to char failed");
    }
    // buffer automatcally
    return string {buffer, static_cast<std::size_t>(result.ptr - buffer)};
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
inline string to_string(T value)
{
    char buffer[32];
    std::to_chars_result result = std::to_chars(std::begin(buffer),
        std::end(buffer), value, std::chars_format::general);
    if (result.ec != std::errc {}) {
        TH_THROW(std::runtime_error, "Conversion to char failed");
    }
    // buffer automatcally
    return string {buffer, static_cast<std::size_t>(result.ptr - buffer)};
}

template <typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
string to_string(T value)
{
    if (value) {
        return string {"true", 4};
    } else {
        return string {"false", 5};
    }
}

inline std::size_t hash_impl(adl_helper_t, string_view val)
{
    return std::hash<std::string_view> {}(val);
}

inline bool same_impl(adl_helper_t, string_view lhs, string_view rhs)
{
    return lhs == rhs;
}

template <>
struct as_abi<string_view> {
    using type = TString;
};

template <>
struct as_abi<string> {
    using type = TString;
};

template <>
struct as_param<string> {
    using type = string_view;
};
}  // namespace taihe
// NOLINTEND

#endif // TAIHE_STRING_HPP
