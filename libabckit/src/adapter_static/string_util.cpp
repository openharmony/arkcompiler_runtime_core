/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <sstream>

namespace {
constexpr std::string_view UNIO_TYPE_PREFIX = "{U";
constexpr std::string_view UNIO_TYPE_SUFFIX = "}";
constexpr std::string_view UNIO_TYPE_DELIMITER = ",";
constexpr std::string_view LEFT_SQUARE_BRACKET = "[";
constexpr std::string_view RIGHT_SQUARE_BRACKET = "]";
}  // namespace

bool libabckit::StringUtil::IsEndWith(const std::string &str, const std::string &subStr)
{
    if (subStr.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - subStr.size(), subStr.size(), subStr) == 0;
}

std::string libabckit::StringUtil::GetTypeNameStr(const AbckitType *type)
{
    if (type == nullptr) {
        return "";
    }

    if (type->types.empty()) {
        return type->name->impl.data();
    }

    std::string unionStr(UNIO_TYPE_PREFIX);
    for (const auto &item : type->types) {
        unionStr.append(item->name->impl.data());
        unionStr.append(UNIO_TYPE_DELIMITER);
    }
    unionStr.pop_back();
    unionStr.append(UNIO_TYPE_SUFFIX);
    return unionStr;
}

std::string libabckit::StringUtil::GetFuncNameWithSquareBrackets(const char *name)
{
    return LEFT_SQUARE_BRACKET.data() + std::string(name) + RIGHT_SQUARE_BRACKET.data() + " ";
}
