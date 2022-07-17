/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "source_file.h"

#include "bytecode_instruction-inl.h"
#include "code_data_accessor.h"
#include "method_data_accessor-inl.h"
#include "tooling/pt_location.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <utility>

namespace panda::tooling::inspector {
SourceFile::SourceFile(size_t scriptId, const char *fileName, const panda_file::File *pandaFile,
                       panda_file::DebugInfoExtractor debugInfo)
    : scriptId_(scriptId), fileName_(fileName), pandaFile_(pandaFile), debugInfo_(std::move(debugInfo))
{
    std::stringstream buffer;
    buffer << std::ifstream(fileName).rdbuf();

    sourceCode_ = buffer.str();
    if (!sourceCode_.empty() && sourceCode_.back() != '\n') {
        sourceCode_.push_back('\n');
    }
}

void SourceFile::EnumerateLocations(size_t lineNumber, const std::function<bool(PtLocation)> &function) const
{
    for (auto methodId : debugInfo_.GetMethodIdList()) {
        auto &table = debugInfo_.GetLineNumberTable(methodId);

        auto iter = std::lower_bound(table.begin(), table.end(), lineNumber,
                                     [](auto &entry, auto line) { return entry.line - 1 < line; });
        if (iter == table.end() || iter->line - 1 != lineNumber) {
            continue;
        }

        uint32_t startOffset = iter->offset;
        uint32_t endOffset = (++iter != table.end()) ? iter->offset : ~0U;

        auto codeId = panda_file::MethodDataAccessor(*pandaFile_, methodId).GetCodeId();
        if (!codeId) {
            continue;
        }

        panda_file::CodeDataAccessor cda(*pandaFile_, *codeId);

        for (auto bytecodeOffset = startOffset; bytecodeOffset < endOffset;) {
            if (!function(PtLocation(pandaFile_->GetFilename().c_str(), methodId, bytecodeOffset))) {
                return;
            }

            BytecodeInstruction inst(cda.GetInstructions() + bytecodeOffset);
            if (inst.IsTerminator()) {
                break;
            }

            bytecodeOffset += inst.GetSize();
        }
    }
}
}  // namespace panda::tooling::inspector
