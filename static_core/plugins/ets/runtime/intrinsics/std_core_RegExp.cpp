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
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/intrinsics/helpers/regexp_preparser.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "libarkbase/macros.h"
#include "runtime/include/mem/panda_string.h"
#include "ets_exceptions.h"

#include <cstdint>

namespace ark::ets::intrinsics {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

enum RegExpFlagBit : uint32_t {
    FLAG_G = 1U << 0U,
    FLAG_I = 1U << 1U,
    FLAG_M = 1U << 2U,
    FLAG_S = 1U << 3U,
    FLAG_U = 1U << 4U,
    FLAG_Y = 1U << 5U,
    FLAG_D = 1U << 6U,
    FLAG_V = 1U << 7U,
};

struct FlagParseResult {
    uint32_t pcreFlags = 0;
    bool unicodeMode = false;
};

constexpr PCRE2_SIZE PCRE2_ERROR_BUFFER_SIZE = 256U;
constexpr uint32_t HEX_ALPHA_DIGIT_OFFSET = 10U;
constexpr uint32_t UTF8_2BYTE_LIMIT = 0x80U;
constexpr uint32_t UTF8_3BYTE_LIMIT = 0x800U;
constexpr uint32_t UTF8_4BYTE_LIMIT = 0x10000U;
constexpr uint32_t UTF8_2BYTE_PREFIX = 0xC0U;
constexpr uint32_t UTF8_3BYTE_PREFIX = 0xE0U;
constexpr uint32_t UTF8_4BYTE_PREFIX = 0xF0U;
constexpr uint32_t UTF8_CONT_MASK = 0x3FU;
constexpr uint32_t UTF8_CONT_PREFIX = 0x80U;
constexpr uint32_t UTF8_3BYTE_SHIFT = 12U;
constexpr uint32_t UTF8_4BYTE_SHIFT = 18U;

constexpr uint32_t HIGH_SURROGATE_MIN = 0xD800U;
constexpr uint32_t HIGH_SURROGATE_MAX = 0xDBFFU;
constexpr uint32_t LOW_SURROGATE_MIN = 0xDC00U;
constexpr uint32_t LOW_SURROGATE_MAX = 0xDFFFU;
constexpr uint32_t SURROGATE_OFFSET = 0x10000U;
constexpr uint32_t SURROGATE_HALF = 0x400U;
constexpr size_t UNICODE_ESC_LEN = 6U;  // \uXXXX

static void ThrowRegExpSyntaxError(const PandaString &msg)
{
    auto coro = EtsCoroutine::GetCurrent();
    ThrowEtsException(coro, PlatformTypes(coro)->coreSyntaxError, msg);
}

static void ValidateAndMarkRegExpFlag(char c, uint32_t bit, uint32_t &seenMask)
{
    if ((seenMask & bit) != 0U) {
        PandaString msg = "SyntaxError: Duplicate RegExp flag '";
        msg += c;
        msg += "'";
        ThrowRegExpSyntaxError(msg);
    }
    seenMask |= bit;
}

static void ApplyUnicodeRegExpFlag(FlagParseResult &result)
{
    result.pcreFlags |= PCRE2_UTF | PCRE2_UCP;
    result.unicodeMode = true;
}

static void ApplyRegExpFlag(char c, FlagParseResult &result, uint32_t &seenMask)
{
    switch (c) {
        case 'g':
            ValidateAndMarkRegExpFlag(c, FLAG_G, seenMask);
            break;
        case 'i':
            ValidateAndMarkRegExpFlag(c, FLAG_I, seenMask);
            result.pcreFlags |= PCRE2_CASELESS | PCRE2_UCP;
            break;
        case 'm':
            ValidateAndMarkRegExpFlag(c, FLAG_M, seenMask);
            result.pcreFlags |= PCRE2_MULTILINE;
            break;
        case 's':
            ValidateAndMarkRegExpFlag(c, FLAG_S, seenMask);
            result.pcreFlags |= PCRE2_DOTALL;
            break;
        case 'u':
            ValidateAndMarkRegExpFlag(c, FLAG_U, seenMask);
            ApplyUnicodeRegExpFlag(result);
            break;
        case 'v':
            ValidateAndMarkRegExpFlag(c, FLAG_V, seenMask);
            ApplyUnicodeRegExpFlag(result);
            break;
        case 'y':
            ValidateAndMarkRegExpFlag(c, FLAG_Y, seenMask);
            result.pcreFlags |= PCRE2_ANCHORED;
            break;
        case 'd':
            ValidateAndMarkRegExpFlag(c, FLAG_D, seenMask);
            break;
        default: {
            PandaString msg = "SyntaxError: Invalid RegExp flag '";
            msg += c;
            msg += "'";
            ThrowRegExpSyntaxError(msg);
        }
    }
}

static FlagParseResult ParseRegExpFlags(const PandaString &flagsStr)
{
    FlagParseResult result;
    uint32_t seenMask = 0;

    for (char c : flagsStr) {
        ApplyRegExpFlag(c, result, seenMask);
    }

    return result;
}

static bool IsHexChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static uint32_t HexCharValue(char c)
{
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<uint32_t>(c - 'a') + HEX_ALPHA_DIGIT_OFFSET;
    }
    return static_cast<uint32_t>(c - 'A') + HEX_ALPHA_DIGIT_OFFSET;
}

