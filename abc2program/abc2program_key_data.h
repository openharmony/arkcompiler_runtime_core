/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef ABC2PROGRAM_ABC2PROGRAM_KEY_DATA_H
#define ABC2PROGRAM_ABC2PROGRAM_KEY_DATA_H

#include <string>
#include <map>
#include <assembly-program.h>
#include "file.h"
#include "abc_string_table.h"

namespace panda::abc2program {

class Abc2ProgramKeyData {
public:
    Abc2ProgramKeyData(const panda_file::File &file, AbcStringTable &string_table, pandasm::Program &program)
        : file_(file), string_table_(string_table), program_(program) {}
    const panda_file::File &GetAbcFile() const;
    AbcStringTable &GetAbcStringTable() const;
    pandasm::Program &GetProgram() const;
    bool AddRecord(panda_file::File::EntityId class_id, const pandasm::Record &record);
    bool AddFunction(panda_file::File::EntityId method_id, const pandasm::Function &function);
    bool AddField(panda_file::File::EntityId field_id, const pandasm::Field &field);
    const pandasm::Record *GetRecordById(panda_file::File::EntityId class_id) const;
    const pandasm::Function *GetFunctionById(panda_file::File::EntityId method_id) const;
    const pandasm::Field *GetFieldById(panda_file::File::EntityId field_id) const;
    std::string GetFullRecordNameById(const panda_file::File::EntityId &class_id) const;

private:
    const panda_file::File &file_;
    AbcStringTable &string_table_;
    pandasm::Program &program_;
    std::map<panda_file::File::EntityId, const pandasm::Record*> record_map_;
    std::map<panda_file::File::EntityId, const pandasm::Function*> function_map_;
    std::map<panda_file::File::EntityId, const pandasm::Field*> field_map_;
}; // class Abc2ProgramKeyData

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC2PROGRAM_KEY_DATA_H
