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

#include "disassembler.h"

#include "utils/logger.h"

namespace panda::disasm {

void Disassembler::DumpLiteralArrayByOffset(uint32_t offset, std::stringstream &ss,
                                            std::unordered_set<uint32_t> &visiting) const
{
    if (!visiting.insert(offset).second) {
        LOG(ERROR, DISASSEMBLER) << "> recursive literal array reference at 0x" << std::hex << offset;
        ss << "\"<recursive literal array 0x" << std::hex << offset << ">\"";
        return;
    }

    pandasm::LiteralArray lit_array;
    GetLiteralArrayByOffset(&lit_array, panda_file::File::EntityId(offset));
    DumpLiteralArrayContent(lit_array, ss, visiting);
    visiting.erase(offset);
}

void Disassembler::DumpLiteralArrayContent(const pandasm::LiteralArray &literal_array, std::stringstream &ss,
                                           std::unordered_set<uint32_t> &visiting) const
{
    ss << "[";
    bool firstItem = true;
    for (const auto &item : literal_array.literals_) {
        if (!firstItem) {
            ss << ", ";
        } else {
            firstItem = false;
        }

        switch (item.tag_) {
            case panda_file::LiteralTag::DOUBLE: {
                ss << std::get<double>(item.value_);
                break;
            }
            case panda_file::LiteralTag::BOOL: {
                ss << std::get<bool>(item.value_);
                break;
            }
            case panda_file::LiteralTag::STRING: {
                ss << "\"" << std::get<std::string>(item.value_) << "\"";
                break;
            }
            case panda_file::LiteralTag::LITERALARRAY: {
                std::string offsetStr = std::get<std::string>(item.value_);
                uint32_t litArrayOffset = std::stoi(offsetStr, nullptr, 16);
                DumpLiteralArrayByOffset(litArrayOffset, ss, visiting);
                break;
            }
            case panda_file::LiteralTag::BUILTINTYPEINDEX: {
                // By convention, BUILTINTYPEINDEX is used to store type of empty arrays,
                // therefore it has no value
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }
    }
    ss << "]";
}

void Disassembler::DumpLiteralArray(const pandasm::LiteralArray &literal_array, std::stringstream &ss) const
{
    std::unordered_set<uint32_t> visiting;
    DumpLiteralArrayContent(literal_array, ss, visiting);
}

std::string Disassembler::getLiteralArrayTypeByOffset(uint32_t offset, std::unordered_set<uint32_t> &visiting) const
{
    if (!visiting.insert(offset).second) {
        LOG(ERROR, DISASSEMBLER) << "> recursive literal array type reference at 0x" << std::hex << offset;
        return "any[]";
    }

    pandasm::LiteralArray lit_array;
    GetLiteralArrayByOffset(&lit_array, panda_file::File::EntityId(offset));
    auto result = getLiteralArrayTypeContent(lit_array, visiting);
    visiting.erase(offset);
    return result;
}

std::string Disassembler::getLiteralArrayTypeContent(const pandasm::LiteralArray &literal_array,
                                                     std::unordered_set<uint32_t> &visiting) const
{
    [[maybe_unused]] auto size = literal_array.literals_.size();
    ASSERT(size > 0);
    switch (literal_array.literals_[0].tag_) {
        case panda_file::LiteralTag::DOUBLE: {
            return "f64[]";
        }
        case panda_file::LiteralTag::BOOL: {
            return "u1[]";
        }
        case panda_file::LiteralTag::STRING: {
            return "panda.String[]";
        }
        case panda_file::LiteralTag::LITERALARRAY: {
            std::string offsetStr = std::get<std::string>(literal_array.literals_[0].value_);
            uint32_t litArrayOffset = std::stoi(offsetStr, nullptr, 16);
            return getLiteralArrayTypeByOffset(litArrayOffset, visiting) + "[]";
        }
        case panda_file::LiteralTag::BUILTINTYPEINDEX: {
            uint8_t typeIndex = std::get<uint8_t>(literal_array.literals_[0].value_);
            static constexpr uint8_t emptyLiteralArrayWithNumberType = 0;
            static constexpr uint8_t emptyLiteralArrayWithBooleanType = 1;
            static constexpr uint8_t emptyLiteralArrayWithStringType = 2;
            switch (typeIndex) {
                case emptyLiteralArrayWithNumberType:
                    return "f64[]";
                case emptyLiteralArrayWithBooleanType:
                    return "u1[]";
                case emptyLiteralArrayWithStringType:
                    return "panda.String[]";
                default:
                    UNREACHABLE();
                    break;
            }
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

std::string Disassembler::getLiteralArrayTypeFromValue(const pandasm::LiteralArray &literal_array) const
{
    std::unordered_set<uint32_t> visiting;
    return getLiteralArrayTypeContent(literal_array, visiting);
}

}  // namespace panda::disasm
