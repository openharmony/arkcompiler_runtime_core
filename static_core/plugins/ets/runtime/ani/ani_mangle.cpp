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

#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

namespace ark::ets::ani {

static size_t ParseArrayBody(const std::string_view data, PandaStringStream &ss);

/*static*/
PandaString Mangle::ConvertDescriptor(const std::string_view descriptor, bool allowArray)
{
    if (descriptor.empty() || descriptor.back() == ';' || descriptor.find('/') != std::string::npos) {
        // The 'descriptor' does not have a new format, so no conversion is required.
        return PandaString(descriptor);
    }

    PandaStringStream ss;
    if (allowArray) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (descriptor.size() >= 2U && descriptor[0] == 'A' && descriptor[1] == '{') {
            auto bodySize = ParseArrayBody(descriptor.substr(1), ss);
            if (bodySize == std::string_view::npos) {
                // The 'descriptor' has wrong format, so can't be convert
                return PandaString("");
            }
            return ss.str();
        }
    }

    ss << 'L';
    ss << descriptor;
    ss << ';';
    PandaString oldDescriptor = ss.str();
    std::replace(oldDescriptor.begin(), oldDescriptor.end(), '.', '/');

    return oldDescriptor;
}

static constexpr size_t MIN_BODY_SIZE = sizeof('{') + 1 + sizeof('}');

static size_t ParseType(char type, const std::string_view data, PandaStringStream &ss);

static size_t ParseArrayBody(const std::string_view data, PandaStringStream &ss)
{
    if (data.size() < MIN_BODY_SIZE || data[0] != '{') {
        return std::string_view::npos;
    }
    ss << '[';

    char type = data[1];
    const std::string_view typeData = data.substr(1);
    size_t size = ParseType(type, typeData, ss);
    if (size == std::string_view::npos || size >= typeData.size() || typeData[size] != '}') {
        return std::string_view::npos;
    }
    return sizeof('{') + size + sizeof('}');
}

static size_t ParseBody(char type, const std::string_view data, PandaStringStream &ss)
{
    ASSERT(type != 'A');

    if (data.size() < MIN_BODY_SIZE || data[0] != '{') {
        return std::string_view::npos;
    }
    for (size_t pos = 1; pos < data.size(); ++pos) {
        if (data[pos] != '}') {
            continue;
        }
        PandaString oldName(data.substr(1, pos - 1));
        std::replace(oldName.begin(), oldName.end(), '.', '/');
        ss << 'L';
        ss << oldName;
        ss << (type == 'P' ? "$partial;" : ";");
        return pos + 1;
    }
    return std::string_view::npos;
}

static size_t ParseType(char type, const std::string_view data, PandaStringStream &ss)
{
    size_t bodySize = std::string_view::npos;
    // clang-format off
    switch (type) {
        case 'z': ss << 'Z'; return 1;
        case 'c': ss << 'C'; return 1;
        case 'b': ss << 'B'; return 1;
        case 's': ss << 'S'; return 1;
        case 'i': ss << 'I'; return 1;
        case 'l': ss << 'J'; return 1;
        case 'f': ss << 'F'; return 1;
        case 'd': ss << 'D'; return 1;
        case 'N': ss << panda_file_items::class_descriptors::OBJECT; return 1;
        case 'U': ss << panda_file_items::class_descriptors::OBJECT; return 1;
        case 'A': bodySize = ParseArrayBody(data.substr(1), ss); break;
        case 'C':
        case 'E':
        case 'P': bodySize = ParseBody(type, data.substr(1), ss); break;
        default:
            // The 'descriptor' does not have a new format, so no conversion is required.
            return std::string_view::npos;
    }
    // clang-format on

    if (bodySize == std::string_view::npos) {
        return std::string_view::npos;
    }
    return 1 + bodySize;
}

/*static*/
PandaString Mangle::ConvertSignature(const std::string_view descriptor)
{
    PandaStringStream ss;
    int nr = -1;
    int k = -1;
    for (size_t i = 0; i < descriptor.size(); ++i) {
        char type = descriptor[i];
        if (type == ':') {
            ss << ':';
            nr = 0;
            k = 1;
            continue;
        }
        size_t sz = ParseType(type, descriptor.substr(i), ss);
        if (sz == std::string_view::npos) {
            // The 'descriptor' does not have a new format, so no conversion is required.
            return PandaString(descriptor);
        }
        i += sz - 1;
        nr += k;
    }
    if (k == -1) {
        // The 'descriptor' does not have a ':' symbol
        return PandaString(descriptor);
    }
    if (nr == 0) {
        ss << 'V';
    }
    return ss.str();
}

}  // namespace ark::ets::ani
