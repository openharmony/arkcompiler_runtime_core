/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_CORETYPES_ALGORITHM_SAFEPOINT_PROVIDER
#define PANDA_RUNTIME_CORETYPES_ALGORITHM_SAFEPOINT_PROVIDER

#include "runtime/coretypes/algorithm/string_handle_view.h"
#include "runtime/include/managed_thread.h"
#include <type_traits>
#include <utility>

namespace ark::coretypes::algo {
namespace detail {
template <size_t>
class StringHandleViewHolder {
public:
    // NOLINTNEXTLINE(*-explicit-constructor)
    StringHandleViewHolder(StringHandleView str) : str_(str) {}

    StringHandleView Str() const
    {
        return str_;
    }

private:
    StringHandleView str_;
};

template <typename>
class SafepointProviderImpl;

template <size_t... I>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SafepointProviderImpl<std::index_sequence<I...>> : public StringHandleViewHolder<I>... {
public:
    template <typename... Ts>
    explicit SafepointProviderImpl(ManagedThread *mThread, Ts... str)
        : StringHandleViewHolder<I>(str)..., mThread_(mThread)
    {
        static_assert(
            std::conjunction_v<std::is_same<std::remove_cv_t<std::remove_reference_t<Ts>>, StringHandleView>...>);
    }

    template <typename... Ts>
    void PutSafepoint(Span<const Ts> &...spans) const
    {
        static_assert(sizeof...(Ts) == sizeof...(I), "Every string must have a type for the corresponding span");
        PutSafepointImpl(spans...);
    }

private:
    template <typename... Ts>
    void PutSafepointImpl(Span<const Ts> &...spans) const
    {
        mThread_->Safepoint();
        // If GC suspends worker during an intrinsic execution, it may move the strings pointed by Hs to different
        // memory locations; therefore, the new string addresses should be re-read.
        std::tie(spans...) = GetSpans<Ts...>(GetString<I>()...);
    }

    template <size_t IDX>
    StringHandleView GetString() const
    {
        return static_cast<const StringHandleViewHolder<IDX> *>(this)->Str();
    }

    ManagedThread *mThread_;
};
}  // namespace detail

template <size_t N>
class SafepointProvider final : public detail::SafepointProviderImpl<std::make_index_sequence<N>> {
public:
    using Base = detail::SafepointProviderImpl<std::make_index_sequence<N>>;

    template <typename... Ts>
    explicit SafepointProvider(ManagedThread *mThread, Ts... args) : Base(mThread, std::move(args)...)
    {
    }
};

template <typename... Ts>
SafepointProvider(ManagedThread *, Ts...) -> SafepointProvider<sizeof...(Ts)>;

using SoloSafepointProvider = SafepointProvider<1U>;
using DuoSafepointProvider = SafepointProvider<2U>;

}  // namespace ark::coretypes::algo

#endif  // PANDA_RUNTIME_CORETYPES_ALGORITHM_SAFEPOINT_PROVIDER
