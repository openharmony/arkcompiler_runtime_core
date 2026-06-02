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

#ifndef PANDA_RUNTIME_ON_STACK_STRING_H
#define PANDA_RUNTIME_ON_STACK_STRING_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <string_view>
#include <type_traits>

namespace ark {

template <size_t STRING_LEN, typename CharT = char>
class OnStackString {
public:
    using size_type = size_t;
    using const_pointer = const CharT *;
    using IosBaseManipulator = std::ios_base &(*)(std::ios_base &);

    static_assert(STRING_LEN > 0, "OnStackString size must be greater than zero");

    OnStackString() = default;

    size_type size() const
    {
        return size_;
    }

    const_pointer data() const
    {
        return buffer_.data();
    }

    const_pointer c_str() const
    {
        return data();
    }

    void clear()
    {
        size_ = 0;
        buffer_[0] = CharT {};
    }

    OnStackString &operator<<(std::basic_string_view<CharT> sv)
    {
        AppendStringView(sv);
        return *this;
    }

    OnStackString &operator<<(const_pointer text)
    {
        AppendCString(text);
        return *this;
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>, void>>
    OnStackString &operator<<(T value)
    {
        AppendIntegral(value);
        return *this;
    }

    OnStackString &operator<<(const void *ptr)
    {
        AppendStringView(std::basic_string_view<CharT>(kHexPrefix, kHexPrefixSize));
        AppendUnsigned(static_cast<size_t>(reinterpret_cast<uintptr_t>(ptr)), kHexBase);
        return *this;
    }

    OnStackString &operator<<(IosBaseManipulator manip)
    {
        if (manip == static_cast<IosBaseManipulator>(std::hex)) {
            base_ = kHexBase;
        } else if (manip == static_cast<IosBaseManipulator>(std::dec)) {
            base_ = kDecimalBase;
        }
        return *this;
    }

private:
    static constexpr unsigned kDecimalBase = 10;
    static constexpr unsigned kHexBase = 16;
    static constexpr size_type kHexPrefixSize = 2;
    static constexpr size_type kMaxDecimalDigitsPerByte = 3;
    static constexpr CharT kHexPrefix[] = {'0', 'x'};

    void Append(const_pointer data, size_type count)
    {
        for (size_type i = 0; i < count && size_ < STRING_LEN; i++) {
            buffer_[size_++] = data[i];
        }
        buffer_[size_] = CharT {};
    }

    void AppendCString(const_pointer text)
    {
        if (text == nullptr) {
            return;
        }
        while (*text != CharT {} && size_ < STRING_LEN) {
            buffer_[size_++] = *text++;
        }
        buffer_[size_] = CharT {};
    }

    void AppendStringView(std::basic_string_view<CharT> sv)
    {
        Append(sv.data(), sv.size());
    }

    template <typename T>
    void AppendIntegral(T value)
    {
        if constexpr (std::is_signed_v<T>) {
            using UnsignedT = std::make_unsigned_t<T>;
            if (value < 0) {
                static constexpr CharT minus[1] = {'-'};
                Append(minus, 1);
                AppendUnsigned(static_cast<UnsignedT>(-(value + 1)) + 1, base_);
                return;
            }
        }
        AppendUnsigned(static_cast<std::make_unsigned_t<T>>(value), base_);
    }

    template <typename T>
    void AppendUnsigned(T value, unsigned base)
    {
        if (base == 0) {
            base = kDecimalBase;
        }
        std::array<CharT, sizeof(T) * kMaxDecimalDigitsPerByte + 1> digits {};
        size_type digitCount = 0;
        do {
            auto currentDigit = static_cast<unsigned>(value % base);
            digits[digitCount++] = currentDigit < kDecimalBase
                                       ? static_cast<CharT>('0' + currentDigit)
                                       : static_cast<CharT>('a' + (currentDigit - kDecimalBase));
            value /= base;
        } while (value != 0);

        while (digitCount != 0) {
            Append(&digits[--digitCount], 1);
        }
    }

    std::array<CharT, STRING_LEN + 1> buffer_ {};
    size_type size_ {0};
    unsigned base_ {kDecimalBase};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_ON_STACK_STRING_H
