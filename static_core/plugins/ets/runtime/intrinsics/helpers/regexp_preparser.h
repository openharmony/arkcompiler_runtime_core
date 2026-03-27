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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_HELPERS_REGEXP_PREPARSER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_HELPERS_REGEXP_PREPARSER_H

#include "include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ark::ets::intrinsics::helpers {

class JsRegExpPreParser {
public:
    JsRegExpPreParser(std::string_view pattern, std::string_view flags, bool unicodeMode);

    bool Validate(PandaString &errorMsg);

private:
    bool ValidateNextToken(PandaString &errorMsg);
    char Peek() const;
    bool HasMore() const;
    void Advance();
    bool TryConsume(char expected);
    void SkipLazyModifier();
    bool ConsumeEscapeAsQuantifiable(PandaString &errorMsg);
    bool ParseEscape(PandaString &errorMsg);
    bool ParseDecimalEscapeUnicode(PandaString &errorMsg);
    bool ParsePropertyEscape(PandaString &errorMsg);
    bool ParseUnicodeEscape(PandaString &errorMsg);
    bool ParseBracedUnicodeEscape(PandaString &errorMsg);
    bool ParseHexEscape(PandaString &errorMsg);
    bool ConsumeExactHexDigits(size_t count, const char *msg, PandaString &errorMsg);
    bool ConsumeCharacterClass(PandaString &errorMsg);
    bool ConsumeGroupOpen(PandaString &errorMsg);
    bool ConsumeGroupClose(PandaString &errorMsg);
    bool ParseGroupSpecifier();
    bool ConsumeLoneBracket(char c, PandaString &errorMsg);
    bool ConsumeSimpleQuantifier(PandaString &errorMsg);
    bool ConsumeBraceQuantifier(PandaString &errorMsg);
    bool HandleMalformedQuantifier(size_t rollbackPos, PandaString &errorMsg);
    PandaString CreateQuantifierOutOfOrderError() const;

    std::string_view pattern_;
    std::string_view flags_;
    bool unicodeMode_;
    size_t pos_ = 0;
    int parenDepth_ = 0;
    bool inClass_ = false;
    bool lastWasQuantifiable_ = false;
    uint32_t capturingGroupCount_ = 0;
    PandaVector<uint32_t> backrefs_;
};

}  // namespace ark::ets::intrinsics::helpers

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_HELPERS_REGEXP_PREPARSER_H
