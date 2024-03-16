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

#ifndef ABC2PROGRAM_ABC_FILE_PROCESSOR_H
#define ABC2PROGRAM_ABC_FILE_PROCESSOR_H

#include "abc2program_key_data.h"

namespace panda::abc2program {

class AbcFileProcessor {
public:
    explicit AbcFileProcessor(Abc2ProgramKeyData &key_data);
    bool ProcessFile();

private:
    void ProcessClasses();
    void FillProgramStrings();
    Abc2ProgramKeyData &key_data_;
    const panda_file::File *file_ = nullptr;
    AbcStringTable *string_table_ = nullptr;
    pandasm::Program *program_ = nullptr;
};

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC_FILE_PROCESSOR_H
