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
#ifndef RUNTIME_INCLUDE_TAIHE_OBJECT_HPP_
#define RUNTIME_INCLUDE_TAIHE_OBJECT_HPP_
// NOLINTBEGIN

#include <taihe/object.abi.h>
#include <taihe/common.hpp>

// Raw Data Handler //

namespace taihe {
struct data_view;
struct data_holder;

struct data_view {
    DataBlockHead *data_ptr;

    explicit data_view(DataBlockHead *other_handle) : data_ptr(other_handle) {}
};

struct data_holder : public data_view {
    explicit data_holder(DataBlockHead *other_handle) : data_view(other_handle) {}

    data_holder &operator=(data_holder other)
    {
        std::swap(this->data_ptr, other.data_ptr);
        return *this;
    }

    ~data_holder()
    {
        tobj_drop(this->data_ptr);
    }

    data_holder(data_view const &other) : data_holder(tobj_dup(other.data_ptr)) {}

    data_holder(data_holder const &other) : data_holder(tobj_dup(other.data_ptr)) {}

    data_holder(data_holder &&other) : data_holder(other.data_ptr)
    {
        other.data_ptr = nullptr;
    }
};

inline bool same_impl(adl_helper_t, data_view lhs, data_view rhs)
{
    return lhs.data_ptr == rhs.data_ptr;
}

inline std::size_t hash_impl(adl_helper_t, data_view val)
{
    return reinterpret_cast<std::size_t>(val.data_ptr);
}
}  // namespace taihe

// Specific Impl Type Object Handler //

namespace taihe {
template <typename Impl>
struct data_block_full : DataBlockHead {
    Impl impl;

    template <typename... Args>
    data_block_full(TypeInfo const *rtti_ptr, Args &&...args) : impl(std::forward<Args>(args)...)
    {
        tobj_init(this, rtti_ptr);
    }
};

template <typename Impl>
inline Impl *cast_data_ptr(struct DataBlockHead *data_ptr)
{
    return &static_cast<data_block_full<Impl> *>(data_ptr)->impl;
}

template <typename Impl, typename... Args>
inline DataBlockHead *new_data_ptr(TypeInfo const *rtti_ptr, Args &&...args)
{
    return new data_block_full<Impl>(rtti_ptr, std::forward<Args>(args)...);
}

template <typename Impl>
inline void del_data_ptr(struct DataBlockHead *data_ptr)
{
    delete static_cast<data_block_full<Impl> *>(data_ptr);
}

template <typename Impl, typename InterfaceHolder, typename... Args>
inline InterfaceHolder make_holder(Args &&...args)
{
    return InterfaceHolder::template from<Impl>(std::forward<Args>(args)...);
}
}  // namespace taihe
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_OBJECT_HPP_