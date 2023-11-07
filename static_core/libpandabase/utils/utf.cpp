/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "utf.h"

#include <cstddef>
#include <cstring>

#include <limits>
#include <tuple>
#include <utility>

// NOLINTNEXTLINE(hicpp-signed-bitwise)
static constexpr int32_t U16_SURROGATE_OFFSET = (0xd800 << 10UL) + 0xdc00 - 0x10000;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define U16_GET_SUPPLEMENTARY(lead, trail) \
    ((static_cast<int32_t>(lead) << 10UL) + static_cast<int32_t>(trail) - U16_SURROGATE_OFFSET)

namespace panda::utf {

/*
 * MUtf-8
 *
 * U+0000 => C0 80
 *
 * N  Bits for     First        Last        Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      Byte 6
 *    code point   code point   code point
 * 1  7            U+0000       U+007F      0xxxxxxx
 * 2  11           U+0080       U+07FF      110xxxxx    10xxxxxx
 * 3  16           U+0800       U+FFFF      1110xxxx    10xxxxxx    10xxxxxx
 * 6  21           U+10000      U+10FFFF    11101101    1010xxxx    10xxxxxx    11101101    1011xxxx    10xxxxxx
 * for U+10000 -- U+10FFFF encodes the following (value - 0x10000)
 */

/*
 * Convert mutf8 sequence to utf16 pair and return pair: [utf16 code point, mutf8 size].
 * In case of invalid sequence return first byte of it.
 */
std::pair<uint32_t, size_t> ConvertMUtf8ToUtf16Pair(const uint8_t *data, size_t max_bytes)
{
    // NOTE(d.kovalneko): make the function safe
    Span<const uint8_t> sp(data, max_bytes);
    uint8_t d0 = sp[0];
    if ((d0 & MASK1) == 0) {
        return {d0, 1};
    }

    if (max_bytes < CONST_2) {
        return {d0, 1};
    }
    uint8_t d1 = sp[1];
    if ((d0 & MASK2) == 0) {
        return {((d0 & MASK_5BIT) << DATA_WIDTH) | (d1 & MASK_6BIT), 2};
    }

    if (max_bytes < CONST_3) {
        return {d0, 1};
    }
    uint8_t d2 = sp[CONST_2];
    if ((d0 & MASK3) == 0) {
        return {((d0 & MASK_4BIT) << (DATA_WIDTH * CONST_2)) | ((d1 & MASK_6BIT) << DATA_WIDTH) | (d2 & MASK_6BIT),
                CONST_3};
    }

    if (max_bytes < CONST_4) {
        return {d0, 1};
    }
    uint8_t d3 = sp[CONST_3];
    uint32_t code_point = ((d0 & MASK_4BIT) << (DATA_WIDTH * CONST_3)) | ((d1 & MASK_6BIT) << (DATA_WIDTH * CONST_2)) |
                          ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT);

    uint32_t pair = 0;
    pair |= ((code_point >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD) & MASK_16BIT;
    pair <<= PAIR_ELEMENT_WIDTH;
    pair |= (code_point & MASK_10BIT) + U16_TAIL;

    return {pair, CONST_4};
}

static constexpr uint32_t CombineTwoU16(uint16_t d0, uint16_t d1)
{
    uint32_t code_point = d0 - DECODE_LEAD_LOW;
    code_point <<= (PAIR_ELEMENT_WIDTH - DATA_WIDTH);
    code_point |= d1 - DECODE_TRAIL_LOW;  // NOLINT(hicpp-signed-bitwise
    code_point += DECODE_SECOND_FACTOR;
    return code_point;
}

bool IsMUtf8OnlySingleBytes(const uint8_t *mutf8_in)
{
    while (*mutf8_in != '\0') {    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*mutf8_in >= MASK1) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return false;
        }
        mutf8_in += 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return true;
}

size_t ConvertRegionUtf16ToMUtf8(const uint16_t *utf16_in, uint8_t *mutf8_out, size_t utf16_len, size_t mutf8_len,
                                 size_t start)
{
    return ConvertRegionUtf16ToUtf8(utf16_in, mutf8_out, utf16_len, mutf8_len, start, true);
}

