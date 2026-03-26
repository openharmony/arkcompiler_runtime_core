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

#include "plugins/ets/runtime/intrinsics/helpers/regexp_preparser.h"

#include "libarkbase/macros.h"

namespace ark::ets::intrinsics::helpers {
namespace {

constexpr uint32_t DIG_COUNT = 10U;

bool IsHexDigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool IsSimpleQuantifier(char c)
{
    return c == '*' || c == '+' || c == '?';
}

bool IsPropertyNameChar(char c)
{
    return IsAlpha(c) || IsDigit(c) || c == '=' || c == '-';
}

struct ParsedNumber {
    uint32_t value = 0;
    bool hasDigits = false;
};

ParsedNumber ConsumeUint32(std::string_view pattern, size_t &pos)
{
    constexpr uint32_t DECIMAL_BASE = 10U;
    ParsedNumber result;
    while (pos < pattern.size() && IsDigit(pattern[pos])) {
        result.hasDigits = true;
        auto digit = static_cast<uint32_t>(pattern[pos] - '0');  // NOLINT(readability-magic-numbers)
        if (result.value > (UINT32_MAX - digit) / DECIMAL_BASE) {
            result.value = UINT32_MAX;
        } else {
            result.value = result.value * DECIMAL_BASE + digit;
        }
        pos++;
    }
    return result;
}

}  // namespace

JsRegExpPreParser::JsRegExpPreParser(std::string_view pattern, std::string_view flags, bool unicodeMode)
    : pattern_(pattern), flags_(flags), unicodeMode_(unicodeMode)
{
}

bool JsRegExpPreParser::Validate(PandaString &errorMsg)
{
    while (pos_ < pattern_.size()) {
        if (!ValidateNextToken(errorMsg)) {
            return false;
        }
    }
    if (unicodeMode_) {
        for (uint32_t backref : backrefs_) {
            if (backref > capturingGroupCount_) {
                errorMsg = "SyntaxError: Invalid decimal escape";
                return false;
            }
        }
    }
    return true;
}

bool JsRegExpPreParser::ValidateNextToken(PandaString &errorMsg)
{
    ASSERT(pos_ < pattern_.size());
    char c = Peek();
    if (c == '\\') {
        return ConsumeEscapeAsQuantifiable(errorMsg);
    }
    if (c == '[' && !inClass_) {
        return ConsumeCharacterClass(errorMsg);
    }
    if (c == '(' && !inClass_) {
        return ConsumeGroupOpen(errorMsg);
    }
    if (c == ')' && !inClass_) {
        return ConsumeGroupClose(errorMsg);
    }
    if (c == '{' && !inClass_) {
        return ConsumeBraceQuantifier(errorMsg);
    }
    if (IsSimpleQuantifier(c) && !inClass_) {
        return ConsumeSimpleQuantifier(errorMsg);
    }
    if ((c == ']' || c == '}') && !inClass_) {
        return ConsumeLoneBracket(c, errorMsg);
    }

    Advance();
    lastWasQuantifiable_ = true;
    return true;
}

char JsRegExpPreParser::Peek() const
{
    ASSERT(pos_ < pattern_.size());
    return pattern_[pos_];
}

bool JsRegExpPreParser::HasMore() const
{
    return pos_ < pattern_.size();
}

void JsRegExpPreParser::Advance()
{
    ASSERT(pos_ < pattern_.size());
    pos_++;
}

bool JsRegExpPreParser::TryConsume(char expected)
{
    if (HasMore() && Peek() == expected) {
        Advance();
        return true;
    }
    return false;
}

void JsRegExpPreParser::SkipLazyModifier()
{
    TryConsume('?');
}

bool JsRegExpPreParser::ConsumeEscapeAsQuantifiable(PandaString &errorMsg)
{
    if (!ParseEscape(errorMsg)) {
        return false;
    }
    lastWasQuantifiable_ = true;
    return true;
}

bool JsRegExpPreParser::ParseEscape(PandaString &errorMsg)
{
    ASSERT(Peek() == '\\');
    Advance();
    if (!HasMore()) {
        errorMsg = "SyntaxError: Unterminated escape sequence";
        return false;
    }

    char c = Peek();
    if (unicodeMode_ && IsDigit(c)) {
        return ParseDecimalEscapeUnicode(errorMsg);
    }
    if (c == 'p' || c == 'P') {
        return ParsePropertyEscape(errorMsg);
    }
    if (c == 'u') {
        return ParseUnicodeEscape(errorMsg);
    }
    if (c == 'x') {
        return ParseHexEscape(errorMsg);
    }

    Advance();
    return true;
}

bool JsRegExpPreParser::ParseDecimalEscapeUnicode(PandaString &errorMsg)
{
    ASSERT(unicodeMode_ && IsDigit(Peek()));
    char c = Peek();
    if (c == '0') {
        Advance();
        if (HasMore() && IsDigit(Peek())) {
            errorMsg = "SyntaxError: Invalid decimal escape";
            return false;
        }
        return true;
    }

    Advance();
    auto backrefValue = static_cast<uint32_t>(c - '0');
    while (HasMore() && IsDigit(Peek())) {
        backrefValue = backrefValue * DIG_COUNT + static_cast<uint32_t>(Peek() - '0');
        Advance();
    }
    backrefs_.push_back(backrefValue);
    return true;
}

bool JsRegExpPreParser::ParsePropertyEscape(PandaString &errorMsg)
{
    ASSERT(Peek() == 'p' || Peek() == 'P');
    Advance();
    if (!HasMore() || Peek() != '{') {
        errorMsg = "SyntaxError: Invalid property escape";
        return false;
    }
    Advance();
    if (!HasMore() || Peek() == '}') {
        errorMsg = "SyntaxError: Invalid property escape";
        return false;
    }

    while (HasMore() && Peek() != '}') {
        if (!IsPropertyNameChar(Peek())) {
            errorMsg = "SyntaxError: Invalid property escape";
            return false;
        }
        Advance();
    }

    if (!HasMore()) {
        errorMsg = "SyntaxError: Invalid property escape";
        return false;
    }
    Advance();
    return true;
}

bool JsRegExpPreParser::ParseUnicodeEscape(PandaString &errorMsg)
{
    ASSERT(Peek() == 'u');
    Advance();
    if (!HasMore()) {
        errorMsg = "SyntaxError: Invalid Unicode escape";
        return false;
    }

    if (Peek() == '{') {
        return ParseBracedUnicodeEscape(errorMsg);
    }
    return ConsumeExactHexDigits(4U, "SyntaxError: Invalid Unicode escape", errorMsg);
}

bool JsRegExpPreParser::ParseBracedUnicodeEscape(PandaString &errorMsg)
{
    ASSERT(Peek() == '{');
    Advance();
    if (!HasMore() || Peek() == '}') {
        errorMsg = "SyntaxError: Invalid Unicode escape";
        return false;
    }

    size_t hexCount = 0;
    while (HasMore() && Peek() != '}') {
        if (!IsHexDigit(Peek())) {
            errorMsg = "SyntaxError: Invalid Unicode escape";
            return false;
        }
        hexCount++;
        Advance();
    }

    if (!HasMore() || hexCount == 0) {
        errorMsg = "SyntaxError: Invalid Unicode escape";
        return false;
    }
    Advance();
    return true;
}

bool JsRegExpPreParser::ParseHexEscape(PandaString &errorMsg)
{
    ASSERT(Peek() == 'x');
    Advance();
    if (!unicodeMode_) {
        // In non-unicode mode, \x without exactly 2 hex digits is an IdentityEscape matching 'x'
        size_t savedPos = pos_;
        if (HasMore() && IsHexDigit(Peek())) {
            Advance();
            if (HasMore() && IsHexDigit(Peek())) {
                Advance();
                return true;  // Valid \xHH
            }
        }
        pos_ = savedPos;  // Rollback: treat as identity escape
        return true;
    }
    return ConsumeExactHexDigits(2U, "SyntaxError: Invalid hex escape", errorMsg);
}

bool JsRegExpPreParser::ConsumeExactHexDigits(size_t count, const char *msg, PandaString &errorMsg)
{
    for (size_t i = 0; i < count; i++) {
        if (!HasMore() || !IsHexDigit(Peek())) {
            errorMsg = msg;
            return false;
        }
        Advance();
    }
    return true;
}

bool JsRegExpPreParser::ConsumeCharacterClass(PandaString &errorMsg)
{
    ASSERT(Peek() == '[' && !inClass_);
    Advance();
    inClass_ = true;
    while (HasMore() && Peek() != ']') {
        if (Peek() == '\\') {
            if (!ParseEscape(errorMsg)) {
                inClass_ = false;
                return false;
            }
            continue;
        }
        Advance();
    }

    inClass_ = false;
    if (!HasMore()) {
        errorMsg = "SyntaxError: Unterminated character class";
        return false;
    }
    Advance();
    lastWasQuantifiable_ = true;
    return true;
}

bool JsRegExpPreParser::ConsumeGroupOpen([[maybe_unused]] PandaString &errorMsg)
{
    ASSERT(Peek() == '(' && !inClass_);
    Advance();
    parenDepth_++;
    bool isCapturing = ParseGroupSpecifier();
    if (isCapturing) {
        capturingGroupCount_++;
    }
    lastWasQuantifiable_ = true;
    return true;
}

bool JsRegExpPreParser::ConsumeGroupClose(PandaString &errorMsg)
{
    ASSERT(Peek() == ')' && !inClass_);
    if (parenDepth_ == 0) {
        errorMsg = "SyntaxError: Unmatched ')'";
        return false;
    }
    parenDepth_--;
    Advance();
    lastWasQuantifiable_ = true;
    return true;
}

bool JsRegExpPreParser::ParseGroupSpecifier()
{
    if (!HasMore() || Peek() != '?') {
        return true;
    }
    Advance();
    if (!HasMore()) {
        return true;
    }

    char next = Peek();
    if (next == ':') {
        Advance();
        return false;
    }
    if (next == '=' || next == '!') {
        Advance();
        return false;
    }
    if (next == '<') {
        Advance();
        if (HasMore() && (Peek() == '=' || Peek() == '!')) {
            Advance();
            return false;
        }
        return true;
    }
    if (next == 'P' || next == 'i' || next == 'm' || next == 's' || next == 'x') {
        Advance();
    }
    return true;
}

bool JsRegExpPreParser::ConsumeLoneBracket(char c, PandaString &errorMsg)
{
    ASSERT((c == ']' || c == '}') && !inClass_);
    if (unicodeMode_) {
        errorMsg = "SyntaxError: Unmatched '";
        errorMsg += c;
        errorMsg += "'";
        return false;
    }
    Advance();
    lastWasQuantifiable_ = true;
    return true;
}

bool JsRegExpPreParser::ConsumeSimpleQuantifier(PandaString &errorMsg)
{
    ASSERT(IsSimpleQuantifier(Peek()) && !inClass_);
    if (!lastWasQuantifiable_) {
        errorMsg = "SyntaxError: Nothing to repeat";
        return false;
    }
    Advance();
    SkipLazyModifier();
    lastWasQuantifiable_ = false;
    return true;
}

bool JsRegExpPreParser::ConsumeBraceQuantifier(PandaString &errorMsg)
{
    ASSERT(Peek() == '{' && !inClass_);
    Advance();
    size_t rollbackPos = pos_;
    ParsedNumber minNum = ConsumeUint32(pattern_, pos_);
    if (!minNum.hasDigits) {
        return HandleMalformedQuantifier(rollbackPos, errorMsg);
    }
    if (!HasMore()) {
        return HandleMalformedQuantifier(rollbackPos, errorMsg);
    }

    uint32_t maxValue = minNum.value;
    bool hasComma = TryConsume(',');
    if (hasComma) {
        ParsedNumber maxNum = ConsumeUint32(pattern_, pos_);
        maxValue = maxNum.hasDigits ? maxNum.value : UINT32_MAX;
    }

    if (!HasMore() || Peek() != '}') {
        return HandleMalformedQuantifier(rollbackPos, errorMsg);
    }
    if (hasComma && maxValue != UINT32_MAX && minNum.value > maxValue) {
        errorMsg = CreateQuantifierOutOfOrderError();
        return false;
    }

    Advance();
    SkipLazyModifier();
    lastWasQuantifiable_ = false;
    return true;
}

bool JsRegExpPreParser::HandleMalformedQuantifier(size_t rollbackPos, PandaString &errorMsg)
{
    if (!unicodeMode_) {
        pos_ = rollbackPos;
        lastWasQuantifiable_ = true;
        return true;
    }
    errorMsg = "SyntaxError: Incomplete quantifier";
    return false;
}

PandaString JsRegExpPreParser::CreateQuantifierOutOfOrderError() const
{
    PandaString errorMsg = "Invalid regular expression: /";
    errorMsg.append(pattern_.data(), pattern_.size());
    errorMsg += "/";
    errorMsg.append(flags_.data(), flags_.size());
    errorMsg += ": numbers out of order in {} quantifier";
    return errorMsg;
}

}  // namespace ark::ets::intrinsics::helpers
