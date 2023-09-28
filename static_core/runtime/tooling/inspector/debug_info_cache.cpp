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

#include "debug_info_cache.h"

#include "debug_info_extractor.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "tooling/pt_location.h"

namespace panda::tooling::inspector {
void DebugInfoCache::AddPandaFile(const panda_file::File &file)
{
    os::memory::LockHolder lock(debug_infos_mutex_);
    debug_infos_.emplace(std::piecewise_construct, std::forward_as_tuple(&file),
                         std::forward_as_tuple(file, [this, &file](auto method_id, auto source_name) {
                             os::memory::LockHolder l(disassemblies_mutex_);
                             disassemblies_.emplace(std::piecewise_construct, std::forward_as_tuple(source_name),
                                                    std::forward_as_tuple(file, method_id));
                         }));
}

void DebugInfoCache::GetSourceLocation(const PtFrame &frame, std::string_view &source_file,
                                       std::string_view &method_name, size_t &line_number)
{
    auto method = frame.GetMethod();
    auto panda_file = method->GetPandaFile();
    auto &debug_info = GetDebugInfo(panda_file);
    source_file = debug_info.GetSourceFile(method->GetFileId());

    method_name = utf::Mutf8AsCString(method->GetName().data);

    // Line number entry corresponding to the bytecode location is
    // the last such entry for which the bytecode offset is not greater than
    // the given offset. Use `std::upper_bound` to find the first entry
    // for which the condition is not true, and then step back.
    auto &table = debug_info.GetLineNumberTable(method->GetFileId());
    auto line_number_iter = std::upper_bound(table.begin(), table.end(), frame.GetBytecodeOffset(),
                                             [](auto offset, auto &entry) { return offset < entry.offset; });
    ASSERT(line_number_iter != table.begin());
    line_number = std::prev(line_number_iter)->line;
}

std::unordered_set<PtLocation, HashLocation> DebugInfoCache::GetCurrentLineLocations(const PtFrame &frame)
{
    std::unordered_set<PtLocation, HashLocation> locations;

    auto method = frame.GetMethod();
    auto panda_file = method->GetPandaFile();
    auto method_id = method->GetFileId();
    auto &table = GetDebugInfo(panda_file).GetLineNumberTable(method_id);
    auto it = std::upper_bound(table.begin(), table.end(), frame.GetBytecodeOffset(),
                               [](auto offset, auto entry) { return offset < entry.offset; });
    if (it == table.begin()) {
        return locations;
    }
    auto line_number = (--it)->line;

    for (it = table.begin(); it != table.end(); ++it) {
        if (it->line != line_number) {
            continue;
        }

        auto next = it + 1;
        auto next_offset = next != table.end() ? next->offset : method->GetCodeSize();
        for (auto o = it->offset; o < next_offset; o++) {
            locations.emplace(panda_file->GetFilename().c_str(), method_id, o);
        }
    }

    return locations;
}

std::unordered_set<PtLocation, HashLocation> DebugInfoCache::GetContinueToLocations(std::string_view source_file,
                                                                                    size_t line_number)
{
    std::unordered_set<PtLocation, HashLocation> locations;
    EnumerateLineEntries([](auto, auto &) { return true; },
                         [source_file](auto, auto &debug_info, auto method_id) {
                             return debug_info.GetSourceFile(method_id) == source_file;
                         },
                         [line_number, &locations](auto panda_file, auto &, auto method_id, auto &entry, auto next) {
                             if (entry.line == line_number) {
                                 uint32_t next_offset;
                                 if (next == nullptr) {
                                     panda_file::MethodDataAccessor mda(*panda_file, method_id);
                                     if (auto code_id = mda.GetCodeId()) {
                                         next_offset =
                                             panda_file::CodeDataAccessor(*panda_file, *code_id).GetCodeSize();
                                     } else {
                                         next_offset = 0;
                                     }
                                 } else {
                                     next_offset = next->offset;
                                 }

                                 for (auto o = entry.offset; o < next_offset; o++) {
                                     locations.emplace(panda_file->GetFilename().data(), method_id, o);
                                 }
                             }
                             return true;
                         });
    return locations;
}

std::vector<PtLocation> DebugInfoCache::GetBreakpointLocations(
    const std::function<bool(std::string_view)> &source_file_filter, size_t line_number,
    std::set<std::string_view> &source_files)
{
    std::vector<PtLocation> locations;
    source_files.clear();
    EnumerateLineEntries([](auto, auto &) { return true; },
                         [&source_file_filter](auto, auto &debug_info, auto method_id) {
                             return source_file_filter(debug_info.GetSourceFile(method_id));
                         },
                         [line_number, &source_files, &locations](auto panda_file, auto &debug_info, auto method_id,
                                                                  auto &entry, auto /* next */) {
                             if (entry.line == line_number) {
                                 source_files.insert(debug_info.GetSourceFile(method_id));
                                 locations.emplace_back(panda_file->GetFilename().data(), method_id, entry.offset);
                             }

                             return true;
                         });
    return locations;
}

std::set<size_t> DebugInfoCache::GetValidLineNumbers(std::string_view source_file, size_t start_line, size_t end_line,
                                                     bool restrict_to_function)
{
    std::set<size_t> line_numbers;
    EnumerateLineEntries([](auto, auto &) { return true; },
                         [source_file, start_line, restrict_to_function](auto, auto &debug_info, auto method_id) {
                             if (debug_info.GetSourceFile(method_id) != source_file) {
                                 return false;
                             }

                             bool has_less = false;
                             bool has_greater = false;
                             if (restrict_to_function) {
                                 for (auto &entry : debug_info.GetLineNumberTable(method_id)) {
                                     if (entry.line <= start_line) {
                                         has_less = true;
                                     }

                                     if (entry.line >= start_line) {
                                         has_greater = true;
                                     }

                                     if (has_less && has_greater) {
                                         break;
                                     }
                                 }
                             }

                             return !restrict_to_function || (has_less && has_greater);
                         },
                         [start_line, end_line, &line_numbers](auto, auto &, auto, auto &entry, auto /* next */) {
                             if (entry.line >= start_line && entry.line < end_line) {
                                 line_numbers.insert(entry.line);
                             }

                             return true;
                         });
    return line_numbers;
}

// NOLINTBEGIN(readability-magic-numbers)
static TypedValue CreateTypedValueFromReg(uint64_t reg, panda_file::Type::TypeId type)
{
    switch (type) {
        case panda_file::Type::TypeId::INVALID:
            return TypedValue::Invalid();
        case panda_file::Type::TypeId::VOID:
            return TypedValue::Void();
        case panda_file::Type::TypeId::U1:
            return TypedValue::U1((reg & 0x1ULL) != 0);
        case panda_file::Type::TypeId::I8:
            return TypedValue::I8(reg & 0xffULL);
        case panda_file::Type::TypeId::U8:
            return TypedValue::U8(reg & 0xffULL);
        case panda_file::Type::TypeId::I16:
            return TypedValue::I16(reg & 0xffffULL);
        case panda_file::Type::TypeId::U16:
            return TypedValue::U16(reg & 0xffffULL);
        case panda_file::Type::TypeId::I32:
            return TypedValue::I32(reg & 0xffffffffULL);
        case panda_file::Type::TypeId::U32:
            return TypedValue::U32(reg & 0xffffffffULL);
        case panda_file::Type::TypeId::F32:
            return TypedValue::F32(bit_cast<float>(static_cast<int32_t>(reg & 0xffffffffULL)));
        case panda_file::Type::TypeId::F64:
            return TypedValue::F64(bit_cast<double>(reg & 0xffffffffULL));
        case panda_file::Type::TypeId::I64:
            return TypedValue::I64(reg);
        case panda_file::Type::TypeId::U64:
            return TypedValue::U64(reg);
        case panda_file::Type::TypeId::REFERENCE:
            return TypedValue::Reference(reinterpret_cast<ObjectHeader *>(reg));
        case panda_file::Type::TypeId::TAGGED:
            return TypedValue::Tagged(coretypes::TaggedValue(static_cast<coretypes::TaggedType>(reg)));
    }
    UNREACHABLE();
}
// NOLINTEND(readability-magic-numbers)

static panda_file::Type::TypeId GetTypeIdBySignature(char signature)
{
    switch (signature) {
        case 'V':
            return panda_file::Type::TypeId::VOID;
        case 'Z':
            return panda_file::Type::TypeId::U1;
        case 'B':
            return panda_file::Type::TypeId::I8;
        case 'H':
            return panda_file::Type::TypeId::U8;
        case 'S':
            return panda_file::Type::TypeId::I16;
        case 'C':
            return panda_file::Type::TypeId::U16;
        case 'I':
            return panda_file::Type::TypeId::I32;
        case 'U':
            return panda_file::Type::TypeId::U32;
        case 'F':
            return panda_file::Type::TypeId::F32;
        case 'D':
            return panda_file::Type::TypeId::F64;
        case 'J':
            return panda_file::Type::TypeId::I64;
        case 'Q':
            return panda_file::Type::TypeId::U64;
        case 'A':
            return panda_file::Type::TypeId::TAGGED;
        case 'L':
        case '[':
            return panda_file::Type::TypeId::REFERENCE;
        default:
            return panda_file::Type::TypeId::INVALID;
    }
}

std::map<std::string, TypedValue> DebugInfoCache::GetLocals(const PtFrame &frame)
{
    std::map<std::string, TypedValue> result;

    auto local_handler = [&result](const std::string &name, const std::string &signature, uint64_t reg,
                                   PtFrame::RegisterKind kind) {
        auto type = signature.empty() ? panda_file::Type::TypeId::INVALID : GetTypeIdBySignature(signature[0]);
        if (type == panda_file::Type::TypeId::INVALID) {
            switch (kind) {
                case PtFrame::RegisterKind::PRIMITIVE:
                    type = panda_file::Type::TypeId::U64;
                    break;
                case PtFrame::RegisterKind::REFERENCE:
                    type = panda_file::Type::TypeId::REFERENCE;
                    break;
                case PtFrame::RegisterKind::TAGGED:
                    type = panda_file::Type::TypeId::TAGGED;
                    break;
            }
        }

        result.emplace(name, CreateTypedValueFromReg(reg, type));
    };

    auto method = frame.GetMethod();
    auto method_id = method->GetFileId();
    auto &debug_info = GetDebugInfo(method->GetPandaFile());
    auto &parameters = debug_info.GetParameterInfo(method_id);
    for (auto i = 0U; i < parameters.size(); i++) {
        auto &parameter = parameters[i];
        local_handler(parameter.name, parameter.signature, frame.GetArgument(i), frame.GetArgumentKind(i));
    }

    auto &variables = debug_info.GetLocalVariableTable(method_id);
    for (auto &variable : variables) {
        auto frame_offset = frame.GetBytecodeOffset();
        if (variable.start_offset <= frame_offset && frame_offset < variable.end_offset) {
            local_handler(variable.name, variable.type_signature,
                          // We introduced a hack in DisasmBackedDebugInfoExtractor, assigning -1 to Accumulator
                          variable.reg_number == -1 ? frame.GetAccumulator() : frame.GetVReg(variable.reg_number),
                          variable.reg_number == -1 ? frame.GetAccumulatorKind()
                                                    : frame.GetVRegKind(variable.reg_number));
        }
    }

    return result;
}

std::string DebugInfoCache::GetSourceCode(std::string_view source_file)
{
    {
        os::memory::LockHolder lock(disassemblies_mutex_);

        auto it = disassemblies_.find(source_file);
        if (it != disassemblies_.end()) {
            return GetDebugInfo(&it->second.first).GetSourceCode(it->second.second);
        }
    }

    if (!os::file::File::IsRegularFile(source_file.data())) {
        return {};
    }

    std::string result;

    std::stringstream buffer;
    buffer << std::ifstream(source_file.data()).rdbuf();

    result = buffer.str();
    if (!result.empty() && result.back() != '\n') {
        result += "\n";
    }

    return result;
}

const panda_file::DebugInfoExtractor &DebugInfoCache::GetDebugInfo(const panda_file::File *file)
{
    os::memory::LockHolder lock(debug_infos_mutex_);
    auto it = debug_infos_.find(file);
    ASSERT(it != debug_infos_.end());
    return it->second;
}
}  // namespace panda::tooling::inspector