void ConvertMUtf8ToUtf16(const uint8_t *mutf8_in, size_t mutf8_len, uint16_t *utf16_out)
{
    size_t in_pos = 0;
    while (in_pos < mutf8_len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8_in, mutf8_len - in_pos);
        auto [p_hi, p_lo] = SplitUtf16Pair(pair);

        if (p_hi != 0) {
            *utf16_out++ = p_hi;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        *utf16_out++ = p_lo;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        mutf8_in += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        in_pos += nbytes;
    }
}

size_t ConvertRegionMUtf8ToUtf16(const uint8_t *mutf8_in, uint16_t *utf16_out, size_t mutf8_len, size_t utf16_len,
                                 size_t start)
{
    size_t in_pos = 0;
    size_t out_pos = 0;
    while (in_pos < mutf8_len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8_in, mutf8_len - in_pos);
        auto [p_hi, p_lo] = SplitUtf16Pair(pair);

        mutf8_in += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        in_pos += nbytes;
        if (start > 0) {
            start -= nbytes;
            continue;
        }

        if (p_hi != 0) {
            if (out_pos++ >= utf16_len - 1) {  // check for place for two uint16
                --out_pos;
                break;
            }
            *utf16_out++ = p_hi;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        if (out_pos++ >= utf16_len) {
            --out_pos;
            break;
        }
        *utf16_out++ = p_lo;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return out_pos;
}

int CompareMUtf8ToMUtf8(const uint8_t *mutf8_1, const uint8_t *mutf8_2)
{
    uint32_t c1;
    uint32_t c2;
    uint32_t n1;
    uint32_t n2;

    do {
        c1 = *mutf8_1;
        c2 = *mutf8_2;

        if (c1 == 0 && c2 == 0) {
            return 0;
        }

        if (c1 == 0 && c2 != 0) {
            return -1;
        }

        if (c1 != 0 && c2 == 0) {
            return 1;
        }

        std::tie(c1, n1) = ConvertMUtf8ToUtf16Pair(mutf8_1);
        std::tie(c2, n2) = ConvertMUtf8ToUtf16Pair(mutf8_2);

        mutf8_1 += n1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        mutf8_2 += n2;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    } while (c1 == c2);

    auto [c1p1, c1p2] = SplitUtf16Pair(c1);
    auto [c2p1, c2p2] = SplitUtf16Pair(c2);

    auto result = static_cast<int>(c1p1 - c2p1);
    if (result != 0) {
        return result;
    }

    return c1p2 - c2p2;
}

// compare plain utf8, which allows 0 inside a string
int CompareUtf8ToUtf8(const uint8_t *utf8_1, size_t utf8_1_length, const uint8_t *utf8_2, size_t utf8_2_length)
{
    uint32_t c1;
    uint32_t c2;
    uint32_t n1;
    uint32_t n2;

    uint32_t utf8_1_index = 0;
    uint32_t utf8_2_index = 0;

    do {
        if (utf8_1_index == utf8_1_length && utf8_2_index == utf8_2_length) {
            return 0;
        }

        if (utf8_1_index == utf8_1_length && utf8_2_index < utf8_2_length) {
            return -1;
        }

        if (utf8_1_index < utf8_1_length && utf8_2_index == utf8_2_length) {
            return 1;
        }

        c1 = *utf8_1;
        c2 = *utf8_2;

        std::tie(c1, n1) = ConvertMUtf8ToUtf16Pair(utf8_1);
        std::tie(c2, n2) = ConvertMUtf8ToUtf16Pair(utf8_2);

        utf8_1 += n1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf8_2 += n2;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf8_1_index += n1;
        utf8_2_index += n2;
    } while (c1 == c2);

    auto [c1p1, c1p2] = SplitUtf16Pair(c1);
    auto [c2p1, c2p2] = SplitUtf16Pair(c2);

    auto result = static_cast<int>(c1p1 - c2p1);
    if (result != 0) {
        return result;
    }

    return c1p2 - c2p2;
}

size_t Mutf8Size(const uint8_t *mutf8)
{
    return strlen(Mutf8AsCString(mutf8));
}

size_t MUtf8ToUtf16Size(const uint8_t *mutf8)
{
    // NOTE(d.kovalenko): make it faster
    size_t res = 0;
    while (*mutf8 != '\0') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8);
        res += pair > MAX_U16 ? CONST_2 : 1;
        mutf8 += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return res;
}

