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

#include <assembly-program.h>
#include "abc_string_table.h"
#include "file.h"

namespace panda::abc2program {

class AbcFileProcessor {
public:
    AbcFileProcessor(const panda_file::File &abc_file, AbcStringTable &abc_string_table, pandasm::Program &program)
        : abc_file_(abc_file), abc_string_table_(abc_string_table), program_(program) {}
    bool ProcessFile();

private:
    void ProcessClasses();
    void FillUpProgramStrings();
    void FillUpProgramArrayTypes();
    const panda_file::File &abc_file_;
    AbcStringTable &abc_string_table_;
    pandasm::Program &program_;
};

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC_FILE_PROCESSOR_H
