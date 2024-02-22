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

#include "abc_string_table.h"

namespace panda::abc2program {

void AbcStringTable::AddStringId(uint32_t string_id)
{
    AddStringId(string_id, abc_file_);
}

void AbcStringTable::AddStringId(uint32_t string_id, const panda_file::File &abc_file)
{
    if (&abc_file != &abc_file_) {
        return;
    }
    auto it = sting_id_set_.find(string_id);
    if (it != sting_id_set_.end()) {
        return;
    }
    sting_id_set_.insert(string_id);
}

std::string AbcStringTable::GetStringById(uint32_t string_id)
{
    return GetStringById(string_id, abc_file_);
}

std::string AbcStringTable::GetStringById(uint32_t string_id, const panda_file::File &abc_file)
{
    panda_file::File::EntityId entity_id(string_id);
    return GetStringById(entity_id, abc_file);
}

std::string AbcStringTable::GetStringById(panda_file::File::EntityId entity_id)
{
    return GetStringById(entity_id, abc_file_);
}

std::string AbcStringTable::GetStringById(panda_file::File::EntityId entity_id, const panda_file::File &abc_file)
{
    return StringDataToString(abc_file.GetStringData(entity_id));
}

std::string AbcStringTable::StringDataToString(panda_file::File::StringData sd)
{
    return (reinterpret_cast<const char *>(sd.data));
}

} // namespace panda::abc2program
