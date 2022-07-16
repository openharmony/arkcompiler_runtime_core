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

#include "verification/config/debug_breakpoint/breakpoint_private.h"
#include "verification/config/process/config_process.h"
#include "verification/config/whitelist/whitelist_private.h"
#include "verification/util/parser/parser.h"

#include "runtime/include/method.h"
#include "runtime/include/mem/panda_string.h"

#include "utils/logger.h"

#include <string>
#include <cstring>
#include <cstdint>

namespace panda::verifier::debug {

using panda::parser::action;
using panda::parser::parser;
using panda::verifier::config::Section;

const auto &WhitelistMethodParser()
{
    struct Whitelist;

    using panda::parser::charset;
    using p = parser<PandaString, const char, const char *>::next<Whitelist>;
    using p1 = p::p;

    static const auto WS = p::of_charset(" \t");

    static const auto METHOD_NAME_HANDLER = [](action a, PandaString &c, auto from, auto to) {
        if (a == action::PARSED) {
            c = PandaString {from, to};
        }
        return true;
    };

    static const auto METHOD_NAME = p1::of_charset(!charset {" \t,"}) |= METHOD_NAME_HANDLER;  // NOLINT

    static const auto WHITELIST_METHOD = ~WS >> METHOD_NAME >> ~WS >> p::end() | ~WS >> p::end();

    return WHITELIST_METHOD;
}

void RegisterConfigHandlerWhitelist()
{
    static const auto CONFIG_DEBUG_WHITELIST_VERIFIER = [](const Section &section) {
        for (const auto &s : section.sections) {
            WhitelistKind kind;
            if (s.name == "class") {
                kind = WhitelistKind::CLASS;
            } else if (s.name == "method") {
                kind = WhitelistKind::METHOD;
            } else if (s.name == "method_call") {
                kind = WhitelistKind::METHOD_CALL;
            } else {
                LOG(DEBUG, VERIFIER) << "Wrong debug verifier whitelist section: '" << s.name << "'";
                return false;
            }
            for (const auto &i : s.items) {
                PandaString c;
                const char *start = i.c_str();
                const char *end = i.c_str() + i.length();  // NOLINT
                if (!WhitelistMethodParser()(c, start, end)) {
                    LOG(DEBUG, VERIFIER) << "Wrong whitelist line: '" << i << "'";
                    return false;
                }
                if (!c.empty()) {
                    if (kind == WhitelistKind::CLASS) {
                        LOG(DEBUG, VERIFIER) << "Added to whitelist config '" << s.name << "' methods from class " << c;
                    } else {
                        LOG(DEBUG, VERIFIER) << "Added to whitelist config '" << s.name << "' methods named " << c;
                    }
                    AddWhitelistMethodConfig(kind, c);
                }
            }
        }
        return true;
    };

    config::RegisterConfigHandler("config.debug.whitelist.verifier", CONFIG_DEBUG_WHITELIST_VERIFIER);
}

}  // namespace panda::verifier::debug
