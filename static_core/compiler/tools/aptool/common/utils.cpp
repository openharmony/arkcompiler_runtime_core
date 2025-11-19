/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <iomanip>
#include <sstream>

#include "compiler/tools/aptool/common/utils.h"

namespace ark::aptool::common {

std::string QuoteString(std::string_view value)
{
    std::string result;
    result.reserve(value.size() + 2U);
    result.push_back('"');
    for (char ch : value) {
        switch (ch) {
            case '\\':
            case '"':
                result.push_back('\\');
                result.push_back(ch);
                break;
            case '\n':
                result += "\\n";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20U) {
                    std::ostringstream oss;
                    oss << "\\x" << std::hex << std::setw(2) << std::setfill('0')  // NOLINT
                        << static_cast<int>(static_cast<unsigned char>(ch));
                    result += oss.str();
                } else {
                    result.push_back(ch);
                }
                break;
        }
    }
    result.push_back('"');
    return result;
}

std::string FormatHex(uint32_t value)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << value;  // NOLINT
    return oss.str();
}

}  // namespace ark::aptool::common
