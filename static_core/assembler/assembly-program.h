/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H
#define PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H

#include <string>
#include <unordered_set>

#include "assembly-function.h"
#include "assembly-record.h"
#include "assembly-type.h"
#include "assembly-methodhandle.h"
#include "assembly-literals.h"
#include "extensions/extensions.h"
#include "macros.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Program {
    using StringT = std::set<std::string>;

    ark::panda_file::SourceLang lang {ark::panda_file::SourceLang::PANDA_ASSEMBLY};
    std::map<std::string, ark::pandasm::Record> recordTable;
    std::map<std::string, ark::pandasm::Function> functionInstanceTable;
    std::map<std::string, ark::pandasm::Function> functionStaticTable;
    std::vector<std::pair<std::string, std::string>> exportStrMap;
    std::unordered_map<std::string, std::vector<std::string>>
        functionSynonyms;  // we might keep unordered, since we don't iterate over it
    std::map<std::string, ark::pandasm::LiteralArray> literalarrayTable;
    StringT strings;
    std::set<Type> arrayTypes;

    /*
     * Returns a JSON string with the program structure and scopes locations
     */
    PANDA_PUBLIC_API std::string JsonDump() const;

    void AddToFunctionTable(ark::pandasm::Function func, const std::string &name = "")
    {
        if (func.IsStatic()) {
            functionStaticTable.emplace(name.empty() ? func.name : name, std::move(func));
        } else {
            functionInstanceTable.emplace(name.empty() ? func.name : name, std::move(func));
        }
    }

    std::optional<std::map<std::basic_string<char>, ark::pandasm::Function>::const_iterator> FindAmongAllFunctions(
        const std::string &name, bool isAmbiguous = false, bool isStatic = false) const
    {
        const auto &firstSearchTable = isStatic ? functionStaticTable : functionInstanceTable;
        const auto &secondSearchTable = isStatic ? functionInstanceTable : functionStaticTable;
        auto it = firstSearchTable.find(name);
        if (it == firstSearchTable.cend()) {
            if (isAmbiguous && !isStatic) {
                return std::nullopt;
            }
            it = secondSearchTable.find(name);
            if (it == secondSearchTable.cend()) {
                return std::nullopt;
            }
        }
        return it;
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H
