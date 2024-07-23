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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_METHODHANDLE_H
#define PANDA_ASSEMBLER_ASSEMBLY_METHODHANDLE_H

#include <string>

namespace ark::pandasm {

struct MethodHandle {
    std::string itemName;                    // NOLINT(misc-non-private-member-variables-in-classes)
    ark::panda_file::MethodHandleType type;  // NOLINT(misc-non-private-member-variables-in-classes)
    MethodHandle(std::string s, ark::panda_file::MethodHandleType t) : itemName(std::move(s)), type(t) {}
};

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_METHODHANDLE_H
