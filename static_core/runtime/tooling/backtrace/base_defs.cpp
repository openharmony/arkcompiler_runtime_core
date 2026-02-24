/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/tooling/backtrace/base_defs.h"

#include "libarkfile/class_data_accessor.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/debug_helpers.h"
#include "runtime/include/class.h"
#include "runtime/include/class_helper.h"

namespace ark::tooling {

std::optional<MethodInfo> ReadMethodInfo(panda_file::MethodDataAccessor &mda)
{
    uintptr_t methodId = mda.GetMethodId().GetOffset();
    auto codeId = mda.GetCodeId();
    if (!codeId) {
        return std::nullopt;
    }
    panda_file::CodeDataAccessor cda(mda.GetPandaFile(), codeId.value());
    uint32_t codeSize = cda.GetCodeSize();
    auto codeBegin = reinterpret_cast<uintptr_t>(cda.GetInstructions());
    return std::make_optional<MethodInfo>(methodId, codeBegin, codeSize);
}

std::vector<MethodInfo> ReadAllMethodInfos(const panda_file::File *file)
{
    std::vector<MethodInfo> result;
    Span<const uint32_t> classIndexes = file->GetClasses();
    for (const uint32_t index : classIndexes) {
        panda_file::File::EntityId classId(index);
        if (file->IsExternal(classId)) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*file, classId);
        cda.EnumerateMethods([&result](panda_file::MethodDataAccessor &mda) {
            auto info = ReadMethodInfo(mda);
            if (!info) {
                return;
            }
            result.push_back(info.value());
        });
    }

    std::sort(result.begin(), result.end());
    return result;
}

// CC-OFFNXT(readability-function-size_parameters)
bool SymbolizeByNativeFrameImpl(uintptr_t pc, uintptr_t mapBase, uintptr_t loadOffset, const panda_file::File *file,
                                const std::vector<MethodInfo> &methodInfos, Function *function)
{
    if (file == nullptr || methodInfos.empty() || function == nullptr) {
        LOG(ERROR, RUNTIME) << "SymbolizeByNativeFrameImpl invalid params";
        return false;
    }

    loadOffset = loadOffset % os::mem::GetPageSize();
    uintptr_t realOffset = pc - mapBase - loadOffset;
    uintptr_t realPc = realOffset + reinterpret_cast<uintptr_t>(file->GetBase());
    int32_t left = 0;
    int32_t right = static_cast<int32_t>(methodInfos.size()) - 1;
    for (; left <= right;) {
        int32_t mid = (left + right) / 2;
        bool isRight = realPc >= (methodInfos[mid].codeBegin + methodInfos[mid].codeSize);
        bool isLeft = realPc < methodInfos[mid].codeBegin;
        if (!isRight && !isLeft) {
            panda_file::File::EntityId id(methodInfos[mid].methodId);
            panda_file::MethodDataAccessor mda(*file, id);
            panda_file::ClassDataAccessor cda(*file, mda.GetClassId());
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            int size = snprintf_s(function->functionName, FUNCTIONNAME_MAX, FUNCTIONNAME_MAX - 1, "%s.%s",
                                  ClassHelper::GetName(cda.GetDescriptor()).c_str(), mda.GetName().data);
            if (size < 0) {
                LOG(ERROR, RUNTIME) << "copy funtionname failed!";
            }
            uint32_t bcOffset = realPc - methodInfos[mid].codeBegin;
            function->line = static_cast<int32_t>(panda_file::debug_helpers::GetLineNumber(mda, bcOffset, file));
            function->column = 1;
            function->codeBegin = methodInfos[mid].codeBegin;
            function->codeSize = methodInfos[mid].codeSize;
            auto sourceFileId = cda.GetSourceFileId();
            // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
            if (sourceFileId.has_value() &&
                strcpy_s(function->url, URL_MAX,
                         reinterpret_cast<const char *>(file->GetStringData(sourceFileId.value()).data)) == EOK) {
                return true;
            }
            LOG(ERROR, RUNTIME) << "sourceFile not exist";
            return false;
        } else if (isRight) {  // NOLINT(readability-else-after-return)
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    LOG(ERROR, RUNTIME) << "Can not find ark method";
    return false;
}
}  // namespace ark::tooling
