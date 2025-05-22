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
#ifndef TAIHE_OBJECT_HPP
#define TAIHE_OBJECT_HPP

#include <taihe/object.abi.h>
#include <taihe/common.hpp>

#include <array>
#include <exception>
#include <stdexcept>
#include <type_traits>

// This file is used as a standard library and needs to be easy to use.
// The rule that single-parameter constructors need to be explicit does not apply.
// NOLINTBEGIN

//////////////////////
// Raw Data Handler //
//////////////////////

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

///////////////////////////////////////
// Specific Impl Type Object Handler //
///////////////////////////////////////

namespace taihe {
template <typename Impl>
struct data_block_impl : DataBlockHead, Impl {
    template <typename... Args>
    data_block_impl(TypeInfo const *rtti_ptr, Args &&...args) : Impl(std::forward<Args>(args)...)
    {
        tobj_init(this, rtti_ptr);
    }
};

template <typename Impl, typename... Args>
inline DataBlockHead *new_data_ptr(TypeInfo const *rtti_ptr, Args &&...args)
{
    return new data_block_impl<Impl>(rtti_ptr, std::forward<Args>(args)...);
}

template <typename Impl>
inline void del_data_ptr(struct DataBlockHead *data_ptr)
{
    delete static_cast<data_block_impl<Impl> *>(data_ptr);
}

template <typename Impl>
inline Impl *cast_data_ptr(struct DataBlockHead *data_ptr)
{
    return static_cast<Impl *>(static_cast<data_block_impl<Impl> *>(data_ptr));
}

template <typename Impl, typename... InterfaceHolders>
struct impl_view;

template <typename Impl, typename... InterfaceHolders>
struct impl_holder;

template <typename Impl, typename... InterfaceHolders>
struct impl_view {
    DataBlockHead *data_ptr;

    explicit impl_view(DataBlockHead *other_handle) : data_ptr(other_handle) {}

    template <typename InterfaceView, std::enable_if_t<!InterfaceView::is_holder, int> = 0>
    operator InterfaceView() const &
    {
        DataBlockHead *ret_handle = this->data_ptr;
        return InterfaceView {{
            reinterpret_cast<typename InterfaceView::vtable_t const *>(this->template vtbl_ptr<InterfaceView::iid>),
            ret_handle,
        }};
    }

    template <typename InterfaceHolder, std::enable_if_t<InterfaceHolder::is_holder, int> = 0>
    operator InterfaceHolder() const &
    {
        DataBlockHead *ret_handle = tobj_dup(this->data_ptr);
        return InterfaceHolder {{
            reinterpret_cast<typename InterfaceHolder::vtable_t const *>(this->template vtbl_ptr<InterfaceHolder::iid>),
            ret_handle,
        }};
    }

    operator data_view() const &
    {
        DataBlockHead *ret_handle = this->data_ptr;
        return data_view {ret_handle};
    }

    operator data_holder() const &
    {
        DataBlockHead *ret_handle = tobj_dup(this->data_ptr);
        return data_holder {ret_handle};
    }

    Impl *operator->() const
    {
        return cast_data_ptr<Impl>(this->data_ptr);
    }

    Impl &operator*() const
    {
        return *cast_data_ptr<Impl>(this->data_ptr);
    }

public:
    static constexpr struct typeinfo_t {
        uint64_t version;
        void (*free_ptr)(struct DataBlockHead *);
        uint64_t len = 0;
        struct IdMapItem idmap[((sizeof(InterfaceHolders::template idmap_impl<Impl>) / sizeof(IdMapItem)) + ... + 1)] =
            {};
    } rtti = [] {
        struct typeinfo_t info = {0, &del_data_ptr<Impl>};
        (
            [&] {
                for (std::size_t j = 0; j < sizeof(InterfaceHolders::template idmap_impl<Impl>) / sizeof(IdMapItem);
                     info.len++, j++) {
                    info.idmap[info.len] = InterfaceHolders::template idmap_impl<Impl>[j];
                }
            }(),
            ...);
        return info;
    }();

