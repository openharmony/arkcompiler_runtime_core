/*
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

#include <iostream>
#include "assembly-ins.h"

namespace ark::pandasm {

std::string ark::pandasm::Ins::RegsToString(bool &first, bool printArgs, size_t firstArgIdx) const
{
    std::stringstream translator;
    for (const auto &reg : this->regs) {
        if (!first) {
            translator << ",";
        } else {
            first = false;
        }

        if (printArgs && reg >= firstArgIdx) {
            translator << " a" << reg - firstArgIdx;
        } else {
            translator << " v" << reg;
        }
    }
    return translator.str();
}

std::string ark::pandasm::Ins::ImmsToString(bool &first) const
{
    std::stringstream translator;
    for (const auto &imm : this->imms) {
        if (!first) {
            translator << ",";
        } else {
            first = false;
        }

        auto *number = std::get_if<double>(&imm);
        if (number != nullptr) {
            translator << " " << std::scientific << *number;
        } else {
            translator << " 0x" << std::hex << std::get<int64_t>(imm);
        }
        translator.clear();
    }
    return translator.str();
}

std::string ark::pandasm::Ins::IdsToString(bool &first) const
{
    std::stringstream translator;
    for (const auto &id : this->ids) {
        if (!first) {
            translator << ",";
        } else {
            first = false;
        }

        translator << " " << id;
    }
    return translator.str();
}

std::string ark::pandasm::Ins::OperandsToString(bool printArgs, size_t firstArgIdx) const
{
    bool first = true;
    std::stringstream ss {};

    ss << this->RegsToString(first, printArgs, firstArgIdx) << this->ImmsToString(first) << this->IdsToString(first);

    return ss.str();
}

std::string ark::pandasm::Ins::RegToString(size_t idx, bool isFirst, bool printArgs, size_t firstArgIdx) const
{
    if (idx >= regs.size()) {
        return std::string("");
    }

    std::stringstream translator;

    if (!isFirst) {
        translator << ", ";
    } else {
        translator << " ";
    }

    if (printArgs && regs[idx] >= firstArgIdx) {
        translator << "a" << regs[idx] - firstArgIdx;
    } else {
        translator << "v" << regs[idx];
    }

    return translator.str();
}

std::string ark::pandasm::Ins::ImmToString(size_t idx, bool isFirst) const
{
    if (idx >= imms.size()) {
        return std::string("");
    }

    auto *number = std::get_if<double>(&(imms[idx]));
    std::stringstream translator;

    if (!isFirst) {
        translator << ", ";
    } else {
        translator << " ";
    }

    if (number != nullptr) {
        translator << std::scientific << *number;
    } else {
        translator << "0x" << std::hex << std::get<int64_t>(imms[idx]);
    }

    return translator.str();
}

std::string ark::pandasm::Ins::IdToString(size_t idx, bool isFirst) const
{
    if (idx >= ids.size()) {
        return std::string("");
    }

    std::stringstream translator;

    if (!isFirst) {
        translator << ", ";
    } else {
        translator << " ";
    }

    translator << ids[idx];

    return translator.str();
}
}  // namespace ark::pandasm
