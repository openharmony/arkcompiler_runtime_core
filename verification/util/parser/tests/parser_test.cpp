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

#include <string>

#include <gtest/gtest.h>

#include "util/parser/parser.h"

namespace panda::parser::test {
namespace {
struct Context {
};
struct smth;

static const auto FOO = [](action act, [[maybe_unused]] Context &, auto &it) {
    switch (act) {
        case action::START:
            if (*it != 'f') {
                return false;
            }
            ++it;
            if (*it != 'o') {
                return false;
            }
            ++it;
            if (*it != 'o') {
                return false;
            }
            ++it;
            return true;

        case action::CANCEL:
            return true;

        case action::PARSED:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
};

static const auto BAR = [](action act, [[maybe_unused]] Context &, auto &it) {
    switch (act) {
        case action::START:
            if (*it != 'b') {
                return false;
            }
            ++it;
            if (*it != 'a') {
                return false;
            }
            ++it;
            if (*it != 'r') {
                return false;
            }
            ++it;
            return true;

        case action::CANCEL:
            return true;

        case action::PARSED:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
};

using p = typename parser<Context, const char, const char *>::template next<smth>;
using p1 = typename p::p;
using p2 = typename p1::p;
using p3 = typename p2::p;
using p4 = typename p3::p;
using p5 = typename p4::p;
using p6 = typename p5::p;

using it = const char *;
}  // namespace

TEST(VerifierParserTest, Parser)
{
    Context cont;

    static const auto abcp = p::of_charset(charset {"abcABC"});
    static const auto defp = p1::of_charset(charset {"defDEF"});
    static const auto stringp = p2::of_string("string");
    std::string aBc {"aBc"};
    it start = &(aBc[0]);
    it end = &(aBc[3]);
    EXPECT_TRUE(abcp(cont, start, end));
    start = &(aBc[1]);
    EXPECT_TRUE(abcp(cont, start, end));
    start = &(aBc[0]);
    EXPECT_FALSE(defp(cont, start, end));
    start = &(aBc[0]);
    EXPECT_FALSE(stringp(cont, start, end));
    std::string string {"string"};
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(abcp(cont, start, end));
    start = &(string[0]);
    EXPECT_FALSE(defp(cont, start, end));
    start = &(string[0]);
    EXPECT_TRUE(stringp(cont, start, end));
    std::string d {"d"};
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_FALSE(abcp(cont, start, end));
    start = &(d[0]);
    EXPECT_TRUE(defp(cont, start, end));
    start = &(d[0]);
    EXPECT_FALSE(stringp(cont, start, end));
    start = &(string[0]);
    end = &(string[3]);
    EXPECT_FALSE(abcp(cont, start, end));
    start = &(string[0]);
    EXPECT_FALSE(defp(cont, start, end));
    start = &(string[0]);
    EXPECT_FALSE(stringp(cont, start, end));

    static const auto endp = p3::end();
    start = &(string[0]);
    end = &(string[0]);
    EXPECT_TRUE(endp(cont, start, end));
    end = &(string[2]);
    EXPECT_FALSE(endp(cont, start, end));

    static const auto acstringp = ~abcp >> stringp;
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_TRUE(acstringp(cont, start, end));
    std::string acstring {"ACstring"};
    start = &(acstring[0]);
    end = &(acstring[8]);
    EXPECT_TRUE(acstringp(cont, start, end));
    end = &(acstring[7]);
    EXPECT_FALSE(acstringp(cont, start, end));

    static const auto fooabcp = abcp |= FOO;
    static const auto barabcp = abcp |= BAR;
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(fooabcp(cont, start, end));
    std::string fooAcB {"fooAcB"};
    start = &(fooAcB[0]);
    end = &(fooAcB[6]);
    EXPECT_TRUE(fooabcp(cont, start, end));
    start = &(fooAcB[0]);
    EXPECT_FALSE(barabcp(cont, start, end));

    static const auto abcdefp = abcp | defp;
    start = &(aBc[0]);
    end = &(aBc[3]);
    EXPECT_TRUE(abcdefp(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(abcdefp(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_TRUE(abcdefp(cont, start, end));

    static const auto emptyp = abcp & defp;
    start = &(aBc[0]);
    end = &(aBc[3]);
    EXPECT_FALSE(emptyp(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(emptyp(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_FALSE(emptyp(cont, start, end));

    static const auto abc2p = abcp << stringp >> stringp;
    start = &(acstring[0]);
    end = &(acstring[8]);
    EXPECT_TRUE(abc2p(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(abc2p(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_FALSE(abc2p(cont, start, end));

    static const auto noabcp = !abcp;
    start = &(aBc[0]);
    end = &(aBc[3]);
    EXPECT_FALSE(noabcp(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_TRUE(noabcp(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_TRUE(noabcp(cont, start, end));

    static const auto stringstringendp = *stringp >> endp;
    static const auto stringendp = stringp >> endp;
    std::string stringstring {"stringstring"};
    start = &(stringstring[0]);
    end = &(stringstring[12]);
    EXPECT_FALSE(stringendp(cont, start, end));
    start = &(stringstring[0]);
    EXPECT_TRUE(stringstringendp(cont, start, end));
}
}  // namespace panda::parser::test