    template <void const *InterfaceID>
    static constexpr void const *vtbl_ptr = [] {
        for (uint64_t i = 0; i < rtti.len; i++) {
            if (InterfaceID == rtti.idmap[i].id) {
                return rtti.idmap[i].vtbl_ptr;
            }
        }
        TH_THROW(std::runtime_error, "Interface ID not found");
    }();
};

template <typename Impl, typename... InterfaceHolders>
struct impl_holder : public impl_view<Impl, InterfaceHolders...> {
    using impl_view<Impl, InterfaceHolders...>::rtti;
    using impl_view<Impl, InterfaceHolders...>::vtbl_ptr;

    explicit impl_holder(DataBlockHead *other_handle) : impl_view<Impl, InterfaceHolders...>(other_handle) {}

    template <typename... Args>
    static impl_holder make(Args &&...args)
    {
        return impl_holder(new_data_ptr<Impl>(reinterpret_cast<TypeInfo const *>(&rtti), std::forward<Args>(args)...));
    }

    impl_holder &operator=(impl_holder other)
    {
        std::swap(this->data_ptr, other.data_ptr);
        return *this;
    }

    ~impl_holder()
    {
        tobj_drop(this->data_ptr);
    }

    impl_holder(impl_view<Impl, InterfaceHolders...> const &other) : impl_holder(tobj_dup(other.data_ptr)) {}

    impl_holder(impl_holder<Impl, InterfaceHolders...> const &other) : impl_holder(tobj_dup(other.data_ptr)) {}

    impl_holder(impl_holder<Impl, InterfaceHolders...> &&other) : impl_holder(other.data_ptr)
    {
        other.data_ptr = nullptr;
    }

    template <typename InterfaceView, std::enable_if_t<!InterfaceView::is_holder, int> = 0>
    operator InterfaceView() const &
    {
        DataBlockHead *ret_handle = this->data_ptr;
        return InterfaceView {{
            reinterpret_cast<typename InterfaceView::vtable_t const *>(this->template vtbl_ptr<InterfaceView::iid>),
            ret_handle,
        }};
    }

    template <typename InterfaceHolder, std::enable_if_t<InterfaceHolder::is_holder, int> = 0>
    operator InterfaceHolder() const &
    {
        DataBlockHead *ret_handle = tobj_dup(this->data_ptr);
        return InterfaceHolder {{
            reinterpret_cast<typename InterfaceHolder::vtable_t const *>(this->template vtbl_ptr<InterfaceHolder::iid>),
            ret_handle,
        }};
    }

    template <typename InterfaceHolder, std::enable_if_t<InterfaceHolder::is_holder, int> = 0>
    operator InterfaceHolder() &&
    {
        DataBlockHead *ret_handle = this->data_ptr;
        this->data_ptr = nullptr;
        return InterfaceHolder {{
            reinterpret_cast<typename InterfaceHolder::vtable_t const *>(this->template vtbl_ptr<InterfaceHolder::iid>),
            ret_handle,
        }};
    }

    operator data_view() const &
    {
        DataBlockHead *ret_handle = this->data_ptr;
        return data_view {ret_handle};
    }

    operator data_holder() const &
    {
        DataBlockHead *ret_handle = tobj_dup(this->data_ptr);
        return data_holder {ret_handle};
    }

    operator data_holder() &&
    {
        DataBlockHead *ret_handle = this->data_ptr;
        this->data_ptr = nullptr;
        return data_holder {ret_handle};
    }
};

template <typename Impl, typename... InterfaceHolders, typename... Args>
inline auto make_holder(Args &&...args)
{
    return impl_holder<Impl, InterfaceHolders...>::make(std::forward<Args>(args)...);
}

template <typename... InterfaceHolders, typename Impl>
inline auto into_holder(Impl &&impl)
{
    return impl_holder<Impl, InterfaceHolders...>::make(std::forward<Impl>(impl));
}
}  // namespace taihe
// NOLINTEND

#endif // TAIHE_OBJECT_HPP
