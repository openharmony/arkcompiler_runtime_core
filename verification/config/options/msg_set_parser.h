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

namespace panda::verifier::debug {

struct MessageSetContext {
    panda::PandaVector<std::pair<size_t, size_t>> stack;
    panda::PandaUnorderedSet<size_t> nums;
};

namespace {
using panda::parser::action;

static const auto NAME_HANDLER = [](action a, MessageSetContext &c, auto from, auto to) {
    if (a == action::PARSED) {
        std::string_view name {from, static_cast<size_t>(to - from)};
        size_t num = static_cast<size_t>(panda::verifier::StringToVerifierMessage(name));
        c.stack.push_back(std::make_pair(num, num));
    }
    return true;
};

static const auto NUM_HANDLER = [](action a, MessageSetContext &c, auto from) {
    if (a == action::PARSED) {
        size_t num = std::strtol(from, nullptr, 0);
        c.stack.push_back(std::make_pair(num, num));
    }
    return true;
};

static const auto RANGE_HANDLER = [](action a, MessageSetContext &c) {
    if (a == action::PARSED) {
        auto num_end = c.stack.back();
        c.stack.pop_back();
        auto num_start = c.stack.back();
        c.stack.pop_back();

        c.stack.push_back(std::make_pair(num_start.first, num_end.first));
    }
    return true;
};

static const auto ITEM_HANDLER = [](action a, MessageSetContext &c) {
    if (a == action::START) {
        c.stack.clear();
    }
    if (a == action::PARSED) {
        auto range = c.stack.back();
        c.stack.pop_back();

        for (auto i = range.first; i <= range.second; ++i) {
            c.nums.insert(i);
        }
    }
    return true;
};
}  // namespace

const auto &MessageSetParser()
{
    using panda::parser::action;
    using panda::parser::charset;
    using panda::parser::parser;

    using p = parser<MessageSetContext, const char, const char *>;
    using p1 = typename p::p;
    using p2 = typename p1::p;
    using p3 = typename p2::p;
    using p4 = typename p3::p;

    static const auto WS = p::of_charset(" \t\r\n");
    static const auto COMMA = p1::of_charset(",");
    static const auto DEC = p2::of_charset("0123456789");

    static const auto NAME = p3::of_charset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789") |=
        NAME_HANDLER;

    static const auto NUM = DEC |= NUM_HANDLER;

    static const auto MSG = NUM | NAME;

    static const auto MSG_RANGE = MSG >> ~WS >> p4::of_string("-") >> ~WS >> MSG |= RANGE_HANDLER;

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

}  // namespace panda::verifier::debug

#endif  //! PANDA_VERIFIER_DEBUG_MSG_SET_PARSER_H_