static bool TryParseHexSeq(const PandaString &pattern, size_t pos, size_t count, uint32_t &value)
{
    if (pos + count > pattern.size()) {
        return false;
    }
    value = 0;
    for (size_t i = 0; i < count; i++) {
        if (!IsHexChar(pattern[pos + i])) {
            return false;
        }
        value = (value << 4U) | HexCharValue(pattern[pos + i]);
    }
    return true;
}

static void AppendCodePointUtf8(PandaString &out, uint32_t cp)
{
    if (cp < UTF8_2BYTE_LIMIT) {
        out += static_cast<char>(cp);
    } else if (cp < UTF8_3BYTE_LIMIT) {
        out += static_cast<char>(UTF8_2BYTE_PREFIX | (cp >> 6U));
        out += static_cast<char>(UTF8_CONT_PREFIX | (cp & UTF8_CONT_MASK));
    } else if (cp < UTF8_4BYTE_LIMIT) {
        out += static_cast<char>(UTF8_3BYTE_PREFIX | (cp >> UTF8_3BYTE_SHIFT));
        out += static_cast<char>(UTF8_CONT_PREFIX | ((cp >> 6U) & UTF8_CONT_MASK));
        out += static_cast<char>(UTF8_CONT_PREFIX | (cp & UTF8_CONT_MASK));
    } else {
        out += static_cast<char>(UTF8_4BYTE_PREFIX | (cp >> UTF8_4BYTE_SHIFT));
        out += static_cast<char>(UTF8_CONT_PREFIX | ((cp >> UTF8_3BYTE_SHIFT) & UTF8_CONT_MASK));
        out += static_cast<char>(UTF8_CONT_PREFIX | ((cp >> 6U) & UTF8_CONT_MASK));
        out += static_cast<char>(UTF8_CONT_PREFIX | (cp & UTF8_CONT_MASK));
    }
}

// If pattern[i] starts a high surrogate \uHHHH followed immediately by a low surrogate \uLLLL,
// appends the corresponding UTF-8 code point to result, sets newPos to just past the pair, and
// returns true.  Otherwise returns false and leaves result/newPos unchanged.
static bool TryConsumeSurrogatePair(const PandaString &pattern, size_t i, uint32_t high, PandaString &result,
                                    size_t &newPos)
{
    size_t j = i + UNICODE_ESC_LEN;
    uint32_t low;
    if (j + UNICODE_ESC_LEN <= pattern.size() && pattern[j] == '\\' && pattern[j + 1U] == 'u' &&
        TryParseHexSeq(pattern, j + 2U, 4U, low) && low >= LOW_SURROGATE_MIN && low <= LOW_SURROGATE_MAX) {
        uint32_t cp = SURROGATE_OFFSET + (high - HIGH_SURROGATE_MIN) * SURROGATE_HALF + (low - LOW_SURROGATE_MIN);
        AppendCodePointUtf8(result, cp);
        newPos = j + UNICODE_ESC_LEN;
        return true;
    }
    return false;
}

