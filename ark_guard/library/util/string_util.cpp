/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "string_util.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <unordered_map>
#include <vector>

#include "assert_util.h"
#include "logger.h"
#include "configuration/configuration_constants.h"

namespace {
constexpr int BRACE_SIZE = 2;  // 2 is the size of brace characters

bool ContainRegexSymbol(const std::string &str)
{
    const std::string regexSymbols = R"(\.^$-+()[]{}|?*)";
    for (const auto &c : str) {
        if (regexSymbols.find(c) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Write to output buffer to avoid allocation
static void EscapeParenthesesToBuffer(std::string &out, const std::string &str)
{
    out.clear();
    out.reserve(str.size() + BRACE_SIZE);
    for (const char c : str) {
        if (c == '{' || c == '}') {
            out += '\\';
        }
        out += c;
    }
}

std::string EscapeParentheses(const std::string &str)
{
    std::string result;
    EscapeParenthesesToBuffer(result, str);
    return result;
}

// Write to output buffer to avoid allocation
static void RemoveSpacesToBuffer(std::string &out, const std::string &str)
{
    out = str;
    out.erase(std::remove(out.begin(), out.end(), ' '), out.end());
}

std::string RemoveSpaces(const std::string &str)
{
    std::string result;
    RemoveSpacesToBuffer(result, str);
    return result;
}

// Lightweight: replace single dots with "\\.", keep multiple dots as-is (no regex)
std::string ReplaceDot(const std::string &str)
{
    std::string result;
    result.reserve(str.size() + 8);  // 8 is the buffer size for escaped dots
    for (size_t i = 0; i < str.size();) {
        if (str[i] != '.') {
            result += str[i++];
            continue;
        }
        size_t dotCount = 0;
        while (i < str.size() && str[i] == '.') {
            ++dotCount;
            ++i;
        }
        if (dotCount == 1) {
            result.append("\\.");
        } else {
            result.append(dotCount, '.');
        }
    }
    return result;
}

// Lightweight: replace <N> with matchedPatterns[N-1] (no regex, no substr)
static int ParseBackrefNum(const std::string &str, size_t start, size_t end)
{
    int val = 0;
    for (size_t k = start; k < end; ++k) {
        val = val * 10 + static_cast<int>(str[k] - '0');  // 10 is the base of decimal number
    }
    return val;
}

std::string ReplaceNumericWildcards(const std::string &str, const std::vector<std::string> &matchedPatterns)
{
    std::string result;
    result.reserve(str.size());
    size_t i = 0;
    while (i < str.size()) {
        if (str[i] != '<') {
            result += str[i++];
            continue;
        }
        if (i + 1 >= str.size() || !std::isdigit(static_cast<unsigned char>(str[i + 1]))) {
            result += str[i++];
            continue;
        }
        size_t j = i + 1;
        while (j < str.size() && std::isdigit(static_cast<unsigned char>(str[j]))) {
            ++j;
        }
        if (j >= str.size() || str[j] != '>') {
            result += str[i++];
            continue;
        }
        int matchNum = ParseBackrefNum(str, i + 1, j);
        // CC-OFFNXT(G.STD.16) fatal error
        ARK_GUARD_ASSERT(
            matchNum <= 0, ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
            "ClassSpecification parsing failed: Back reference can not be 0, found invalid back reference <" +
                str.substr(i + 1, j - i - 1) + ">");
        size_t num = static_cast<size_t>(matchNum - 1);
        // CC-OFFNXT(G.FUN.01) fatal error
        if (num < matchedPatterns.size()) {
            result.append(matchedPatterns[num]);
        }
        i = j + 1;
    }
    return result;
}

std::string ReplaceWildcardToRegex(const std::vector<std::pair<std::string, std::string>> &wildcardRules,
                                   const std::string &input)
{
    std::string result;
    const auto replaceDot = ReplaceDot(input);
    const std::string_view sv(replaceDot);
    size_t i = 0;
    std::vector<std::string> matchedPatterns;
    while (i < sv.size()) {
        bool matched = false;
        for (const auto &[pattern, replace] : wildcardRules) {
            const auto patternLen = pattern.size();
            if (i + patternLen <= sv.size() && sv.substr(i, patternLen) == pattern) {
                result += replace;
                matchedPatterns.emplace_back(replace);
                i += patternLen;
                matched = true;
                break;
            }
        }
        if (!matched) {
            result += sv[i];
            ++i;
        }
    }
    return ReplaceNumericWildcards(result, matchedPatterns);
}

static bool MatchSimpleWildcardRegexImpl(std::string_view str, std::string_view pat);

static bool IsValidEscapedChar(char c)
{
    return c == '.' || c == '(' || c == ')' || c == '{' || c == '}';
}

static bool IsValidSpecialChar(char c)
{
    return c == '(' || c == '|' || c == '+' || c == '{' || c == '}' || c == '?' || c == '^' || c == '$';
}

static bool CheckEscapedChar(std::string_view pattern, size_t &i)
{
    if (i + 1 >= pattern.size()) {
        return false;
    }
    char c = pattern[i + 1];
    if (!IsValidEscapedChar(c)) {
        return false;
    }
    i += 2;  // 2 is the length of escaped character sequence
    return true;
}

static bool CheckDotStar(std::string_view pattern, size_t &i)
{
    if (i + 1 < pattern.size() && pattern[i + 1] == '*') {
        i += 2;  // 2 is the length of escaped character sequence
        return true;
    }
    return false;
}

static bool CheckBracketPattern(std::string_view pattern, size_t &i)
{
    if (pattern.substr(i).compare(0, 7, "[^/\\.]*") == 0) {  // 7 is the length of "[^/\\.]*"
        i += 7;
        return true;
    }
    if (pattern.substr(i).compare(0, 6, "[^/\\.]") == 0) {  // 6 is the length of "[^/\\.]"
        i += 6;
        return true;
    }
    return false;
}

static bool IsSimpleWildcardRegexPattern(std::string_view pattern)
{
    for (size_t i = 0; i < pattern.size();) {
        if (pattern[i] == '\\') {
            if (!CheckEscapedChar(pattern, i)) {
                return false;
            }
            continue;
        }
        if (pattern[i] == '.') {
            if (!CheckDotStar(pattern, i)) {
                return false;
            }
            continue;
        }
        if (pattern[i] == '[') {
            if (!CheckBracketPattern(pattern, i)) {
                return false;
            }
            continue;
        }
        if (IsValidSpecialChar(pattern[i])) {
            return false;
        }
        ++i;
    }
    return true;
}

static bool IsEscapedChar(char c)
{
    return c == '.' || c == '(' || c == ')' || c == '{' || c == '}';
}

static bool MatchEscapedPattern(std::string_view str, std::string_view pat)
{
    char esc = pat[1];
    if (!IsEscapedChar(esc)) {
        return false;
    }
    if (str.empty() || str[0] != esc) {
        return false;
    }
    return MatchSimpleWildcardRegexImpl(str.substr(1), pat.substr(2));  // 2 is length of escaped character sequence
}

static bool MatchNotSlashDotPattern(std::string_view str, std::string_view pat)
{
    if (str.empty() || str[0] == '/' || str[0] == '.') {
        return false;
    }
    return MatchSimpleWildcardRegexImpl(str.substr(1), pat.substr(6));  // 6 is the length of "[^/\\.]"
}

static bool MatchSimpleWildcardRegexImpl(std::string_view str, std::string_view pat)
{
    if (pat.empty()) {
        return str.empty();
    }
    if (pat.size() >= 2 && pat.compare(0, 2, ".*") == 0) {  // 2 is the length of ".*"
        for (size_t k = 0; k <= str.size(); ++k) {
            if (MatchSimpleWildcardRegexImpl(str.substr(k), pat.substr(2))) {  // 2 is the length of ".*"
                return true;
            }
        }
        return false;
    }
    if (pat.size() >= 7 && pat.compare(0, 7, "[^/\\.]*") == 0) {  // 7 is the length of "[^/\\.]*"
        for (size_t k = 0;; ++k) {
            if (MatchSimpleWildcardRegexImpl(str.substr(k), pat.substr(7))) {  // 7 is the length of "[^/\\.]*"
                return true;
            }
            if (k >= str.size() || str[k] == '/' || str[k] == '.') {
                break;
            }
        }
        return false;
    }
    if (pat.size() >= 6 && pat.compare(0, 6, "[^/\\.]") == 0) {  // 6 is the length of "[^/\\.]"
        return MatchNotSlashDotPattern(str, pat);
    }
    if (pat[0] == '\\' && pat.size() >= 2) {  // 2 is the length of escaped character sequence
        return MatchEscapedPattern(str, pat);
    }
    if (!str.empty() && pat[0] == str[0]) {
        return MatchSimpleWildcardRegexImpl(str.substr(1), pat.substr(1));
    }
    return false;
}

static bool MatchSimpleWildcardRegex(const std::string &str, const std::string &pattern)
{
    return MatchSimpleWildcardRegexImpl(std::string_view(str), std::string_view(pattern));
}

// Cache for compiled regexes - use optimize to reduce regex compilation cost
static const std::regex &GetCachedRegex(const std::string &regexStr)
{
    static constexpr size_t maxCacheSize = 256;  // 256 is the maximum cache size limit
    static std::unordered_map<std::string, std::regex> cache;
    static std::vector<std::string> keyOrder;  // For simple FIFO eviction

    auto it = cache.find(regexStr);
    if (it != cache.end()) {
        return it->second;
    }
    if (cache.size() >= maxCacheSize) {
        cache.erase(keyOrder.front());
        keyOrder.erase(keyOrder.begin());
    }
    const auto flags = std::regex_constants::ECMAScript | std::regex_constants::optimize;
    auto [inserted, _] = cache.emplace(regexStr, std::regex(regexStr, flags));
    keyOrder.push_back(regexStr);
    return inserted->second;
}

}  // namespace

std::string ark::guard::StringUtil::ReplaceSlashWithDot(const std::string &str)
{
    std::string result = str;
    std::replace(result.begin(), result.end(), '/', '.');
    return result;
}

std::vector<std::string> ark::guard::StringUtil::ReplaceSlashWithDot(const std::vector<std::string> &strs)
{
    std::vector<std::string> result;
    result.reserve(strs.size());
    for (const auto &str : strs) {
        result.emplace_back(ReplaceSlashWithDot(str));
    }
    return result;
}

bool ark::guard::StringUtil::IsSubStrMatched(const std::string &str, const std::string &subStr, size_t start /* = 0 */)
{
    if (start > str.size() || start + subStr.size() > str.size()) {
        return false;
    }
    return str.compare(start, subStr.size(), subStr) == 0;
}

bool ark::guard::StringUtil::IsMatch(const std::string &inputStr, const std::string &pattern)
{
    if (pattern.empty()) {
        return false;
    }
    thread_local std::string g_scratchInput;
    thread_local std::string g_scratchPattern;
    thread_local std::string g_scratchEscaped;

    RemoveSpacesToBuffer(g_scratchInput, inputStr);
    RemoveSpacesToBuffer(g_scratchPattern, pattern);
    if (!ContainRegexSymbol(g_scratchPattern)) {
        return g_scratchInput == g_scratchPattern;
    }
    EscapeParenthesesToBuffer(g_scratchEscaped, g_scratchPattern);
    if (IsSimpleWildcardRegexPattern(g_scratchEscaped)) {
        return MatchSimpleWildcardRegex(g_scratchInput, g_scratchEscaped);
    }
    return std::regex_match(g_scratchInput, GetCachedRegex(g_scratchEscaped));
}

bool ark::guard::StringUtil::RegexMatch(const std::string &str, const std::string &regexPattern)
{
    if (regexPattern.empty()) {
        return false;
    }
    // RegexMatch handles user-provided patterns (e.g. universalReservedFileNames "x[a-z]x")
    // which can be arbitrary ECMAScript regex - must use std::regex, not simple wildcard
    return std::regex_match(str, GetCachedRegex(regexPattern));
}

std::string ark::guard::StringUtil::ConvertWildcardToRegex(const std::string &input)
{
    static const std::vector<std::pair<std::string, std::string>> wildcardRules = {
        {"**", ".*"}, {"*", "[^/\\.]*"}, {"?", "[^/\\.]"}};
    return ReplaceWildcardToRegex(wildcardRules, input);
}

std::string ark::guard::StringUtil::ConvertWildcardToRegexEx(const std::string &input)
{
    static const std::vector<std::pair<std::string, std::string>> wildcardRules = {
        {"***", ".*"},
        {"...", ".*"},
        {"**", ".*"},
        {"*", "[^/\\.]*"},
        {"?", "[^/\\.]"},
        {"%",
         "(f32|f64|i8|i16|i32|i64|i32|i64|u16|u1|std\\.core\\.String|std\\.core\\.BigInt|void|std\\.core\\.Null|std\\."
         "core\\.Object|std\\.core\\.Int|std\\.core\\.Byte|std\\.core\\.Short|std\\.core\\.Long|std\\.core\\.Double|"
         "std\\.core\\.Float|std\\.core\\.Char|std\\.core\\.Boolean)"}};
    return ReplaceWildcardToRegex(wildcardRules, input);
}

std::string ark::guard::StringUtil::RemovePrefixIfMatches(const std::string &inputStr, const std::string &prefix)
{
    ARK_GUARD_ASSERT(prefix.empty(), ErrorCode::GENERIC_ERROR, "remove invalid prefix to: " + inputStr);
    std::string_view sv(inputStr);
    if (sv.size() >= prefix.size() && sv.substr(0, prefix.size()) == prefix) {
        return std::string(sv.substr(prefix.size()));
    }
    return inputStr;
}

std::string ark::guard::StringUtil::EnsureStartWithPrefix(const std::string &inputStr, const std::string &prefix)
{
    if (inputStr.length() < prefix.length() || inputStr.substr(0, prefix.size()) != prefix) {
        return prefix + inputStr;
    }
    return inputStr;
}

std::string ark::guard::StringUtil::EnsureEndWithSuffix(const std::string &inputStr, const std::string &suffix)
{
    if (inputStr.length() < suffix.length() || inputStr.substr(inputStr.length() - suffix.length()) != suffix) {
        return inputStr + suffix;
    }
    return inputStr;
}

size_t ark::guard::StringUtil::FindLongestMatchedPrefix(const std::vector<std::string> &prefixes,
                                                        const std::string &input)
{
    size_t maxLength = 0;
    for (const auto &prefix : prefixes) {
        if (prefix.size() > maxLength && IsSubStrMatched(input, prefix)) {
            maxLength = prefix.size();
        }
    }
    return maxLength;
}

size_t ark::guard::StringUtil::FindLongestMatchedPrefixReg(const std::vector<std::string> &prefixes,
                                                           const std::string &input)
{
    size_t maxLength = 0;
    std::smatch match;
    for (const auto &regexStr : prefixes) {
        const auto &pattern = GetCachedRegex(regexStr);
        if (std::regex_search(input, match, pattern)) {
            maxLength = std::max(maxLength, static_cast<size_t>(match[0].length()));
        }
    }
    return maxLength;
}

std::string ark::guard::StringUtil::RemoveAllSpaces(const std::string &input)
{
    std::string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}

void ark::guard::StringUtil::WarmupRegexCache(const std::vector<std::string> &patterns)
{
    for (const auto &p : patterns) {
        if (!p.empty()) {
            GetCachedRegex(p);
        }
    }
}

void ark::guard::StringUtil::WarmupRegexCacheForIsMatch(const std::vector<std::string> &patterns)
{
    for (const auto &p : patterns) {
        if (p.empty()) {
            continue;
        }
        const std::string processed = PreparePatternForMatch(p);
        GetCachedRegex(processed);
    }
}

std::string ark::guard::StringUtil::PreparePatternForMatch(const std::string &pattern)
{
    return EscapeParentheses(RemoveSpaces(pattern));
}