size_t MUtf8ToUtf16Size(const uint8_t *mutf8, size_t mutf8_len)
{
    size_t pos = 0;
    size_t res = 0;
    while (pos != mutf8_len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8, mutf8_len - pos);
        if (nbytes == 0) {
            nbytes = 1;
        }
        res += pair > MAX_U16 ? CONST_2 : 1;
        mutf8 += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pos += nbytes;
    }
    return res;
}

bool IsEqual(Span<const uint8_t> utf8_1, Span<const uint8_t> utf8_2)
{
    if (utf8_1.size() != utf8_2.size()) {
        return false;
    }

    return memcmp(utf8_1.data(), utf8_2.data(), utf8_1.size()) == 0;
}

bool IsEqual(const uint8_t *mutf8_1, const uint8_t *mutf8_2)
{
    return strcmp(Mutf8AsCString(mutf8_1), Mutf8AsCString(mutf8_2)) == 0;
}

bool IsValidModifiedUTF8(const uint8_t *elems)
{
    ASSERT(elems);

    while (*elems != '\0') {
        // NOLINTNEXTLINE(hicpp-signed-bitwise, readability-magic-numbers)
        switch (*elems & 0xf0) {
            case 0x00:
            case 0x10:  // NOLINT(readability-magic-numbers)
            case 0x20:  // NOLINT(readability-magic-numbers)
            case 0x30:  // NOLINT(readability-magic-numbers)
            case 0x40:  // NOLINT(readability-magic-numbers)
            case 0x50:  // NOLINT(readability-magic-numbers)
            case 0x60:  // NOLINT(readability-magic-numbers)
            case 0x70:  // NOLINT(readability-magic-numbers)
                // pattern 0xxx
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                break;
            case 0x80:  // NOLINT(readability-magic-numbers)
            case 0x90:  // NOLINT(readability-magic-numbers)
            case 0xa0:  // NOLINT(readability-magic-numbers)
            case 0xb0:  // NOLINT(readability-magic-numbers)
                // pattern 10xx is illegal start
                return false;

            case 0xf0:  // NOLINT(readability-magic-numbers)
                // pattern 1111 0xxx starts four byte section
                if ((*elems & 0x08) == 0) {  // NOLINT(hicpp-signed-bitwise)
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    ++elems;
                    if ((*elems & 0xc0) != 0x80) {  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
                        return false;
                    }
                } else {
                    return false;
                }
                // no need break
                [[fallthrough]];

            case 0xe0:  // NOLINT(readability-magic-numbers)
                // pattern 1110
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                if ((*elems & 0xc0) != 0x80) {  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
                    return false;
                }
                // no need break
                [[fallthrough]];

            case 0xc0:  // NOLINT(readability-magic-numbers)
            case 0xd0:  // NOLINT(readability-magic-numbers)
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                if ((*elems & 0xc0) != 0x80) {  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
                    return false;
                }
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                break;
        }
    }
    return true;
}

