/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

namespace panda::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Program {
    panda::panda_file::SourceLang lang {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    std::unordered_map<std::string, panda::pandasm::Record> record_table;
    std::unordered_map<std::string, panda::pandasm::Function> function_table;
    std::unordered_map<std::string, std::vector<std::string>> function_synonyms;
    std::map<std::string, panda::pandasm::LiteralArray> literalarray_table;
    std::unordered_set<std::string> strings;
    std::unordered_set<Type> array_types;

    /*
     * Returns a JSON string with the program structure and scopes locations
     */
    PANDA_PUBLIC_API std::string JsonDump() const;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace panda::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H
