/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_VERIFICATION_UTIL_IS_SYSTEM_H
#define PANDA_VERIFICATION_UTIL_IS_SYSTEM_H

#include "runtime/include/runtime.h"

namespace panda::verifier {

inline bool IsSystemFile(const panda_file::File &file)
{
    const std::string &name = file.GetFilename();
    const RuntimeOptions &options = Runtime::GetCurrent()->GetOptions();
    const auto &boot_panda_files = options.GetBootPandaFiles();
    size_t files_len = options.GetPandaFiles().empty() ? boot_panda_files.size() - 1 : boot_panda_files.size();
    for (size_t i = 0; i < files_len; i++) {
        if (name == boot_panda_files[i]) {
            return true;
        }
    }
    return false;
}

template <bool include_synthetic_classes = false>
bool IsSystemClass(const Class &klass)
{
    const auto *file = klass.GetPandaFile();
    if (file == nullptr) {
        return include_synthetic_classes;
    }
    return IsSystemFile(*file);
}

inline bool IsSystemOrSyntheticClass(const Class &klass)
{
    return IsSystemClass<true>(klass);
}

}  // namespace panda::verifier
#endif  // PANDA_VERIFICATION_UTIL_IS_SYSTEM_H