uint32_t UTF16Decode(uint16_t lead, uint16_t trail)
{
    ASSERT((lead >= DECODE_LEAD_LOW && lead <= DECODE_LEAD_HIGH) &&
           (trail >= DECODE_TRAIL_LOW && trail <= DECODE_TRAIL_HIGH));
    uint32_t cp = (lead - DECODE_LEAD_LOW) * DECODE_FIRST_FACTOR + (trail - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
    return cp;
}

bool IsValidUTF8(const std::vector<uint8_t> &data)
{
    uint32_t length = data.size();
    switch (length) {
        case UtfLength::ONE:
            if (data.at(0) >= BIT_MASK_1) {
                return false;
            }
            break;
        case UtfLength::TWO:
            if ((data.at(0) & BIT_MASK_3) != BIT_MASK_2) {
                return false;
            }
            break;
        case UtfLength::THREE:
            if ((data.at(0) & BIT_MASK_4) != BIT_MASK_3) {
                return false;
            }
            break;
        case UtfLength::FOUR:
            if ((data.at(0) & BIT_MASK_5) != BIT_MASK_4) {
                return false;
            }
            break;
        default:
            UNREACHABLE();
            break;
    }

    for (uint32_t i = 1; i < length; i++) {
        if ((data.at(i) & BIT_MASK_2) != BIT_MASK_1) {
            return false;
        }
    }
    return true;
}

Utf8Char ConvertUtf16ToUtf8(uint16_t d0, uint16_t d1, bool modify)
{
    // when first utf16 code is in 0xd800-0xdfff and second utf16 code is 0,
    // means that is a single code point, it needs to be represented by three UTF8 code.
    if (d1 == 0 && d0 >= DECODE_LEAD_LOW && d0 <= DECODE_TRAIL_HIGH) {
        auto ch0 = static_cast<uint8_t>(UTF8_3B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::TWELVE));
        auto ch1 = static_cast<uint8_t>(UTF8_3B_SECOND | (static_cast<uint8_t>(d0 >> UtfOffset::SIX) & MASK_6BIT));
        auto ch2 = static_cast<uint8_t>(UTF8_3B_THIRD | (d0 & MASK_6BIT));
        return {UtfLength::THREE, {ch0, ch1, ch2}};
    }

    if (d0 == 0) {
        if (modify) {
            // special case for \u0000 ==> C080 - 1100'0000 1000'0000
            return {UtfLength::TWO, {UTF8_2B_FIRST, UTF8_2B_SECOND}};
        }
        // For print string, just skip '\u0000'
        return {0, {0x00U}};
    }
    if (d0 <= UTF8_1B_MAX) {
        return {UtfLength::ONE, {static_cast<uint8_t>(d0)}};
    }
    if (d0 <= UTF8_2B_MAX) {
        auto ch0 = static_cast<uint8_t>(UTF8_2B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::SIX));
        auto ch1 = static_cast<uint8_t>(UTF8_2B_SECOND | (d0 & MASK_6BIT));
        return {UtfLength::TWO, {ch0, ch1}};
    }
    if (d0 < DECODE_LEAD_LOW || d0 > DECODE_LEAD_HIGH) {
        auto ch0 = static_cast<uint8_t>(UTF8_3B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::TWELVE));
        auto ch1 = static_cast<uint8_t>(UTF8_3B_SECOND | (static_cast<uint8_t>(d0 >> UtfOffset::SIX) & MASK_6BIT));
        auto ch2 = static_cast<uint8_t>(UTF8_3B_THIRD | (d0 & MASK_6BIT));
        return {UtfLength::THREE, {ch0, ch1, ch2}};
    }
    if (d1 < DECODE_TRAIL_LOW || d1 > DECODE_TRAIL_HIGH) {
        // Bad sequence
        UNREACHABLE();
    }

    uint32_t code_point = CombineTwoU16(d0, d1);

    auto ch0 = static_cast<uint8_t>((code_point >> UtfOffset::EIGHTEEN) | UTF8_4B_FIRST);
    auto ch1 = static_cast<uint8_t>(((code_point >> UtfOffset::TWELVE) & MASK_6BIT) | MASK1);
    auto ch2 = static_cast<uint8_t>(((code_point >> UtfOffset::SIX) & MASK_6BIT) | MASK1);
    auto ch3 = static_cast<uint8_t>((code_point & MASK_6BIT) | MASK1);

    return {UtfLength::FOUR, {ch0, ch1, ch2, ch3}};
}

