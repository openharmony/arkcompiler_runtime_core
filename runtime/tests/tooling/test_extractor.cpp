/*
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

#include "test_extractor.h"

#include <algorithm>
#include <limits>

#include "libpandabase/utils/leb128.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/debug_data_accessor-inl.h"
#include "libpandafile/helpers.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"

namespace panda::tooling::test {

TestExtractor::TestExtractor(const panda_file::File *pf)
{
    lang_extractor_ = MakePandaUnique<panda_file::DebugInfoExtractor>(pf);
}

std::pair<EntityId, uint32_t> TestExtractor::GetBreakpointAddress(const SourceLocation &source_location)
{
    auto pos = source_location.path.find_last_of("/\\");
    auto name = source_location.path;

    if (pos != PandaString::npos) {
        name = name.substr(pos + 1);
    }

    std::vector<panda_file::File::EntityId> methods = lang_extractor_->GetMethodIdList();
    for (const auto &method : methods) {
        auto srcName = PandaString(lang_extractor_->GetSourceFile(method));
        auto pos_sf = srcName.find_last_of("/\\");
        if (pos_sf != PandaString::npos) {
            srcName = srcName.substr(pos_sf + 1);
        }
        if (srcName == name) {
            const panda_file::LineNumberTable &line_table = lang_extractor_->GetLineNumberTable(method);
            if (line_table.empty()) {
                continue;
            }

            std::optional<size_t> offset = GetOffsetByTableLineNumber(line_table, source_location.line);
            if (offset == std::nullopt) {
                continue;
            }
            return {method, offset.value()};
        }
    }
    return {EntityId(), 0};
}

PandaList<PtStepRange> TestExtractor::GetStepRanges(EntityId method_id, uint32_t current_offset)
{
    const panda_file::LineNumberTable &line_table = lang_extractor_->GetLineNumberTable(method_id);
    if (line_table.empty()) {
        return {};
    }

    std::optional<size_t> line = GetLineNumberByTableOffset(line_table, current_offset);
    if (line == std::nullopt) {
        return {};
    }

    PandaList<PtStepRange> res;
    for (auto it = line_table.begin(); it != line_table.end(); ++it) {
        if (it->line == line) {
            size_t idx = it - line_table.begin();
            if (it + 1 != line_table.end()) {
                res.push_back({line_table[idx].offset, line_table[idx + 1].offset});
            } else {
                res.push_back({line_table[idx].offset, std::numeric_limits<uint32_t>::max()});
            }
        }
    }
    return res;
}

std::vector<panda_file::LocalVariableInfo> TestExtractor::GetLocalVariableInfo(EntityId method_id, size_t offset)
{
    const std::vector<panda_file::LocalVariableInfo> &variables = lang_extractor_->GetLocalVariableTable(method_id);
    std::vector<panda_file::LocalVariableInfo> result;

    for (const auto &variable : variables) {
        if (variable.start_offset <= offset && offset <= variable.end_offset) {
            result.push_back(variable);
        }
    }
    return result;
}

std::optional<size_t> TestExtractor::GetLineNumberByTableOffset(const panda_file::LineNumberTable &table,
                                                                uint32_t offset)
{
    for (const auto &value : table) {
        if (value.offset == offset) {
            return value.line;
        }
    }
    return std::nullopt;
}

std::optional<uint32_t> TestExtractor::GetOffsetByTableLineNumber(const panda_file::LineNumberTable &table, size_t line)
{
    for (const auto &value : table) {
        if (value.line == line) {
            return value.offset;
        }
    }
    return std::nullopt;
}

SourceLocation TestExtractor::GetSourceLocation(EntityId method_id, uint32_t bytecode_offset)
{
    const panda_file::LineNumberTable &line_table = lang_extractor_->GetLineNumberTable(method_id);
    if (line_table.empty()) {
        return SourceLocation();
    }

    std::optional<size_t> line = GetLineNumberByTableOffset(line_table, bytecode_offset);
    if (line == std::nullopt) {
        return SourceLocation();
    }

    return SourceLocation {lang_extractor_->GetSourceFile(method_id), line.value()};
}
}  // namespace  panda::tooling::test
