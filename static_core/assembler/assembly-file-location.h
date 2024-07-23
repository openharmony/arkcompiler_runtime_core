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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_FILE_LOCATION_H
#define PANDA_ASSEMBLER_ASSEMBLY_FILE_LOCATION_H

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class FileLocation {
public:
    std::string wholeLine; /* The line in which the field is defined */
                           /*  Or line in which the field met, if the field is not defined */
    size_t boundLeft = 0;
    size_t boundRight = 0;
    size_t lineNumber = 0;
    bool isDefined = false;

public:
    FileLocation(std::string &fC, size_t bL, size_t bR, size_t lN, bool d)
        : wholeLine(std::move(fC)), boundLeft(bL), boundRight(bR), lineNumber(lN), isDefined(d)
    {
    }
    ~FileLocation() = default;

    DEFAULT_MOVE_SEMANTIC(FileLocation);
    DEFAULT_COPY_SEMANTIC(FileLocation);
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_FILE_LOCATION_H
