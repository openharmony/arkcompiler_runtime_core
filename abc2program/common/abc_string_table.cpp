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

#include "abc_string_table.h"

namespace panda::abc2program {

std::string AbcStringTable::GetStringById(uint32_t string_id) const
{
    panda_file::File::EntityId entity_id(string_id);
    return GetStringById(entity_id);
}

std::string AbcStringTable::GetStringById(panda_file::File::EntityId entity_id) const
{
    panda_file::File::StringData sd = file_.GetStringData(entity_id);
    return (reinterpret_cast<const char *>(sd.data));
}

void AbcStringTable::AddStringId(uint32_t string_id)
{
    string_id_set.insert(string_id);
}

void AbcStringTable::AddStringId(const panda_file::File::EntityId &string_id)
{
    AddStringId(string_id.GetOffset());
}

} // namespace panda::abc2program
