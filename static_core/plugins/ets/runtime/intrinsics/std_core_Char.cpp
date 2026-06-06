/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <charconv>
#include <algorithm>

#include "common_interfaces/objects/string/line_string-inl.h"
#include "intrinsics.h"
#include "helpers/ets_intrinsics_helpers.h"
#include "types/ets_primitives.h"
#include "runtime/include/coretypes/string_flatten.h"

namespace ark::ets::intrinsics {

using ark::coretypes::FlatStringInfo;

static constexpr uint32_t BITS_IN_HEX_DIGIT = 4;
static constexpr uint32_t HEX_ERROR = 127;  // the code of DEL as a sign of error
static constexpr uint32_t HEX_DIGITS_IN_ETS_CHAR = 4;
static constexpr uint32_t HEX_DIGITS_COUNT = 16;
using HexDigits = std::array<uint8_t, HEX_DIGITS_COUNT>;

const char LOWER_TO_UPPER_OFFSET = 'a' - 'A';

extern "C" EtsBoolean StdCoreCharIsUpperCase(EtsChar value)
{
    return ToEtsBoolean(value >= 'A' && value <= 'Z');
}
extern "C" EtsChar StdCoreCharToUpperCase(EtsChar value)
{
    return (value >= 'a' && value <= 'z') ? value - LOWER_TO_UPPER_OFFSET : value;
}
extern "C" EtsBoolean StdCoreCharIsLowerCase(EtsChar value)
{
    return ToEtsBoolean(value >= 'a' && value <= 'z');
}
extern "C" EtsChar StdCoreCharToLowerCase(EtsChar value)
{
    return (value >= 'A' && value <= 'Z') ? value + LOWER_TO_UPPER_OFFSET : value;
}
extern "C" EtsBoolean StdCoreCharIsWhiteSpace(EtsChar value)
{
    return ToEtsBoolean(utf::IsWhiteSpaceChar(value));
}

EtsByte StdCoreCharToByte(EtsChar val)
{
    return static_cast<int8_t>(val);
}

EtsShort StdCoreCharToShort(EtsChar val)
{
    return static_cast<int16_t>(val);
}

EtsInt StdCoreCharToInt(EtsChar val)
{
    return static_cast<int32_t>(val);
}

EtsLong StdCoreCharToLong(EtsChar val)
{
    return static_cast<int64_t>(val);
}

EtsFloat StdCoreCharToFloat(EtsChar val)
{
    return static_cast<float>(val);
}

EtsDouble StdCoreCharToDouble(EtsChar val)
{
    return static_cast<double>(val);
}

static FlatStringInfo FlattenHexStr(EtsString *hexStr)
{
    auto *hex = hexStr->GetCoreType();
    if (hex->IsTreeString()) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        auto thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<coretypes::String> h(thread, hex);
        // We want FlatStringInfo::FlattenTreeString() to use FlattenedStringCache.
        // The cache is used iff 'withNativeMemory' == false (see FlatStringInfo::FlattenTreeString).
        constexpr bool WITH_NATIVE_MEMORY = false;
        return FlatStringInfo::FlattenTreeString(h, ctx, WITH_NATIVE_MEMORY);
    }
    if (hex->IsSlicedString()) {
        // VMHandle is not needed as FlattenSlicedString does not allocate managed objects
        return FlatStringInfo::FlattenSlicedString(hex);
    }
    ASSERT(hex->IsLineString());
    // VMHandle is not needed as 'hex' is LineString
    return FlatStringInfo(hex, 0, hex->GetLength());
}

template <typename T>
uint32_t HexCharToNum(T c)
{
    constexpr uint32_t HEX_10 = 10;
    constexpr uint32_t HEX_11 = 11;
    constexpr uint32_t HEX_12 = 12;
    constexpr uint32_t HEX_13 = 13;
    constexpr uint32_t HEX_14 = 14;
    constexpr uint32_t HEX_15 = 15;

    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    switch (c) {
        case 'a': /* fall-through */
        case 'A':
            return HEX_10;
        case 'b': /* fall-through */
        case 'B':
            return HEX_11;
        case 'c': /* fall-through */
        case 'C':
            return HEX_12;
        case 'd': /* fall-through */
        case 'D':
            return HEX_13;
        case 'e': /* fall-through */
        case 'E':
            return HEX_14;
        case 'f': /* fall-through */
        case 'F':
            return HEX_15;
        default:
            return HEX_ERROR;
    }
}

template <typename T>
uint32_t ConvertHexCharsToCharCodeStrict(const common_vm::Span<const T> &hex, uint32_t length)
{
    ASSERT(0 < length && length <= HEX_DIGITS_IN_ETS_CHAR);
    ASSERT(hex.size() >= length);
    uint32_t value = 0;
    uint32_t shift = (length - 1) * BITS_IN_HEX_DIGIT;
    for (uint32_t i = 0; i < length; ++i) {
        auto num = HexCharToNum<T>(hex[i]);
        if UNLIKELY (num == HEX_ERROR) {
            // There must be strictly 'length' hex chars
            ThrowIllegalArgumentException("Invalid hex character is encountered");
            return static_cast<EtsChar>(0);
        }
        value += (num << shift);
        shift -= BITS_IN_HEX_DIGIT;
    }
    ASSERT(value <= static_cast<decltype(value)>(std::numeric_limits<std::uint16_t>::max()));
    return value;
}

