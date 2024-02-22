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

#include "abc_file_entity_processor.h"

namespace panda::abc2program {

AbcFileEntityProcessor::AbcFileEntityProcessor(panda_file::File::EntityId entity_id, Abc2ProgramKeyData &key_data)
    : entity_id_(entity_id), key_data_(key_data)
{
    file_ = &(key_data_.GetAbcFile());
    string_table_ = &(key_data_.GetAbcStringTable());
    program_ = &(key_data_.GetProgram());
}

}  // namespace panda::abc2program