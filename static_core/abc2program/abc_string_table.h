/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ABC2PROGRAM_ABC_STRING_TABLE_H
#define ABC2PROGRAM_ABC_STRING_TABLE_H

#include <set>
#include <cstdint>
#include <string>
#include <iostream>
#include <assembly-program.h>
#include "file-inl.h"
#include "file.h"

namespace ark::abc2program {

class AbcStringTable {
public:
    explicit AbcStringTable(const panda_file::File &file) : file_(file) {}
    std::string GetStringById(uint32_t stringId) const;
    std::string GetStringById(panda_file::File::EntityId entityId) const;
    void AddStringId(uint32_t stringId);
    void AddStringId(panda_file::File::EntityId entityId);
    std::set<std::string> GetStringSet() const;
    std::string StringDataToString(panda_file::File::StringData sd) const;
    void Dump(std::ostream &os) const;

private:
    void DumpStringById(std::ostream &os, uint32_t stringId) const;
    const panda_file::File &file_;
    std::set<uint32_t> stingIdSet_;
};

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_STRING_TABLE_H
