/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "json_builder.h"

#include "libarkbase/utils/string_helpers.h"

#include <algorithm>

using ark::helpers::string::Format;

namespace ark {
void JsonEscape(std::ostream &os, std::string_view string)
{
    os << '"';

    while (!string.empty()) {
        auto iter = std::find_if(string.begin(), string.end(), [](char ch) {
            auto uc = static_cast<unsigned char>(ch);
            return uc == '"' || uc == '\\' || uc < ' ';
        });
        auto pos = iter - string.begin();

        os << string.substr(0, pos);

        if (iter == string.end()) {
            break;
        }

        os << '\\';

        auto ch = static_cast<unsigned char>(*iter);
        switch (ch) {
            case '"':
            case '\\':
                os << static_cast<char>(ch);
                break;
            case '\b':
                os << 'b';
                break;
            case '\f':
                os << 'f';
                break;
            case '\n':
                os << 'n';
                break;
            case '\r':
                os << 'r';
                break;
            case '\t':
                os << 't';
                break;
            default:
                os << Format("u%04X", static_cast<int>(ch));  // NOLINT(cppcoreguidelines-pro-type-vararg)
                break;
        }

        string.remove_prefix(pos + 1);
    }

    os << '"';
}
}  // namespace ark
