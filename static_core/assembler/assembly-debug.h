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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_DEBUG_H
#define PANDA_ASSEMBLER_ASSEMBLY_DEBUG_H

#include <cstdint>
#include <string>

namespace ark::pandasm::debuginfo {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Ins {
    size_t lineNumber = 0;
    uint32_t columnNumber = 0;
    std::string wholeLine;  // NOTE(mbolshov): redundant given file and line_number
    size_t boundLeft = 0;
    size_t boundRight = 0;

    void SetLineNumber(size_t ln)
    {
        lineNumber = ln;
    }

    void SetColumnNumber(size_t cn)
    {
        columnNumber = cn;
    }

    Ins() = default;
    Ins(size_t lN, std::string &fC, size_t bL, size_t bR)
        : lineNumber(lN), wholeLine(std::move(fC)), boundLeft(bL), boundRight(bR)
    {
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

struct LocalVariable {
    std::string name;
    std::string signature;
    std::string signatureType;
    int32_t reg = 0;
    uint32_t start = 0;
    uint32_t length = 0;
};

}  // namespace ark::pandasm::debuginfo

#endif  // PANDA_ASSEMBLER_ASSEMBLY_DEBUG_H
