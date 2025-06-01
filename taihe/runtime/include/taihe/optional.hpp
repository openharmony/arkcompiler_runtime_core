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
#ifndef TAIHE_OPTIONAL_HPP
#define TAIHE_OPTIONAL_HPP

#include <taihe/optional.abi.h>
#include <taihe/common.hpp>

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

// This file is used as a standard library and needs to be easy to use.
// The rule that single-parameter constructors need to be explicit does not apply.
// NOLINTBEGIN
namespace taihe {
template <typename cpp_owner_t>
struct optional_view;

template <typename cpp_owner_t>
struct optional;

template <typename cpp_owner_t>
struct optional_view {
    optional_view(cpp_owner_t *handle) noexcept : m_handle(handle) {}  // main constructor

    optional_view() noexcept : optional_view(nullptr) {}

    optional_view(std::nullopt_t) : optional_view(nullptr) {}

    explicit operator bool() const
    {
        return m_handle;
    }

    bool has_value() const
    {
        return m_handle;
    }

    cpp_owner_t const *operator->() const
    {
        return m_handle;
    }

    cpp_owner_t const &operator*() const
    {
        return *m_handle;
    }

    cpp_owner_t const &value() const
    {
        return *m_handle;
    }

    cpp_owner_t value_or(cpp_owner_t &&default_value) const
    {
        if (m_handle) {
            return *m_handle;
        } else {
            return std::move(default_value);
        }
    }

protected:
    cpp_owner_t *m_handle;
};

template <typename cpp_owner_t>
struct optional : public optional_view<cpp_owner_t> {
    explicit optional(cpp_owner_t *handle) noexcept : optional_view<cpp_owner_t>(handle) {}  // main constructor

    optional() noexcept : optional(nullptr) {}

    optional(std::nullopt_t) : optional(nullptr) {}

    template <typename... Args>
    optional(std::in_place_t, Args &&...args) : optional(new cpp_owner_t(std::forward<Args>(args)...))
    {
    }

    template <typename... Args>
    static optional make(Args &&...args)
    {
        return optional(std::in_place_t {}, std::forward<Args>(args)...);
    }

    optional(optional_view<cpp_owner_t> const &other) : optional(new cpp_owner_t(*other)) {}

    optional(optional<cpp_owner_t> const &other) : optional(new cpp_owner_t(*other)) {}

    optional(optional<cpp_owner_t> &&other) : optional(std::exchange(other.m_handle, nullptr)) {}

    optional &operator=(optional other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    ~optional()
    {
        if (this->m_handle) {
            delete this->m_handle;
        }
    }
};

template <typename cpp_owner_t>
inline std::size_t hash_impl(adl_helper_t, optional_view<cpp_owner_t> val)
{
    return val ? hash(*val) + 0x9e3779b9 : 0;
}

template <typename cpp_owner_t>
inline bool same_impl(adl_helper_t, optional_view<cpp_owner_t> lhs, optional_view<cpp_owner_t> rhs)
{
    return !lhs && !rhs || same(*lhs, *rhs);
}

template <typename cpp_owner_t>
struct as_abi<optional_view<cpp_owner_t>> {
    using type = struct TOptional;
};

template <typename cpp_owner_t>
struct as_abi<optional<cpp_owner_t>> {
    using type = struct TOptional;
};

template <typename cpp_owner_t>
struct as_param<optional<cpp_owner_t>> {
    using type = optional_view<cpp_owner_t>;
};
}  // namespace taihe
// NOLINTEND

#endif // TAIHE_OPTIONAL_HPP
