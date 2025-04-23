/**
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

#include "regexp.h"

#include "regexp_8.h"
#include "regexp_16.h"
#include "plugins/ets/runtime/ets_exceptions.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

namespace ark::ets {

constexpr uint64_t UTF16_HEX_SHIFT = 4U;
constexpr uint64_t UTF16_TECHNICAL_CODE_START = 0xD800;
constexpr uint64_t UTF16_TECHNICAL_CODE_END = 0xDFFF;
constexpr uint8_t UTF16_BYTE_SHIFT = 8U;
constexpr uint8_t UTF16_BYTE_MAX = 0xFF;
constexpr uint64_t UTF16_BYTE5_START = 0x10000;
constexpr uint8_t UTF16_BYTE5_ZONE1_START = 0xD8;
constexpr uint8_t UTF16_BYTE5_ZONE2_START = 0xDC;
constexpr uint64_t UTF16_BYTE5_MIN = 1U >> 20U;
constexpr uint8_t UTF16_BYTE5_ZONE1_BYTE1_SHIFT = 18U;
constexpr uint8_t UTF16_BYTE5_ZONE1_BYTE2_SHIFT = 10U;
constexpr uint8_t UTF16_BYTE5_ZONE2_BYTE1_SHIFT = 8U;
constexpr uint8_t UTF16_BYTE5_ZONE2_BYTE1_NEEDED_BITS = 0x3;
constexpr size_t UTF16_WIDTH = 2;
constexpr uint8_t DIRECT_UNICODE_BYTES = 4U;

bool EtsRegExp::Compile(const PandaVector<uint8_t> &pattern, const bool isUtf16, const int len)
{
    utf16_ |= isUtf16;
    uint32_t flags = 0U;
    if (flagMultiline_) {
        flags |= PCRE2_MULTILINE;
    }
    if (flagCaseInsentitive_) {
        flags |= PCRE2_CASELESS;
    }
    if (flagSticky_) {
        flags |= PCRE2_ANCHORED;
    }
    if (flagDotAll_) {
        flags |= PCRE2_DOTALL;
    }
    utf16_ |= flagUnicode_;
    if (utf16_) {
        flags |= PCRE2_UTF;
    }
    PandaVector<uint8_t> preparedPattern = pattern;
    auto patternIsCorrect = EtsRegExp::PreparePattern(preparedPattern, utf16_);
    if (!patternIsCorrect) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                          "invalid regular expression (failed to parse unicode)");
        return false;
    }
    int length = len;
    if (preparedPattern.size() != pattern.size()) {
        if (utf16_) {
            length = preparedPattern.size() / UTF16_WIDTH - 1;
        } else {
            length = preparedPattern.size() - 1;
        }
    }
    if (utf16_) {
        re_ = RegExp16::CreatePcre2Object(reinterpret_cast<const uint16_t *>(preparedPattern.data()), flags, length);
    } else {
        re_ = RegExp8::CreatePcre2Object(preparedPattern.data(), flags, length);
    }
    return re_ != nullptr;
}

ark::RegExpMatchResult<PandaString> EtsRegExp::Execute(const PandaVector<uint8_t> &str, const int len,
                                                       const int startOffset)
{
    ark::RegExpMatchResult<PandaString> result;
    if (utf16_) {
        result = RegExp16::Execute(re_, reinterpret_cast<const uint16_t *>(str.data()), len, startOffset);
    } else {
        result = RegExp8::Execute(re_, str.data(), len, startOffset);
    }
    return result;
}

void EtsRegExp::Destroy()
{
    if (utf16_) {
        RegExp16::FreePcre2Object(re_);
    } else {
        RegExp8::FreePcre2Object(re_);
    }
}

void EtsRegExp::SetUnicodeFlag(const char &chr)
{
    if (chr == 'u') {
        if (flagUnicode_ || flagVnicode_) {
            ThrowBadFlagsException();
        }
        flagUnicode_ = true;
    } else if (chr == 'v') {
        if (flagVnicode_ || flagUnicode_) {
            ThrowBadFlagsException();
        }
        flagVnicode_ = true;
    }
}

void EtsRegExp::ThrowBadFlagsException()
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                      "invalid regular expression flags");
}

void EtsRegExp::SetIfNotSet(bool &flag)
{
    if (flag) {
        ThrowBadFlagsException();
    }
    flag = true;
}

void EtsRegExp::SetFlag(const char &chr)
{
    switch (chr) {
        case 'g':
            SetIfNotSet(flagGlobal_);
            break;
        case 'm':
            SetIfNotSet(flagMultiline_);
            break;
        case 'i':
            SetIfNotSet(flagCaseInsentitive_);
            break;
        case 'y':
            SetIfNotSet(flagSticky_);
            break;
        case 's':
            SetIfNotSet(flagDotAll_);
            break;
        case 'd':
            SetIfNotSet(flagIndices_);
            break;
        default: {
            SetUnicodeFlag(chr);
        }
    }
}

void EtsRegExp::SetFlags(EtsString *flagsStr)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> flagsHandle(coroutine, flagsStr->GetCoreType());
    for (int i = 0; i < flagsHandle.GetPtr()->GetLength(); i++) {
        SetFlag(flagsHandle.GetPtr()->At(i));
    }
    utf16_ |= flagUnicode_;
}

bool EtsRegExp::PreparePattern(PandaVector<uint8_t> &pattern, const bool utf16)
{
    if (utf16) {
        return EtsRegExp::ReplaceUnicode(pattern);
    }
    return true;
}

uint8_t EtsRegExp::HexCharToUint(const char &chr)
{
    uint8_t result = 0U;
    if (chr < 'A') {
        result = static_cast<uint8_t>(chr - '0');
    } else if (chr < 'a') {
        result = static_cast<uint8_t>(chr - 'A');
    } else {
        result = static_cast<uint8_t>(chr - 'a');
    }
    return result;
}

bool EtsRegExp::IsHex(const char &chr)
{
    return (chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
}

PandaVector<uint8_t> EtsRegExp::ReadLongUnicode(const PandaVector<uint8_t> &pattern, size_t &readingCurrent)
{
    PandaVector<uint8_t> bytes;
    while (pattern[readingCurrent] != '}') {
        auto &currentByte = pattern[readingCurrent];
        if (currentByte == 0 || pattern[readingCurrent + 1] != 0) {
            return {};
        }
        if (IsHex(currentByte)) {
            bytes.push_back(currentByte);
        }
        readingCurrent += UTF16_WIDTH;
    }
    readingCurrent += UTF16_WIDTH;
    return bytes;
}

PandaVector<uint8_t> EtsRegExp::ReadShortUnicode(const PandaVector<uint8_t> &pattern, size_t &readingCurrent)
{
    PandaVector<uint8_t> bytes;
    for (size_t i = 0; i < DIRECT_UNICODE_BYTES; i++) {
        auto &currentByte = pattern[readingCurrent];
        if (currentByte == 0 || pattern[readingCurrent + 1] != 0) {
            return {};
        }
        if (IsHex(currentByte)) {
            bytes.push_back(currentByte);
        }
        readingCurrent += UTF16_WIDTH;
    }
    return bytes;
}

PandaVector<uint8_t> EtsRegExp::ReadUnicodeChars(const PandaVector<uint8_t> &pattern, size_t &readingCurrent)
{
    if (pattern[readingCurrent] == '{' && pattern[readingCurrent + 1] == 0) {
        return ReadLongUnicode(pattern, readingCurrent);
    }
    return ReadShortUnicode(pattern, readingCurrent);
}

uint64_t EtsRegExp::GetCodeFromHex(const PandaVector<uint8_t> &bytes)
{
    uint64_t code = 0U;
    for (const auto &byte : bytes) {
        code <<= UTF16_HEX_SHIFT;
        code += HexCharToUint(byte);
    }
    return code;
}

bool EtsRegExp::ReplaceUnicode(PandaVector<uint8_t> &pattern)
{
    size_t readingCurrent = 0U;
    size_t writingCurrent = 0U;
    while (pattern[readingCurrent] != 0 || pattern[readingCurrent + 1] != 0) {
        if (pattern[readingCurrent] == '\\' &&
            (pattern[readingCurrent + UTF16_WIDTH] == 'u' || pattern[readingCurrent + UTF16_WIDTH] == 'U')) {
            readingCurrent += UTF16_WIDTH + UTF16_WIDTH;
            auto bytes = ReadUnicodeChars(pattern, readingCurrent);
            if (bytes.empty()) {
                return false;
            }
            auto code = GetCodeFromHex(bytes);
            if (code >= UTF16_TECHNICAL_CODE_START && code <= UTF16_TECHNICAL_CODE_END) {
                return false;
            }
            if (code < UTF16_BYTE5_START) {
                pattern[writingCurrent] = static_cast<uint8_t>(code >> UTF16_BYTE_SHIFT);
                pattern[writingCurrent + 1] = static_cast<uint8_t>(code & UTF16_BYTE_MAX);
                writingCurrent += UTF16_WIDTH;
            } else if (code >= UTF16_BYTE5_MIN) {
                code -= UTF16_BYTE5_START;
                pattern[writingCurrent] =
                    UTF16_BYTE5_ZONE1_START + static_cast<uint8_t>(code >> UTF16_BYTE5_ZONE1_BYTE1_SHIFT);
                pattern[writingCurrent + 1] =
                    static_cast<uint8_t>((code >> UTF16_BYTE5_ZONE1_BYTE2_SHIFT) & UTF16_BYTE_MAX);
                writingCurrent += UTF16_WIDTH;
                pattern[writingCurrent] =
                    UTF16_BYTE5_ZONE2_START +
                    static_cast<uint8_t>((code >> UTF16_BYTE5_ZONE2_BYTE1_SHIFT) & UTF16_BYTE5_ZONE2_BYTE1_NEEDED_BITS);
                pattern[writingCurrent + 1] = static_cast<uint8_t>(code & UTF16_BYTE_MAX);
                writingCurrent += UTF16_WIDTH;
            } else {
                return false;
            }
        } else {
            pattern[writingCurrent] = pattern[readingCurrent];
            pattern[writingCurrent + 1] = pattern[readingCurrent + 1];
            writingCurrent += UTF16_WIDTH;
        }
        readingCurrent += UTF16_WIDTH;
    }
    if (writingCurrent != readingCurrent) {
        pattern[writingCurrent] = 0;
        pattern[writingCurrent + 1] = 0;
        writingCurrent += UTF16_WIDTH;
        pattern.resize(writingCurrent);
    }
    return true;
}

}  // namespace ark::ets