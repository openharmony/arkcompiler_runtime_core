/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TOOLING_BACKTRACE_BACKTRACE_H
#define PANDA_RUNTIME_TOOLING_BACKTRACE_BACKTRACE_H

#include <cstdint>
#include <vector>
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/code_data_accessor.h"
#include "libpandafile/debug_helpers.h"
#include "libpandafile/file.h"
#include "libpandafile/method_data_accessor.h"
#include "runtime/include/class.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/stack_walker.h"

namespace ark::tooling {

constexpr uint16_t FUNCTIONNAME_MAX = 1024;
constexpr uint16_t PACKAGENAME_MAX = 1024;
constexpr uint16_t URL_MAX = 1024;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct Function {
    char functionName[FUNCTIONNAME_MAX];  // NOLINT(modernize-avoid-c-arrays)
    char packageName[PACKAGENAME_MAX];    // NOLINT(modernize-avoid-c-arrays)
    char url[URL_MAX];                    // NOLINT(modernize-avoid-c-arrays)
    int32_t line = 0;
    int32_t column = 0;
    uintptr_t codeBegin;
    uintptr_t codeSize;
};

struct MethodInfo {
    uintptr_t methodId;   // NOLINT(misc-non-private-member-variables-in-classes)
    uintptr_t codeBegin;  // NOLINT(misc-non-private-member-variables-in-classes)
    uint32_t codeSize;    // NOLINT(misc-non-private-member-variables-in-classes)
    MethodInfo(uintptr_t id, uintptr_t begin, uint32_t size) : methodId(id), codeBegin(begin), codeSize(size) {}
    friend bool operator<(const MethodInfo &lhs, const MethodInfo &rhs)
    {
        return lhs.codeBegin < rhs.codeBegin;
    }
};

class Backtrace {
public:
    using ReadMemFunc = bool (*)(void *, uintptr_t, uintptr_t *, bool);
    // CC-OFFNXT(readability-function-size_parameters)
    static int StepArk(void *ctx, ReadMemFunc readMem, uintptr_t *fp, uintptr_t *sp, uintptr_t *pc,
                       uintptr_t *bcOffset);
    // CC-OFFNXT(readability-function-size_parameters)
    static int Symbolize(uintptr_t pc, uintptr_t mapBase, uint32_t bcOffset, uint8_t *abcData, uint64_t abcSize,
                         Function *function);
    static std::vector<MethodInfo> ReadAllMethodInfos(const ark::panda_file::File *pandaFile);
    static std::optional<MethodInfo> ReadMethodInfo(ark::panda_file::MethodDataAccessor &mda);
};

}  // namespace ark::tooling
#endif  // PANDA_RUNTIME_TOOLING_BACKTRACE_BACKTRACE_H
