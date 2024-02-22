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

#include "abc_string_table.h"
#include "file.h"

namespace panda::abc2program {

class AbcFileEntityProcessor {
public:
    AbcFileEntityProcessor(panda_file::File::EntityId entity_id, const panda_file::File &abc_file,
                           AbcStringTable &abc_string_table)
        : entity_id_(entity_id), abc_file_(abc_file), abc_string_table_(abc_string_table) {}

protected:
    virtual void FillUpProgramData();
    panda_file::File::EntityId entity_id_;
    const panda_file::File &abc_file_;
    AbcStringTable &abc_string_table_;
}; // class AbcFileEntityProcessor

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
