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

#ifndef PANDA_VERIFIER_DEBUG_MSG_SET_PARSER_H_
#define PANDA_VERIFIER_DEBUG_MSG_SET_PARSER_H_

#include "runtime/include/mem/panda_containers.h"
#include "verification/util/parser/parser.h"
#include "verifier_messages.h"

namespace ark::verifier::debug {

struct MessageSetContext {
    ark::PandaVector<std::pair<size_t, size_t>> stack;
    ark::PandaUnorderedSet<size_t> nums;
};

inline const auto &MessageSetParser()
{
    using ark::parser::Action;
    using ark::parser::Charset;
    using ark::parser::Parser;

    using P = Parser<MessageSetContext, const char, const char *>;
    using P1 = typename P::P;
    using P2 = typename P1::P;
    using P3 = typename P2::P;
    using P4 = typename P3::P;

    static const auto WS = P::OfCharset(" \t\r\n");
    static const auto COMMA = P1::OfCharset(",");
    static const auto DEC = P2::OfCharset("0123456789");

    static const auto NAME_HANDLER = [](Action a, MessageSetContext &c, auto from, auto to) {
        if (a == Action::PARSED) {
            std::string_view name {from, static_cast<size_t>(to - from)};
            auto num = static_cast<size_t>(ark::verifier::StringToVerifierMessage(name));
            c.stack.push_back(std::make_pair(num, num));
        }
        return true;
    };

    static const auto NUM_HANDLER = [](Action a, MessageSetContext &c, auto from) {
        if (a == Action::PARSED) {
            auto num = static_cast<size_t>(std::strtol(from, nullptr, 0));
            c.stack.push_back(std::make_pair(num, num));
        }
        return true;
    };

    static const auto RANGE_HANDLER = [](Action a, MessageSetContext &c) {
        if (a == Action::PARSED) {
            auto numEnd = c.stack.back();
            c.stack.pop_back();
            auto numStart = c.stack.back();
            c.stack.pop_back();

            c.stack.push_back(std::make_pair(numStart.first, numEnd.first));
        }
        return true;
    };

    static const auto ITEM_HANDLER = [](Action a, MessageSetContext &c) {
        if (a == Action::START) {
            c.stack.clear();
        }
        if (a == Action::PARSED) {
            auto range = c.stack.back();
            c.stack.pop_back();

            for (auto i = range.first; i <= range.second; ++i) {
                c.nums.insert(i);
            }
        }
        return true;
    };

    static const auto NAME = P3::OfCharset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789") |=
        NAME_HANDLER;

    static const auto NUM = DEC |= NUM_HANDLER;

    static const auto MSG = NUM | NAME;

    static const auto MSG_RANGE = MSG >> ~WS >> P4::OfString("-") >> ~WS >> MSG |= RANGE_HANDLER;

    static const auto ITEM = ~WS >> ((MSG_RANGE | MSG) |= ITEM_HANDLER) >> ~WS >> ~COMMA;

    // clang-tidy bug, use lambda instead of ITEMS = *ITEM
    static const auto ITEMS = [](MessageSetContext &c, const char *&start, const char *end) {
        while (true) {
            auto saved = start;
            if (!ITEM(c, start, end)) {
                start = saved;
                break;
            }
        }
        return true;
    };

    return ITEMS;
}

}  // namespace ark::verifier::debug

#endif  // PANDA_VERIFIER_DEBUG_MSG_SET_PARSER_H_
