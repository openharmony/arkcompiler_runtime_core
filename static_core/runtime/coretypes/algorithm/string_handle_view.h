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

#ifndef PANDA_RUNTIME_CORETYPES_ALGORITHM_STRING_HANDLE_VIEW
#define PANDA_RUNTIME_CORETYPES_ALGORITHM_STRING_HANDLE_VIEW

#include "libarkbase/utils/span.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/vm_handle.h"
#include <tuple>

namespace ark::coretypes::algo {

class StringHandleView final {
public:
    explicit StringHandleView(const VMHandle<String> &handle) noexcept : handle_ {handle}, length_ {handle->GetLength()}
    {
    }

    StringHandleView(const VMHandle<String> &handle, uint32_t length, uint32_t startIndex = 0) noexcept
        : handle_ {handle}, length_ {length}, startIndex_ {startIndex}
    {
    }

    template <typename S>
    Span<const S> AsSpan() const = delete;

    bool IsUtf8() const
    {
        return handle_->IsUtf8();
    }

    bool IsUtf16() const
    {
        return handle_->IsUtf16();
    }

    bool IsEmpty() const
    {
        return handle_->IsEmpty();
    }

private:
    VMHandle<String> handle_;
    uint32_t length_ {0};
    uint32_t startIndex_ {0};
};

template <>
inline Span<const uint16_t> StringHandleView::AsSpan() const
{
    ASSERT(handle_->IsUtf16());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return {handle_->GetDataUtf16() + startIndex_, length_};
}

template <>
inline Span<const uint8_t> StringHandleView::AsSpan() const
{
    ASSERT(handle_->IsUtf8());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return {handle_->GetDataUtf8() + startIndex_, length_};
}

template <typename S>
Span<const S> GetSpan(StringHandleView shv)
{
    return shv.AsSpan<S>();
}

template <typename... Ts, typename... S>
std::tuple<Span<const Ts>...> GetSpans(S... str)
{
    return {GetSpan<Ts>(str)...};
}

}  // namespace ark::coretypes::algo

#endif  // PANDA_RUNTIME_CORETYPES_ALGORITHM_STRING_HANDLE_VIEW
