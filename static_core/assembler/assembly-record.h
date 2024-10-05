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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_RECORD_H
#define PANDA_ASSEMBLER_ASSEMBLY_RECORD_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "assembly-field.h"
#include "extensions/extensions.h"
#include "ide_helpers.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Record {
    std::string name;
    bool conflict = false; /* Name is conflict with panda primitive types. Need special handle. */
    ark::panda_file::SourceLang language;
    std::unique_ptr<RecordMetadata> metadata;
    std::vector<Field> fieldList; /* class fields list */
    size_t paramsNum = 0;
    bool bodyPresence = false;
    SourceLocation bodyLocation;
    std::string sourceFile; /* The file in which the record is defined or empty */
    std::optional<FileLocation> fileLocation;

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    Record(std::string s, ark::panda_file::SourceLang lang, size_t bL, size_t bR, std::string fC, bool d, size_t lN)
        : name(std::move(s)),
          language(lang),
          metadata(extensions::MetadataExtension::CreateRecordMetadata(lang)),
          fileLocation({fC, bL, bR, lN, d})
    {
    }

    Record(std::string s, ark::panda_file::SourceLang lang)
        : name(std::move(s)), language(lang), metadata(extensions::MetadataExtension::CreateRecordMetadata(lang))
    {
    }

    bool HasImplementation() const
    {
        return !metadata->IsForeign();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_RECORD_H