// Normalizes non-unicode mode patterns to handle ECMA-262 IdentityEscape rules:
// - \x not followed by exactly 2 hex digits becomes literal 'x'
// - \uHHHH\uLLLL surrogate pairs are converted to actual UTF-8 encoded code points
static PandaString NormalizeNonUnicodePattern(const PandaString &pattern)
{
    PandaString result;
    result.reserve(pattern.size());
    size_t i = 0;

    while (i < pattern.size()) {
        if (pattern[i] != '\\' || i + 1U >= pattern.size()) {
            result += pattern[i++];
            continue;
        }

        char next = pattern[i + 1U];
        if (next == 'x') {
            uint32_t dummy;
            if (TryParseHexSeq(pattern, i + 2U, 2U, dummy)) {
                // Valid \xHH - keep as-is
                result += pattern[i++];
                result += pattern[i++];
                result += pattern[i++];
                result += pattern[i++];
            } else {
                // Identity escape: \x becomes literal 'x'
                i += 2U;
                result += 'x';
            }
        } else if (next == 'u') {
            uint32_t high;
            size_t newPos;
            if (TryParseHexSeq(pattern, i + 2U, 4U, high) && high >= HIGH_SURROGATE_MIN && high <= HIGH_SURROGATE_MAX &&
                TryConsumeSurrogatePair(pattern, i, high, result, newPos)) {
                i = newPos;
                continue;
            }
            // Not a surrogate pair - keep \u as-is
            result += pattern[i++];
            result += pattern[i++];
        } else {
            result += pattern[i++];
        }
    }

    return result;
}

static pcre2_code *CompileWithPcre2(const PandaString &patternStr, uint32_t pcreFlags)
{
    int errorCode = 0;
    PCRE2_SIZE errorOffset = 0;

    pcre2_compile_context *ctx = pcre2_compile_context_create(nullptr);
    pcre2_set_newline(ctx, PCRE2_NEWLINE_ANY);
    pcre2_set_compile_extra_options(ctx, PCRE2_EXTRA_ALT_BSUX);

    // Always use UTF mode; skip validation for patterns without explicit /u flag
    uint32_t compileFlags = pcreFlags | PCRE2_MATCH_UNSET_BACKREF | PCRE2_ALLOW_EMPTY_CLASS;
    if ((pcreFlags & PCRE2_UTF) == 0U) {
        compileFlags |= PCRE2_UTF | PCRE2_NO_UTF_CHECK;
    }

    pcre2_code *re = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(patternStr.data()), patternStr.size(), compileFlags,
                                   &errorCode, &errorOffset, ctx);

    pcre2_compile_context_free(ctx);

    if (re == nullptr) {
        PCRE2_UCHAR buffer[PCRE2_ERROR_BUFFER_SIZE];
        pcre2_get_error_message(errorCode, buffer, sizeof(buffer));

        PandaString msg = "SyntaxError: ";
        msg += reinterpret_cast<char *>(buffer);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        ThrowRegExpSyntaxError(msg);
    }

    return re;
}

extern "C" void StdCoreRegExpParse(EtsString *pattern, EtsString *flags)
{
    PandaString patternStr = pattern->GetUtf8();
    PandaString flagsStr = flags->GetUtf8();

    FlagParseResult flagResult = ParseRegExpFlags(flagsStr);
    helpers::JsRegExpPreParser preParser(patternStr, flagsStr, flagResult.unicodeMode);

    PandaString preParseError;
    if (!preParser.Validate(preParseError)) {
        ThrowRegExpSyntaxError(preParseError);
        return;
    }

    PandaString processedPattern = flagResult.unicodeMode ? patternStr : NormalizeNonUnicodePattern(patternStr);
    pcre2_code *re = CompileWithPcre2(processedPattern, flagResult.pcreFlags);
    if (re != nullptr) {
        pcre2_code_free(re);
    }
}

}  // namespace ark::ets::intrinsics
