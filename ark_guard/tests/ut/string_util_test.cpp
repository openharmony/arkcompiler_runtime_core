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

#include <gtest/gtest.h>

#include "util/string_util.h"

using namespace testing::ext;

/**
 * @tc.name: string_util_test_001
 * @tc.desc: test whether names with wildcard characters can be successfully converted using conversion functions.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_001, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> wildcardMap = {
        {"?", "[^/\\.]"},
        {"*", "[^/\\.]*"},
        {"**", ".*"},
        {"com.example.Test?", "com\\.example\\.Test[^/\\.]"},
        {"com.example.*Test*", "com\\.example\\.[^/\\.]*Test[^/\\.]*"},
        {"com.example.**Test", "com\\.example\\..*Test"},
        {"com.example.*Test<2>", "com\\.example\\.[^/\\.]*Test"},
        {"com.example.*Te?st<2>", "com\\.example\\.[^/\\.]*Te[^/\\.]st[^/\\.]"},
    };

    for (const auto &[pattern, expect] : wildcardMap) {
        auto result = ark::guard::StringUtil::ConvertWildcardToRegex(pattern);
        EXPECT_EQ(result, expect);
    }
}

/**
 * @tc.name: string_util_test_002
 * @tc.desc: test whether names with wildcard characters can be successfully converted using conversion functions.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_002, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> wildcardMap = {
        {"***", ".*"},
        {"...", ".*"},
        {"**", ".*"},
        {"*", "[^/\\.]*"},
        {"?", "[^/\\.]"},
        {"%",
         "(f32|f64|i8|i16|i32|i64|i32|i64|u16|u1|std\\.core\\.String|std\\.core\\.BigInt|void|std\\.core\\.Null|std\\."
         "core\\.Object|std\\.core\\.Int|std\\.core\\.Byte|std\\.core\\.Short|std\\.core\\.Long|std\\.core\\.Double|"
         "std\\.core\\.Float|std\\.core\\.Char|std\\.core\\.Boolean)"},
        {"Test?", "Test[^/\\.]"},
        {"*Test*", "[^/\\.]*Test[^/\\.]*"},
        {"Test**Test", "Test.*Test"},
        {"Test.*Test<2>", "Test\\.[^/\\.]*Test"},
        {"int,%",
         "int,(f32|f64|i8|i16|i32|i64|i32|i64|u16|u1|std\\.core\\.String|std\\.core\\.BigInt|void|std\\.core\\.Null|"
         "std\\."
         "core\\.Object|std\\.core\\.Int|std\\.core\\.Byte|std\\.core\\.Short|std\\.core\\.Long|std\\.core\\.Double|"
         "std\\.core\\.Float|std\\.core\\.Char|std\\.core\\.Boolean)"},
        {"(...)", "(.*)"},
        {"()***", "().*"},
    };

    for (const auto &[pattern, expect] : wildcardMap) {
        auto result = ark::guard::StringUtil::ConvertWildcardToRegexEx(pattern);
        EXPECT_EQ(result, expect);
    }
}

/**
 * @tc.name: string_util_test_003
 * @tc.desc: test back reference is 0
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_003, TestSize.Level0)
{
    std::string wildcardStr = "Test.*Test<0>";
    EXPECT_DEATH(ark::guard::StringUtil::ConvertWildcardToRegex(wildcardStr), "");
    EXPECT_DEATH(ark::guard::StringUtil::ConvertWildcardToRegexEx(wildcardStr), "");
}

/**
 * @tc.name: string_util_test_004
 * @tc.desc: test prefix remove function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_004, TestSize.Level0)
{
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("HelloWorld", "Hello"), "World");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("HelloWorld", "hi"), "HelloWorld");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("", "prefix"), "");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("prefix", "prefix"), "");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("pre", "prefix"), "pre");
}

/**
 * @tc.name: string_util_test_005
 * @tc.desc: test ensure start/end function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_005, TestSize.Level0)
{
    // EnsureStartWithPrefix
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("prefix_string", "prefix"), "prefix_string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("string", "prefix_"), "prefix_string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("", "prefix"), "prefix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("string", ""), "string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("pre_string", "prefix"), "prefixpre_string");
    // EnsureEndWithSuffix
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string_suffix", "suffix"), "string_suffix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string", "_suffix"), "string_suffix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("", "suffix"), "suffix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string", ""), "string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string_suf", "suffix"), "string_sufsuffix");
}

/**
 * @tc.name: string_util_test_006
 * @tc.desc: test FindLongestMatchedPrefix function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_006, TestSize.Level0)
{
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"hello", "hi"}, "hello world"), 5);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"h", "he", "hel", "hello"}, "hello world"), 5);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"foo", "bar"}, "hello world"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"prefix"}, ""), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({""}, "string"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({}, "string"), 0);
}

/**
 * @tc.name: string_util_test_007
 * @tc.desc: test FindLongestMatchedPrefixReg function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_007, TestSize.Level0)
{
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^h.*o", "^hi.*"}, "hello world"), 8);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^h", "^he.*", "^hel+"}, "hello world"), 11);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^h\\d+", "^h[a-z]+"}, "h123abc"), 4);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^foo", "^bar"}, "hello world"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^[A-Z][a-z]+", "^[a-z]{2,5}"}, "HelloWorld"), 5);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^prefix"}, ""), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({""}, "string"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({}, "string"), 0);
}

/**
 * @tc.name: string_util_test_008
 * @tc.desc: test RemoveAllSpaces function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_008, TestSize.Level0)
{
    std::string origin = " {Uxxx1, xxx2 } ";
    std::string expect = "{Uxxx1,xxx2}";
    std::string result = ark::guard::StringUtil::RemoveAllSpaces(origin);
    EXPECT_EQ(result, expect);
}

/**
 * @tc.name: IsMatch_test_001
 * @tc.desc: test IsMatch function with plain string matching
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, IsMatch_test_001, TestSize.Level0)
{
    // Plain string matching (no regex symbols)
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", "test"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("hello world", "hello world"));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", "test1"));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test1", "test"));
    // Empty pattern should return false
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", ""));
}

/**
 * @tc.name: IsMatch_test_002
 * @tc.desc: test IsMatch function with spaces handling
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, IsMatch_test_002, TestSize.Level0)
{
    // Spaces should be removed before matching
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test string", "teststring"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("teststring", "test string"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test  string", "test string"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", "t e s t"));
}

/**
 * @tc.name: IsMatch_test_003
 * @tc.desc: test IsMatch function with regex patterns converted from wildcards
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, IsMatch_test_003, TestSize.Level0)
{
    // IsMatch expects regex patterns, not wildcard patterns
    // Use ConvertWildcardToRegex to convert wildcards to regex first
    std::string pattern1 = ark::guard::StringUtil::ConvertWildcardToRegex("test*");
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", pattern1));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123", pattern1));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test/file", pattern1));  // Contains /
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test.file", pattern1));  // Contains .

    std::string pattern2 = ark::guard::StringUtil::ConvertWildcardToRegex("test**");
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.file", pattern2));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", pattern2));

    std::string pattern3 = ark::guard::StringUtil::ConvertWildcardToRegex("test?");
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("testa", pattern3));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", pattern3));  // Needs one character
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("testab", pattern3));  // Two characters

    std::string pattern4 = ark::guard::StringUtil::ConvertWildcardToRegex("test??");
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", pattern4));  // Needs two characters
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("testab", pattern4));
}


/**
 * @tc.name: PreparePatternForMatch_test_001
 * @tc.desc: test PreparePatternForMatch function (EscapeParentheses and RemoveSpaces)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, PreparePatternForMatch_test_001, TestSize.Level0)
{
    // Test PreparePatternForMatch = EscapeParentheses(RemoveSpaces(pattern))
    // Test brace escaping
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("{test}"), "\\{test\\}");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("test{value}"), "test\\{value\\}");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("{a}{b}"), "\\{a\\}\\{b\\}");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("{"), "\\{");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("}"), "\\}");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("{{{}}}"), "\\{\\{\\{\\}\\}\\}");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("a{b}c{d}e"), "a\\{b\\}c\\{d\\}e");

    // Test space removal
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("test string"), "teststring");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("test  string"), "teststring");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch(" test string "), "teststring");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("   "), "");

    // Test combined: braces and spaces
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch(" { test } "), "\\{test\\}");

    // Edge cases
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch("test"), "test");
    EXPECT_EQ(ark::guard::StringUtil::PreparePatternForMatch(""), "");
}

/**
 * @tc.name: ReplaceDot_test_001
 * @tc.desc: test ReplaceDot function through ConvertWildcardToRegex
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, ReplaceDot_test_001, TestSize.Level0)
{
    // Test ReplaceDot through ConvertWildcardToRegex
    // Single dots should be escaped, multiple dots should remain as-is
    std::string result1 = ark::guard::StringUtil::ConvertWildcardToRegex("com.example.Test");
    EXPECT_EQ(result1, "com\\.example\\.Test");

    std::string result2 = ark::guard::StringUtil::ConvertWildcardToRegex("com..example");
    EXPECT_EQ(result2, "com..example");  // Multiple dots remain unchanged

    std::string result3 = ark::guard::StringUtil::ConvertWildcardToRegex("com...example");
    EXPECT_EQ(result3, "com...example");  // Multiple dots remain unchanged

    std::string result4 = ark::guard::StringUtil::ConvertWildcardToRegex("test");
    EXPECT_EQ(result4, "test");  // No dots

    std::string result5 = ark::guard::StringUtil::ConvertWildcardToRegex(".");
    EXPECT_EQ(result5, "\\.");  // Single dot escaped
}

/**
 * @tc.name: ReplaceNumericWildcards_test_001
 * @tc.desc: test ReplaceNumericWildcards function through ConvertWildcardToRegex
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, ReplaceNumericWildcards_test_001, TestSize.Level0)
{
    // Test ReplaceNumericWildcards through ConvertWildcardToRegex
    // <N> should be replaced with the Nth matched pattern (1-indexed)
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("com.example.*Test<1>"),
              "com\\.example\\.[^/\\.]*Test[^/\\.]*");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("com.*Test*<2>"),
              "com\\.[^/\\.]*Test[^/\\.]*[^/\\.]*");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("com.example.*Te?st<2>"),
              "com\\.example\\.[^/\\.]*Te[^/\\.]st[^/\\.]");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("test*<1>*<2>"),
              "test[^/\\.]*[^/\\.]*[^/\\.]*[^/\\.]*");

    // Edge cases: no wildcards before back reference
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("<1>test"), "test");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("test<1>middle"), "testmiddle");

    // Edge cases: out of range or invalid format
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("test*<10>"), "test[^/\\.]*");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("test*<100>"), "test[^/\\.]*");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("test*<1"), "test[^/\\.]*<1");
    EXPECT_EQ(ark::guard::StringUtil::ConvertWildcardToRegex("test*<>"), "test[^/\\.]*<>");
}

/**
 * @tc.name: MatchSimpleWildcardRegex_test_001
 * @tc.desc: test MatchSimpleWildcardRegex function through IsMatch with various patterns
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, MatchSimpleWildcardRegex_test_001, TestSize.Level0)
{
    // Test .* patterns
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", ".*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123", ".*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("", ".*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.file", ".*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", "test.*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123", "test.*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", ".*test"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("123test", ".*test"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123test", ".*test.*"));

    // Test escaped characters
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.file", "test\\..*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.file123", "test\\..*"));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("testfile", "test\\..*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test(file)", "test\\(file\\)"));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("testfile", "test\\(file\\)"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test{value}", "test{value}"));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("testvalue", "test{value}"));
}

/**
 * @tc.name: MatchSimpleWildcardRegex_test_005
 * @tc.desc: test MatchSimpleWildcardRegex function with complex combinations
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, MatchSimpleWildcardRegex_test_005, TestSize.Level0)
{
    // Test MatchSimpleWildcardRegex through IsMatch with complex pattern combinations
    // Convert wildcard patterns to regex first
    std::string pattern1 = ark::guard::StringUtil::ConvertWildcardToRegex("test*file");
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123file", pattern1));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("testfile", pattern1));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test/file", pattern1));  // Contains /

    std::string pattern2 = ark::guard::StringUtil::ConvertWildcardToRegex("test?file");
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("testafile", pattern2));
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test/file", pattern2));  // Contains /
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("testabfile", pattern2)); // Two chars

    // Direct regex pattern
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.file", "test\\..*file"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.abcfile", "test\\..*file"));
}

/**
 * @tc.name: GetCachedRegex_test_001
 * @tc.desc: test GetCachedRegex function through RegexMatch, FindLongestMatchedPrefixReg and IsMatch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, GetCachedRegex_test_001, TestSize.Level0)
{
    // Test GetCachedRegex through RegexMatch
    EXPECT_TRUE(ark::guard::StringUtil::RegexMatch("test", "test"));
    EXPECT_TRUE(ark::guard::StringUtil::RegexMatch("test123", "test[0-9]+"));
    EXPECT_TRUE(ark::guard::StringUtil::RegexMatch("test", "^test$"));
    EXPECT_FALSE(ark::guard::StringUtil::RegexMatch("test", "test1"));
    EXPECT_FALSE(ark::guard::StringUtil::RegexMatch("test", "^test1$"));

    // Test caching behavior (same pattern should use cache)
    EXPECT_TRUE(ark::guard::StringUtil::RegexMatch("test", "test"));
    EXPECT_TRUE(ark::guard::StringUtil::RegexMatch("test456", "test[0-9]+"));

    // Test GetCachedRegex through FindLongestMatchedPrefixReg
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^test.*"}, "test123"), 7);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^test[0-9]+"}, "test123"), 7);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^test.*", "^test[0-9]+"}, "test123"), 7);

    // Test through IsMatch (also uses GetCachedRegex for complex patterns)
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123", "test[0-9]+"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test456", "test[0-9]+"));
}

/**
 * @tc.name: IsSimpleWildcardRegexPattern_test_001
 * @tc.desc: test IsSimpleWildcardRegexPattern function through IsMatch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, IsSimpleWildcardRegexPattern_test_001, TestSize.Level0)
{
    // Test IsSimpleWildcardRegexPattern through IsMatch
    // Direct regex patterns that use simple wildcard matching (faster path)
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test.file", "test\\..*"));
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test(file)", "test\\(file\\)"));
    // IsMatch automatically escapes braces, so use unescaped braces in pattern
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test{value}", "test{value}"));

    // Patterns that should NOT use simple wildcard matching (use std::regex)
    // These contain complex regex symbols that IsSimpleWildcardRegexPattern returns false for
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", "test"));  // Plain string, no regex
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test123", "test[0-9]+"));  // Complex regex with character class
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", "^test$"));  // Anchors
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("test", "test|test2"));  // Alternation
}

/**
 * @tc.name: MatchSimpleWildcardRegex_test_006
 * @tc.desc: test MatchSimpleWildcardRegex function with empty strings and edge cases
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, MatchSimpleWildcardRegex_test_006, TestSize.Level0)
{
    // Test MatchSimpleWildcardRegex through IsMatch with edge cases
    // Empty pattern
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", ""));

    // Empty string with .*
    EXPECT_TRUE(ark::guard::StringUtil::IsMatch("", ".*"));

    // Empty string with [^/\\.]* - pattern is "test[^/\\.]*", empty string doesn't match "test"
    std::string pattern = ark::guard::StringUtil::ConvertWildcardToRegex("test*");
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("", pattern));  // Empty string doesn't match "test" prefix

    // Empty string with [^/\\.]
    std::string pattern2 = ark::guard::StringUtil::ConvertWildcardToRegex("test?");
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("", pattern2));  // Needs one character

    // Pattern longer than string
    std::string pattern3 = ark::guard::StringUtil::ConvertWildcardToRegex("test*file");
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", pattern3));
    std::string pattern4 = ark::guard::StringUtil::ConvertWildcardToRegex("test?file");
    EXPECT_FALSE(ark::guard::StringUtil::IsMatch("test", pattern4));
}