template <typename T>
uint32_t ConvertHexCharsToCharCode(const common_vm::Span<const T> &hex)
{
    ASSERT(!hex.empty());
    const uint32_t count = std::min(hex.size(), static_cast<size_t>(HEX_DIGITS_IN_ETS_CHAR));
    uint32_t value = 0;
    uint32_t shift = (count - 1) * BITS_IN_HEX_DIGIT;
    for (uint32_t i = 0; i < count; ++i) {
        auto num = HexCharToNum<T>(hex[i]);
        if (UNLIKELY(num == HEX_ERROR)) {
            // Accept 'x','xx','xxx' and 'xxxx' hex patters, where 'x' is a hex char.
            // Only the first character must be convertible to hex number.
            if (i == 0) {
                ThrowIllegalArgumentException("Invalid hex character is encountered");
                return static_cast<EtsChar>(0);
            }
            // Unrecognizable hex char is encountered.
            // This is not the first char, so return what we have recognized so far.
            value >>= ((count - i) * BITS_IN_HEX_DIGIT);
            break;
        }
        value += (num << shift);
        shift -= BITS_IN_HEX_DIGIT;
    }
    ASSERT(value <= static_cast<decltype(value)>(std::numeric_limits<std::uint16_t>::max()));
    return value;
}

extern "C" EtsChar StdCoreCharFromHexImpl(EtsString *hexStr, int startIndex, int length)
{
    if (UNLIKELY(hexStr == nullptr)) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return static_cast<EtsChar>(0);
    }
    auto hexLen = hexStr->GetLength();
    if (UNLIKELY(hexLen <= 0)) {
        return static_cast<EtsChar>(0);
    }
    if (startIndex < 0) {
        startIndex = 0;
    } else if (startIndex >= hexLen) {
        ThrowStringIndexOutOfBoundsException(startIndex, static_cast<uint32_t>(hexLen));
        return static_cast<EtsChar>(0);
    }
    if (length < 0) {
        length = 0;
    } else if (length > 0) {
        if (length > static_cast<int>(HEX_DIGITS_IN_ETS_CHAR)) {
            length = static_cast<int>(HEX_DIGITS_IN_ETS_CHAR);
        }
        auto end = startIndex + length;
        if (end > hexLen) {
            ThrowStringIndexOutOfBoundsException(end, static_cast<uint32_t>(hexLen));
            return static_cast<EtsChar>(0);
        }
    }
    // FlattenHexString may invalidate hexStr. Do not use it after this call.
    auto flat = FlattenHexStr(hexStr);
    if (UNLIKELY(flat.GetString() == nullptr)) {
        // Looks like we have encountered OutOfMemoryError
        return static_cast<EtsChar>(0);
    }
    auto remaining = flat.GetLength();
    if (startIndex != 0) {
        remaining -= static_cast<uint32_t>(startIndex);
        flat.SetStartIndex(flat.GetStartIndex() + static_cast<uint32_t>(startIndex));
    }

    if (flat.IsUtf16()) {
        common_vm::Span<const uint16_t> hex(flat.GetDataUtf16(), remaining);
        if (length == 0) {
            return static_cast<EtsChar>(ConvertHexCharsToCharCode(hex));
        }
        return static_cast<EtsChar>(ConvertHexCharsToCharCodeStrict(hex, static_cast<uint32_t>(length)));
    }
    common_vm::Span<const uint8_t> hex(flat.GetDataUtf8(), remaining);
    if (length == 0) {
        return static_cast<EtsChar>(ConvertHexCharsToCharCode(hex));
    }
    return static_cast<EtsChar>(ConvertHexCharsToCharCodeStrict(hex, static_cast<uint32_t>(length)));
}

extern "C" EtsString *StdCoreCharToHexImpl(EtsChar thisChar, EtsInt padNum, EtsChar padChar)
{
    constexpr uint32_t MASK_1 = 0x000f;
    constexpr uint32_t MASK_2 = 0x00f0;
    constexpr uint32_t MASK_3 = 0x0f00;
    constexpr uint32_t MASK_4 = 0xf000;
    constexpr uint32_t SHIFT_4 = 4;
    constexpr uint32_t SHIFT_8 = 8;
    constexpr uint32_t SHIFT_12 = 12;
    static constexpr HexDigits DIGITS({'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'});
    SmallVector<uint8_t, HEX_DIGITS_IN_ETS_CHAR> buf;

    const auto c = static_cast<uint32_t>(thisChar);
    uint8_t i = (c & MASK_4) >> SHIFT_12;
    if (i != 0) {
        buf.push_back(DIGITS[i]);
    }
    i = (c & MASK_3) >> SHIFT_8;
    if (i != 0 || !buf.empty()) {
        buf.push_back(DIGITS[i]);
    }
    i = (c & MASK_2) >> SHIFT_4;
    if (i != 0 || !buf.empty()) {
        buf.push_back(DIGITS[i]);
    }
    buf.push_back(DIGITS[c & MASK_1]);

    ASSERT(buf.size() <= HEX_DIGITS_IN_ETS_CHAR);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    if (padNum < 0) {
        padNum = static_cast<uint32_t>(0);
    }
    auto *result = coretypes::LineString::CreatePaddedFromMutf8(padChar, padNum, buf.data(), buf.size(), ctx, vm);
    return reinterpret_cast<EtsString *>(result);
}

}  // namespace ark::ets::intrinsics