size_t Utf16ToUtf8Size(const uint16_t *utf16, uint32_t length, bool modify)
{
    size_t res = 1;  // zero byte
    // when utf16 data length is only 1 and code in 0xd800-0xdfff,
    // means that is a single code point, it needs to be represented by three UTF8 code.
    if (length == 1 && utf16[0] >= DECODE_LEAD_LOW &&  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf16[0] <= DECODE_TRAIL_HIGH) {               // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        res += UtfLength::THREE;
        return res;
    }

    for (uint32_t i = 0; i < length; ++i) {
        if (utf16[i] == 0) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (modify) {
                res += UtfLength::TWO;  // special case for U+0000 => C0 80
            }
        } else if (utf16[i] <= UTF8_1B_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res += 1;
        } else if (utf16[i] <= UTF8_2B_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res += UtfLength::TWO;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (utf16[i] < DECODE_LEAD_LOW || utf16[i] > DECODE_LEAD_HIGH) {
            res += UtfLength::THREE;
        } else {
            if (i < length - 1 &&
                utf16[i + 1] >= DECODE_TRAIL_LOW &&   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                utf16[i + 1] <= DECODE_TRAIL_HIGH) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                res += UtfLength::FOUR;
                ++i;
            } else {
                res += UtfLength::THREE;
            }
        }
    }
    return res;
}

size_t Utf16ToMUtf8Size(const uint16_t *mutf16, uint32_t length)
{
    return Utf16ToUtf8Size(mutf16, length, true);
}

size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16_in, uint8_t *utf8_out, size_t utf16_len, size_t utf8_len,
                                size_t start, bool modify)
{
    size_t utf8_pos = 0;
    if (utf16_in == nullptr || utf8_out == nullptr || utf8_len == 0) {
        return 0;
    }
    size_t end = start + utf16_len;
    for (size_t i = start; i < end; ++i) {
        uint16_t next16_code = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if ((i + 1) != end && IsAvailableNextUtf16Code(utf16_in[i + 1])) {
            next16_code = utf16_in[i + 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Utf8Char ch = ConvertUtf16ToUtf8(utf16_in[i], next16_code, modify);
        if (utf8_pos + ch.n > utf8_len) {
            break;
        }
        for (size_t c = 0; c < ch.n; ++c) {
            utf8_out[utf8_pos++] = ch.ch[c];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        if (ch.n == UtfLength::FOUR) {  // Two UTF-16 chars are used
            ++i;
        }
    }
    return utf8_pos;
}

std::pair<uint32_t, size_t> ConvertUtf8ToUtf16Pair(const uint8_t *data, bool combine)
{
    uint8_t d0 = data[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if ((d0 & MASK1) == 0) {
        return {d0, 1};
    }

    uint8_t d1 = data[1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if ((d0 & MASK2) == 0) {
        return {((d0 & MASK_5BIT) << DATA_WIDTH) | (d1 & MASK_6BIT), UtfLength::TWO};
    }

    uint8_t d2 = data[UtfLength::TWO];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if ((d0 & MASK3) == 0) {
        return {((d0 & MASK_4BIT) << UtfOffset::TWELVE) | ((d1 & MASK_6BIT) << DATA_WIDTH) | (d2 & MASK_6BIT),
                UtfLength::THREE};
    }

    uint8_t d3 = data[UtfLength::THREE];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t code_point = ((d0 & MASK_4BIT) << UtfOffset::EIGHTEEN) | ((d1 & MASK_6BIT) << UtfOffset::TWELVE) |
                          ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT);

    uint32_t pair = 0;
    if (combine) {
        uint32_t lead = ((code_point >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD);
        uint32_t tail = ((code_point & MASK_10BIT) + U16_TAIL) & MASK_16BIT;
        pair = U16_GET_SUPPLEMENTARY(lead, tail);  // NOLINT(hicpp-signed-bitwise)
    } else {
        pair |= ((code_point >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD) << PAIR_ELEMENT_WIDTH;
        pair |= ((code_point & MASK_10BIT) + U16_TAIL) & MASK_16BIT;
    }

    return {pair, UtfLength::FOUR};
}

size_t Utf8ToUtf16Size(const uint8_t *utf8, size_t utf8_len)
{
    return MUtf8ToUtf16Size(utf8, utf8_len);
}

size_t ConvertRegionUtf8ToUtf16(const uint8_t *utf8_in, uint16_t *utf16_out, size_t utf8_len, size_t utf16_len,
                                size_t start)
{
    return ConvertRegionMUtf8ToUtf16(utf8_in, utf16_out, utf8_len, utf16_len, start);
}

bool IsUTF16SurrogatePair(const uint16_t lead)
{
    return lead >= DECODE_LEAD_LOW && lead <= DECODE_LEAD_HIGH;
}

}  // namespace panda::utf
