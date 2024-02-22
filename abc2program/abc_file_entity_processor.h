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

#ifndef ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
#define ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H

#include "abc2program_key_data.h"
#include "abc2program_log.h"
#include "abc_file_utils.h"

namespace panda::abc2program {

class AbcFileEntityProcessor {
public:
    AbcFileEntityProcessor(panda_file::File::EntityId entity_id, Abc2ProgramKeyData &key_data);

    bool AddRecord(panda_file::File::EntityId class_id, const pandasm::Record &record)
    {
        return key_data_.AddRecord(class_id, record);
    }

    bool AddFunction(panda_file::File::EntityId method_id, const pandasm::Function &function)
    {
        return key_data_.AddFunction(method_id, function);
    }

    bool AddField(panda_file::File::EntityId field_id, const pandasm::Field &field)
    {
        return key_data_.AddField(field_id, field);
    }

    const pandasm::Record *GetRecordById(panda_file::File::EntityId class_id) const
    {
        return key_data_.GetRecordById(class_id);
    }

    const pandasm::Function *GetFunctionById(panda_file::File::EntityId method_id) const
    {
        return key_data_.GetFunctionById(method_id);
    }

    const pandasm::Field *GetFieldById(panda_file::File::EntityId field_id) const
    {
        return key_data_.GetFieldById(field_id);
    }

protected:
    virtual void FillProgramData() = 0;
    panda_file::File::EntityId entity_id_;
    Abc2ProgramKeyData &key_data_;
    const panda_file::File *file_ = nullptr;
    AbcStringTable *string_table_ = nullptr;
    pandasm::Program *program_ = nullptr;
}; // class AbcFileEntityProcessor

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
