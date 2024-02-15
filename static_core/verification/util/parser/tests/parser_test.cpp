/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::parser::test {
namespace {
struct Context {};
struct smth;

static const auto FOO = [](Action act, [[maybe_unused]] Context &, auto &it) {
    switch (act) {
        case Action::START:
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

        case Action::CANCEL:
            return true;

        case Action::PARSED:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
};

static const auto BAR = [](Action act, [[maybe_unused]] Context &, auto &it) {
    switch (act) {
        case Action::START:
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

        case Action::CANCEL:
            return true;

        case Action::PARSED:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
};

using P = typename Parser<Context, const char, const char *>::template Next<smth>;
using P1 = typename P::P;
using P2 = typename P1::P;
using P3 = typename P2::P;
using P4 = typename P3::P;
using P5 = typename P4::P;
using P6 = typename P5::P;

using It = const char *;
}  // namespace

TEST(VerifierParserTest, Parser)
{
    Context cont;

    static const auto ABCP = P::OfCharset(Charset {"abcABC"});
    static const auto DEFP = P1::OfCharset(Charset {"defDEF"});
    static const auto STRINGP = P2::OfString("string");
    std::string aBc {"aBc"};
    It start = &(aBc[0]);
    It end = &(aBc[3]);
    EXPECT_TRUE(ABCP(cont, start, end));
    start = &(aBc[1]);
    EXPECT_TRUE(ABCP(cont, start, end));
    start = &(aBc[0]);
    EXPECT_FALSE(DEFP(cont, start, end));
    start = &(aBc[0]);
    EXPECT_FALSE(STRINGP(cont, start, end));
    std::string string {"string"};
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(ABCP(cont, start, end));
    start = &(string[0]);
    EXPECT_FALSE(DEFP(cont, start, end));
    start = &(string[0]);
    EXPECT_TRUE(STRINGP(cont, start, end));
    std::string d {"d"};
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_FALSE(ABCP(cont, start, end));
    start = &(d[0]);
    EXPECT_TRUE(DEFP(cont, start, end));
    start = &(d[0]);
    EXPECT_FALSE(STRINGP(cont, start, end));
    start = &(string[0]);
    end = &(string[3]);
    EXPECT_FALSE(ABCP(cont, start, end));
    start = &(string[0]);
    EXPECT_FALSE(DEFP(cont, start, end));
    start = &(string[0]);
    EXPECT_FALSE(STRINGP(cont, start, end));

    static const auto ENDP = P3::End();
    start = &(string[0]);
    end = &(string[0]);
    EXPECT_TRUE(ENDP(cont, start, end));
    end = &(string[2]);
    EXPECT_FALSE(ENDP(cont, start, end));

    static const auto ACSTRINGP = ~ABCP >> STRINGP;
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_TRUE(ACSTRINGP(cont, start, end));
    std::string acstring {"ACstring"};
    start = &(acstring[0]);
    end = &(acstring[8]);
    EXPECT_TRUE(ACSTRINGP(cont, start, end));
    end = &(acstring[7]);
    EXPECT_FALSE(ACSTRINGP(cont, start, end));

    static const auto FOOABCP = ABCP |= FOO;
    static const auto BARABCP = ABCP |= BAR;
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(FOOABCP(cont, start, end));
    std::string fooAcB {"fooAcB"};
    start = &(fooAcB[0]);
    end = &(fooAcB[6]);
    EXPECT_TRUE(FOOABCP(cont, start, end));
    start = &(fooAcB[0]);
    EXPECT_FALSE(BARABCP(cont, start, end));

    static const auto ABCDEFP = ABCP | DEFP;
    start = &(aBc[0]);
    end = &(aBc[3]);
    EXPECT_TRUE(ABCDEFP(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(ABCDEFP(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_TRUE(ABCDEFP(cont, start, end));

    static const auto EMPTYP = ABCP & DEFP;
    start = &(aBc[0]);
    end = &(aBc[3]);
    EXPECT_FALSE(EMPTYP(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(EMPTYP(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_FALSE(EMPTYP(cont, start, end));

    static const auto ABC2P = ABCP << STRINGP >> STRINGP;
    start = &(acstring[0]);
    end = &(acstring[8]);
    EXPECT_TRUE(ABC2P(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_FALSE(ABC2P(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_FALSE(ABC2P(cont, start, end));

    static const auto NOABCP = !ABCP;
    start = &(aBc[0]);
    end = &(aBc[3]);
    EXPECT_FALSE(NOABCP(cont, start, end));
    start = &(string[0]);
    end = &(string[6]);
    EXPECT_TRUE(NOABCP(cont, start, end));
    start = &(d[0]);
    end = &(d[1]);
    EXPECT_TRUE(NOABCP(cont, start, end));

    static const auto STRINGSTRINGENDP = *STRINGP >> ENDP;
    static const auto STRINGENDP = STRINGP >> ENDP;
    std::string stringstring {"stringstring"};
    start = &(stringstring[0]);
    end = &(stringstring[12]);
    EXPECT_FALSE(STRINGENDP(cont, start, end));
    start = &(stringstring[0]);
    EXPECT_TRUE(STRINGSTRINGENDP(cont, start, end));
}
}  // namespace ark::parser::test