/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_LABEL_H
#define PANDA_ASSEMBLER_ASSEMBLY_LABEL_H

#include <string>
#include "assembly-file-location.h"

namespace ark::pandasm {

struct Label {
    std::string name;                          // NOLINT(misc-non-private-member-variables-in-classes)
    std::optional<FileLocation> fileLocation;  // NOLINT(misc-non-private-member-variables-in-classes)

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    Label(std::string s, size_t bL, size_t bR, std::string fC, bool d, size_t lN)
        : name(std::move(s)), fileLocation({fC, bL, bR, lN, d})
    {
    }

    explicit Label(std::string s) : name(std::move(s)) {}
};

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_LABEL_H
